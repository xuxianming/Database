#include "DataModel/Design/HPin.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/HNet.h"
#include "DataModel/Design/Module.h"

namespace db {
size_t HPin::GetSize() const { return size_t(); }

ModulePort* HPin::GetModulePort() const {
    if (m_port_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModulePort(m_port_id);
}

Module* HPin::GetModule() const {
    HInst* hinst = GetHInst();
    if (hinst == nullptr) {
        return nullptr;
    }
    return hinst->GetModule();
}

Design* HPin::GetMasterDesign() const {
    Module* module = GetModule();
    if (module != nullptr) {
        return module->GetMasterDesign();
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
    hnet->ConectHPin(this);
}

void HPin::ConnectLowerHNet(HNet* hnet) {
    if (hnet == nullptr) {
        return;
    }
    hnet->ConectHPin(this);
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
