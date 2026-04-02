#include "DataModel/Design/IdArrayMgr.h"
#include <algorithm>
#include <cstring>
#include "IdArrayMgr.h"
#include "Utils/Log.h"

namespace db {

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::DynamicArrayTable () {
    free_list_.SetCompare(SortIdByLength(this));
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::~DynamicArrayTable () {
    Clear();
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
IdType DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::FindSuitableFreeId(
    IdType length) {
    if (free_list_.Empty() || length >= FREELIST_THRESHOLD) {
        return 0;
    }

    auto it = std::lower_bound(free_list_.Begin(), free_list_.End(), length,
                               [this](IdType id, IdType target_len) {
                                   return GetLength(id) < target_len;
                               });
    if (it != free_list_.End() && GetLength(*it) == length) {
        IdType free_id = *it;
        free_list_.EraseAt(std::distance(free_list_.Begin(), it));
        return free_id;
    }
    return 0;
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
IdType DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::MallocArray(
    uint32_t length) {
    if (length < min_array_length_) {
        length = min_array_length_;
    }

    uint32_t array_size = length + 2;

    if (array_size > MAX_BLOCK_COUNT - 1) {
        return MallocLargeArray(length);
    }

    if (length < FREELIST_THRESHOLD) {
        IdType new_id = FindSuitableFreeId(length);
        if (new_id != 0) {
            auto [block, offset]                = GetBlockAndOffset(new_id);
            block[offset]                       = length;
            block[offset + 1]                   = 0;
            block[offset + MAX_BLOCK_COUNT - 1] = 0;
            last_block_cache_.erase(new_id);
            return new_id;
        }
    }

    if (current_block_ == nullptr || current_block_remainder_ < array_size) {
        AllocateNewBlock();
    }

    if (current_block_remainder_ + current_offset_ != MAX_BLOCK_COUNT - 1) {
        throw std::runtime_error(
            "Inconsistent state: current_offset + current_block_remainder "
            "must equal MAX_BLOCK_COUNT - 1");
    }

    IdType id            = MakeId(data_.size() - 1, current_offset_);
    auto [block, offset] = GetBlockAndOffset(id);
    block[offset]        = length;
    block[offset + 1]    = 0;
    block[offset + MAX_BLOCK_COUNT - 1] = 0;

    current_block_remainder_ -= array_size;
    current_offset_ += array_size;
    return id;
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
void DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::DestroyArray(
    IdType array_id) {
    if (array_id == 0) return;

    auto [block, offset] = GetBlockAndOffset(array_id);
    IdType next_id       = block[offset + MAX_BLOCK_COUNT - 1];

    last_block_cache_.erase(array_id);
    IdType current_id = next_id;
    while (current_id != 0) {
        last_block_cache_.erase(current_id);
        auto [b, o] = GetBlockAndOffset(current_id);
        current_id  = b[o + MAX_BLOCK_COUNT - 1];
    }

    if (next_id != 0) {
        FreeLargeArray(array_id);
    } else {
        uint32_t capacity = block[offset];
        if (capacity > 0 && capacity < FREELIST_THRESHOLD) {
            block[offset]                       = capacity;
            block[offset + 1]                   = 0;
            block[offset + MAX_BLOCK_COUNT - 1] = 0;
            free_list_.Insert(MakeId(GetBlockIndex(array_id), offset + 2));
        }
    }
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
void DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::AppendToArray(IdType array_id,
                                                                ValueT value) {
    if (array_id == 0) return;

    IdType last_id       = GetLastBlockId(array_id);
    auto [block, offset] = GetBlockAndOffset(last_id);
    uint32_t capacity    = block[offset];
    uint32_t count       = block[offset + 1];

    if (count < capacity) {
        block[offset + 2 + count] = value;
        block[offset + 1]         = count + 1;
    } else {
        IdType new_id = AllocateNewBlockForArray(capacity);
        if (new_id == 0) {
            throw std::runtime_error("Failed to allocate new block for append");
        }

        auto [new_block, new_offset]                = GetBlockAndOffset(new_id);
        new_block[new_offset]                       = capacity;
        new_block[new_offset + 1]                   = 1;
        new_block[new_offset + 2]                   = value;
        new_block[new_offset + MAX_BLOCK_COUNT - 1] = 0;

        block[offset + MAX_BLOCK_COUNT - 1] = new_id;
        last_block_cache_[array_id]         = new_id;
    }
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
ValueT DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::GetElementAt(
    IdType array_id, uint32_t index) const {
    if (array_id == 0) {
        throw std::out_of_range("Invalid array ID");
    }

    IdType current_id = array_id;
    while (current_id != 0) {
        auto [block, offset] = GetBlockAndOffset(current_id);
        uint32_t count       = block[offset + 1];
        if (index < count) {
            return block[offset + 2 + index];
        }
        index -= count;
        current_id = block[offset + MAX_BLOCK_COUNT - 1];
    }

    throw std::out_of_range("Index out of range");
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
uint32_t DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::GetArraySize(
    IdType array_id) const {
    if (array_id == 0) return 0;

    uint32_t total_count = 0;
    IdType   current_id  = array_id;
    while (current_id != 0) {
        auto [block, offset] = GetBlockAndOffset(current_id);
        total_count += block[offset + 1];
        current_id = block[offset + MAX_BLOCK_COUNT - 1];
    }
    return total_count;
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
uint32_t DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::GetArrayCapacity(
    IdType array_id) const {
    if (array_id == 0) return 0;
    auto [block, offset] = GetBlockAndOffset(array_id);
    return block[offset];
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
bool DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::IsEmpty(
    IdType array_id) const {
    if (array_id == 0) return true;
    auto [block, offset] = GetBlockAndOffset(array_id);
    return block[offset + 1] == 0;
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
void DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::AllocateNewBlock() {
    if (current_block_ != nullptr) {
        int data_length = current_block_remainder_ - 2;
        if (data_length > 0 && data_length < FREELIST_THRESHOLD) {
            current_block_[current_offset_]                       = data_length;
            current_block_[current_offset_ + 1]                   = 0;
            current_block_[current_offset_ + MAX_BLOCK_COUNT - 1] = 0;
            IdType free_id = MakeId(data_.size() - 1, current_offset_ + 2);
            free_list_.Insert(free_id);
        }
    }

    current_block_ =
        static_cast<ValueT*>(malloc(sizeof(ValueT) * MAX_BLOCK_COUNT));
    if (current_block_ == nullptr) {
        throw std::runtime_error("Failed to allocate new block");
    }
    memset(current_block_, 0, sizeof(ValueT) * MAX_BLOCK_COUNT);
    current_block_[MAX_BLOCK_COUNT - 1] = 0;

    if (data_.empty()) {
        current_block_remainder_ = MAX_BLOCK_COUNT - 2;
        current_offset_          = 1;
    } else {
        current_block_remainder_ = MAX_BLOCK_COUNT - 1;
        current_offset_          = 0;
    }
    data_.push_back(current_block_);
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
IdType DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::GetLastBlockId(
    IdType array_id) const {
    auto it = last_block_cache_.find(array_id);
    if (it != last_block_cache_.end()) {
        auto [block, offset] = GetBlockAndOffset(it->second);
        if (block[offset + MAX_BLOCK_COUNT - 1] == 0) {
            return it->second;
        }
    }

    IdType current_id = array_id;
    IdType next_id    = 0;
    do {
        auto [block, offset] = GetBlockAndOffset(current_id);
        next_id              = block[offset + MAX_BLOCK_COUNT - 1];
        if (next_id != 0) {
            current_id = next_id;
        }
    } while (next_id != 0);

    const_cast<std::map<IdType, IdType>&>(last_block_cache_)[array_id] =
        current_id;
    return current_id;
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
IdType DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::TryAllocateContiguousBlock(
    uint32_t length) {
    uint32_t array_size = length + 2;

    if (current_block_ != nullptr && current_block_remainder_ >= array_size) {
        IdType id            = MakeId(data_.size() - 1, current_offset_);
        auto [block, offset] = GetBlockAndOffset(id);
        block[offset]        = length;
        block[offset + 1]    = 0;
        block[offset + MAX_BLOCK_COUNT - 1] = 0;

        current_block_remainder_ -= array_size;
        current_offset_ += array_size;
        return id;
    }

    if (data_.size() < MAX_BLOCK_COUNT) {
        ValueT* new_block =
            static_cast<ValueT*>(malloc(sizeof(ValueT) * MAX_BLOCK_COUNT));
        if (new_block != nullptr) {
            memset(new_block, 0, sizeof(ValueT) * MAX_BLOCK_COUNT);
            new_block[MAX_BLOCK_COUNT - 1] = 0;
            data_.push_back(new_block);

            IdType id                           = MakeId(data_.size() - 1, 0);
            auto [block, offset]                = GetBlockAndOffset(id);
            block[offset]                       = length;
            block[offset + 1]                   = 0;
            block[offset + MAX_BLOCK_COUNT - 1] = 0;

            if (current_block_ == nullptr) {
                current_block_           = new_block;
                current_block_remainder_ = MAX_BLOCK_COUNT - 1 - array_size;
                current_offset_          = array_size;
            }

            return id;
        }
    }

    return 0;
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
IdType DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::MallocLargeArray(
    uint32_t length) {
    if (length <= MAX_BLOCK_COUNT - 3) {
        IdType contiguous_id = TryAllocateContiguousBlock(length);
        if (contiguous_id != 0) {
            return contiguous_id;
        }
    }
    return MallocLargeArraySegmented(length);
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
IdType DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::MallocLargeArraySegmented(
    uint32_t length) {
    uint32_t elements_per_block = MAX_BLOCK_COUNT - 3;
    uint32_t remaining_elements = length;
    IdType   first_id           = 0;
    IdType   prev_id            = 0;

    while (remaining_elements > 0) {
        uint32_t segment_elements =
            std::min(remaining_elements, elements_per_block);
        uint32_t segment_size = segment_elements + 2;

        IdType current_id = 0;

        if (segment_elements < FREELIST_THRESHOLD) {
            current_id = FindSuitableFreeId(segment_elements);
        }

        if (current_id == 0) {
            if (current_block_ == nullptr ||
                current_block_remainder_ < segment_size + 1) {
                AllocateNewBlock();
            }

            current_id = MakeId(data_.size() - 1, current_offset_);
            current_block_remainder_ -= (segment_size + 1);
            current_offset_ += (segment_size + 1);
        }

        auto [block, offset]                = GetBlockAndOffset(current_id);
        block[offset]                       = segment_elements;
        block[offset + 1]                   = 0;
        block[offset + MAX_BLOCK_COUNT - 1] = 0;

        if (prev_id != 0) {
            auto [prev_block, prev_offset] = GetBlockAndOffset(prev_id);
            prev_block[prev_offset + MAX_BLOCK_COUNT - 1] = current_id;
        }

        if (first_id == 0) {
            first_id = current_id;
        }

        prev_id = current_id;
        remaining_elements -= segment_elements;
    }

    return first_id;
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
void DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::FreeLargeArray(
    IdType first_id) {
    IdType current_id = first_id;
    while (current_id != 0) {
        auto [block, offset] = GetBlockAndOffset(current_id);
        IdType   next_id     = block[offset + MAX_BLOCK_COUNT - 1];
        uint32_t capacity    = block[offset];

        if (capacity < FREELIST_THRESHOLD) {
            block[offset]                       = capacity;
            block[offset + 1]                   = 0;
            block[offset + MAX_BLOCK_COUNT - 1] = 0;
            free_list_.Insert(MakeId(GetBlockIndex(current_id), offset + 2));
        }

        last_block_cache_.erase(current_id);
        current_id = next_id;
    }
}

template <typename ValueT, typename IdType, int BLOCK_WIDTH>
IdType DynamicArrayTable <ValueT, IdType, BLOCK_WIDTH>::AllocateNewBlockForArray(
    uint32_t capacity) {
    if (capacity < FREELIST_THRESHOLD) {
        IdType id = FindSuitableFreeId(capacity);
        if (id != 0) {
            return id;
        }
    }

    if (current_block_ == nullptr || current_block_remainder_ < capacity + 3) {
        AllocateNewBlock();
    }

    IdType id = MakeId(data_.size() - 1, current_offset_);
    current_block_remainder_ -= (capacity + 3);
    current_offset_ += (capacity + 3);
    return id;
}

template class DynamicArrayTable <ObjectId, PinArrayId, 16>;
template class DynamicArrayTable <ObjectId, PortArrayId, 16>;
template class DynamicArrayTable <ObjectId, InstArrayId, 16>;  



// IdArrayMgr Implementation
IdArrayMgr::IdArrayMgr(ObjectId design_id) : design_id_(design_id) {}

IdArrayMgr::~IdArrayMgr() {}

// IdArrayMgrMaintainer Implementation
IdArrayMgr* IdArrayMgrMaintainer::GetOrCreateIdArrayMgr(ObjectId design_id) {
    auto it = id_array_mgrs_.find(design_id);
    if (it != id_array_mgrs_.end()) {
        return it->second;
    }
    IdArrayMgr* new_mgr       = new IdArrayMgr(design_id);
    id_array_mgrs_[design_id] = new_mgr;
    return new_mgr;
}

IdArrayMgr* IdArrayMgrMaintainer::CreateIdArrayMgr(ObjectId design_id) {
    auto it = id_array_mgrs_.find(design_id);
    if (it != id_array_mgrs_.end()) {
        DB_LOG(WARNING) << "IdArrayMgr for design_id " << design_id
                        << " already exists. Overwriting.";
        delete it->second;
        it->second = new IdArrayMgr(design_id);
        return it->second;
    }
    IdArrayMgr* new_mgr       = new IdArrayMgr(design_id);
    id_array_mgrs_[design_id] = new_mgr;
    return new_mgr;
}

IdArrayMgr* IdArrayMgrMaintainer::GetIdArrayMgr(ObjectId design_id) const {
    auto it = id_array_mgrs_.find(design_id);
    if (it != id_array_mgrs_.end()) {
        return it->second;
    }
    return nullptr;
}

void IdArrayMgrMaintainer::DestroyIdArrayMgr(ObjectId design_id) {
    auto it = id_array_mgrs_.find(design_id);
    if (it != id_array_mgrs_.end()) {
        delete it->second;
        id_array_mgrs_.erase(it);
    } else {
        DB_LOG(WARNING) << "IdArrayMgr for design_id " << design_id
                        << " does not exist. Cannot destroy.";
    }
}

void IdArrayMgrMaintainer::Clear() {
    for (auto& pair : id_array_mgrs_) {
        delete pair.second;
    }
    id_array_mgrs_.clear();
}

void IdArrayMgrMaintainer::Save(const std::string& dir) const {
    DB_LOG(INFO) << "Save IdArrayMgr to " << dir;
}

void IdArrayMgrMaintainer::Load(const std::string& dir) {
    DB_LOG(INFO) << "Load IdArrayMgr from " << dir;
}

}  // namespace db