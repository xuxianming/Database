#include "DataModel/Design/Net.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Design/Pin.h"

namespace db {
size_t Net::GetSize() const { return sizeof(Net); }

Design* Net::GetDesign() const {
    if (m_design_id != kInvalidId) {
        if (Design* design = DesignMemoryMgr::GetDesign(m_design_id); design != nullptr) {
            return design;
        }
    }
    if (Module* top_module = GetModule(); top_module != nullptr) {
        return top_module->GetDesign();
    }
    if (HInst* hinst = GetHInst(); hinst != nullptr) {
        return hinst->GetDesign();
    }
    return nullptr;
}

Module* Net::GetModule() const {
    if (m_top_module_id != kInvalidId) {
        if (Module* top_module = DesignMemoryMgr::GetModule(m_top_module_id);
            top_module != nullptr) {
            return top_module;
        }
    }
    if (HInst* hinst = GetHInst(); hinst != nullptr) {
        return hinst->GetModule();
    }
    return nullptr;
}

HInst* Net::GetHInst() const {
    if (m_hinst_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHInst(m_hinst_id);
}

void Net::ConnectPin(Pin* pin) {
    if (pin == nullptr) {
        return;
    }

    if (pin->m_net_id != kInvalidId) {
        Net* old_net = DesignMemoryMgr::GetNet(pin->m_net_id);
        old_net->DisConnectPin(pin);
    }
    pin->m_net_id      = GetObjectId();
    pin->m_next_pin_id = m_pin_head_id;
    pin->m_prev_pin_id = kInvalidId;

    if (m_pin_head_id != kInvalidId) {
        Pin* old_head = DesignMemoryMgr::GetPin(m_pin_head_id);
        if (old_head) {
            old_head->m_prev_pin_id = pin->GetObjectId();
        }
    }
    m_pin_head_id = pin->GetObjectId();
}

void Net::DisConnectPin(Pin* pin) {
    if (pin == nullptr) {
        return;
    }

    if (pin->m_net_id != GetObjectId()) {
        return;
    }

    Pin* prev =
        (pin->m_prev_pin_id != kInvalidId) ? DesignMemoryMgr::GetPin(pin->m_prev_pin_id) : nullptr;
    Pin* next =
        (pin->m_next_pin_id != kInvalidId) ? DesignMemoryMgr::GetPin(pin->m_next_pin_id) : nullptr;

    if (prev != nullptr) {
        prev->m_next_pin_id = pin->m_next_pin_id;
    }
    if (next != nullptr) {
        next->m_prev_pin_id = pin->m_prev_pin_id;
    }
    if (m_pin_head_id == pin->GetObjectId()) {
        m_pin_head_id = pin->m_next_pin_id;
    }
    pin->m_net_id      = kInvalidId;
    pin->m_prev_pin_id = kInvalidId;
    pin->m_next_pin_id = kInvalidId;
}

}  // namespace db
