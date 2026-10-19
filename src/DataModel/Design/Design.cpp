#include "DataModel/Design/Design.h"
#include "DataModel/DFG/DFGGraph.h"
#include "DataModel/DFG/MemoryMgr.h"
#include "DataModel/Design/HInst.h"
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

bool Design::AttachDFG(DFGGraph* graph) {
    if (graph == nullptr || graph->GetGraphType() != DFGGraphType::Module) {
        return false;
    }
    if (m_dfg_graph_id != kInvalidId && m_dfg_graph_id != graph->GetObjectId()) {
        return false;
    }
    if (graph->GetDesignId() != kInvalidId && graph->GetDesignId() != GetObjectId()) {
        return false;
    }
    m_dfg_graph_id       = graph->GetObjectId();
    graph->m_design_id   = GetObjectId();
    return true;
}

DFGGraph* Design::GetDFG() const {
    if (m_dfg_graph_id == kInvalidId) {
        return nullptr;
    }
    return DFGMemoryMgr::GetGraph(m_dfg_graph_id);
}

void    Design::SetCurrentDesign(ObjectId designId) { m_current_design_id = designId; }
Design* Design::GetCurrentDesign() {
    if (m_current_design_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetDesign(m_current_design_id);
}

HInst* Design::CreateHInst(HInst* parent, Module* template_module, std::string_view name) {
    if (template_module == nullptr || GetStringMgr() == nullptr) {
        return nullptr;
    }

    const ObjectId top_module_id =
        parent != nullptr ? parent->m_module_id : (GetTopModule() == nullptr
                                                       ? kInvalidId
                                                       : GetTopModule()->GetObjectId());
    if (top_module_id == kInvalidId) {
        return nullptr;
    }

    HInst* hinst = DesignMemoryMgr::CreateHInst();
    if (hinst == nullptr) {
        return nullptr;
    }
    hinst->m_design_id      = GetObjectId();
    hinst->m_module_id      = top_module_id;
    hinst->m_template_id    = template_module->GetObjectId();
    hinst->m_upper_hinst_id = parent != nullptr ? parent->GetObjectId() : kInvalidId;
    hinst->m_name_id        = GetStringMgr()->AddString(name);

    if (parent != nullptr) {
        hinst->m_next_hinst_id        = parent->m_child_hinst_head_id;
        parent->m_child_hinst_head_id = hinst->GetObjectId();
    } else {
        Module* top_module = GetTopModule();
        if (top_module == nullptr) {
            DesignMemoryMgr::DestroyHInst(hinst->GetObjectId());
            return nullptr;
        }
        hinst->m_next_hinst_id     = top_module->m_hinst_head_id;
        top_module->m_hinst_head_id = hinst->GetObjectId();
    }
    return hinst;
}

Module* Design::CreateModule(const std::string& name) {
    Module* module = DesignMemoryMgr::CreateModule();
    if (module == nullptr) {
        return nullptr;
    }
    module->m_design_id = GetObjectId();
    if (m_top_module_id == kInvalidId) {
        m_top_module_id = module->GetObjectId();
    }
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
    if (m_top_module_id == kInvalidId) {
        m_top_module_id = module->GetObjectId();
    }
    if (DesignStringMgr* mgr = GetStringMgr(); mgr != nullptr) {
        module->m_name_id = mgr->AddString(name);
    }
    return module;
}

}  // namespace db
