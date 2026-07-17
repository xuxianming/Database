#include "DataModel/Design/Net.h"
#include "DataModel/Design/Pin.h"

namespace db {
size_t Net::GetSize() const { return sizeof(Net); }

void Net::ConnectPin(Pin* pin) {
    if (pin == nullptr) {
        return;
    }

    if (pin->m_net_id != kInvalidId) {
        Net* old_net = DesignMemoryMgr::GetNet(pin->m_net_id);
        old_net->DisConnectPin(pin);
    }
    pin->m_net_id      = GetObjectId();
    pin->m_next_pin_id = pin_list_id;
    pin->m_prev_pin_id = kInvalidId;

    if (pin_list_id != kInvalidId) {
        Pin* old_head = DesignMemoryMgr::GetPin(pin_list_id);
        if (old_head) {
            old_head->m_prev_pin_id = pin->GetObjectId();
        }
    }
    pin_list_id = pin->GetObjectId();
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
    if (pin_list_id == pin->GetObjectId()) {
        pin_list_id = pin->m_next_pin_id;
    }
    pin->m_net_id      = kInvalidId;
    pin->m_prev_pin_id = kInvalidId;
    pin->m_next_pin_id = kInvalidId;
}

}  // namespace db
