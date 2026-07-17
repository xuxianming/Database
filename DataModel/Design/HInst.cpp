#include "DataModel/Design/HInst.h"
#include "DataModel/Design/HNet.h"
#include "DataModel/Design/HPin.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Design/ModulePort.h"
#include "DataModel/Design/Net.h"
#include "DataModel/Design/Pin.h"

namespace db {

size_t HInst::GetSize() const { return sizeof(HInst); }

Design* HInst::GetMasterDesign() const {
    Module* module = GetModule();
    if (module != nullptr) {
        return module->GetMasterDesign();
    }
    return nullptr;
}

Module* HInst::GetModule() const {
    if (m_module_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModule(m_module_id);
}

HInst* HInst::GetUpperHInst() const {
    if (m_unfold == 0) {
        // can not get upper hinst if not unfolded, because the template module is shared by all
        // hinsts with same template module
        return nullptr;
    }
    Module* module = GetModule();
    if (module == nullptr) {
        return nullptr;
    }
    return module->GetViewHInst();
}

Module* HInst::GetTemplateModule() const {
    if (m_template_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModule(m_template_id);
}

void HInst::Unfold() {
    if (m_unfold) {
        return;
    }
    Module* template_module = GetTemplateModule();
    if (template_module == nullptr) {
        return;
    }
    if (template_module->GetViewHInstCount() != 1) {
        template_module = template_module->Clone();
        return;
    }
    if (template_module == nullptr) {
        return;
    }
    HierarchyExpansion(template_module);
    m_unfold = 1;
}

void HInst::HierarchyExpansion(Module* template_module) {
    if (template_module == nullptr) {
        return;
    }
    m_template_id = template_module->GetObjectId();
    HInstHPinIterator  hpin_iter(this);
    ModulePortIterator port_iter(template_module);
    while (port_iter.HasNext() && hpin_iter.HasNext()) {
        ModulePort* port = port_iter.Next();
        if (port == nullptr) {
            continue;
        }
        Pin* iopin = port->GetIoPin();
        if (iopin == nullptr) {
            continue;
        }
        HNet* lower_hnet = iopin->GetHNet();
        if (lower_hnet == nullptr) {
            continue;
        }
        HPin* hpin = hpin_iter.Next();
        if (hpin == nullptr) {
            continue;
        }
        hpin->ConnectLowerHNet(lower_hnet);
        Net* net = hpin->GetNet();
        if (net == nullptr) {
            continue;
        }
        lower_hnet->AttachToNet(net);

        hpin->m_next_hpin_id = m_hpin_head_id;
        m_hpin_head_id       = hpin->GetObjectId();
    }
}

HInstHInstIterator::HInstHInstIterator(HInst* hinst) {
    if (hinst == nullptr || hinst->m_unfold == 0) {
        return;
    }
    m_template_module = hinst->GetTemplateModule();
    if (m_template_module == nullptr) {
        return;
    }
    m_next_hinst_id = m_template_module->m_hinst_head_id;
}

HInst* HInstHInstIterator::Next() {
    if (m_next_hinst_id == kInvalidId) {
        return nullptr;
    }
    HInst* hinst = DesignMemoryMgr::GetHInst(m_next_hinst_id);
    if (hinst == nullptr) {
        return nullptr;
    }
    m_next_hinst_id = hinst->m_next_hinst_id;
    return hinst;
}
HInstInstIterator::HInstInstIterator(HInst* hinst) {
    if (hinst == nullptr || hinst->m_unfold == 0) {
        return;
    }
    m_template_module = hinst->GetTemplateModule();
    if (m_template_module == nullptr) {
        return;
    }
    m_next_inst_id = m_template_module->m_inst_head_id;
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
HInstNetIterator::HInstNetIterator(HInst* hinst) {
    if (hinst == nullptr || hinst->m_unfold == 0) {
        return;
    }
    m_template_module = hinst->GetTemplateModule();
    if (m_template_module == nullptr) {
        return;
    }
    m_next_net_id = m_template_module->m_net_head_id;
}
Net* HInstNetIterator::Next() {
    if (m_next_net_id == kInvalidId) {
        return nullptr;
    }
    Net* net = DesignMemoryMgr::GetNet(m_next_net_id);
    if (net == nullptr) {
        return nullptr;
    }
    m_next_net_id = net->m_next_net_id;
    return net;
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
    if (hinst == nullptr || hinst->m_unfold == 0) {
        return;
    }
    m_template_module = hinst->GetTemplateModule();
    if (m_template_module == nullptr) {
        return;
    }
    m_next_hnet_id = m_template_module->m_hnet_head_id;
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
