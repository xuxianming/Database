#include "DataModel/Design/IdArrayMgr.h"
#include <algorithm>
#include <cstring>
#include "IdArrayMgr.h"
#include "Utils/Log.h"

namespace db {


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