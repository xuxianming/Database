
#include "DataModel/Design/Pin.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/HNet.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/ModulePort.h"
#include "DataModel/Design/Net.h"
#include "DataModel/Liberty/LibPort.h"

namespace db {

size_t Pin::GetSize() const { return sizeof(Pin); }

ModulePort* Pin::GetModulePort() const {
    if (m_port_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModulePort(m_port_id);
}

LibPort* Pin::GetLibPort() const {
    if (m_port_id == kInvalidId) {
        return nullptr;
    }
    return LibertyMemoryMgr::GetLibPort(m_port_id);
}

Net* Pin::GetNet() const {
    if (m_net_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetNet(m_net_id);
}

HNet* Pin::GetHNet() const {
    if (m_hnet_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHNet(m_hnet_id);
}

Inst* Pin::GetInst() const {
    if (m_inst_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetInst(m_inst_id);
}

void Pin::ConnectNet(Net* net) {
    if (net == nullptr) {
        return;
    }
    net->ConnectPin(this);
}

void Pin::DisConnectNet() {
    Net* net = GetNet();
    if (net == nullptr) {
        return;
    }
    net->DisConnectPin(this);
}

void Pin::ConnectHNet(HNet* hnet) {
    if (hnet == nullptr) {
        return;
    }
    hnet->ConnectPin(this);
}

void Pin::DisConnectHNet() {
    HNet* hnet = GetHNet();
    if (hnet == nullptr) {
        return;
    }
    hnet->DisConnectPin(this);
}

Pin* Pin::Create(Inst* inst, ObjectId port_id) {
    if (inst == nullptr || port_id == kInvalidId) {
        return nullptr;
    }
    Pin* pin = DesignMemoryMgr::CreatePin();
    if (pin == nullptr) {
        return nullptr;
    }
    pin->m_inst_id = inst->GetObjectId();
    pin->m_port_id = port_id;
    return pin;
}

Pin* Pin::Create(Inst* inst, LibPort* port) {
    if (inst == nullptr || port == nullptr) {
        return nullptr;
    }
    Pin* pin = DesignMemoryMgr::CreatePin();
    if (pin == nullptr) {
        return nullptr;
    }
    pin->m_inst_id = inst->GetObjectId();
    pin->m_port_id = port->GetObjectId();
    return pin;
}

Pin* Pin::Create(ObjectId inst_id, ObjectId port_id) {
    if (inst_id == kInvalidId || port_id == kInvalidId) {
        return nullptr;
    }
    Pin* pin = DesignMemoryMgr::CreatePin();
    if (pin == nullptr) {
        return nullptr;
    }
    pin->m_inst_id = inst_id;
    pin->m_port_id = port_id;
    return pin;
}

Pin* Pin::Create(ObjectId inst_id, LibPort* port) {
    if (inst_id == kInvalidId || port == nullptr) {
        return nullptr;
    }
    Pin* pin = DesignMemoryMgr::CreatePin();
    if (pin == nullptr) {
        return nullptr;
    }
    pin->m_inst_id = inst_id;
    pin->m_port_id = port->GetObjectId();
    return pin;
}

}  // namespace db