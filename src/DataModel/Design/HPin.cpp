#include "DataModel/Design/HPin.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/HNet.h"
#include "DataModel/Design/ModulePort.h"

namespace db {
size_t HPin::GetSize() const { return sizeof(HPin); }

ModulePort* HPin::GetModulePort() const {
    if (m_port_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModulePort(m_port_id);
}

Design* HPin::GetDesign() const {
    if (m_design_id != kInvalidId) {
        if (Design* design = DesignMemoryMgr::GetDesign(m_design_id); design != nullptr) {
            return design;
        }
    }
    if (HInst* hinst = GetHInst(); hinst != nullptr) {
        return hinst->GetDesign();
    }
    return nullptr;
}

HInst* HPin::GetHInst() const {
    if (m_hinst_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHInst(m_hinst_id);
}

HNet* HPin::GetUpperHNet() const {
    if (m_upper_hnet_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHNet(m_upper_hnet_id);
}

HNet* HPin::GetLowerHNet() const {
    if (m_lower_hnet_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHNet(m_lower_hnet_id);
}

Net* HPin::GetNet() const {
    HNet* upper_hnet = GetUpperHNet();
    if (upper_hnet != nullptr) {
        return upper_hnet->GetNet();
    }
    HNet* lower_hnet = GetLowerHNet();
    if (lower_hnet != nullptr) {
        return lower_hnet->GetNet();
    }
    return nullptr;
}

void HPin::ConnectUpperHNet(HNet* hnet) {
    if (hnet == nullptr) {
        return;
    }
    if (m_upper_hnet_id != kInvalidId && m_upper_hnet_id != hnet->GetObjectId()) {
        if (HNet* old = DesignMemoryMgr::GetHNet(m_upper_hnet_id); old != nullptr) {
            old->DisConnectHPin(this);
        }
    }
    m_upper_hnet_id = hnet->GetObjectId();
    hnet->ConnectHPin(this);
}

void HPin::ConnectLowerHNet(HNet* hnet) {
    if (hnet == nullptr) {
        return;
    }
    if (m_lower_hnet_id != kInvalidId && m_lower_hnet_id != hnet->GetObjectId()) {
        if (HNet* old = DesignMemoryMgr::GetHNet(m_lower_hnet_id); old != nullptr) {
            old->DisConnectHPin(this);
        }
    }
    m_lower_hnet_id = hnet->GetObjectId();
    hnet->ConnectHPin(this);
}
void HPin::DisConnectUpperHNet() {
    if (m_upper_hnet_id == kInvalidId) {
        return;
    }
    HNet* hnet = DesignMemoryMgr::GetHNet(m_upper_hnet_id);
    if (hnet != nullptr) {
        hnet->DisConnectHPin(this);
    }
    m_upper_hnet_id = kInvalidId;
}
void HPin::DisConnectLowerHNet() {
    if (m_lower_hnet_id == kInvalidId) {
        return;
    }
    HNet* hnet = DesignMemoryMgr::GetHNet(m_lower_hnet_id);
    if (hnet != nullptr) {
        hnet->DisConnectHPin(this);
    }
    m_lower_hnet_id = kInvalidId;
}
}  // namespace db
