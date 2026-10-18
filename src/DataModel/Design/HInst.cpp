#include "DataModel/Design/HInst.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HNet.h"
#include "DataModel/Design/HPin.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/Module.h"

namespace db {

size_t HInst::GetSize() const { return sizeof(HInst); }

std::string_view HInst::GetName() const {
    Design* design = GetDesign();
    if (design == nullptr || design->GetStringMgr() == nullptr) {
        return {};
    }
    return design->GetStringMgr()->GetString(m_name_id);
}

// Synthesizes the hierarchical path on demand; flat paths are not interned.
std::string HInst::PathName() {
    std::string base;
    if (HInst* upper = GetHInst(); upper != nullptr) {
        base = upper->PathName();
    } else if (Module* top_module = GetModule(); top_module != nullptr) {
        base = std::string(top_module->GetName());
    }

    const std::string name(GetName());
    if (base.empty()) {
        return name;
    }
    if (name.empty()) {
        return base;
    }
    return base + "/" + name;
}

bool HInst::Rename(std::string_view name) {
    Module* top_module = GetModule();
    Design* design     = top_module == nullptr ? nullptr : top_module->GetDesign();
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
    return true;
}

Design* HInst::GetDesign() const {
    if (m_design_id != kInvalidId) {
        if (Design* design = DesignMemoryMgr::GetDesign(m_design_id); design != nullptr) {
            return design;
        }
    }
    if (Module* module = GetModule(); module != nullptr) {
        return module->GetDesign();
    }
    return nullptr;
}

Module* HInst::GetModule() const {
    if (m_module_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModule(m_module_id);
}

HInst* HInst::GetHInst() const {
    if (m_upper_hinst_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHInst(m_upper_hinst_id);
}

Module* HInst::GetTemplateModule() const {
    if (m_template_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModule(m_template_id);
}

HInstHInstIterator::HInstHInstIterator(HInst* hinst) {
    if (hinst == nullptr) {
        return;
    }
    m_next_hinst_id = hinst->m_child_hinst_head_id;
}

HInst* HInstHInstIterator::Next() {
    if (m_next_hinst_id == kInvalidId) {
        return nullptr;
    }
    HInst* hinst = DesignMemoryMgr::GetHInst(m_next_hinst_id);
    if (hinst == nullptr) {
        m_next_hinst_id = kInvalidId;
        return nullptr;
    }
    m_next_hinst_id = hinst->m_next_hinst_id;
    return hinst;
}

HInstInstIterator::HInstInstIterator(HInst* hinst) {
    if (hinst == nullptr) {
        return;
    }
    m_next_inst_id = hinst->m_inst_head_id;
}

Inst* HInstInstIterator::Next() {
    if (m_next_inst_id == kInvalidId) {
        return nullptr;
    }
    Inst* inst = DesignMemoryMgr::GetInst(m_next_inst_id);
    if (inst == nullptr) {
        return nullptr;
    }
    m_next_inst_id = inst->m_next_inst_id;
    return inst;
}

HInstHPinIterator::HInstHPinIterator(HInst* hinst) {
    if (hinst == nullptr) {
        return;
    }
    m_hinst        = hinst;
    m_next_hpin_id = hinst->m_hpin_head_id;
}

HPin* HInstHPinIterator::Next() {
    if (m_next_hpin_id == kInvalidId) {
        return nullptr;
    }
    HPin* hpin = DesignMemoryMgr::GetHPin(m_next_hpin_id);
    if (hpin == nullptr) {
        return nullptr;
    }
    m_next_hpin_id = hpin->m_next_hpin_id;
    return hpin;
}

HInstHNetIterator::HInstHNetIterator(HInst* hinst) {
    if (hinst == nullptr) {
        return;
    }
    m_next_hnet_id = hinst->m_hnet_head_id;
}

HNet* HInstHNetIterator::Next() {
    if (m_next_hnet_id == kInvalidId) {
        return nullptr;
    }
    HNet* hnet = DesignMemoryMgr::GetHNet(m_next_hnet_id);
    if (hnet == nullptr) {
        return nullptr;
    }
    m_next_hnet_id = hnet->m_next_hnet_id;
    return hnet;
}

}  // namespace db
