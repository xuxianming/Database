#ifndef DATAMODEL_DESIGN_IDARRAYMGR_H_
#define DATAMODEL_DESIGN_IDARRAYMGR_H_

#include <cstring>
#include <iterator>
#include <map>
#include <vector>
#include "BasePlatfrom/SortArray.h"
#include "DataModel/Design/Type.h"
#include "DataModel/Type.h"
#include "Utils/Log.h"

namespace db {

class IdArrayMgr;
class ArrayTableBase {
public:
    virtual ~ArrayTableBase() = default;
};

/**
 * DynamicArrayTable supports variable-length arrays that can grow by appending
 new elements. It uses a linked-block structure to allow arrays to span multiple
 blocks if they exceed the block size. It also maintains a freelist for
 efficient reuse of destroyed arrays.
 * Features:
 * - Variable-length arrays
 * - Efficient memory management with linked-block structure
 * - Freelist for reuse of destroyed arrays
 */
template <typename ValueT, typename IdType, int BLOCK_WIDTH = 16>
class DynamicArrayTable : public ArrayTableBase {
    static_assert(std::is_integral<IdType>::value, "IdType must be integral");
    static_assert(BLOCK_WIDTH > 0 && BLOCK_WIDTH < sizeof(IdType) * 8,
                  "BLOCK_WIDTH must be between 1 and (sizeof(IdType)*8 - 1)");
    friend class IdArrayMgr;

public:
    static constexpr uint32_t FREELIST_THRESHOLD = 32;

    template <bool IsConst>
    class BasicIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = ValueT;
        using difference_type   = std::ptrdiff_t;
        using pointer =
            typename std::conditional<IsConst, const ValueT*, ValueT*>::type;
        using reference =
            typename std::conditional<IsConst, const ValueT&, ValueT&>::type;

        BasicIterator()
            : table_(nullptr),
              current_id_(0),
              block_offset_(0),
              block_(nullptr),
              block_count_(0),
              block_index_(0) {}

        BasicIterator(const DynamicArrayTable* table, IdType start_id)
            : table_(table),
              current_id_(start_id),
              block_offset_(0),
              block_(nullptr),
              block_count_(0),
              block_index_(0) {
            if (start_id != 0) {
                LoadCurrentBlock();
                while (block_index_ >= block_count_ && current_id_ != 0) {
                    MoveToNextBlock();
                }
            }
        }

        reference operator*() const {
            if (block_ == nullptr || block_index_ >= block_count_) {
                throw std::out_of_range("Iterator dereference out of range");
            }
            return block_[block_offset_ + 2 + block_index_];
        }

        pointer operator->() const { return &(operator*()); }

        BasicIterator& operator++() {
            if (block_ != nullptr) {
                ++block_index_;
                if (block_index_ >= block_count_) {
                    MoveToNextBlock();
                    while (block_index_ >= block_count_ && current_id_ != 0) {
                        MoveToNextBlock();
                    }
                }
            }
            return *this;
        }

        BasicIterator operator++(int) {
            BasicIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const BasicIterator& other) const {
            return table_ == other.table_ && current_id_ == other.current_id_ &&
                   block_index_ == other.block_index_;
        }

        bool operator!=(const BasicIterator& other) const {
            return !(*this == other);
        }

    private:
        void LoadCurrentBlock() {
            if (current_id_ == 0 || table_ == nullptr) {
                block_       = nullptr;
                block_count_ = 0;
                block_index_ = 0;
                return;
            }

            auto [block, offset] = table_->GetBlockAndOffset(current_id_);
            block_               = block;
            block_offset_        = offset;
            block_count_         = block[offset + 1];
            block_index_         = 0;
        }

        void MoveToNextBlock() {
            if (current_id_ == 0 || table_ == nullptr) {
                return;
            }

            auto [block, offset] = table_->GetBlockAndOffset(current_id_);
            current_id_          = block[offset + MAX_BLOCK_COUNT - 1];

            if (current_id_ != 0) {
                LoadCurrentBlock();
            } else {
                block_       = nullptr;
                block_count_ = 0;
                block_index_ = 0;
            }
        }

        const DynamicArrayTable* table_;
        IdType                   current_id_;
        uint32_t                 block_offset_;
        const ValueT*            block_;
        uint32_t                 block_count_;
        uint32_t                 block_index_;
    };

    using Iterator      = BasicIterator<false>;
    using ConstIterator = BasicIterator<true>;

    DynamicArrayTable();
    virtual ~DynamicArrayTable() override;

    Iterator begin(IdType array_id) { return Iterator(this, array_id); }

    ConstIterator begin(IdType array_id) const {
        return ConstIterator(this, array_id);
    }

    ConstIterator cbegin(IdType array_id) const {
        return ConstIterator(this, array_id);
    }

    Iterator end(IdType) { return Iterator(); }

    ConstIterator end(IdType) const { return ConstIterator(); }

    ConstIterator cend(IdType) const { return ConstIterator(); }

    std::pair<Iterator, Iterator> GetArrayIterator(IdType array_id) {
        return {begin(array_id), end(array_id)};
    }

    std::pair<ConstIterator, ConstIterator> GetArrayIterator(
        IdType array_id) const {
        return {cbegin(array_id), cend(array_id)};
    }

    IdType FindSuitableFreeId(IdType length);
    IdType MallocArray(uint32_t length = 17);
    void   DestroyArray(IdType array_id);
    void   AppendToArray(IdType array_id, ValueT value);

    template <typename Func>
    void ForEach(IdType array_id, Func&& func) const {
        if (array_id == 0) return;

        IdType current_id = array_id;
        while (current_id != 0) {
            auto [block, offset] = GetBlockAndOffset(current_id);
            uint32_t count       = block[offset + 1];
            for (uint32_t i = 0; i < count; ++i) {
                func(block[offset + 2 + i]);
            }
            current_id = block[offset + MAX_BLOCK_COUNT - 1];
        }
    }

    template <typename Func>
    bool ForEachUntil(IdType array_id, Func&& func) const {
        if (array_id == 0) return true;

        IdType current_id = array_id;
        while (current_id != 0) {
            auto [block, offset] = GetBlockAndOffset(current_id);
            uint32_t count       = block[offset + 1];
            for (uint32_t i = 0; i < count; ++i) {
                if (!func(block[offset + 2 + i])) {
                    return false;
                }
            }
            current_id = block[offset + MAX_BLOCK_COUNT - 1];
        }
        return true;
    }

    ValueT   GetElementAt(IdType array_id, uint32_t index) const;
    uint32_t GetArraySize(IdType array_id) const;
    uint32_t GetArrayCapacity(IdType array_id) const;
    bool     IsEmpty(IdType array_id) const;
    void     SetMinArrayLength(uint32_t len) { min_array_length_ = len; }
    uint32_t GetMinArrayLength() const { return min_array_length_; }

    inline void AddBlock(ValueT* block) {
        if (data_.size() >= MAX_BLOCK_COUNT) {
            throw std::runtime_error(
                "DynamicArrayTable  has reached maximum block count");
        }
        data_.push_back(block);
        if (data_.size() == 1) {
            current_block_ = block;
        }
    }

    inline ValueT* GetBlock(IdType block_id) const {
        if (block_id >= data_.size()) {
            return nullptr;
        }
        return data_[block_id];
    }

    inline std::pair<ValueT*, IdType> GetBlockAndOffset(IdType id) const {
        IdType block_id = id >> INDEX_WIDTH_VALUE;
        if (block_id >= data_.size()) {
            throw std::out_of_range("Invalid ID: block ID out of range");
        }
        ValueT* block = data_[block_id];
        if (block == nullptr) {
            throw std::runtime_error("Invalid block");
        }
        IdType offset = id & INDEX_MASK;
        if (offset > MAX_OFFSET) {
            throw std::out_of_range("Invalid ID: offset out of range");
        }
        return {block, offset};
    }

    inline ValueT* GetObject(IdType id) const {
        auto [block, offset] = GetBlockAndOffset(id);
        return block + offset;
    }

    inline IdType MakeId(IdType block_id, IdType offset) const {
        if (block_id > MAX_BLOCK_COUNT) {
            throw std::out_of_range("Block ID out of range");
        }
        if (offset > MAX_OFFSET) {
            throw std::out_of_range("Offset out of range");
        }
        return (block_id << INDEX_WIDTH_VALUE) | offset;
    }

    inline IdType GetBlockIndex(IdType id) const {
        return id >> INDEX_WIDTH_VALUE;
    }

    inline IdType GetOffset(IdType id) const { return id & INDEX_MASK; }

    inline IdType GetLength(IdType id) const {
        auto [block, offset] = GetBlockAndOffset(id);
        return block[offset];
    }

    inline size_t GetBlockCount() const { return data_.size(); }
    inline bool   Empty() const { return data_.empty(); }

    inline void Clear() {
        for (auto block : data_) {
            free(block);
        }
        data_.clear();
        current_block_           = nullptr;
        current_block_remainder_ = MAX_BLOCK_COUNT;
        current_offset_          = 0;
        free_list_.Clear();
        last_block_cache_.clear();
    }

    inline ValueT* GetCurrentBlock() const { return current_block_; }
    inline void    SetCurrentBlock(ValueT* block) { current_block_ = block; }

    static constexpr int    GetBlockWidth() { return BLOCK_WIDTH; }
    static constexpr int    GetIndexWidth() { return INDEX_WIDTH_VALUE; }
    static constexpr IdType GetIndexMask() { return INDEX_MASK; }
    static constexpr IdType GetMaxOffset() { return MAX_OFFSET; }
    static constexpr IdType GetMaxBlockCount() { return MAX_BLOCK_COUNT; }

protected:
    static constexpr int BLOCK_WIDTH_VALUE = BLOCK_WIDTH;
    static constexpr int INDEX_WIDTH_VALUE = sizeof(IdType) * 8 - BLOCK_WIDTH;
    static constexpr IdType INDEX_MASK =
        (static_cast<IdType>(1) << INDEX_WIDTH_VALUE) - 1;
    static constexpr IdType MAX_OFFSET = INDEX_MASK;
    static constexpr IdType MAX_BLOCK_COUNT =
        (static_cast<IdType>(1) << BLOCK_WIDTH) - 1;

    struct SortIdByLength {
        SortIdByLength() = default;
        SortIdByLength(DynamicArrayTable* this_table) : table(this_table) {}
        bool operator()(IdType id1, IdType id2) const {
            return table->GetLength(id1) < table->GetLength(id2);
        }

    private:
        DynamicArrayTable* table = nullptr;
    };

    std::vector<ValueT*> data_;
    ValueT*              current_block_           = nullptr;
    IdType               current_block_remainder_ = MAX_BLOCK_COUNT;
    IdType               current_offset_          = 0;
    SortArray<IdType, SortIdByLength> free_list_;

    uint32_t                         min_array_length_ = 17;
    mutable std::map<IdType, IdType> last_block_cache_;

    void   AllocateNewBlock();
    IdType GetLastBlockId(IdType array_id) const;
    IdType TryAllocateContiguousBlock(uint32_t length);
    IdType MallocLargeArray(uint32_t length);
    IdType MallocLargeArraySegmented(uint32_t length);
    void   FreeLargeArray(IdType first_id);
    IdType AllocateNewBlockForArray(uint32_t capacity);
};

// Features:
//   - Fixed length after allocation (no append/delete)
//   - No large array support (single block only)
//   - No cross-block storage
//   - WITH freelist for space reuse (arrays can be destroyed and reused)
//   - Optimized for speed - minimal overhead
// ============================================================================
template <typename ValueT, typename IdType, int BLOCK_WIDTH = 16>
class FixedArrayTable : public ArrayTableBase {
    static_assert(std::is_integral<IdType>::value, "IdType must be integral");
    static_assert(BLOCK_WIDTH > 0 && BLOCK_WIDTH < sizeof(IdType) * 8,
                  "BLOCK_WIDTH must be between 1 and (sizeof(IdType)*8 - 1)");
    friend class IdArrayMgr;

public:
    static constexpr uint32_t FREELIST_THRESHOLD = 32;

    // Simple forward iterator (single block only)
    template <bool IsConst>
    class BasicIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = ValueT;
        using difference_type   = std::ptrdiff_t;
        using pointer =
            typename std::conditional<IsConst, const ValueT*, ValueT*>::type;
        using reference =
            typename std::conditional<IsConst, const ValueT&, ValueT&>::type;

        BasicIterator() : data_(nullptr), index_(0), count_(0) {}

        BasicIterator(ValueT* data, uint32_t count)
            : data_(data), index_(0), count_(count) {}

        reference operator*() const { return data_[index_]; }

        pointer operator->() const { return &data_[index_]; }

        BasicIterator& operator++() {
            ++index_;
            return *this;
        }

        BasicIterator operator++(int) {
            BasicIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const BasicIterator& other) const {
            return data_ == other.data_ && index_ == other.index_;
        }

        bool operator!=(const BasicIterator& other) const {
            return !(*this == other);
        }

    private:
        ValueT*  data_;
        uint32_t index_;
        uint32_t count_;
    };

    using Iterator      = BasicIterator<false>;
    using ConstIterator = BasicIterator<true>;

    FixedArrayTable();
    virtual ~FixedArrayTable() override;

    // ========== Core Operations ==========

    // Allocate a fixed-size array
    IdType MallocArray(uint32_t length);

    // Destroy array and add to freelist for reuse
    void DestroyArray(IdType array_id);

    // ========== Read Operations ==========

    inline uint32_t GetArraySize(IdType array_id) const {
        if (array_id == 0) return 0;
        auto [block, offset] = GetBlockAndOffset(array_id);
        return block[offset + 1];
    }

    inline uint32_t GetArrayCapacity(IdType array_id) const {
        if (array_id == 0) return 0;
        auto [block, offset] = GetBlockAndOffset(array_id);
        return block[offset];
    }

    inline ValueT GetElementAt(IdType array_id, uint32_t index) const {
        auto [block, offset] = GetBlockAndOffset(array_id);
        uint32_t count       = block[offset + 1];
        if (index >= count) {
            throw std::out_of_range("Index out of range");
        }
        return block[offset + 2 + index];
    }

    inline ValueT* GetElementPtr(IdType array_id, uint32_t index) {
        auto [block, offset] = GetBlockAndOffset(array_id);
        uint32_t count       = block[offset + 1];
        if (index >= count) {
            return nullptr;
        }
        return &block[offset + 2 + index];
    }

    inline const ValueT* GetElementPtr(IdType array_id, uint32_t index) const {
        auto [block, offset] = GetBlockAndOffset(array_id);
        uint32_t count       = block[offset + 1];
        if (index >= count) {
            return nullptr;
        }
        return &block[offset + 2 + index];
    }

    inline bool IsEmpty(IdType array_id) const {
        if (array_id == 0) return true;
        auto [block, offset] = GetBlockAndOffset(array_id);
        return block[offset + 1] == 0;
    }

    // ========== Iterator Support ==========

    inline Iterator begin(IdType array_id) {
        if (array_id == 0) return Iterator();
        auto [block, offset] = GetBlockAndOffset(array_id);
        return Iterator(block + offset + 2, block[offset + 1]);
    }

    inline ConstIterator begin(IdType array_id) const {
        if (array_id == 0) return ConstIterator();
        auto [block, offset] = GetBlockAndOffset(array_id);
        return ConstIterator(block + offset + 2, block[offset + 1]);
    }

    inline ConstIterator cbegin(IdType array_id) const {
        return begin(array_id);
    }

    inline Iterator end(IdType) { return Iterator(); }

    inline ConstIterator end(IdType) const { return ConstIterator(); }

    inline ConstIterator cend(IdType) const { return ConstIterator(); }

    inline std::pair<Iterator, Iterator> GetArrayIterator(IdType array_id) {
        return {begin(array_id), end(array_id)};
    }

    inline std::pair<ConstIterator, ConstIterator> GetArrayIterator(
        IdType array_id) const {
        return {cbegin(array_id), cend(array_id)};
    }

    // ========== Fast Functional Traversal ==========

    template <typename Func>
    inline void ForEach(IdType array_id, Func&& func) const {
        if (array_id == 0) return;
        auto [block, offset] = GetBlockAndOffset(array_id);
        uint32_t      count  = block[offset + 1];
        const ValueT* data   = block + offset + 2;
        for (uint32_t i = 0; i < count; ++i) {
            func(data[i]);
        }
    }

    template <typename Func>
    inline bool ForEachUntil(IdType array_id, Func&& func) const {
        if (array_id == 0) return true;
        auto [block, offset] = GetBlockAndOffset(array_id);
        uint32_t      count  = block[offset + 1];
        const ValueT* data   = block + offset + 2;
        for (uint32_t i = 0; i < count; ++i) {
            if (!func(data[i])) {
                return false;
            }
        }
        return true;
    }

    template <typename OutputIt>
    inline OutputIt CopyTo(IdType array_id, OutputIt dest) const {
        if (array_id == 0) return dest;
        auto [block, offset] = GetBlockAndOffset(array_id);
        uint32_t      count  = block[offset + 1];
        const ValueT* data   = block + offset + 2;
        return std::copy(data, data + count, dest);
    }

    // ========== Freelist Operations ==========

    // Find a free block with exact length match
    IdType FindSuitableFreeId(IdType length);

    // Add a free block to freelist
    void AddToFreeList(IdType id);

    // ========== Helper Methods ==========

    inline size_t GetBlockCount() const { return data_.size(); }
    inline bool   Empty() const { return data_.empty(); }
    inline size_t GetFreeListSize() const { return free_list_.Size(); }

    inline void Clear() {
        for (auto block : data_) {
            free(block);
        }
        data_.clear();
        current_block_           = nullptr;
        current_block_remainder_ = MAX_BLOCK_COUNT;
        current_offset_          = 0;
        free_list_.Clear();
    }

    static constexpr int    GetBlockWidth() { return BLOCK_WIDTH; }
    static constexpr int    GetIndexWidth() { return INDEX_WIDTH_VALUE; }
    static constexpr IdType GetMaxOffset() { return MAX_OFFSET; }
    static constexpr IdType GetMaxBlockCount() { return MAX_BLOCK_COUNT; }

protected:
    static constexpr int INDEX_WIDTH_VALUE = sizeof(IdType) * 8 - BLOCK_WIDTH;
    static constexpr IdType INDEX_MASK =
        (static_cast<IdType>(1) << INDEX_WIDTH_VALUE) - 1;
    static constexpr IdType MAX_OFFSET = INDEX_MASK;
    static constexpr IdType MAX_BLOCK_COUNT =
        (static_cast<IdType>(1) << BLOCK_WIDTH) - 1;

    struct SortIdByLength {
        SortIdByLength() = default;
        SortIdByLength(FixedArrayTable* this_table) : table(this_table) {}
        bool operator()(IdType id1, IdType id2) const {
            return table->GetArrayCapacity(id1) < table->GetArrayCapacity(id2);
        }

    private:
        FixedArrayTable* table = nullptr;
    };

    inline std::pair<ValueT*, IdType> GetBlockAndOffset(IdType id) const {
        IdType block_id = id >> INDEX_WIDTH_VALUE;
        if (block_id >= data_.size()) {
            throw std::out_of_range("Invalid ID: block ID out of range");
        }
        ValueT* block = data_[block_id];
        if (block == nullptr) {
            throw std::runtime_error("Invalid block");
        }
        IdType offset = id & INDEX_MASK;
        if (offset > MAX_OFFSET) {
            throw std::out_of_range("Invalid ID: offset out of range");
        }
        return {block, offset};
    }

    inline ValueT* GetObject(IdType id) const {
        auto [block, offset] = GetBlockAndOffset(id);
        return block + offset;
    }

    inline IdType MakeId(IdType block_id, IdType offset) const {
        if (block_id > MAX_BLOCK_COUNT) {
            throw std::out_of_range("Block ID out of range");
        }
        if (offset > MAX_OFFSET) {
            throw std::out_of_range("Offset out of range");
        }
        return (block_id << INDEX_WIDTH_VALUE) | offset;
    }

    inline IdType GetBlockIndex(IdType id) const {
        return id >> INDEX_WIDTH_VALUE;
    }

    inline IdType GetOffset(IdType id) const { return id & INDEX_MASK; }

    inline void AddBlock(ValueT* block) {
        if (data_.size() >= MAX_BLOCK_COUNT) {
            throw std::runtime_error(
                "FixedArrayTable has reached maximum block count");
        }
        data_.push_back(block);
        if (data_.size() == 1) {
            current_block_ = block;
        }
    }

    void   AllocateNewBlock();
    IdType AllocateFromCurrentBlock(uint32_t size);
    IdType AllocateDedicatedBlock(uint32_t size);

protected:
    std::vector<ValueT*> data_;
    ValueT*              current_block_           = nullptr;
    IdType               current_block_remainder_ = MAX_BLOCK_COUNT;
    IdType               current_offset_          = 0;
    SortArray<IdType, SortIdByLength> free_list_;
};

// ============================================================================
// Template Implementations
// ============================================================================

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
FixedArrayTable<ValueT, IdType, BLOCK_WIDTH>::FixedArrayTable() {
    current_block_remainder_ = MAX_BLOCK_COUNT;
    current_offset_          = 0;
    current_block_           = nullptr;
    free_list_.SetCompare(SortIdByLength(this));
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
FixedArrayTable<ValueT, IdType, BLOCK_WIDTH>::~FixedArrayTable() {
    Clear();
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
IdType FixedArrayTable<ValueT, IdType, BLOCK_WIDTH>::FindSuitableFreeId(
    IdType length) {
    if (free_list_.Empty()) {
        return 0;
    }

    // Find first free block with capacity >= length
    auto it = std::lower_bound(free_list_.Begin(), free_list_.End(), length,
                               [this](IdType id, IdType target_len) {
                                   return GetArrayCapacity(id) < target_len;
                               });

    if (it != free_list_.End()) {
        IdType free_id = *it;
        free_list_.EraseAt(std::distance(free_list_.Begin(), it));
        return free_id;
    }
    return 0;
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
void FixedArrayTable<ValueT, IdType, BLOCK_WIDTH>::AddToFreeList(IdType id) {
    free_list_.Insert(id);
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
void FixedArrayTable<ValueT, IdType, BLOCK_WIDTH>::AllocateNewBlock() {
    current_block_ =
        static_cast<ValueT*>(malloc(sizeof(ValueT) * MAX_BLOCK_COUNT));
    if (current_block_ == nullptr) {
        throw std::runtime_error("Failed to allocate new block");
    }
    memset(current_block_, 0, sizeof(ValueT) * MAX_BLOCK_COUNT);

    if (data_.empty()) {
        // Reserve first element (ID 0 is invalid)
        current_block_remainder_ = MAX_BLOCK_COUNT - 1;
        current_offset_          = 1;
    } else {
        current_block_remainder_ = MAX_BLOCK_COUNT;
        current_offset_          = 0;
    }
    data_.push_back(current_block_);
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
IdType FixedArrayTable<ValueT, IdType, BLOCK_WIDTH>::AllocateFromCurrentBlock(
    uint32_t size) {
    if (current_block_ == nullptr || current_block_remainder_ < size) {
        AllocateNewBlock();
    }

    IdType id = MakeId(data_.size() - 1, current_offset_);
    current_block_remainder_ -= size;
    current_offset_ += size;
    return id;
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
IdType FixedArrayTable<ValueT, IdType, BLOCK_WIDTH>::AllocateDedicatedBlock(
    uint32_t size) {
    if (data_.size() >= MAX_BLOCK_COUNT) {
        throw std::runtime_error("Maximum block count reached");
    }

    ValueT* block =
        static_cast<ValueT*>(malloc(sizeof(ValueT) * MAX_BLOCK_COUNT));
    if (block == nullptr) {
        return 0;
    }
    memset(block, 0, sizeof(ValueT) * MAX_BLOCK_COUNT);

    data_.push_back(block);
    return MakeId(data_.size() - 1, 0);
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
IdType FixedArrayTable<ValueT, IdType, BLOCK_WIDTH>::MallocArray(
    uint32_t length) {
    if (length == 0) {
        return 0;
    }

    uint32_t array_size = length + 2;  // +2 for capacity and count
    if (array_size > MAX_BLOCK_COUNT) {
        throw std::runtime_error(
            "Array too large for single block (use DynamicArrayTable  "
            "instead)");
    }

    // Try to find a suitable free block first
    IdType id = FindSuitableFreeId(length);
    if (id != 0) {
        auto [block, offset] = GetBlockAndOffset(id);
        block[offset]        = length;  // capacity
        block[offset + 1]    = 0;       // count
        return id;
    }

    // Try to allocate from current block
    if (current_block_ != nullptr && current_block_remainder_ >= array_size) {
        id = MakeId(data_.size() - 1, current_offset_);
        current_block_remainder_ -= array_size;
        current_offset_ += array_size;
    } else {
        // Try dedicated block
        id = AllocateDedicatedBlock(array_size);
        if (id == 0) {
            // Fallback to new current block
            AllocateNewBlock();
            id = MakeId(data_.size() - 1, current_offset_);
            current_block_remainder_ -= array_size;
            current_offset_ += array_size;
        }
    }

    // Initialize the array header
    auto [block, offset] = GetBlockAndOffset(id);
    block[offset]        = length;  // capacity
    block[offset + 1]    = 0;       // count

    return id;
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
void FixedArrayTable<ValueT, IdType, BLOCK_WIDTH>::DestroyArray(
    IdType array_id) {
    if (array_id == 0) return;

    auto [block, offset] = GetBlockAndOffset(array_id);
    uint32_t capacity    = block[offset];
    uint32_t count       = block[offset + 1];

    // Clear the array header
    block[offset]     = 0;
    block[offset + 1] = 0;

    // Add to freelist for reuse (only if capacity is within threshold)
    if (capacity < FREELIST_THRESHOLD || count == 0) {
        free_list_.Insert(array_id);
    }
    // Note: Large capacity arrays are not added to freelist to avoid
    // fragmentation
}

// Explicit template instantiation
template class FixedArrayTable<ObjectId, PinArrayId, 16>;

class IdArrayMgr {
private:
    ObjectId design_id_;

public:
    explicit IdArrayMgr(ObjectId design_id);
    ~IdArrayMgr();

    DynamicArrayTable<ObjectId, PinArrayId, 16>* GetNetPinsArray() {
        return &net_pins_array_;
    }
    const DynamicArrayTable<ObjectId, PinArrayId, 16>* GetNetPinsArray() const {
        return &net_pins_array_;
    }

private:
    DynamicArrayTable<ObjectId, PinArrayId, 16> net_pins_array_;
    std::vector<ArrayTableBase*>                arrays_;
};

class IdArrayMgrMaintainer {
public:
    static IdArrayMgrMaintainer& GetInstance() {
        static IdArrayMgrMaintainer instance;
        return instance;
    }

    IdArrayMgr* GetOrCreateIdArrayMgr(ObjectId design_id);
    IdArrayMgr* CreateIdArrayMgr(ObjectId design_id);
    IdArrayMgr* GetIdArrayMgr(ObjectId design_id) const;
    void        DestroyIdArrayMgr(ObjectId design_id);
    void        Clear();
    void        Save(const std::string& dir) const;
    void        Load(const std::string& dir);

private:
    std::map<ObjectId, IdArrayMgr*> id_array_mgrs_;
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_IDARRAYMGR_H_