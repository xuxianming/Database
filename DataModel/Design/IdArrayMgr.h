// IdArrayMgr.h
#ifndef DATAMODEL_DESIGN_IDARRAYMGR_H_
#define DATAMODEL_DESIGN_IDARRAYMGR_H_

#include <cstring>
#include <map>
#include <vector>
#include "BasePlatfrom/SortArray.h"
#include "DataModel/Type.h"

namespace db {

template <typename valueT, typename idType>
class ArrayTable {
protected:
    ArrayTable(uint8_t block_width = 8) {
        block_width_   = block_width;
        index_width_   = sizeof(idType) * 8 - block_width;
        index_mask_    = (static_cast<idType>(1) << index_width_) - 1;
        max_offset_    = index_mask_;
        current_block_ = nullptr;
    }

    inline std::pair<valueT*, idType> GetBlockAndOffset(idType id) const;

protected:
    std::vector<valueT*> data_;
    uint8_t              block_width_;
    uint8_t              index_width_;
    idType               max_offset_;
    idType               index_mask_;
    valueT*              current_block_;
};

struct BlockSegment {
    uint32_t block_id;
    uint32_t offset;
    uint32_t len;
};

class UInt32Array : public ArrayTable<uint32_t, uint32_t> {
public:
    UInt32Array(uint8_t block_width = 16);
    virtual uint32_t InsertValue(uint32_t array_id, uint32_t value_id);
    uint32_t         FindFreeListIndex(uint32_t length);
    inline uint32_t  FreeListSize() const { return free_list_.Size(); }

protected:
    uint32_t        GetLength(uint32_t id);
    inline uint32_t GetBlockId(uint32_t id) const { return id >> index_width_; }
    inline uint32_t GetOffset(uint32_t id) const { return id & index_mask_; }
    inline uint32_t MakeId(uint32_t block_id, uint32_t offset) const {
        return (block_id << index_width_) | offset;
    }

    struct SortIdByLength {
        SortIdByLength() = default;
        SortIdByLength(UInt32Array* this_table) : table(this_table) {}
        bool operator()(uint32_t id1, uint32_t id2) const {
            return table->GetLength(id1) < table->GetLength(id2);
        }

    private:
        UInt32Array* table = nullptr;
    };

    uint32_t                            remaining_size_;
    uint32_t                            current_offset_;
    uint8_t                             length_width_;
    SortArray<uint32_t, SortIdByLength> free_list_;
};

class NonContiguousArray : public UInt32Array {
public:
    NonContiguousArray(uint8_t block_width = 16);
    uint32_t InsertValue(uint32_t array_id, uint32_t value_id) override;
    uint32_t AddLargeArray(const std::vector<uint32_t>& value);
    std::vector<uint32_t> GetLargeArray(uint32_t array_id) const;
    void                  DestroyLargeArray(uint32_t array_id);

private:
    uint32_t AllocateContiguousSpace(uint32_t                   need_len,
                                     std::vector<BlockSegment>& segments);
    void     WriteToSegments(const std::vector<BlockSegment>& segments,
                             const std::vector<uint32_t>&     data);
    std::vector<uint32_t> ReadFromSegments(
        const std::vector<BlockSegment>& segments) const;
    void FreeSegments(const std::vector<BlockSegment>& segments);
    void AddFreeBlock(uint32_t block_id, uint32_t offset, uint32_t len);

    std::map<uint32_t, std::vector<BlockSegment>> array_segments_;
};

class ContiguousArray : public UInt32Array {
public:
    ContiguousArray(uint8_t block_width = 16);
    uint32_t InsertValue(uint32_t array_id, uint32_t value_id) override;
    uint32_t AddArrayValue(const std::vector<uint32_t>& value);
};

class DesignIdArrayMgr {
private:
    uint32_t design_id_;
};

template <typename valueT, typename idType>
inline std::pair<valueT*, idType> ArrayTable<valueT, idType>::GetBlockAndOffset(
    idType id) const {
    idType block_id = id >> index_width_;
    if (block_id >= data_.size()) {
        throw std::out_of_range("Invalid ID: block ID out of range");
    }
    valueT* block = data_[block_id];
    if (block == nullptr) {
        throw std::runtime_error("Invalid block");
    }
    idType offset = id & index_mask_;
    if (offset >= max_offset_) {
        throw std::out_of_range("Invalid ID: offset out of range");
    }
    return {block, offset};
}

class Int32Array : public ArrayTable<int32_t, uint32_t> {};

class FloatArray : public ArrayTable<float, uint32_t> {};

class DoubleArray : public ArrayTable<double, uint32_t> {};

}  // namespace db

#endif