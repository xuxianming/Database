#ifndef BASEPLATFROM_OBJECTTABLE_H_
#define BASEPLATFROM_OBJECTTABLE_H_

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>
#include "Utils/Log.h"

namespace db {
namespace fs = std::filesystem;

template <int N, class IdType>
constexpr IdType Pow2() {
    constexpr int max_shift = sizeof(IdType) * 8;
    static_assert(N >= 0 && N < max_shift,
                  "Power must be between 0 and (31 or 63) depending on IdType");
    return 1u << N;
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
class TableBlock {
    static_assert(std::is_integral<IdType>::value, "IdType must be integral");
    static_assert(
        OBJECT_INDEX_BITS > 0 && OBJECT_INDEX_BITS < sizeof(IdType) * 8,
        "OBJECT_INDEX_BITS must be between 1 and (sizeof(IdType)*8 - 1)");

public:
    explicit TableBlock(uint32_t block_idx);
    ~TableBlock() = default;

    uint32_t    Index() const { return m_block_idx; }
    TYPE*       GetObject(uint32_t idx) { return &m_objects[idx]; }
    const TYPE* GetObject(uint32_t idx) const { return &m_objects[idx]; }

    void WriteToFile(std::ofstream& file) const;
    void ReadFromFile(std::ifstream& file);

    static constexpr uint32_t BLOCK_SIZE  = Pow2<OBJECT_INDEX_BITS, IdType>();
    static constexpr size_t   BLOCK_BYTES = BLOCK_SIZE * sizeof(TYPE);
    static constexpr int      INDEX_BITS  = OBJECT_INDEX_BITS;
    static constexpr uint32_t INDEX_MASK  = BLOCK_SIZE - 1;

private:
    TYPE     m_objects[BLOCK_SIZE];
    uint32_t m_block_idx;
};

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
TableBlock<TYPE, IdType, OBJECT_INDEX_BITS>::TableBlock(uint32_t block_idx)
    : m_block_idx(block_idx) {
    static_assert(BLOCK_SIZE <= 65536, "Block size too large, max 65536");
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
void TableBlock<TYPE, IdType, OBJECT_INDEX_BITS>::WriteToFile(
    std::ofstream& file) const {
    file.write(reinterpret_cast<const char*>(m_objects), BLOCK_BYTES);
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
void TableBlock<TYPE, IdType, OBJECT_INDEX_BITS>::ReadFromFile(
    std::ifstream& file) {
    file.read(reinterpret_cast<char*>(m_objects), BLOCK_BYTES);
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS = 7>
class ObjectTable {
    static_assert(std::is_integral<IdType>::value, "IdType must be integral");
    static_assert(
        OBJECT_INDEX_BITS > 0 && OBJECT_INDEX_BITS < sizeof(IdType) * 8,
        "OBJECT_INDEX_BITS must be between 1 and (sizeof(IdType)*8 - 1)");

public:
    explicit ObjectTable(uint8_t type, const std::string& dbPath = ".");
    ~ObjectTable();

    IdType GetNextObjId();
    TYPE*  Make();
    void   Destroy(TYPE* object);
    TYPE*  Pointer(IdType id) const;
    size_t Size() const { return m_size; }
    void   Clear();

    bool Save();
    bool Load();

    static constexpr uint32_t GetBlockSize() { return BLOCK_SIZE; }
    static constexpr int      GetIndexBits() { return OBJECT_INDEX_BITS; }
    static constexpr int      GetBlockIndexBits() {
        return sizeof(IdType) * 8 - OBJECT_INDEX_BITS;
    }
    static constexpr IdType GetMaxBlockCount() {
        return (static_cast<IdType>(1) << GetBlockIndexBits()) - 1;
    }
    static constexpr IdType GetMaxObjectCount() {
        return GetMaxBlockCount() * BLOCK_SIZE;
    }

private:
    using BlockType = TableBlock<TYPE, IdType, OBJECT_INDEX_BITS>;

    void        MakeBlock();
    void        FreePush(IdType id);
    std::string GetFileName() const;

    static constexpr uint32_t GetBlockIndex(IdType id) {
        return static_cast<uint32_t>(id >> OBJECT_INDEX_BITS);
    }

    static constexpr uint32_t GetObjectIndex(IdType id) {
        return static_cast<uint32_t>(id & BlockType::INDEX_MASK);
    }

    static constexpr IdType MakeId(uint32_t block_idx, uint32_t obj_idx) {
        return (static_cast<IdType>(block_idx) << OBJECT_INDEX_BITS) | obj_idx;
    }

    size_t                  m_size{};
    IdType                  m_free;
    std::vector<BlockType*> m_blocks;
    uint8_t                 m_type;
    std::string             m_db_path;

    static constexpr IdType   OBJECT_ID_NULL = 0;
    static constexpr uint32_t BLOCK_SIZE     = BlockType::BLOCK_SIZE;
};

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
IdType ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS>::GetNextObjId() {
    if (m_free == OBJECT_ID_NULL) {
        MakeBlock();
    }
    return m_free;
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS>::ObjectTable(
    uint8_t type, const std::string& dbPath)
    : m_free(OBJECT_ID_NULL), m_type(type), m_db_path(dbPath) {
    fs::create_directories(dbPath);

    static_assert(BLOCK_SIZE > 0, "BLOCK_SIZE must be greater than 0");
    static_assert(BLOCK_SIZE <= 65536, "BLOCK_SIZE too large, max 65536");

    if (BLOCK_SIZE == 0) {
        DB_LOG(ERROR) << "Invalid BLOCK_SIZE: 0\n";
    }
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS>::~ObjectTable() {
    for (auto block : m_blocks) {
        delete block;
    }
    m_blocks.clear();
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
std::string ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS>::GetFileName() const {
    return m_db_path + "/" + std::to_string(m_type) + "_" +
           std::to_string(OBJECT_INDEX_BITS) + "bits.db";
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
void ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS>::MakeBlock() {
    auto  block_index = static_cast<uint32_t>(m_blocks.size());
    auto* block       = new BlockType(block_index);
    m_blocks.push_back(block);

    if (block_index > GetMaxBlockCount()) {
        DB_LOG(ERROR) << "Exceeded maximum block count: " << GetMaxBlockCount()
                      << "\n";
        delete block;
        return;
    }

    int start_idx = (block_index == 0) ? 1 : 0;
    for (int i = BLOCK_SIZE - 1; i >= start_idx; --i) {
        IdType id = MakeId(block_index, i);
        FreePush(id);
    }
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
void ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS>::FreePush(IdType id) {
    auto* next_ptr = reinterpret_cast<IdType*>(Pointer(id));
    *next_ptr      = m_free;
    m_free         = id;
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
TYPE* ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS>::Make() {
    if (m_free == OBJECT_ID_NULL) {
        MakeBlock();
    }
    IdType id     = m_free;
    TYPE*  object = Pointer(id);
    if (!object) {
        DB_LOG(ERROR) << "Make() failed: invalid free ID " << id << "\n";
        return nullptr;
    }

    auto* next_ptr = reinterpret_cast<IdType*>(object);
    m_free         = *next_ptr;
    object         = new (object) TYPE();

    ++m_size;
    return object;
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
void ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS>::Destroy(TYPE* object) {
    if (!object) {
        return;
    }

    IdType id = object->GetObjectId();
    if (id == OBJECT_ID_NULL) {
        DB_LOG(ERROR) << "Destroy: object ID is null\n";
        return;
    }
    object->~TYPE();
    FreePush(id);
    --m_size;
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
TYPE* ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS>::Pointer(IdType id) const {
    if (id == OBJECT_ID_NULL) {
        return nullptr;
    }

    uint32_t blk_idx = GetBlockIndex(id);
    uint32_t obj_idx = GetObjectIndex(id);

    if (blk_idx >= m_blocks.size()) {
        DB_LOG(WARNING) << "Pointer: invalid block index " << blk_idx
                        << " (max: " << m_blocks.size() << ")\n";
        return nullptr;
    }

    if (obj_idx >= BLOCK_SIZE) {
        DB_LOG(WARNING) << "Pointer: invalid object index " << obj_idx
                        << " (max: " << BLOCK_SIZE << ")\n";
        return nullptr;
    }

    return m_blocks[blk_idx]->GetObject(obj_idx);
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
void ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS>::Clear() {
    for (auto block : m_blocks) {
        delete block;
    }
    m_blocks.clear();
    m_size = 0;
    m_free = OBJECT_ID_NULL;
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
bool ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS>::Save() {
    std::string   filename = GetFileName();
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        DB_LOG(ERROR) << "Failed to open file for writing: " << filename
                      << "\n";
        return false;
    }

    try {
        uint32_t magic = 0x54424C4B;
        file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));

        uint32_t version = 3;
        file.write(reinterpret_cast<const char*>(&version), sizeof(version));

        uint32_t index_bits = OBJECT_INDEX_BITS;
        file.write(reinterpret_cast<const char*>(&index_bits),
                   sizeof(index_bits));

        uint32_t id_type_size = sizeof(IdType);
        file.write(reinterpret_cast<const char*>(&id_type_size),
                   sizeof(id_type_size));

        uint8_t type = m_type;
        file.write(reinterpret_cast<const char*>(&type), sizeof(type));

        auto count = static_cast<uint64_t>(m_size);
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));

        auto block_count = static_cast<uint32_t>(m_blocks.size());
        file.write(reinterpret_cast<const char*>(&block_count),
                   sizeof(block_count));
        file.write(reinterpret_cast<const char*>(&m_free), sizeof(m_free));

        for (auto block : m_blocks) {
            block->WriteToFile(file);
        }

        file.close();

        DB_LOG(INFO) << "Saved " << m_size << " objects in " << block_count
                     << " blocks (block size: " << BLOCK_SIZE << ")\n";
        return true;
    } catch (const std::exception& e) {
        DB_LOG(ERROR) << "Exception during Save: " << e.what() << "\n";
        return false;
    }
}

template <class TYPE, class IdType, uint8_t OBJECT_INDEX_BITS>
bool ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS>::Load() {
    std::string   filename = GetFileName();
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        DB_LOG(INFO) << "File not found, starting with empty table: "
                     << filename << "\n";
        return true;
    }

    try {
        uint32_t magic;
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        if (!file.good() || magic != 0x54424C4B) {
            DB_LOG(ERROR) << "Invalid magic number\n";
            return false;
        }

        uint32_t version;
        file.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (!file.good()) {
            return false;
        }

        if (version == 1 || version == 2) {
            DB_LOG(ERROR) << "Old version file not supported with bit-field "
                             "configuration. "
                          << "Please migrate data or use legacy code.\n";
            return false;
        } else if (version == 3) {
            uint32_t saved_index_bits;
            file.read(reinterpret_cast<char*>(&saved_index_bits),
                      sizeof(saved_index_bits));
            if (!file.good() || saved_index_bits != OBJECT_INDEX_BITS) {
                DB_LOG(ERROR)
                    << "Index bits mismatch: expected " << OBJECT_INDEX_BITS
                    << ", got " << saved_index_bits << "\n";
                return false;
            }

            uint32_t saved_id_size;
            file.read(reinterpret_cast<char*>(&saved_id_size),
                      sizeof(saved_id_size));
            if (saved_id_size != sizeof(IdType)) {
                DB_LOG(ERROR)
                    << "IdType size mismatch: expected " << sizeof(IdType)
                    << ", got " << saved_id_size << "\n";
                return false;
            }
        } else {
            DB_LOG(ERROR) << "Unsupported version: " << version << "\n";
            return false;
        }

        uint8_t type;
        file.read(reinterpret_cast<char*>(&type), sizeof(type));
        if (!file.good() || type != m_type) {
            DB_LOG(ERROR) << "Type mismatch: expected " << (int)m_type
                          << ", got " << (int)type << "\n";
            return false;
        }

        uint64_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));

        uint32_t block_count;
        file.read(reinterpret_cast<char*>(&block_count), sizeof(block_count));
        file.read(reinterpret_cast<char*>(&m_free), sizeof(m_free));

        if (!file.good()) {
            return false;
        }

        if (block_count > GetMaxBlockCount()) {
            DB_LOG(ERROR) << "Block count " << block_count
                          << " exceeds maximum " << GetMaxBlockCount() << "\n";
            return false;
        }

        Clear();
        m_blocks.reserve(block_count);

        for (uint32_t i = 0; i < block_count; ++i) {
            auto* block = new BlockType(i);
            block->ReadFromFile(file);
            if (!file.good()) {
                DB_LOG(ERROR) << "Failed to read block " << i << "\n";
                Clear();
                return false;
            }
            m_blocks.push_back(block);
        }

        m_size = static_cast<size_t>(count);
        file.close();

        DB_LOG(INFO) << "Loaded " << m_size << " objects from " << block_count
                     << " blocks (block size: " << BLOCK_SIZE << ")\n";
        return true;
    } catch (const std::exception& e) {
        DB_LOG(ERROR) << "Exception during Load: " << e.what() << "\n";
        return false;
    }
}

}  // namespace db

#endif  // BASEPLATFROM_OBJECTTABLE_H_