#ifndef DATAMODEL_DESIGN_STRINGMGR_H_
#define DATAMODEL_DESIGN_STRINGMGR_H_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "BasePlatfrom/SortArray.h"
#include "DataModel/Type.h"

namespace db {

template <typename IdType, int LENGTH_SIZE_BITS = 16, int BLOCK_INDEX_BITS = 8>
class StringTable {
    static_assert(std::is_integral<IdType>::value, "IdType must be integral");
    static_assert(LENGTH_SIZE_BITS > 0 && LENGTH_SIZE_BITS < 32,
                  "LENGTH_SIZE_BITS must be between 1 and 31");
    static_assert(
        BLOCK_INDEX_BITS > 0 && BLOCK_INDEX_BITS < sizeof(IdType) * 8,
        "BLOCK_INDEX_BITS must be between 1 and (sizeof(IdType)*8 - 1)");

public:
    static constexpr int    LENGTH_SIZE = LENGTH_SIZE_BITS;
    static constexpr IdType MAX_STRING_LENGTH =
        (static_cast<IdType>(1) << LENGTH_SIZE) - 1;
    static constexpr int BLOCK_INDEX_WIDTH = BLOCK_INDEX_BITS;
    static constexpr int OBJECT_INDEX_BITS =
        sizeof(IdType) * 8 - BLOCK_INDEX_BITS;
    static constexpr IdType BLOCK_INDEX_SHIFT = OBJECT_INDEX_BITS;
    static constexpr IdType BLOCK_INDEX_MASK =
        (static_cast<IdType>(1) << BLOCK_INDEX_BITS) - 1;
    static constexpr IdType BLOCK_OFFSET_MASK =
        (static_cast<IdType>(1) << OBJECT_INDEX_BITS) - 1;
    static constexpr IdType BLOCK_SIZE = static_cast<IdType>(1)
                                         << OBJECT_INDEX_BITS;
    static constexpr IdType MAX_BLOCK_COUNT =
        (static_cast<IdType>(1) << BLOCK_INDEX_BITS) - 1;
    static constexpr IdType MAX_OBJECT_COUNT = MAX_BLOCK_COUNT * BLOCK_SIZE;

    StringTable();
    ~StringTable();

    IdType           AddString(const std::string& str);
    std::string_view GetString(IdType id) const;
    void             Clear();
    void             DestroyString(IdType id);
    void             Save(std::ofstream& ofs) const;
    void             Load(std::ifstream& ifs);

    static constexpr IdType GetBlockSize() { return BLOCK_SIZE; }
    static constexpr int    GetObjectIndexBits() { return OBJECT_INDEX_BITS; }
    static constexpr int    GetBlockIndexBits() { return BLOCK_INDEX_BITS; }
    static constexpr IdType GetMaxBlockCount() { return MAX_BLOCK_COUNT; }
    static constexpr IdType GetMaxObjectCount() { return MAX_OBJECT_COUNT; }

private:
    bool     FindMostSuitableIdInFreeList(const std::string& str, IdType& id);
    uint16_t GetLength(IdType id) const;

    static constexpr IdType MakeId(IdType block_idx, IdType offset) {
        return (static_cast<IdType>(block_idx) << OBJECT_INDEX_BITS) | offset;
    }

    static constexpr IdType GetBlockIndex(IdType id) {
        return static_cast<IdType>(id >> OBJECT_INDEX_BITS);
    }

    static constexpr IdType GetOffset(IdType id) {
        return id & BLOCK_OFFSET_MASK;
    }

    struct SortIdByLength {
        SortIdByLength() = default;
        SortIdByLength(StringTable* this_table) : table(this_table) {}
        bool operator()(IdType id1, IdType id2) const {
            return table->GetLength(id1) < table->GetLength(id2);
        }

    private:
        StringTable* table = nullptr;
    };

private:
    IdType                            remaining_size;
    IdType                            current_offset;
    std::vector<char*>                strings_;
    SortArray<IdType, SortIdByLength> free_list;
    char*                             current_block = nullptr;
};

template <typename IdType, int LENGTH_SIZE_BITS, int BLOCK_INDEX_BITS>
StringTable<IdType, LENGTH_SIZE_BITS, BLOCK_INDEX_BITS>::StringTable() {
    remaining_size = BLOCK_SIZE;
    current_offset = 0;
    free_list.SetCompare(SortIdByLength(this));
}

template <typename IdType, int LENGTH_SIZE_BITS, int BLOCK_INDEX_BITS>
StringTable<IdType, LENGTH_SIZE_BITS, BLOCK_INDEX_BITS>::~StringTable() {
    Clear();
}

template <typename IdType, int LENGTH_SIZE_BITS, int BLOCK_INDEX_BITS>
IdType StringTable<IdType, LENGTH_SIZE_BITS, BLOCK_INDEX_BITS>::AddString(
    const std::string& str) {
    IdType id;
    if (FindMostSuitableIdInFreeList(str, id)) {
        return id;
    }

    size_t len = str.size();
    if (len > MAX_STRING_LENGTH) {
        throw std::length_error("String exceeds maximum length");
    }

    IdType need_size = len + LENGTH_SIZE;
    if (current_offset + need_size > remaining_size) {
        int leng = remaining_size - LENGTH_SIZE;
        if (leng > 0) {
            IdType block_index = strings_.size() - 1;
            IdType free_id     = MakeId(block_index, current_offset);
            *reinterpret_cast<uint16_t*>(current_block + current_offset) = leng;
            free_list.Insert(free_id);
        }
        if (strings_.size() >= MAX_BLOCK_COUNT) {
            throw std::runtime_error(
                "StringTable has reached maximum block count: " +
                std::to_string(MAX_BLOCK_COUNT));
        }
        char* new_block = (char*)malloc(BLOCK_SIZE);
        if (!new_block) {
            throw std::bad_alloc();
        }
        memset(new_block, 0, BLOCK_SIZE);
        strings_.push_back(new_block);
        current_block = new_block;
        if (strings_.size() == 1) {
            current_offset = 1;
            remaining_size = BLOCK_SIZE - 1;
        } else {
            current_offset = 0;
            remaining_size = BLOCK_SIZE;
        }
    }

    IdType block_id = strings_.size() - 1;
    id              = MakeId(block_id, current_offset);
    *reinterpret_cast<uint16_t*>(current_block + current_offset) = len;
    std::memcpy(current_block + current_offset + LENGTH_SIZE, str.data(), len);
    current_offset += need_size;
    remaining_size -= need_size;
    return id;
}

template <typename IdType, int LENGTH_SIZE_BITS, int BLOCK_INDEX_BITS>
std::string_view StringTable<IdType, LENGTH_SIZE_BITS,
                             BLOCK_INDEX_BITS>::GetString(IdType id) const {
    IdType block_id = GetBlockIndex(id);
    IdType offset   = GetOffset(id);

    if (block_id >= strings_.size()) {
        throw std::out_of_range("Invalid string ID: block ID out of range");
    }

    const char* block = strings_[block_id];
    IdType      block_used =
        (block_id == strings_.size() - 1) ? current_offset : BLOCK_SIZE;

    if (offset >= block_used) {
        throw std::out_of_range("Invalid string ID: offset out of range");
    }

    IdType length = *reinterpret_cast<const uint16_t*>(block + offset);
    return std::string_view(block + offset + LENGTH_SIZE, length);
}

template <typename IdType, int LENGTH_SIZE_BITS, int BLOCK_INDEX_BITS>
void StringTable<IdType, LENGTH_SIZE_BITS, BLOCK_INDEX_BITS>::Clear() {
    for (char* block : strings_) {
        free(block);
    }
    strings_.clear();
    free_list.Clear();
    current_block  = nullptr;
    remaining_size = 0;
    current_offset = 0;
}

template <typename IdType, int LENGTH_SIZE_BITS, int BLOCK_INDEX_BITS>
void StringTable<IdType, LENGTH_SIZE_BITS, BLOCK_INDEX_BITS>::DestroyString(
    IdType id) {
    IdType block_id = GetBlockIndex(id);
    IdType offset   = GetOffset(id);

    if (block_id >= strings_.size()) {
        throw std::out_of_range("Invalid string ID: block ID out of range");
    }

    char*  block = strings_[block_id];
    IdType block_used =
        (block_id == strings_.size() - 1) ? current_offset : BLOCK_SIZE;

    if (offset >= block_used) {
        throw std::out_of_range("Invalid string ID: offset out of range");
    }
    auto it = free_list.Find(id);
    if (it) {
        throw std::runtime_error("String already destroyed");
    }
    free_list.Insert(id);
}

template <typename IdType, int LENGTH_SIZE_BITS, int BLOCK_INDEX_BITS>
void StringTable<IdType, LENGTH_SIZE_BITS, BLOCK_INDEX_BITS>::Save(
    std::ofstream& ofs) const {
    ofs.write(reinterpret_cast<const char*>(&remaining_size),
              sizeof(remaining_size));
    ofs.write(reinterpret_cast<const char*>(&current_offset),
              sizeof(current_offset));
    auto block_count = strings_.size();
    ofs.write(reinterpret_cast<const char*>(&block_count), sizeof(block_count));

    for (const char* block : strings_) {
        ofs.write(block, BLOCK_SIZE);
    }
    auto free_count = free_list.Size();
    ofs.write(reinterpret_cast<const char*>(&free_count), sizeof(free_count));
    for (auto id : free_list) {
        ofs.write(reinterpret_cast<const char*>(&id), sizeof(id));
    }
}

template <typename IdType, int LENGTH_SIZE_BITS, int BLOCK_INDEX_BITS>
void StringTable<IdType, LENGTH_SIZE_BITS, BLOCK_INDEX_BITS>::Load(
    std::ifstream& ifs) {
    Clear();
    if (!ifs.read(reinterpret_cast<char*>(&remaining_size),
                  sizeof(remaining_size))) {
        throw std::runtime_error("Failed to read remaining_size");
    }
    if (!ifs.read(reinterpret_cast<char*>(&current_offset),
                  sizeof(current_offset))) {
        throw std::runtime_error("Failed to read current_offset");
    }

    auto block_count = strings_.size();
    if (!ifs.read(reinterpret_cast<char*>(&block_count), sizeof(block_count))) {
        throw std::runtime_error("Failed to read block_count");
    }

    if (block_count > MAX_BLOCK_COUNT) {
        throw std::runtime_error(
            "Invalid block_count: " + std::to_string(block_count) +
            ", max: " + std::to_string(MAX_BLOCK_COUNT));
    }

    strings_.reserve(block_count);
    for (uint32_t i = 0; i < block_count; ++i) {
        char* block = static_cast<char*>(malloc(BLOCK_SIZE));
        if (!block) {
            throw std::bad_alloc();
        }
        if (!ifs.read(block, BLOCK_SIZE)) {
            free(block);
            throw std::runtime_error("Failed to read block data");
        }
        strings_.push_back(block);
    }

    uint32_t free_count = 0;
    if (!ifs.read(reinterpret_cast<char*>(&free_count), sizeof(free_count))) {
        if (ifs.eof()) {
            free_count = 0;
        } else {
            throw std::runtime_error("Failed to read free_count");
        }
    }

    if (free_count > 0) {
        free_list.Reserve(free_count);
        for (uint32_t i = 0; i < free_count; ++i) {
            IdType id = 0;
            if (!ifs.read(reinterpret_cast<char*>(&id), sizeof(id))) {
                throw std::runtime_error("Failed to read free list entry " +
                                         std::to_string(i));
            }
            IdType block_id = GetBlockIndex(id);
            IdType offset   = GetOffset(id);

            if (block_id >= strings_.size()) {
                throw std::runtime_error("Invalid free list ID: block " +
                                         std::to_string(block_id) +
                                         " out of range");
            }
            if (offset >= BLOCK_SIZE) {
                throw std::runtime_error("Invalid free list ID: offset " +
                                         std::to_string(offset) +
                                         " out of range");
            }
            free_list.PushBack(id);
        }
    }

    if (!strings_.empty()) {
        current_block = strings_.back();
        if (current_offset > BLOCK_SIZE) {
            throw std::runtime_error("Invalid current_offset: " +
                                     std::to_string(current_offset));
        }
    } else {
        current_block  = nullptr;
        current_offset = 0;
        remaining_size = BLOCK_SIZE;
    }
    if (!strings_.empty() && current_block == nullptr) {
        throw std::runtime_error(
            "Inconsistent state: blocks exist but current_block is null");
    }
}

template <typename IdType, int LENGTH_SIZE_BITS, int BLOCK_INDEX_BITS>
bool StringTable<IdType, LENGTH_SIZE_BITS, BLOCK_INDEX_BITS>::
    FindMostSuitableIdInFreeList(const std::string& str, IdType& id) {
    if (free_list.Empty()) {
        return false;
    }
    size_t len = str.size();
    if (len > MAX_STRING_LENGTH) {
        throw std::length_error("String exceeds maximum length");
    }

    auto it = std::lower_bound(free_list.Begin(), free_list.End(), len,
                               [this](IdType id, uint16_t target_len) {
                                   return GetLength(id) < target_len;
                               });

    if (it == free_list.End()) {
        return false;
    }

    IdType found_id                              = *it;
    IdType block_len                             = GetLength(found_id);
    IdType block_id                              = GetBlockIndex(found_id);
    IdType offset                                = GetOffset(found_id);
    char*  block                                 = strings_[block_id];
    *reinterpret_cast<uint16_t*>(block + offset) = len;
    std::memcpy(block + offset + LENGTH_SIZE, str.data(), len);
    free_list.EraseAt(std::distance(free_list.Begin(), it));
    IdType remaining = block_len - len;
    if (remaining > LENGTH_SIZE) {
        IdType new_id = MakeId(block_id, offset + LENGTH_SIZE + len);
        free_list.Insert(new_id);
        *reinterpret_cast<uint16_t*>(block + offset + LENGTH_SIZE + len) =
            remaining - LENGTH_SIZE;
    }
    id = found_id;
    return true;
}

template <typename IdType, int LENGTH_SIZE_BITS, int BLOCK_INDEX_BITS>
uint16_t StringTable<IdType, LENGTH_SIZE_BITS, BLOCK_INDEX_BITS>::GetLength(
    IdType id) const {
    IdType block_id = GetBlockIndex(id);
    IdType offset   = GetOffset(id);

    if (block_id >= strings_.size()) {
        throw std::out_of_range("Invalid string ID: block ID out of range");
    }

    const char* block = strings_[block_id];
    return *reinterpret_cast<const uint16_t*>(block + offset);
}

class StringMgr;
class StringMgrMaintainer {
    StringMgrMaintainer()  = default;
    ~StringMgrMaintainer() = default;

public:
    static StringMgrMaintainer& GetInstance() {
        static StringMgrMaintainer instance;
        return instance;
    }
    /**
     * Get the StringMgr for the given design_id. If it does not exist, create a
     * new one. Ensure legality of design_id before calling this function. 
     */
    StringMgr* GetOrCreateStringMgr(ObjectId design_id);
    StringMgr* CreateStringMgr(ObjectId design_id);
    StringMgr* GetStringMgr(ObjectId design_id) const;
    void       DestroyStringMgr(ObjectId design_id);
    void       Clear();
    void       Save(const std::string& dir) const;
    void       Load(const std::string& dir);

private:
    // for design_id, store the corresponding StringMgr pointer
    std::map<ObjectId, StringMgr*> string_mgrs_;
};

class StringMgr {
    friend class StringMgrMaintainer;
    StringMgr(ObjectId design_id);
    ~StringMgr();

public:
    using StringTableImpl = StringTable<ObjectNameArrayId, 16, 8>;
    inline std::string_view GetName(DMObjectType      type,
                                    ObjectNameArrayId id) const {
        if (type >= DMObjectType::DESIGNOBJ_COUNT) {
            throw std::out_of_range("Invalid object type");
        }
        return name_tables_[(ObjectEnumType)type]->GetString(id);
    }

    inline std::string_view GetPathname(DMObjectType      type,
                                        ObjectNameArrayId id) const {
        if (type >= DMObjectType::DESIGNOBJ_COUNT) {
            throw std::out_of_range("Invalid object type");
        }
        return pathname_tables_[(ObjectEnumType)type]->GetString(id);
    }

    inline ObjectNameArrayId SetName(DMObjectType       type,
                                     const std::string& name) {
        if (type >= DMObjectType::DESIGNOBJ_COUNT) {
            throw std::out_of_range("Invalid object type");
        }
        return name_tables_[(ObjectEnumType)type]->AddString(name);
    }

    inline ObjectNameArrayId SetPathname(DMObjectType       type,
                                         const std::string& pathname) {
        if (type >= DMObjectType::DESIGNOBJ_COUNT) {
            throw std::out_of_range("Invalid object type");
        }
        return pathname_tables_[(ObjectEnumType)type]->AddString(pathname);
    }

    inline void DestroyName(DMObjectType type, ObjectNameArrayId id) {
        if (type >= DMObjectType::DESIGNOBJ_COUNT) {
            throw std::out_of_range("Invalid object type");
        }
        name_tables_[(ObjectEnumType)type]->DestroyString(id);
    }

    inline void DestroyPathName(DMObjectType type, ObjectNameArrayId id) {
        if (type >= DMObjectType::DESIGNOBJ_COUNT) {
            throw std::out_of_range("Invalid object type");
        }
        pathname_tables_[(ObjectEnumType)type]->DestroyString(id);
    }

    void Save(const std::string& dir) const;
    void Load(const std::string& dir);

private:
    ObjectId                      m_design_id;
    std::vector<StringTableImpl*> name_tables_;
    std::vector<StringTableImpl*> pathname_tables_;
};

}  // namespace db

#endif