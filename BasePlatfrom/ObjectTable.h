#ifndef BASEPLATFROM_OBJECTTABLE_H_
#define BASEPLATFROM_OBJECTTABLE_H_
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
namespace db {

namespace fs = std::filesystem;
template <class TYPE>
class TableBlock {
public:
    explicit TableBlock(uint32_t block_idx);
    ~TableBlock() = default;

    uint32_t    Index() const { return m_block_idx; }
    TYPE*       GetObject(uint32_t idx) { return &m_objects[idx]; }
    const TYPE* GetObject(uint32_t idx) const { return &m_objects[idx]; }

    void WriteToFile(std::ofstream& file) const;
    void ReadFromFile(std::ifstream& file);

    static constexpr int    BLOCK_SIZE  = 1024;
    static constexpr size_t BLOCK_BYTES = BLOCK_SIZE * sizeof(TYPE);

private:
    TYPE     m_objects[BLOCK_SIZE];
    uint32_t m_block_idx;
};

template <class TYPE>
TableBlock<TYPE>::TableBlock(uint32_t block_idx) : m_block_idx(block_idx) {
    for (auto& object : m_objects) {
        new (&object) TYPE();
    }
}

template <class TYPE>
void TableBlock<TYPE>::WriteToFile(std::ofstream& file) const {
    file.write(reinterpret_cast<const char*>(m_objects), BLOCK_BYTES);
}

template <class TYPE>
void TableBlock<TYPE>::ReadFromFile(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(m_objects), BLOCK_BYTES);
}

template <class TYPE>
class ObjectTable {
public:
    explicit ObjectTable(uint8_t type, const std::string& dbPath = ".");
    ~ObjectTable();

    TYPE*    Make();
    void     Destroy(TYPE* object);
    TYPE*    Pointer(uint32_t id) const;
    uint32_t ObjectId(const TYPE* object);
    size_t   Size() const { return m_size; }
    void     Clear();
    uint8_t  GetType() const { return m_type; }

    bool Save();
    bool Load();

    static constexpr int      IDX_BITS           = 7;  // 2^7 = 128
    static constexpr int      BLOCK_OBJECT_COUNT = (1 << IDX_BITS);
    static constexpr uint32_t IDX_MASK           = BLOCK_OBJECT_COUNT - 1;

private:
    void        MakeBlock();
    void        FreePush(TYPE* object, uint32_t id);
    std::string GetFileName() const;

    size_t                         m_size;
    uint32_t                       m_free;
    std::vector<TableBlock<TYPE>*> m_blocks;
    uint8_t                        m_type;
    std::string                    m_db_path;

    static constexpr uint32_t OBJECT_ID_NULL = 0;
};

template <class TYPE>
ObjectTable<TYPE>::ObjectTable(uint8_t type, const std::string& dbPath)
    : m_free(OBJECT_ID_NULL), m_type(type), m_db_path(dbPath) {
    fs::create_directories(dbPath);
}

template <class TYPE>
ObjectTable<TYPE>::~ObjectTable() {
    for (auto block : m_blocks) {
        delete block;
    }
    m_blocks.clear();
}

template <class TYPE>
std::string ObjectTable<TYPE>::GetFileName() const {
    return m_db_path + "/" + std::to_string(m_type) + ".db";
}

template <class TYPE>
TYPE* ObjectTable<TYPE>::Make() {
    if (m_free == OBJECT_ID_NULL) {
        MakeBlock();
    }

    TYPE*    object    = Pointer(m_free);
    auto*    free_next = reinterpret_cast<uint32_t*>(object);
    uint32_t temp_free = m_free;
    m_free             = *free_next;
    m_size++;
    uint32_t idx = temp_free & IDX_MASK;
    object->SetObjectIdx(static_cast<uint32_t>(idx));
    return object;
}

template <class TYPE>
void ObjectTable<TYPE>::FreePush(TYPE* object, uint32_t id) {
    auto* free_next = reinterpret_cast<uint32_t*>(object);
    *free_next      = m_free;
    m_free          = id;
}

template <class TYPE>
void ObjectTable<TYPE>::MakeBlock() {
    auto  block_index = static_cast<uint32_t>(m_blocks.size());
    auto* block       = new TableBlock<TYPE>(block_index);
    m_blocks.push_back(block);
    int last = (block_index > 0) ? 0 : 1;
    for (int i = BLOCK_OBJECT_COUNT - 1; i >= last; i--) {
        TYPE*    obj = block->GetObject(i);
        uint32_t id  = (static_cast<uint32_t>(block_index) << IDX_BITS) + i;
        FreePush(obj, id);
    }
}

template <class TYPE>
TYPE* ObjectTable<TYPE>::Pointer(uint32_t id) const {
    if (id == OBJECT_ID_NULL) {
        return nullptr;
    }
    auto blk_idx = static_cast<uint32_t>(id >> IDX_BITS);
    auto obj_idx = static_cast<uint32_t>(id & IDX_MASK);
    if (blk_idx >= m_blocks.size()) {
        return nullptr;
    }
    return m_blocks[blk_idx]->GetObject(obj_idx);
}

template <class TYPE>
uint32_t ObjectTable<TYPE>::ObjectId(const TYPE* object) {
    for (size_t blk = 0; blk < m_blocks.size(); ++blk) {
        for (int idx = 0; idx < BLOCK_OBJECT_COUNT; ++idx) {
            if (m_blocks[blk]->GetObject(idx) == object) {
                return (static_cast<uint32_t>(blk) << IDX_BITS) + idx;
            }
        }
    }
    return OBJECT_ID_NULL;
}

template <class TYPE>
void ObjectTable<TYPE>::Destroy(TYPE* object) {
    uint32_t object_id = ObjectId(object);
    object->~TYPE();
    m_size--;
    FreePush(object, object_id);
}

template <class TYPE>
void ObjectTable<TYPE>::Clear() {
    for (auto block : m_blocks) {
        delete block;
    }
    m_blocks.clear();
    m_size = 0;
    m_free = OBJECT_ID_NULL;
}

template <class TYPE>
bool ObjectTable<TYPE>::Save() {
    std::string   filename = GetFileName();
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << '\n';
        return false;
    }

    try {
        uint32_t magic = 0x54424C4B;  // "TBLK"
        file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
        uint32_t version = 1;
        file.write(reinterpret_cast<const char*>(&version), sizeof(version));
        auto type = static_cast<uint8_t>(m_type);
        file.write(reinterpret_cast<const char*>(&type), sizeof(type));
        uint32_t count = m_size;
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));
        auto block_count = static_cast<uint32_t>(m_blocks.size());
        file.write(reinterpret_cast<const char*>(&block_count),
                   sizeof(block_count));
        file.write(reinterpret_cast<const char*>(&m_free), sizeof(m_free));
        for (auto block : m_blocks) {
            block->WriteToFile(file);
        }

        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving table " << m_type << ": " << e.what()
                  << '\n';
        return false;
    }
}

template <class TYPE>
bool ObjectTable<TYPE>::Load() {
    std::string   filename = GetFileName();
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return true;
    }

    try {
        uint32_t magic;
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        if (magic != 0x54424C4B) {
            std::cerr << "Invalid file format for " << m_type << '\n';
            return false;
        }
        uint32_t version;
        file.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != 1) {
            std::cerr << "Unsupported version for " << m_type << '\n';
            return false;
        }
        uint8_t type;
        file.read(reinterpret_cast<char*>(&type), sizeof(type));
        if (static_cast<int>(type) != m_type) {
            std::cerr << "Type mismatch for " << m_type << '\n';
            return false;
        }
        uint32_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        uint32_t block_count;
        file.read(reinterpret_cast<char*>(&block_count), sizeof(block_count));
        file.read(reinterpret_cast<char*>(&m_free), sizeof(m_free));
        Clear();
        m_blocks.reserve(block_count);
        for (uint32_t i = 0; i < block_count; ++i) {
            auto* block = new TableBlock<TYPE>(i);
            block->ReadFromFile(file);
            m_blocks.push_back(block);
        }
        m_size = count;
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading table " << m_type << ": " << e.what()
                  << '\n';
        return false;
    }
}

}  // namespace db

#endif