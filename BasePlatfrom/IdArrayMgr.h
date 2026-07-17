#ifndef _BASE_PLATFORM_IDARRAYMGR_H_
#define _BASE_PLATFORM_IDARRAYMGR_H_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace db {

// ----------------------------------------------------------------------
// Compile-time helpers
// ----------------------------------------------------------------------
template <uint32_t N>
constexpr uint32_t ConstexprILog2() {
    if constexpr (N == 0) {
        return 0;  // or throw an error if you prefer
    }
    if constexpr (N == 1) {
        return 1;
    } else {
        return 1 + ConstexprILog2<(N >> 1)>();
    }
}

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
// Base interface for all tables
// ----------------------------------------------------------------------
template <typename ValueType, typename IdType>
class IArrayTable {
public:
    virtual ~IArrayTable()                                  = default;
    virtual IdArray<ValueType, IdType> Allocate(size_t len) = 0;
    virtual void                       Free(IdType id)      = 0;
    virtual IdArray<ValueType, IdType> Get(IdType id) const = 0;
    virtual void                       Clear()              = 0;
};

// ----------------------------------------------------------------------
// Fixed-length table (each array has fixed size Len)
// Memory layout: block of fixed-size slots, free list for slots
// ----------------------------------------------------------------------
template <typename ValueType, typename IdType, uint16_t Len>
class FixLenArrayTable : public IArrayTable<ValueType, IdType> {
    static_assert(Len > 0 && Len <= 65535, "Len out of range");

    struct Slot {
        ValueType data[Len];
    };

    static constexpr size_t   BLOCK_BYTES     = 64 * 1024;
    static constexpr uint32_t SLOTS_PER_BLOCK = static_cast<uint32_t>(BLOCK_BYTES / sizeof(Slot));
    static_assert(SLOTS_PER_BLOCK > 0, "Block too small");

    static constexpr uint32_t OFFSET_BITS = 8;
    static constexpr uint32_t BLOCK_BITS  = sizeof(IdType) * 8 - OFFSET_BITS;
    static_assert(BLOCK_BITS >= 8, "IdType too small");
    static_assert(SLOTS_PER_BLOCK <= (1u << OFFSET_BITS), "Too many slots per block");

public:
    FixLenArrayTable() {
        blocks_.reserve(16);
        free_list_.reserve(64);
    }

    ~FixLenArrayTable() override { Clear(); }

    IdArray<ValueType, IdType> Allocate(size_t /*len*/) override {
        if (!free_list_.empty()) {
            auto [blk, off] = free_list_.back();
            free_list_.pop_back();
            Slot* slot = GetSlot(blk, off);
            return IdArray<ValueType, IdType>(slot->data, Len, EncodeId(blk, off));
        }
        if (blocks_.empty() || cur_count_ >= SLOTS_PER_BLOCK) {
            cur_block_ = static_cast<uint32_t>(blocks_.size());
            char* mem  = static_cast<char*>(malloc(BLOCK_BYTES));
            if (!mem) throw std::runtime_error("FixLenArrayTable: malloc failed");
            blocks_.push_back(mem);
            cur_count_ = 0;
        }
        Slot*  slot = GetSlot(cur_block_, cur_count_);
        IdType id   = EncodeId(cur_block_, cur_count_);
        ++cur_count_;
        return IdArray<ValueType, IdType>(slot->data, Len, id);
    }

    void Free(IdType id) override {
        auto [blk, off] = DecodeId(id);
        if (blk == cur_block_ && off == cur_count_ - 1) {
            --cur_count_;
            return;
        }
        free_list_.emplace_back(blk, off);
    }

    IdArray<ValueType, IdType> Get(IdType id) const override {
        auto [blk, off]  = DecodeId(id);
        const Slot* slot = GetSlot(blk, off);
        return IdArray<ValueType, IdType>(const_cast<ValueType*>(slot->data), Len, id);
    }

    void Clear() override {
        for (auto blk : blocks_) free(blk);
        blocks_.clear();
        free_list_.clear();
        cur_block_ = 0;
        cur_count_ = 0;
    }

private:
    IdType EncodeId(uint32_t blk, uint32_t off) const {
        return (static_cast<IdType>(blk) << OFFSET_BITS) | off;
    }
    std::pair<uint32_t, uint32_t> DecodeId(IdType id) const {
        uint32_t off = id & ((1u << OFFSET_BITS) - 1);
        uint32_t blk = id >> OFFSET_BITS;
        return {blk, off};
    }

    Slot* GetSlot(uint32_t blk, uint32_t off) {
        char* block = blocks_[blk];
        return reinterpret_cast<Slot*>(block + off * sizeof(Slot));
    }
    const Slot* GetSlot(uint32_t blk, uint32_t off) const {
        const char* block = blocks_[blk];
        return reinterpret_cast<const Slot*>(block + off * sizeof(Slot));
    }

    std::vector<char*>                         blocks_;
    std::vector<std::pair<uint32_t, uint32_t>> free_list_;
    uint32_t                                   cur_block_ = 0;
    uint32_t                                   cur_count_ = 0;
};

// ----------------------------------------------------------------------
// Dynamic-length table (each array can have arbitrary length up to MaxSize)
// Each array stores: [length][data...] in a large block (size = MaxSize * sizeof(ValueType))
// Blocks are allocated as needed, free arrays are indexed by their length
// ----------------------------------------------------------------------
template <typename ValueType, typename IdType, uint32_t MinLen, uint32_t MaxSize = 65535>
class DynLenArrayTable : public IArrayTable<ValueType, IdType> {
    static_assert(MaxSize > 0, "MaxSize must be > 0");
    static_assert(MaxSize <= 0x7FFFFFFF, "MaxSize too large");

    static constexpr size_t ELEMENT_SIZE   = sizeof(ValueType);
    static constexpr size_t BLOCK_ELEMENTS = MaxSize;
    static constexpr size_t BLOCK_BYTES    = BLOCK_ELEMENTS * ELEMENT_SIZE;

public:
    DynLenArrayTable() {
        blocks_.reserve(16);
        data_.reserve(1024);
    }

    ~DynLenArrayTable() override { Clear(); }

    IdArray<ValueType, IdType> Allocate(size_t len) override {
        if (len == 0 || len > MaxSize) {
            throw std::runtime_error("DynLenArrayTable: invalid length");
        }

        // Try to reuse a free array of exactly this length
        auto it = free_by_len_.find(len);
        if (it != free_by_len_.end() && !it->second.empty()) {
            IdType id = it->second.back();
            it->second.pop_back();
            if (it->second.empty()) {
                free_by_len_.erase(it);
            }
            ValueType* data = data_[id];
            // data[0] stores the length (see layout below)
            // The actual user data starts at data+1
            *reinterpret_cast<IdType*>(data) = static_cast<IdType>(len);
            return IdArray<ValueType, IdType>(data + 1, len, id);
        }

        // Try to split a larger free block, +MinLen to avoid too much fragmentation
        auto upper_it = free_by_len_.upper_bound(len + MinLen + 1);
        if (upper_it != free_by_len_.end() && !upper_it->second.empty()) {
            size_t free_len = upper_it->first;
            IdType id       = upper_it->second.back();
            upper_it->second.pop_back();
            if (upper_it->second.empty()) {
                free_by_len_.erase(upper_it);
            }

            ValueType* data                  = data_[id];
            *reinterpret_cast<IdType*>(data) = static_cast<IdType>(len);
            // -1 because we need 1 slot to store the length
            size_t remain_len = free_len - len - 1;
            // +1 because the new array also needs 1 slot to store its length
            ValueType* remain_data                  = data + 1 + len;
            *reinterpret_cast<IdType*>(remain_data) = static_cast<IdType>(remain_len);
            IdType remain_id                        = static_cast<IdType>(data_.size());
            data_.push_back(remain_data);
            free_by_len_[remain_len].push_back(remain_id);
            return IdArray<ValueType, IdType>(data + 1, len, id);
        }

        // Allocate a new block if current block is exhausted, not need to check block_ is empty
        // because cur_offset_ is initialized to BLOCK_ELEMENTS to force allocation on first call
        if (cur_offset_ + len + 1 > BLOCK_ELEMENTS) {
            int remain_len = BLOCK_ELEMENTS - cur_offset_;
            if (remain_len > MinLen) {
                ValueType* remain_data                  = blocks_[cur_block_] + cur_offset_;
                *reinterpret_cast<IdType*>(remain_data) = static_cast<IdType>(remain_len - 1);
                IdType remain_id                        = static_cast<IdType>(data_.size());
                data_.push_back(remain_data);
                free_by_len_[remain_len - 1].push_back(remain_id);
            }
            ValueType* new_block = static_cast<ValueType*>(malloc(BLOCK_BYTES));
            if (!new_block) {
                throw std::runtime_error("DynLenArrayTable: malloc failed");
            }
            cur_block_ = blocks_.size();
            blocks_.push_back(new_block);
            cur_offset_ = 0;
        }

        ValueType* block                 = blocks_[cur_block_];
        ValueType* data                  = block + cur_offset_;
        *reinterpret_cast<IdType*>(data) = static_cast<IdType>(len);
        IdType id                        = static_cast<IdType>(data_.size());
        data_.push_back(data);
        cur_offset_ += (len + 1);
        return IdArray<ValueType, IdType>(data + 1, len, id);
    }

    void Free(IdType id) override {
        assert(id < static_cast<IdType>(data_.size()));
        ValueType* data = data_[id];
        IdType     len  = *reinterpret_cast<IdType*>(data);
        free_by_len_[len].push_back(id);
    }

    IdArray<ValueType, IdType> Get(IdType id) const override {
        assert(id < static_cast<IdType>(data_.size()));
        ValueType* data = data_[id];
        IdType     len  = *reinterpret_cast<IdType*>(data);
        return IdArray<ValueType, IdType>(data + 1, len, id);
    }

    void Clear() override {
        for (auto blk : blocks_) free(blk);
        blocks_.clear();
        data_.clear();
        free_by_len_.clear();
        cur_block_ = 0;
        // init cur_offset_ to BLOCK_ELEMENTS to force allocation of a new block on first Allocate
        // call
        cur_offset_ = BLOCK_ELEMENTS;
    }

private:
    std::vector<ValueType*> blocks_;  // large blocks of size BLOCK_ELEMENTS
    std::vector<ValueType*> data_;    // pointers to each allocated array (starts with length word)
    std::unordered_map<size_t, std::vector<IdType>> free_by_len_;  // free arrays indexed by length
    size_t                                          cur_block_  = 0;
    size_t                                          cur_offset_ = 0;
};

// ----------------------------------------------------------------------
// IdArrayMgr: dispatches to either fixed-length or dynamic table based on requested length
// FixLen must be of the form 2^n - 1 (e.g., 1,3,7,15,31,63,127,...)
// ----------------------------------------------------------------------
template <typename IdType = uint32_t, typename ValueType = uint32_t, uint16_t FixLen = 7>
class IdArrayMgr {
    static_assert(std::is_integral<IdType>::value, "IdType must be integral");
    static_assert(FixLen > 0, "FixLen must be positive");
    static_assert(IsPowerOfTwoMinusOne<FixLen>(), "FixLen must be of the form 2^n - 1");

    static constexpr uint16_t LEN_BITS = ConstexprILog2<FixLen>();
    static constexpr uint16_t ID_BITS  = sizeof(IdType) * 8;
    static_assert(LEN_BITS < ID_BITS, "FixLen too large for IdType");

public:
    IdArrayMgr() {
        tables_.resize(FixLen + 1, nullptr);
        tables_[0] = new DynLenArrayTable<ValueType, IdType, FixLen + 1>();
        InitFixedTables(std::make_index_sequence<FixLen>{});
    }

    ~IdArrayMgr() { Clear(); }

    IdArray<ValueType, IdType> Allocate(size_t len) {
        if (len == 0) {
            throw std::runtime_error("IdArrayMgr: cannot allocate zero-length array");
        }
        if (len <= FixLen) {
            return tables_[len]->Allocate(len);
        } else {
            return tables_[0]->Allocate(len);
        }
    }

    void Free(IdType id) {
        uint16_t len = GetLenFromId(id);
        if (len <= FixLen) {
            tables_[len]->Free(id);
        } else {
            tables_[0]->Free(id);
        }
    }

    IdArray<ValueType, IdType> Get(IdType id) const {
        uint16_t len = GetLenFromId(id);
        if (len <= FixLen) {
            return tables_[len]->Get(id);
        } else {
            return tables_[0]->Get(id);
        }
    }

    void Clear() {
        for (auto tbl : tables_) delete tbl;
        tables_.clear();
    }

private:
    static uint16_t GetLenFromId(IdType id) {
        constexpr uint16_t shift = ID_BITS - LEN_BITS;
        return static_cast<uint16_t>((id >> shift) & ((1u << LEN_BITS) - 1));
    }

    template <size_t... Is>
    void InitFixedTables(std::index_sequence<Is...>) {
        (CreateFixedTable<Is + 1>(), ...);
    }

    template <uint16_t Len>
    void CreateFixedTable() {
        static_assert(Len <= FixLen, "Internal error");
        tables_[Len] = new FixLenArrayTable<ValueType, IdType, Len>();
    }

    std::vector<IArrayTable<ValueType, IdType>*> tables_;
};

}  // namespace db

#endif  // _BASE_PLATFORM_IDARRAYMGR_H_