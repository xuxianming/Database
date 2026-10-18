#include "DataModel/Design/HNet.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/HPin.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Design/Net.h"
#include "DataModel/Design/Pin.h"

namespace db {
size_t HNet::GetSize() const { return size_t(); }
Net*   HNet::GetNet() const {
    if (m_net_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetNet(m_net_id);
}

HInst* HNet::GetHInst() const {
    if (m_hinst_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHInst(m_hinst_id);
}

Module* HNet::GetModule() const {
    if (m_module_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModule(m_module_id);
}

Design* HNet::GetMasterDesign() const {
    Module* module = GetModule();
    if (module != nullptr) {
        return module->GetMasterDesign();
    }
    return nullptr;
}

void HNet::AttachToNet(Net* net) {
    if (net == nullptr) {
        return;
    }
    AttachToNet(net->GetObjectId());
}
void HNet::AttachToNet(ObjectId net_id) {
    if (net_id == m_net_id) {
        return;
    }
    m_net_id = net_id;
}
void HNet::ConnectPin(Pin* pin) {
    if (pin == nullptr) {
        return;
    }
    if (pin->m_hnet_id != kInvalidId) {
        if (HNet* old = DesignMemoryMgr::GetHNet(pin->m_hnet_id); old != nullptr) {
            old->DisConnectPin(pin);
        }
    }
    pin->m_hnet_id           = GetObjectId();
    pin->m_hnet_next_pin_id  = m_pin_head_id;
    pin->m_hnet_prev_pin_id  = kInvalidId;
    if (m_pin_head_id != kInvalidId) {
        if (Pin* old_head = DesignMemoryMgr::GetPin(m_pin_head_id); old_head != nullptr) {
            old_head->m_hnet_prev_pin_id = pin->GetObjectId();
        }
    }
    m_pin_head_id = pin->GetObjectId();
}

void HNet::DisConnectPin(Pin* pin) {
    if (pin == nullptr || pin->m_hnet_id != GetObjectId()) {
        return;
    }
    Pin* prev = pin->m_hnet_prev_pin_id != kInvalidId
                    ? DesignMemoryMgr::GetPin(pin->m_hnet_prev_pin_id)
                    : nullptr;
    Pin* next = pin->m_hnet_next_pin_id != kInvalidId
                    ? DesignMemoryMgr::GetPin(pin->m_hnet_next_pin_id)
                    : nullptr;
    if (prev != nullptr) {
        prev->m_hnet_next_pin_id = pin->m_hnet_next_pin_id;
    }
    if (next != nullptr) {
        next->m_hnet_prev_pin_id = pin->m_hnet_prev_pin_id;
    }
    if (m_pin_head_id == pin->GetObjectId()) {
        m_pin_head_id = pin->m_hnet_next_pin_id;
    }
    pin->m_hnet_id          = kInvalidId;
    pin->m_hnet_prev_pin_id = kInvalidId;
    pin->m_hnet_next_pin_id = kInvalidId;
}

// HNet's current API does not distinguish upper/lower HPin connections; keep
// the intrusive list and leave direction assignment to the caller.
void HNet::ConectHPin(HPin* hpin) {
    if (hpin == nullptr) {
        return;
    }
    hpin->m_next_hpin_id = m_hpin_head_id;
    m_hpin_head_id       = hpin->GetObjectId();
}

void HNet::DisConnectHPin(HPin* hpin) {
    if (hpin == nullptr) {
        return;
    }
    if (m_hpin_head_id == hpin->GetObjectId()) {
        m_hpin_head_id = hpin->m_next_hpin_id;
    } else {
        ObjectId cur = m_hpin_head_id;
        while (cur != kInvalidId) {
            HPin* h = DesignMemoryMgr::GetHPin(cur);
            if (h == nullptr) {
                break;
            }
            if (h->m_next_hpin_id == hpin->GetObjectId()) {
                h->m_next_hpin_id = hpin->m_next_hpin_id;
                break;
            }
            cur = h->m_next_hpin_id;
        }
    }
    hpin->m_next_hpin_id = kInvalidId;
}

}  // namespace db
