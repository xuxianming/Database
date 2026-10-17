#ifndef _BASE_PLATFORM_IDARRAYMGR_H_
#define _BASE_PLATFORM_IDARRAYMGR_H_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <istream>
#include <limits>
#include <map>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace db {

// ----------------------------------------------------------------------
// Compile-time helpers
// ----------------------------------------------------------------------
template <uint32_t N>
constexpr bool IsPowerOfTwoMinusOne() {
    return (N & (N + 1)) == 0 && N != 0;
}

// ----------------------------------------------------------------------
// IdArray: non-owning array view
// ----------------------------------------------------------------------
template <typename ValueType, typename IdType>
class IdArray {
public:
    IdArray() = default;
    IdArray(ValueType* data, size_t capacity, IdType id)
        : data_(data), length_(0), capacity_(capacity), id_(id) {}
    // Used when materialising an already-filled array (e.g. after a lookup):
    // the element count is known up front.
    IdArray(ValueType* data, size_t capacity, size_t length, IdType id)
        : data_(data), length_(length), capacity_(capacity), id_(id) {}

    IdArray(const IdArray&)            = default;
    IdArray& operator=(const IdArray&) = default;

    IdArray(IdArray&& other) noexcept
        : data_(other.data_), length_(other.length_), capacity_(other.capacity_), id_(other.id_) {
        other.data_     = nullptr;
        other.length_   = 0;
        other.capacity_ = 0;
        other.id_       = IdType();
    }
    IdArray& operator=(IdArray&& other) noexcept {
        if (this != &other) {
            data_           = other.data_;
            length_         = other.length_;
            capacity_       = other.capacity_;
            id_             = other.id_;
            other.data_     = nullptr;
            other.length_   = 0;
            other.capacity_ = 0;
            other.id_       = IdType();
        }
        return *this;
    }

    const ValueType& operator[](size_t index) const {
        assert(index < length_);
        return data_[index];
    }
    ValueType& operator[](size_t index) {
        assert(index < length_);
        return data_[index];
    }

    size_t Size() const { return length_; }
    size_t Capacity() const { return capacity_; }
    IdType Id() const { return id_; }

    ValueType*       begin() { return data_; }
    ValueType*       end() { return data_ + length_; }
    const ValueType* begin() const { return data_; }
    const ValueType* end() const { return data_ + length_; }

    void PushBack(const ValueType& value) {
        if (length_ < capacity_) {
            data_[length_++] = value;
        }
    }

    void Reset() {
        std::fill(data_, data_ + length_, ValueType{});
        length_ = 0;
    }

private:
    ValueType* data_     = nullptr;
    size_t     length_   = 0;
    size_t     capacity_ = 0;
    IdType     id_       = IdType();
};

// ----------------------------------------------------------------------
// Dynamic-length table (each array can have arbitrary length up to MaxSize)
// Each array stores: [length][data...] in a large block.
// Blocks are allocated as needed; free arrays are indexed by their length.
// ----------------------------------------------------------------------
template <typename ValueType, typename IdType, uint32_t MinLen, uint32_t MaxSize = 65535>
class DynLenArrayTable {
    static_assert(MaxSize > 0, "MaxSize must be > 0");
    static_assert(MaxSize <= 0x7FFFFFFF, "MaxSize too large");
    static_assert(sizeof(ValueType) >= sizeof(IdType), "ValueType cannot store the length word");
    static_assert(std::is_trivially_copyable_v<ValueType>,
                  "IdArrayMgr persistence requires trivially copyable values");

    // One extra element of every block stores the length word of the array that
    // starts there; MaxSize stays the maximum payload length.
    static constexpr size_t ELEMENT_SIZE   = sizeof(ValueType);
    static constexpr size_t BLOCK_ELEMENTS = static_cast<size_t>(MaxSize) + 1;
    static constexpr size_t BLOCK_BYTES    = BLOCK_ELEMENTS * ELEMENT_SIZE;

public:
    DynLenArrayTable() {
        blocks_.reserve(16);
        data_.reserve(1024);
        // Index 0 is reserved: ID 0 means "empty / invalid".
        data_.push_back(nullptr);
        free_flags_.push_back(false);
        // Force the first Allocate() to reserve a fresh block.
        cur_offset_ = BLOCK_ELEMENTS;
    }

    ~DynLenArrayTable() { Clear(); }

    IdArray<ValueType, IdType> Allocate(size_t len) {
        if (len == 0 || len > MaxSize) {
            throw std::runtime_error("DynLenArrayTable: invalid length");
        }

        // Reuse a free array of exactly this length.
        auto it = free_by_len_.find(len);
        if (it != free_by_len_.end() && !it->second.empty()) {
            IdType id = it->second.back();
            it->second.pop_back();
            if (it->second.empty()) {
                free_by_len_.erase(it);
            }
            ValueType* data = data_[id];
            WriteLength(data, static_cast<IdType>(len));
            free_flags_[id] = false;
            return IdArray<ValueType, IdType>(data + 1, len, id);
        }

        // Split a larger free block, keeping at least MinLen elements of slack.
        auto upper_it = free_by_len_.upper_bound(len + MinLen + 1);
        if (upper_it != free_by_len_.end() && !upper_it->second.empty()) {
            size_t free_len = upper_it->first;
            IdType id       = upper_it->second.back();
            upper_it->second.pop_back();
            if (upper_it->second.empty()) {
                free_by_len_.erase(upper_it);
            }

            ValueType* data = data_[id];
            WriteLength(data, static_cast<IdType>(len));
            free_flags_[id] = false;

            // -1 because the remainder needs one element for its own length word.
            size_t    remain_len  = free_len - len - 1;
            ValueType* remain_data = data + 1 + len;
            WriteLength(remain_data, static_cast<IdType>(remain_len));

            IdType remain_id = static_cast<IdType>(data_.size());
            data_.push_back(remain_data);
            free_flags_.push_back(true);
            free_by_len_[remain_len].push_back(remain_id);
            return IdArray<ValueType, IdType>(data + 1, len, id);
        }

        // Reserve a new block when the current one cannot fit len + length word.
        if (cur_offset_ + len + 1 > BLOCK_ELEMENTS) {
            size_t remain_len = BLOCK_ELEMENTS - cur_offset_;
            if (remain_len > MinLen) {
                ValueType* remain_data = blocks_[cur_block_] + cur_offset_;
                // -1 because the remainder needs one element for its length word.
                size_t free_len = remain_len - 1;
                if (free_len > 0) {
                    WriteLength(remain_data, static_cast<IdType>(free_len));
                    IdType remain_id = static_cast<IdType>(data_.size());
                    data_.push_back(remain_data);
                    free_flags_.push_back(true);
                    free_by_len_[free_len].push_back(remain_id);
                }
            }
            ValueType* new_block = static_cast<ValueType*>(malloc(BLOCK_BYTES));
            if (!new_block) {
                throw std::runtime_error("DynLenArrayTable: malloc failed");
            }
            cur_block_ = blocks_.size();
            blocks_.push_back(new_block);
            cur_offset_ = 0;
        }

        ValueType* block = blocks_[cur_block_];
        ValueType* data  = block + cur_offset_;
        WriteLength(data, static_cast<IdType>(len));
        IdType id = static_cast<IdType>(data_.size());
        data_.push_back(data);
        free_flags_.push_back(false);
        cur_offset_ += (len + 1);
        return IdArray<ValueType, IdType>(data + 1, len, id);
    }

    void Free(IdType id) {
        CheckId(id);
        if (free_flags_[id]) {
            throw std::runtime_error("DynLenArrayTable: double free");
        }
        ValueType* data = data_[id];
        IdType     len  = ReadLength(data);
        free_flags_[id] = true;
        free_by_len_[len].push_back(id);
    }

    IdArray<ValueType, IdType> Get(IdType id) const {
        CheckId(id);
        if (free_flags_[id]) {
            throw std::out_of_range("DynLenArrayTable: array is freed");
        }
        ValueType* data = data_[id];
        IdType     len  = ReadLength(data);
        return IdArray<ValueType, IdType>(data + 1, len, static_cast<size_t>(len), id);
    }

    // Saves every slot (live and free) in ID order so that array IDs stay
    // stable across a Save/Load roundtrip. Free slots keep their length so that
    // Allocate() can reuse or split them after loading.
    bool Save(std::ostream& out) const {
        const uint32_t id_width    = static_cast<uint32_t>(sizeof(IdType));
        const uint32_t value_width = static_cast<uint32_t>(sizeof(ValueType));
        const uint64_t slot_count  = static_cast<uint64_t>(data_.size());

        WritePod(out, kMagic);
        WritePod(out, kVersion);
        WritePod(out, id_width);
        WritePod(out, value_width);
        WritePod(out, slot_count);
        if (!out) {
            return false;
        }

        for (size_t id = 1; id < data_.size(); ++id) {
            const uint8_t  live = free_flags_[id] ? 0 : 1;
            const uint32_t len  = static_cast<uint32_t>(ReadLength(data_[id]));
            WritePod(out, live);
            WritePod(out, len);
            if (live != 0) {
                out.write(reinterpret_cast<const char*>(data_[id] + 1),
                          static_cast<std::streamsize>(len) *
                              static_cast<std::streamsize>(sizeof(ValueType)));
                if (!out) {
                    return false;
                }
            }
        }
        return static_cast<bool>(out);
    }

    // Restores a table written by Save(). IDs are reproduced exactly; free slots
    // become reusable free chunks of their saved length. On any failure the
    // table is left empty.
    bool Load(std::istream& in) {
        Clear();

        uint32_t magic       = 0;
        uint32_t version     = 0;
        uint32_t id_width    = 0;
        uint32_t value_width = 0;
        uint64_t slot_count  = 0;

        ReadPod(in, magic);
        ReadPod(in, version);
        ReadPod(in, id_width);
        ReadPod(in, value_width);
        ReadPod(in, slot_count);

        if (!in || magic != kMagic || version != kVersion || id_width != sizeof(IdType) ||
            value_width != sizeof(ValueType)) {
            Clear();
            return false;
        }
        if (slot_count < 1 || slot_count > kMaxSlotCount) {
            Clear();
            return false;
        }
        if (slot_count - 1 > static_cast<uint64_t>(std::numeric_limits<IdType>::max())) {
            Clear();
            return false;
        }

        for (uint64_t id = 1; id < slot_count; ++id) {
            uint8_t  live = 0;
            uint32_t len  = 0;
            ReadPod(in, live);
            ReadPod(in, len);
            if (!in || live > 1 || len == 0 || len > MaxSize) {
                Clear();
                return false;
            }

            if (cur_offset_ + static_cast<size_t>(len) + 1 > BLOCK_ELEMENTS) {
                ValueType* new_block = static_cast<ValueType*>(malloc(BLOCK_BYTES));
                if (!new_block) {
                    Clear();
                    return false;
                }
                cur_block_ = blocks_.size();
                blocks_.push_back(new_block);
                cur_offset_ = 0;
            }

            ValueType* data = blocks_[cur_block_] + cur_offset_;
            WriteLength(data, static_cast<IdType>(len));
            if (live != 0) {
                in.read(reinterpret_cast<char*>(data + 1),
                        static_cast<std::streamsize>(len) *
                            static_cast<std::streamsize>(sizeof(ValueType)));
                if (!in) {
                    Clear();
                    return false;
                }
            }

            data_.push_back(data);
            free_flags_.push_back(live == 0);
            if (live == 0) {
                free_by_len_[len].push_back(static_cast<IdType>(id));
            }
            cur_offset_ += static_cast<size_t>(len) + 1;
        }

        if (static_cast<uint64_t>(data_.size()) != slot_count) {
            Clear();
            return false;
        }
        return true;
    }

    void Clear() {
        for (auto* blk : blocks_) {
            free(blk);
        }
        blocks_.clear();
        data_.clear();
        data_.push_back(nullptr);  // reserved index 0
        free_flags_.clear();
        free_flags_.push_back(false);
        free_by_len_.clear();
        cur_block_ = 0;
        // Force the next Allocate() to reserve a fresh block.
        cur_offset_ = BLOCK_ELEMENTS;
    }

private:
    static constexpr uint32_t kMagic        = 0x49444152u;  // "IDAR"
    static constexpr uint32_t kVersion      = 1u;
    static constexpr uint64_t kMaxSlotCount = 100'000'000ull;

    template <typename T>
    static void WritePod(std::ostream& out, const T& value) {
        out.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template <typename T>
    static void ReadPod(std::istream& in, T& value) {
        in.read(reinterpret_cast<char*>(&value), sizeof(T));
    }

    static void WriteLength(ValueType* data, IdType len) {
        std::memcpy(data, &len, sizeof(IdType));
    }

    static IdType ReadLength(const ValueType* data) {
        IdType len{};
        std::memcpy(&len, data, sizeof(IdType));
        return len;
    }

    void CheckId(IdType id) const {
        if (id == 0 || static_cast<size_t>(id) >= data_.size()) {
            throw std::out_of_range("DynLenArrayTable: invalid array id");
        }
    }

    std::vector<ValueType*>                         blocks_;
    std::vector<ValueType*>                         data_;
    std::vector<bool>                               free_flags_;
    std::map<size_t, std::vector<IdType>>           free_by_len_;
    size_t                                          cur_block_  = 0;
    size_t                                          cur_offset_ = 0;
};

// ----------------------------------------------------------------------
// IdArrayMgr: owns a single dynamic-length table.
//
// The old fixed-length sub-tables (and the length-tag dispatch in the ID) were
// removed: their IDs never encoded the table length, so Get/Free dispatched to
// the wrong table, and FixLenArrayTable could not even be instantiated for
// Len > 7. FixLen is kept only for source compatibility and as the minimum
// split slack (MinLen) of the dynamic allocator.
// ----------------------------------------------------------------------
template <typename IdType = uint32_t, typename ValueType = uint32_t, uint16_t FixLen = 7>
class IdArrayMgr {
    static_assert(std::is_integral<IdType>::value, "IdType must be integral");
    static_assert(FixLen > 0, "FixLen must be positive");
    static_assert(IsPowerOfTwoMinusOne<FixLen>(), "FixLen must be of the form 2^n - 1");

public:
    IdArrayMgr() : m_table(new DynLenArrayTable<ValueType, IdType, FixLen + 1>()) {}
    ~IdArrayMgr() { delete m_table; }

    IdArrayMgr(const IdArrayMgr&)            = delete;
    IdArrayMgr& operator=(const IdArrayMgr&) = delete;
    IdArrayMgr(IdArrayMgr&&)                 = delete;
    IdArrayMgr& operator=(IdArrayMgr&&)      = delete;

    IdArray<ValueType, IdType> Allocate(size_t len) {
        if (len == 0) {
            return IdArray<ValueType, IdType>();
        }
        return m_table->Allocate(len);
    }

    void Free(IdType id) {
        if (id == 0) {
            return;
        }
        m_table->Free(id);
    }

    IdArray<ValueType, IdType> Get(IdType id) const {
        if (id == 0) {
            return IdArray<ValueType, IdType>();
        }
        return m_table->Get(id);
    }

    void Clear() { m_table->Clear(); }

    bool Save(std::ostream& out) const { return m_table->Save(out); }
    bool Load(std::istream& in) { return m_table->Load(in); }

private:
    DynLenArrayTable<ValueType, IdType, FixLen + 1>* m_table;
};

}  // namespace db

#endif  // _BASE_PLATFORM_IDARRAYMGR_H_
