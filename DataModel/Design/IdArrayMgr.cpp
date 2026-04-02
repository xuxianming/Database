// IdArrayMgr.cpp
#include "DataModel/Design/IdArrayMgr.h"
#include <algorithm>
#include <cstring>
#include "Utils/Log.h"

namespace db {

UInt32Array::UInt32Array(uint8_t block_width)
    : ArrayTable<uint32_t, uint32_t>(block_width) {
    remaining_size_ = max_offset_ - 1;
    current_offset_ = 1;
    free_list_.SetCompare(SortIdByLength(this));
}

uint32_t UInt32Array::InsertValue(uint32_t array_id, uint32_t value_id) {
    return 0;
}

uint32_t UInt32Array::GetLength(uint32_t id) {
    auto [block, index] = GetBlockAndOffset(id);
    return *(block + index);
}

uint32_t UInt32Array::FindFreeListIndex(uint32_t length) {
    if (free_list_.Empty()) {
        return free_list_.Size();
    }

    size_t left  = 0;
    size_t right = free_list_.Size() - 1;

    while (left <= right) {
        size_t mid = left + (right - left) / 2;
        auto   id  = free_list_[mid];
        auto   len = GetLength(id);
        if (len == length) {
            return static_cast<uint32_t>(mid);
        }
        if (len < length) {
            left = mid + 1;
        } else {
            if (mid == 0) break;
            right = mid - 1;
        }
    }
    return free_list_.Size();
}

NonContiguousArray::NonContiguousArray(uint8_t block_width)
    : UInt32Array(block_width) {}

uint32_t NonContiguousArray::InsertValue(uint32_t array_id, uint32_t value_id) {
    return 0;
}

uint32_t NonContiguousArray::AddLargeArray(const std::vector<uint32_t>& value) {
    uint32_t                  len = value.size();
    std::vector<BlockSegment> segments;

    uint32_t array_id = AllocateContiguousSpace(len, segments);
    if (array_id == 0) {
        throw std::runtime_error("Failed to allocate space for large array");
    }

    WriteToSegments(segments, value);
    array_segments_[array_id] = std::move(segments);

    return array_id;
}

uint32_t NonContiguousArray::AllocateContiguousSpace(
    uint32_t need_len, std::vector<BlockSegment>& segments) {
    if (need_len == 0) return 0;

    uint32_t remaining = need_len;
    uint32_t first_id  = 0;

    while (remaining > 0) {
        uint32_t free_index = FindFreeListIndex(remaining);
        if (free_index != free_list_.Size()) {
            uint32_t free_id     = free_list_[free_index];
            auto [block, offset] = GetBlockAndOffset(free_id);
            uint32_t block_len   = GetLength(free_id);

            uint32_t use_len  = std::min(remaining, block_len);
            uint32_t block_id = GetBlockId(free_id);
            segments.push_back({block_id, offset, use_len});

            free_list_.EraseAt(free_index);

            if (block_len > use_len) {
                uint32_t new_offset   = offset + use_len;
                uint32_t new_id       = MakeId(block_id, new_offset);
                *(block + new_offset) = block_len - use_len;
                free_list_.Insert(new_id);
            }

            remaining -= use_len;
            if (first_id == 0) {
                first_id = free_id;
            }
            continue;
        }

        if (current_block_ == nullptr || current_offset_ >= max_offset_) {
            current_block_ = new uint32_t[max_offset_];
            if (!current_block_) throw std::bad_alloc();
            memset(current_block_, 0, max_offset_ * sizeof(uint32_t));
            data_.push_back(current_block_);
            current_offset_ = 0;
            remaining_size_ = max_offset_;
        }

        uint32_t block_remain = max_offset_ - current_offset_;
        uint32_t alloc_len    = std::min(remaining, block_remain);
        uint32_t block_id     = data_.size() - 1;

        segments.push_back({block_id, current_offset_, alloc_len});

        if (first_id == 0) {
            first_id = MakeId(block_id, current_offset_);
        }

        current_offset_ += alloc_len;
        remaining_size_ -= alloc_len;
        remaining -= alloc_len;
    }

    return first_id;
}

void NonContiguousArray::WriteToSegments(
    const std::vector<BlockSegment>& segments,
    const std::vector<uint32_t>&     data) {
    uint32_t data_offset = 0;

    for (const auto& seg : segments) {
        uint32_t id         = MakeId(seg.block_id, seg.offset);
        auto [block, _]     = GetBlockAndOffset(id);
        uint32_t* block_ptr = block;

        if (data_offset == 0) {
            *(block_ptr + seg.offset) = data.size();
        }

        uint32_t start_offset = (data_offset == 0) ? 1 : 0;
        std::memcpy(block_ptr + seg.offset + start_offset,
                    data.data() + data_offset, seg.len * sizeof(uint32_t));

        data_offset += seg.len;
    }
}

std::vector<uint32_t> NonContiguousArray::GetLargeArray(
    uint32_t array_id) const {
    auto it = array_segments_.find(array_id);
    if (it == array_segments_.end()) {
        throw std::runtime_error("Array not found");
    }

    return ReadFromSegments(it->second);
}

std::vector<uint32_t> NonContiguousArray::ReadFromSegments(
    const std::vector<BlockSegment>& segments) const {
    if (segments.empty()) return {};

    uint32_t first_id = MakeId(segments[0].block_id, segments[0].offset);
    auto [first_block, first_offset] = GetBlockAndOffset(first_id);
    uint32_t total_len               = *(first_block + first_offset);

    std::vector<uint32_t> result;
    result.reserve(total_len);

    uint32_t data_offset = 0;
    for (const auto& seg : segments) {
        uint32_t id         = MakeId(seg.block_id, seg.offset);
        auto [block, _]     = GetBlockAndOffset(id);
        uint32_t* block_ptr = block;

        uint32_t start = (data_offset == 0) ? 1 : 0;
        for (uint32_t i = start; i < seg.len; ++i) {
            result.push_back(*(block_ptr + seg.offset + i));
        }
        data_offset += seg.len;
    }

    return result;
}

void NonContiguousArray::DestroyLargeArray(uint32_t array_id) {
    auto it = array_segments_.find(array_id);
    if (it == array_segments_.end()) {
        return;
    }

    FreeSegments(it->second);
    array_segments_.erase(it);
}

void NonContiguousArray::FreeSegments(
    const std::vector<BlockSegment>& segments) {
    for (const auto& seg : segments) {
        AddFreeBlock(seg.block_id, seg.offset, seg.len);
    }
}

void NonContiguousArray::AddFreeBlock(uint32_t block_id, uint32_t offset,
                                      uint32_t len) {
    uint32_t id       = MakeId(block_id, offset);
    auto [block, _]   = GetBlockAndOffset(id);
    *(block + offset) = len - 1;
    free_list_.Insert(id);
}

ContiguousArray::ContiguousArray(uint8_t block_width)
    : UInt32Array(block_width) {}

uint32_t ContiguousArray::InsertValue(uint32_t array_id, uint32_t value_id) {
    return 0;
}

uint32_t ContiguousArray::AddArrayValue(const std::vector<uint32_t>& value) {
    uint32_t len = value.size();
    if (len + 1 > max_offset_) {
        throw std::runtime_error("Array too large for single block");
    }

    uint32_t free_index = FindFreeListIndex(len);
    if (free_index != free_list_.Size()) {
        uint32_t id          = free_list_[free_index];
        auto [block, offset] = GetBlockAndOffset(id);
        std::memcpy(block + offset + 1, value.data(), len * sizeof(uint32_t));
        free_list_.EraseAt(free_index);
        return id;
    }

    if (current_block_ == nullptr || current_offset_ + len + 1 > max_offset_) {
        if (current_block_ != nullptr && current_offset_ < max_offset_) {
            uint32_t tail = max_offset_ - current_offset_;
            if (tail > 1) {
                uint32_t block_id = data_.size() - 1;
                uint32_t free_id  = MakeId(block_id, current_offset_);
                *(current_block_ + current_offset_) = tail - 1;
                free_list_.Insert(free_id);
            }
        }

        current_block_ = new uint32_t[max_offset_];
        if (!current_block_) throw std::bad_alloc();
        memset(current_block_, 0, max_offset_ * sizeof(uint32_t));
        data_.push_back(current_block_);
        current_offset_ = 0;
        remaining_size_ = max_offset_;
    }

    uint32_t block_id                   = data_.size() - 1;
    uint32_t id                         = MakeId(block_id, current_offset_);
    *(current_block_ + current_offset_) = len;
    std::memcpy(current_block_ + current_offset_ + 1, value.data(),
                len * sizeof(uint32_t));
    current_offset_ += len + 1;
    remaining_size_ -= len + 1;

    return id;
}

}  // namespace db