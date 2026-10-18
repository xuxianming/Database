#include "DataModel/Design/Design.h"
#include "DataModel/Design/Module.h"
namespace db {

ObjectId Design::m_current_design_id = kInvalidId;
size_t   Design::GetSize() const { return sizeof(Design); }
uint32_t db::Design::GetMaxFanout() const { return 32; }

Module* Design::GetTopModule() const {
    if (m_top_module_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModule(m_top_module_id);
}

void Design::SetTopModule(ObjectId top_module_id) { m_top_module_id = top_module_id; }

void    Design::SetCurrentDesign(ObjectId designId) { m_current_design_id = designId; }
Design* Design::GetCurrentDesign() {
    if (m_current_design_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetDesign(m_current_design_id);
}
void Design::Unfold(bool copy_module) {
    Module* top_module = GetTopModule();
    if (top_module == nullptr) {
        return;
    }
    top_module->Unfold(copy_module);
}

Module* Design::CreateModule(const std::string& name) {
    Module* module = DesignMemoryMgr::CreateModule();
    if (module == nullptr) {
        return nullptr;
    }
    module->m_design_id = GetObjectId();
    if (DesignStringMgr* mgr = GetStringMgr(); mgr != nullptr) {
        module->m_name_id = mgr->AddString(name);
    }
    return module;
}
Module* Design::CreateModule(OperatorType type, const std::string& name) {
    Module* module = DesignMemoryMgr::CreateModule();
    if (module == nullptr) {
        return nullptr;
    }
    module->m_design_id     = GetObjectId();
    module->m_operator_type = type;
    if (DesignStringMgr* mgr = GetStringMgr(); mgr != nullptr) {
        module->m_name_id = mgr->AddString(name);
    }
    return module;
}
}  // namespace db