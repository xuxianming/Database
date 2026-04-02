
#include "DataModel/Design/StringMgr.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/DesignObject.h"
#include "DataModel/Type.h"
#include "Utils/Log.h"

namespace db {

// StringMgr implementation
StringMgr::StringMgr(uint32_t design_id) : m_design_id(design_id) {
    name_tables_.reserve((uint8_t)DMObjectType::DESIGNOBJ_COUNT);
    for (size_t i = 0; i < (uint8_t)DMObjectType::DESIGNOBJ_COUNT; ++i) {
        name_tables_.push_back(new StringTableImpl());
    }
    pathname_tables_.reserve((uint8_t)DMObjectType::DESIGNOBJ_COUNT);
    for (size_t i = 0; i < (uint8_t)DMObjectType::DESIGNOBJ_COUNT; ++i) {
        pathname_tables_.push_back(new StringTableImpl());
    }
}

StringMgr::~StringMgr() {
    for (StringTableImpl* table : name_tables_) {
        delete table;
    }
    for (StringTableImpl* table : pathname_tables_) {
        delete table;
    }
}

void StringMgr::Save(const std::string& dir) const {
    std::ofstream ofs(dir + "/" + std::to_string(m_design_id) + ".dat",
                      std::ios::binary);
    if (!ofs) {
        DB_LOG(ERROR) << "Failed to open file for writing: "
                      << dir + "/stringmgr.dat";
        return;
    }
    ofs.write(reinterpret_cast<const char*>(&m_design_id), sizeof(m_design_id));
    ofs.close();
    std::string dirname = dir + "/" + std::to_string(m_design_id) + "_";
    for (auto type_int = 0; type_int < name_tables_.size(); ++type_int) {
        std::ofstream ofs(dirname + ToString((DMObjectType)type_int) + ".dat",
                          std::ios::binary);
        if (!ofs) {
            DB_LOG(ERROR) << "Failed to open file for writing: "
                          << dirname + ToString((DMObjectType)type_int) +
                                 ".dat";
            continue;
        }
        name_tables_[type_int]->Save(ofs);
        pathname_tables_[type_int]->Save(ofs);
        ofs.close();
    }
    return;
}

void StringMgr::Load(const std::string& dir) {
    std::string   filename = dir + "/" + std::to_string(m_design_id) + ".dat";
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        DB_LOG(ERROR) << "Failed to open file for reading: " << filename;
        return;
    }
    ObjectId loaded_design_id;
    ifs.read(reinterpret_cast<char*>(&loaded_design_id),
             sizeof(loaded_design_id));
    if (loaded_design_id != m_design_id) {
        DB_LOG(ERROR) << "Design ID mismatch: expected " << m_design_id
                      << ", got " << loaded_design_id;
        return;
    }
    ifs.close();
    std::string dirname = dir + "/" + std::to_string(m_design_id) + "_";
    for (auto type_int = 0; type_int < name_tables_.size(); ++type_int) {
        std::ifstream ifs(dirname + ToString((DMObjectType)type_int) + ".dat",
                          std::ios::binary);
        if (!ifs) {
            DB_LOG(ERROR) << "Failed to open file for reading: "
                          << dirname + ToString((DMObjectType)type_int) +
                                 ".dat";
            continue;
        }
        name_tables_[type_int]->Load(ifs);
        pathname_tables_[type_int]->Load(ifs);
        ifs.close();
    }
}

StringMgr* StringMgrMaintainer::GetOrCreateStringMgr(ObjectId design_id) {
    auto it = string_mgrs_.find(design_id);
    if (it != string_mgrs_.end()) {
        return it->second;
    }
    StringMgr* new_mgr      = new StringMgr(design_id);
    string_mgrs_[design_id] = new_mgr;
    return new_mgr;
}

StringMgr* StringMgrMaintainer::CreateStringMgr(ObjectId design_id) {
    auto it = string_mgrs_.find(design_id);
    if (it != string_mgrs_.end()) {
        DB_LOG(WARNING) << "StringMgr for design_id " << design_id
                        << " already exists. Overwriting.";
        delete it->second;
        it->second = new StringMgr(design_id);
        return it->second;
    }
    StringMgr* new_mgr      = new StringMgr(design_id);
    string_mgrs_[design_id] = new_mgr;
    return new_mgr;
}

StringMgr* StringMgrMaintainer::GetStringMgr(ObjectId design_id) const {
    auto it = string_mgrs_.find(design_id);
    if (it != string_mgrs_.end()) {
        return it->second;
    }
    return nullptr;
}

void StringMgrMaintainer::DestroyStringMgr(ObjectId design_id) {
    auto it = string_mgrs_.find(design_id);
    if (it != string_mgrs_.end()) {
        delete it->second;
        string_mgrs_.erase(it);
    } else {
        DB_LOG(WARNING) << "StringMgr for design_id " << design_id
                        << " does not exist. Cannot destroy.";
    }
}

void StringMgrMaintainer::Clear() {
    for (auto& pair : string_mgrs_) {
        delete pair.second;
    }
    string_mgrs_.clear();
}

void StringMgrMaintainer::Save(const std::string& dir) const {
    std::ofstream ofs(dir + "/index.dat", std::ios::binary);
    if (!ofs) {
        DB_LOG(ERROR) << "Failed to open file for writing: "
                      << dir + "/index.dat";
        return;
    }
    uint32_t count = string_mgrs_.size();
    ofs.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& pair : string_mgrs_) {
        ofs.write(reinterpret_cast<const char*>(&pair.first),
                  sizeof(pair.first));
    }
    ofs.close();
    for (const auto& pair : string_mgrs_) {
        pair.second->Save(dir);
    }
}

void StringMgrMaintainer::Load(const std::string& dir) {
    std::ifstream ifs(dir + "/index.dat", std::ios::binary);
    if (!ifs) {
        DB_LOG(ERROR) << "Failed to open file for reading: "
                      << dir + "/index.dat";
        return;
    }
    uint32_t count;
    ifs.read(reinterpret_cast<char*>(&count), sizeof(count));
    for (auto index = 0; index < count; ++index) {
        uint32_t design_id;
        ifs.read(reinterpret_cast<char*>(&design_id), sizeof(design_id));
        Design* design = DesignObject::GetDesign(design_id);
        if (design == nullptr) {
            DB_LOG(ERROR) << "Invalid design ID in index: " << design_id;
            continue;
        }
        StringMgr* mgr = new StringMgr(design_id);
        design->SetStringMgr(mgr);
        mgr->Load(dir);
        string_mgrs_[design_id] = mgr;
    }
    ifs.close();
}

}  // namespace db