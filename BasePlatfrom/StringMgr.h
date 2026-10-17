#ifndef _BASE_PLATFORM_STRINGMGR_H_
#define _BASE_PLATFORM_STRINGMGR_H_

// StringMgr: a minimal string interning pool (P1.1: arena + open addressing).
//
//  * Interned storage: every unique string is stored exactly once.
//  * ID 0 is invalid; ID 1 is the empty string (pre-interned).
//  * IDs are 1-based indexes into an append-only view table; IDs are never
//    reused and never change.
//  * Bytes live in 64 KiB arena blocks; a string larger than the remaining
//    space gets its own block. Views are therefore stable across later
//    AddString calls, but are invalidated by Clear()/Load().
//  * Lookup uses a power-of-two open-addressing table (FNV-1a 32-bit hash,
//    8-byte buckets, load factor <= 0.7) instead of std::unordered_map.
//  * NOT thread-safe: intended for a single writer, or freeze the pool before
//    concurrent reads.
//  * Names are reference counted: AddString retains, ReleaseString/DestroyString
//    releases. A released name stays readable by ID (arena is not reclaimed and
//    IDs are never reused) but is skipped by Find and can be resurrected by
//    AddString.

#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <ostream>
#include <istream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace db {

template <typename IdType, uint8_t MAX_FIX_LENGTH = 127, IdType MinElementCount = 128,
          uint16_t BlockSizeMultiple = 1>
class StringMgr {
    static_assert(std::is_integral_v<IdType>, "IdType must be integral");

    // MAX_FIX_LENGTH / MinElementCount / BlockSizeMultiple are accepted for
    // source compatibility with the previous implementation and are unused.
    static_assert(MAX_FIX_LENGTH > 0, "MAX_FIX_LENGTH must be greater than 0");
    static_assert(MinElementCount > 0, "MinElementCount must be greater than 0");
    static_assert(BlockSizeMultiple > 0, "BlockSizeMultiple must be greater than 0");

public:
    static constexpr IdType INVALID_ID = 0;
    static constexpr IdType EMPTY_ID   = 1;

    StringMgr() {
        m_views.emplace_back();  // index 0 -> ID 1 (empty string)
        m_refs.push_back(1);     // the empty string starts with one reference
        m_buckets.assign(kInitialCapacity, Bucket{});
    }

    StringMgr(const StringMgr&)            = delete;
    StringMgr& operator=(const StringMgr&) = delete;
    StringMgr(StringMgr&&)                 = delete;
    StringMgr& operator=(StringMgr&&)      = delete;
    ~StringMgr()                           = default;

    // Marks the pool read-only for the build phase that follows. `Find` /
    // `GetString` remain usable; `AddString` throws. `Clear()`/`Load()` reopen.
    void Seal() { m_sealed = true; }
    bool IsSealed() const { return m_sealed; }

    // Interns `s` and returns its ID, incrementing its reference count. Empty
    // strings always map to EMPTY_ID. IDs are append-only and never reused; a
    // previously released ID is resurrected.
    IdType AddString(std::string_view s) {
        if (m_sealed) {
            throw std::logic_error("StringMgr is sealed (read-only)");
        }
        if (s.empty()) {
            if (m_refs[0] == std::numeric_limits<uint32_t>::max()) {
                throw std::overflow_error("StringMgr: refcount overflow");
            }
            ++m_refs[0];
            return EMPTY_ID;
        }
        if (const IdType existing = FindId(s); existing != INVALID_ID) {
            uint32_t& ref = m_refs[static_cast<size_t>(existing) - 1];
            if (ref == std::numeric_limits<uint32_t>::max()) {
                throw std::overflow_error("StringMgr: refcount overflow");
            }
            if (ref == 0) {
                ++m_unique_count;  // resurrect a released name
            }
            ++ref;
            return existing;
        }

        // New ID == m_views.size() + 1 (1-based). Reject if it would overflow.
        const auto max_id = static_cast<uint64_t>(std::numeric_limits<IdType>::max());
        if (static_cast<uint64_t>(m_views.size()) >= max_id) {
            throw std::overflow_error("StringMgr: string ID space exhausted");
        }

        // Make sure the new name fits the 0.7 load factor before inserting.
        EnsureIndexCapacity(m_views.size());

        const std::string_view stored = Store(s);
        m_views.push_back(stored);
        m_refs.push_back(1);
        const IdType id = static_cast<IdType>(m_views.size());  // 1-based
        InsertIntoIndex(stored, id);
        ++m_unique_count;
        return id;
    }

    IdType AddString(const std::string& s) { return AddString(std::string_view(s)); }

    // Convenience overload so string literals are not ambiguous between the
    // std::string and std::string_view overloads.
    IdType AddString(const char* s) { return AddString(std::string_view(s == nullptr ? "" : s)); }

    // Returns the interned string for `id`, or an empty view for invalid IDs.
    // The returned view stays valid until Clear()/Load() and across AddString.
    std::string_view GetString(IdType id) const {
        const auto idx = static_cast<uint64_t>(static_cast<std::make_unsigned_t<IdType>>(id));
        if (idx == 0 || idx > static_cast<uint64_t>(m_views.size())) {
            return {};
        }
        return m_views[static_cast<size_t>(idx - 1)];
    }

    // Returns the ID of a live interned string, if present. Released names are
    // skipped (their IDs remain readable through GetString).
    std::optional<IdType> Find(std::string_view s) const {
        if (s.empty()) {
            return EMPTY_ID;
        }
        const IdType id = FindId(s);
        if (id == INVALID_ID || m_refs[static_cast<size_t>(id) - 1] == 0) {
            return std::nullopt;
        }
        return id;
    }

    // Drops one reference to `id`. Does not free arena bytes or remove the hash
    // entry: GetString(id) stays readable and a later AddString resurrects it.
    // Throws std::out_of_range for an invalid ID and std::runtime_error on an
    // already-released name.
    void ReleaseString(IdType id) {
        const auto idx = static_cast<uint64_t>(static_cast<std::make_unsigned_t<IdType>>(id));
        if (idx == 0) {
            return;
        }
        if (idx > static_cast<uint64_t>(m_views.size())) {
            throw std::out_of_range("StringMgr: invalid string id");
        }
        uint32_t& ref = m_refs[static_cast<size_t>(idx - 1)];
        if (ref == 0) {
            throw std::runtime_error("StringMgr: string already released");
        }
        --ref;
        if (ref == 0 && idx != static_cast<uint64_t>(EMPTY_ID)) {
            --m_unique_count;
        }
    }

    // Alias for ReleaseString (kept for source compatibility).
    void DestroyString(IdType id) { ReleaseString(id); }

    // Drops all strings and re-interns the empty string as ID 1.
    // Previously returned views are invalidated.
    void Clear() {
        m_views.clear();
        m_views.emplace_back();
        m_refs.clear();
        m_refs.push_back(1);
        m_blocks.clear();
        m_cur_block     = nullptr;
        m_cur_block_size = 0;
        m_cur_remaining  = 0;
        m_buckets.assign(kInitialCapacity, Bucket{});
        m_unique_count = 0;
        m_sealed       = false;
    }

    // Number of stored views, including the empty string (dead views included).
    size_t Size() const { return m_views.size(); }

    // Number of live unique non-empty names (refcount > 0).
    size_t Count() const { return m_unique_count; }

    bool Empty() const { return m_unique_count == 0; }

    // Binary logical dump in ID order (DBST v2):
    //   uint32 magic = 0x44425354 ("DBST"), uint32 version = 2,
    //   uint32 id_width = sizeof(IdType), uint64 count,
    //   then `count` times: uint32 length + raw bytes + uint32 refcount.
    // v1 files (no refcount) are still accepted by Load with ref == 1.
    bool Save(std::ostream& out) const {
        const uint32_t width = static_cast<uint32_t>(sizeof(IdType));
        const uint64_t count = static_cast<uint64_t>(m_views.size());

        WritePod(out, kMagic);
        WritePod(out, kVersion);
        WritePod(out, width);
        WritePod(out, count);
        if (!out) {
            return false;
        }

        for (size_t i = 0; i < m_views.size(); ++i) {
            const std::string_view& s = m_views[i];
            if (s.size() > std::numeric_limits<uint32_t>::max()) {
                return false;
            }
            WritePod(out, static_cast<uint32_t>(s.size()));
            if (!s.empty()) {
                out.write(s.data(), static_cast<std::streamsize>(s.size()));
                if (!out) {
                    return false;
                }
            }
            WritePod(out, m_refs[i]);
        }
        return static_cast<bool>(out);
    }

    // Restores a pool written by Save(). IDs match the save order exactly.
    // Duplicates are preserved in storage; the index keeps the first ID.
    // On any failure the pool is cleared (empty string back at ID 1) and
    // false is returned.
    bool Load(std::istream& in) {
        Clear();

        uint32_t magic   = 0;
        uint32_t version = 0;
        uint32_t width   = 0;
        uint64_t count   = 0;

        ReadPod(in, magic);
        ReadPod(in, version);
        ReadPod(in, width);
        ReadPod(in, count);
        if (!in || magic != kMagic || (version != 1u && version != kVersion) ||
            width != sizeof(IdType)) {
            Clear();
            return false;
        }
        if (count < 1 || count > kMaxLoadCount) {
            Clear();
            return false;
        }
        if (count > static_cast<uint64_t>(std::numeric_limits<IdType>::max())) {
            Clear();
            return false;
        }

        m_views.reserve(static_cast<size_t>(count));

        for (uint64_t i = 0; i < count; ++i) {
            uint32_t len = 0;
            ReadPod(in, len);
            if (!in || len > kMaxStringLength) {
                Clear();
                return false;
            }

            // The very first stored string must be the interned empty string.
            // Clear() already installed it at index 0.
            if (i == 0) {
                if (len != 0) {
                    Clear();
                    return false;
                }
                uint32_t ref = 1;
                if (version >= 2u) {
                    ReadPod(in, ref);
                    if (!in) {
                        Clear();
                        return false;
                    }
                }
                m_refs[0] = ref;
                continue;
            }

            if (len == 0) {
                // Preserve duplicate empty-string IDs; they are not indexed.
                m_views.emplace_back();
            } else {
                char* dst = ArenaAlloc(len);
                in.read(dst, static_cast<std::streamsize>(len));
                if (!in) {
                    Clear();
                    return false;
                }
                m_views.emplace_back(dst, len);
            }

            uint32_t ref = 1;
            if (version >= 2u) {
                ReadPod(in, ref);
                if (!in) {
                    Clear();
                    return false;
                }
            }
            m_refs.push_back(ref);
        }

        BuildIndex();
        return true;
    }

    void PrintStats() const {
        size_t bytes = 0;
        for (const std::string_view& s : m_views) {
            bytes += s.size();
        }
        std::cout << "[StringMgr] unique=" << Count() << " size=" << Size()
                  << " bytes=" << bytes << " blocks=" << m_blocks.size()
                  << " sealed=" << (m_sealed ? "yes" : "no") << '\n';
    }

    void PrintOptimizationInfo() const { PrintStats(); }

private:
    struct Bucket {
        uint32_t hash{0};
        IdType   id{0};  // 0 == empty bucket
    };

    static constexpr uint32_t kMagic           = 0x44425354u;  // "DBST"
    static constexpr uint32_t kVersion         = 2u;
    static constexpr uint64_t kMaxLoadCount    = 100'000'000ull;
    static constexpr uint32_t kMaxStringLength = 16u * 1024u * 1024u;  // 16 MiB
    static constexpr size_t   kBlockSize       = 64u * 1024u;
    static constexpr size_t   kInitialCapacity = 128;

    static uint32_t Hash(std::string_view s) {
        // FNV-1a, 32-bit.
        uint32_t h = 2166136261u;
        for (unsigned char c : s) {
            h ^= c;
            h *= 16777619u;
        }
        return h;
    }

    // Copies `len` bytes into the arena and returns the destination. A string
    // that does not fit the current block gets a fresh block (its own size when
    // it is larger than a normal block).
    char* ArenaAlloc(size_t len) {
        if (m_cur_block == nullptr || m_cur_remaining < len) {
            const size_t block_size = len > kBlockSize ? len : kBlockSize;
            m_blocks.emplace_back(std::make_unique<char[]>(block_size));
            m_cur_block      = m_blocks.back().get();
            m_cur_block_size = block_size;
            m_cur_remaining  = block_size;
        }
        char* dst = m_cur_block + (m_cur_block_size - m_cur_remaining);
        m_cur_remaining -= len;
        return dst;
    }

    std::string_view Store(std::string_view s) {
        char* dst = ArenaAlloc(s.size());
        std::memcpy(dst, s.data(), s.size());
        return std::string_view(dst, s.size());
    }

    IdType FindId(std::string_view s) const {
        if (m_buckets.empty()) {
            return INVALID_ID;
        }
        const uint32_t h    = Hash(s);
        const size_t   mask = m_buckets.size() - 1;
        size_t         pos  = h & mask;
        for (size_t probes = 0; probes < m_buckets.size(); ++probes) {
            const Bucket& b = m_buckets[pos];
            if (b.id == 0) {
                return INVALID_ID;
            }
            if (b.hash == h && GetString(b.id) == s) {
                return b.id;
            }
            pos = (pos + 1) & mask;
        }
        return INVALID_ID;
    }

    void InsertIntoIndex(std::string_view s, IdType id) {
        const uint32_t h    = Hash(s);
        const size_t   mask = m_buckets.size() - 1;
        size_t         pos  = h & mask;
        while (m_buckets[pos].id != 0) {
            pos = (pos + 1) & mask;
        }
        m_buckets[pos] = Bucket{h, id};
    }

    void Rehash(size_t new_capacity) {
        std::vector<Bucket> old = std::move(m_buckets);
        m_buckets.assign(new_capacity, Bucket{});
        const size_t mask = new_capacity - 1;
        for (const Bucket& b : old) {
            if (b.id == 0) {
                continue;
            }
            size_t pos = b.hash & mask;
            while (m_buckets[pos].id != 0) {
                pos = (pos + 1) & mask;
            }
            m_buckets[pos] = b;
        }
    }

    // `total_names` is the number of non-empty names that must fit.
    void EnsureIndexCapacity(size_t total_names) {
        size_t cap = m_buckets.empty() ? kInitialCapacity : m_buckets.size();
        while (total_names * 10 > cap * 7) {
            cap <<= 1;
        }
        if (m_buckets.empty() || cap != m_buckets.size()) {
            Rehash(cap);
        }
    }

    // Rebuilds the index from m_views (used by Load); duplicates keep the
    // first ID.
    void BuildIndex() {
        const size_t names = m_views.size() - 1;
        size_t       cap   = kInitialCapacity;
        while (names * 10 > cap * 7) {
            cap <<= 1;
        }
        m_buckets.assign(cap, Bucket{});
        const size_t mask = cap - 1;

        for (size_t i = 1; i < m_views.size(); ++i) {
            const std::string_view s = m_views[i];
            if (s.empty()) {
                continue;
            }
            const uint32_t h   = Hash(s);
            size_t         pos = h & mask;
            bool           exists = false;
            while (m_buckets[pos].id != 0) {
                if (m_buckets[pos].hash == h &&
                    m_views[static_cast<size_t>(m_buckets[pos].id) - 1] == s) {
                    exists = true;
                    break;
                }
                pos = (pos + 1) & mask;
            }
            if (!exists) {
                m_buckets[pos] = Bucket{h, static_cast<IdType>(i + 1)};
                // Dead (ref == 0) views still get indexed so AddString can
                // resurrect them; they do not count as live unique names.
                if (m_refs[i] > 0) {
                    ++m_unique_count;
                }
            }
        }
    }

    template <typename T>
    static void WritePod(std::ostream& out, const T& value) {
        out.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template <typename T>
    static void ReadPod(std::istream& in, T& value) {
        in.read(reinterpret_cast<char*>(&value), sizeof(T));
    }

    std::vector<std::unique_ptr<char[]>> m_blocks;
    char*                                m_cur_block      = nullptr;
    size_t                               m_cur_block_size = 0;
    size_t                               m_cur_remaining  = 0;

    std::vector<std::string_view> m_views;  // index 0 -> ID 1 (empty)
    std::vector<uint32_t>         m_refs;   // parallel to m_views
    std::vector<Bucket>           m_buckets;
    size_t                        m_unique_count = 0;
    bool                          m_sealed       = false;
};

}  // namespace db

#endif  // _BASE_PLATFORM_STRINGMGR_H_
