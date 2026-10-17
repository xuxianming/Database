#include "DataModel/Design/Module.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/HNet.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/ModulePort.h"
#include "DataModel/Design/Net.h"
#include "Module.h"

#include <unordered_map>

namespace db {

namespace {

// Side index: module id -> lazily built name maps. A separate static map keeps
// Module's raw ObjectTable layout unchanged.
struct ModuleNameIndex {
    bool                                       built = false;
    std::unordered_map<ObjectNameId, ObjectId> insts;
    std::unordered_map<ObjectNameId, ObjectId> hinsts;
};

std::unordered_map<ObjectId, ModuleNameIndex> g_module_name_index;

void BuildNameIndex(Module* module, ModuleNameIndex& index) {
    index.insts.clear();
    index.hinsts.clear();
    if (module != nullptr) {
        ModuleInstIterator inst_iter(module);
        while (inst_iter.HasNext()) {
            if (Inst* inst = inst_iter.Next(); inst != nullptr) {
                // List is head-first: duplicates keep the most recent object.
                index.insts.emplace(inst->GetNameId(), inst->GetObjectId());
            }
        }
        ModuleHInstIterator hinst_iter(module);
        while (hinst_iter.HasNext()) {
            if (HInst* hinst = hinst_iter.Next(); hinst != nullptr) {
                index.hinsts.emplace(hinst->GetNameId(), hinst->GetObjectId());
            }
        }
    }
    index.built = true;
}

}  // namespace

void InvalidateModuleNameIndex(ObjectId module_id) { g_module_name_index.erase(module_id); }

void ClearModuleNameIndexes() { g_module_name_index.clear(); }

Module::Module() {}
Module::~Module() {}
Design* Module::GetMasterDesign() const {
    if (m_design_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetDesign(m_design_id);
}

Module* Module::GetTopModule() const {
    if (m_is_top == 1) {
        return const_cast<Module*>(this);
    }
    Design* design = GetMasterDesign();
    if (design == nullptr) {
        return nullptr;
    }
    return design->GetTopModule();
}

HInst* Module::GetViewHInst() const {
    if (m_is_top == 0) {
        return nullptr;
    }
    if (m_view_hinst_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHInst(m_view_hinst_id);
}

uint32_t Module::GetViewHInstCount() const { return 0; }
void     Module::Unfold(bool copy_module) {
    if (m_hinst_head_id == kInvalidId) {
        return;
    }
    auto hinst = DesignMemoryMgr::GetHInst(m_hinst_head_id);
    while (hinst != nullptr) {
        hinst->Unfold();
        if (hinst->m_next_hinst_id == kInvalidId) {
            break;
        }
        hinst = DesignMemoryMgr::GetHInst(hinst->m_next_hinst_id);
    }
}

Inst* Module::CreateInst(LibCell* lib_cell, const std::string& name) {
    if (lib_cell == nullptr) {
        return nullptr;
    }
    Design* design = GetMasterDesign();
    if (design == nullptr || design->GetIdArrayMgr() == nullptr ||
        design->GetStringMgr() == nullptr) {
        return nullptr;
    }
    Inst* inst = Inst::Create(this, lib_cell);
    if (inst == nullptr) {
        return nullptr;
    }
    inst->m_module_id = GetObjectId();
    inst->m_name_id   = design->GetStringMgr()->AddString(name);
    LinkInst(inst);
    InvalidateModuleNameIndex(GetObjectId());
    return inst;
}

Inst* Module::CreateInst(ObjectId lib_cell_id, const std::string& name) {
    if (lib_cell_id == kInvalidId) {
        return nullptr;
    }
    Design* design = GetMasterDesign();
    if (design == nullptr || design->GetIdArrayMgr() == nullptr ||
        design->GetStringMgr() == nullptr) {
        return nullptr;
    }
    Inst* inst = Inst::Create(this, lib_cell_id);
    if (inst == nullptr) {
        return nullptr;
    }
    inst->m_module_id = GetObjectId();
    inst->m_name_id   = design->GetStringMgr()->AddString(name);
    LinkInst(inst);
    InvalidateModuleNameIndex(GetObjectId());
    return inst;
}
Inst* Module::CreateInst(LogicType type, const std::string& name) {
    if (type == LogicType::COUNT) {
        return nullptr;
    }
    Design* design = GetMasterDesign();
    if (design == nullptr || design->GetIdArrayMgr() == nullptr ||
        design->GetStringMgr() == nullptr) {
        return nullptr;
    }
    Inst* inst = Inst::Create(this, type);
    if (inst == nullptr) {
        return nullptr;
    }
    inst->m_module_id = GetObjectId();
    inst->m_name_id   = design->GetStringMgr()->AddString(name);
    LinkInst(inst);
    InvalidateModuleNameIndex(GetObjectId());
    return inst;
}

HInst* Module::CreateHInst(Module* template_module, const std::string& name) {
    if (template_module == nullptr) {
        return nullptr;
    }
    Design* design = GetMasterDesign();
    if (design == nullptr || design->GetStringMgr() == nullptr) {
        return nullptr;
    }
    HInst* hinst = HInst::Create(this, template_module);
    if (hinst == nullptr) {
        return nullptr;
    }
    hinst->m_module_id   = GetObjectId();
    hinst->m_name_id     = design->GetStringMgr()->AddString(name);
    hinst->m_template_id = template_module->GetObjectId();
    LinkHInst(hinst);
    InvalidateModuleNameIndex(GetObjectId());
    return hinst;
}

Module* Module::Clone(const std::string& name) const {
    Module* new_module = DesignMemoryMgr::CreateModule();
    if (new_module == nullptr) {
        return nullptr;
    }
    new_module->m_design_id     = m_design_id;
    new_module->m_template_id   = m_template_id;
    new_module->m_is_top        = m_is_top;
    new_module->m_operator_type = m_operator_type;
    if (Design* design = GetMasterDesign();
        design != nullptr && design->GetStringMgr() != nullptr) {
        new_module->m_name_id = design->GetStringMgr()->AddString(name);
    }
    return new_module;
}

std::string_view Module::GetName() const {
    Design* design = GetMasterDesign();
    if (design == nullptr || design->GetStringMgr() == nullptr) {
        return {};
    }
    return design->GetStringMgr()->GetString(m_name_id);
}

std::string Module::PathName() {
    if (m_view_hinst_id != kInvalidId) {
        if (HInst* hinst = DesignMemoryMgr::GetHInst(m_view_hinst_id); hinst != nullptr) {
            return hinst->PathName();
        }
    }
    return std::string(GetName());
}

bool Module::Rename(std::string_view name) {
    Design* design = GetMasterDesign();
    if (design == nullptr || design->GetStringMgr() == nullptr) {
        return false;
    }
    DesignStringMgr*   mgr    = design->GetStringMgr();
    const ObjectNameId old_id = m_name_id;
    const ObjectNameId new_id = mgr->AddString(name);
    m_name_id                 = new_id;
    if (old_id != 0) {
        mgr->ReleaseString(old_id);
    }
    InvalidateModuleNameIndex(GetObjectId());
    return true;
}

void Module::LinkInst(Inst* inst) {
    if (inst == nullptr) {
        return;
    }
    inst->m_prev_inst_id = kInvalidId;
    inst->m_next_inst_id = m_inst_head_id;
    if (m_inst_head_id != kInvalidId) {
        if (Inst* old_head = DesignMemoryMgr::GetInst(m_inst_head_id); old_head != nullptr) {
            old_head->m_prev_inst_id = inst->GetObjectId();
        }
    }
    m_inst_head_id = inst->GetObjectId();
}

void Module::UnlinkInst(Inst* inst) {
    if (inst == nullptr) {
        return;
    }
    const ObjectId id = inst->GetObjectId();
    if (inst->m_prev_inst_id != kInvalidId) {
        if (Inst* prev = DesignMemoryMgr::GetInst(inst->m_prev_inst_id); prev != nullptr) {
            prev->m_next_inst_id = inst->m_next_inst_id;
        }
    } else if (m_inst_head_id == id) {
        m_inst_head_id = inst->m_next_inst_id;
    }
    if (inst->m_next_inst_id != kInvalidId) {
        if (Inst* next = DesignMemoryMgr::GetInst(inst->m_next_inst_id); next != nullptr) {
            next->m_prev_inst_id = inst->m_prev_inst_id;
        }
    }
    inst->m_prev_inst_id = kInvalidId;
    inst->m_next_inst_id = kInvalidId;
}

void Module::LinkHInst(HInst* hinst) {
    if (hinst == nullptr) {
        return;
    }
    hinst->m_next_hinst_id = m_hinst_head_id;
    m_hinst_head_id        = hinst->GetObjectId();
}

void Module::UnlinkHInst(HInst* hinst) {
    if (hinst == nullptr) {
        return;
    }
    const ObjectId id = hinst->GetObjectId();
    if (m_hinst_head_id == id) {
        m_hinst_head_id = hinst->m_next_hinst_id;
    } else {
        // HInst only has a forward link: walk the list to find the predecessor.
        ObjectId prev_id = m_hinst_head_id;
        while (prev_id != kInvalidId) {
            HInst* prev = DesignMemoryMgr::GetHInst(prev_id);
            if (prev == nullptr) {
                break;
            }
            if (prev->m_next_hinst_id == id) {
                prev->m_next_hinst_id = hinst->m_next_hinst_id;
                break;
            }
            prev_id = prev->m_next_hinst_id;
        }
    }
    hinst->m_next_hinst_id = kInvalidId;
}

Inst* Module::FindInst(std::string_view name) const {
    Design* design = GetMasterDesign();
    if (design == nullptr || design->GetStringMgr() == nullptr) {
        return nullptr;
    }
    const auto name_id = design->GetStringMgr()->Find(name);
    if (!name_id.has_value()) {
        return nullptr;
    }

    auto& index = g_module_name_index[GetObjectId()];
    if (!index.built) {
        BuildNameIndex(const_cast<Module*>(this), index);
    }
    auto it = index.insts.find(*name_id);
    if (it == index.insts.end()) {
        return nullptr;
    }
    return DesignMemoryMgr::GetInst(it->second);
}

HInst* Module::FindHInst(std::string_view name) const {
    Design* design = GetMasterDesign();
    if (design == nullptr || design->GetStringMgr() == nullptr) {
        return nullptr;
    }
    const auto name_id = design->GetStringMgr()->Find(name);
    if (!name_id.has_value()) {
        return nullptr;
    }

    auto& index = g_module_name_index[GetObjectId()];
    if (!index.built) {
        BuildNameIndex(const_cast<Module*>(this), index);
    }
    auto it = index.hinsts.find(*name_id);
    if (it == index.hinsts.end()) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHInst(it->second);
}

ModuleInstIterator::ModuleInstIterator(Module* module) {
    if (module == nullptr) {
        return;
    }
    m_next_inst_id = module->m_inst_head_id;
}

Inst* ModuleInstIterator::Next() {
    Inst* inst = DesignMemoryMgr::GetInst(m_next_inst_id);
    if (inst != nullptr) {
        m_next_inst_id = inst->m_next_inst_id;
        return inst;
    }
    m_next_inst_id = kInvalidId;
    return inst;
}
ModuleHInstIterator::ModuleHInstIterator(Module* module) {
    if (module == nullptr) {
        return;
    }
    m_next_hinst_id = module->m_hinst_head_id;
}
HInst* ModuleHInstIterator::Next() {
    HInst* hinst = DesignMemoryMgr::GetHInst(m_next_hinst_id);
    if (hinst != nullptr) {
        m_next_hinst_id = hinst->m_next_hinst_id;
        return hinst;
    }
    m_next_hinst_id = kInvalidId;
    return hinst;
}
ModuleNetIterator::ModuleNetIterator(Module* module) {
    if (module == nullptr) {
        return;
    }
    m_next_net_id = module->m_net_head_id;
}
Net* ModuleNetIterator::Next() {
    Net* net = DesignMemoryMgr::GetNet(m_next_net_id);
    if (net != nullptr) {
        m_next_net_id = net->m_next_net_id;
        return net;
    }
    m_next_net_id = kInvalidId;
    return net;
}

ModuleHNetIterator::ModuleHNetIterator(Module* module) {
    if (module == nullptr) {
        return;
    }
    m_next_hnet_id = module->m_netbus_head_id;
}

HNet* ModuleHNetIterator::Next() {
    HNet* hnet = DesignMemoryMgr::GetHNet(m_next_hnet_id);
    if (hnet != nullptr) {
        m_next_hnet_id = hnet->m_next_hnet_id;
        return hnet;
    }
    m_next_hnet_id = kInvalidId;
    return hnet;
}

ModulePortIterator::ModulePortIterator(Module* module) {
    if (module == nullptr) {
        return;
    }
    m_next_port_id = module->m_port_head_id;
}

ModulePort* ModulePortIterator::Next() {
    ModulePort* port = DesignMemoryMgr::GetModulePort(m_next_port_id);
    if (port != nullptr) {
        m_next_port_id = port->m_next_port_id;
        return port;
    }
    m_next_port_id = kInvalidId;
    return port;
}
/*
        for (auto *output : outputs) {
            auto *sink_net = cell->FindNet(output);
            if (sink_net == nullptr) {
                continue;
            }
            if (up_net == nullptr) {
                if (up_sink == nullptr) {
                    up_sink = sink_net;
                    continue;
                }
                // merge connection
                auto iter = record_netsmap.find(sink_net);
                if (iter != record_netsmap.end()) {
                    netsmap[iter->second] = up_sink;
                }
                up_sink->AssignWith(sink_net);
                continue;
            }
            //  feed Through or driven by underlying constants
            if (sink_net != nullptr) {
                auto iter = record_netsmap.find(sink_net);
                if (iter != record_netsmap.end()) {
                    netsmap[iter->second] = up_net;
                }
                up_net->AssignWith(sink_net);
            }
        }
*/
}  // namespace db