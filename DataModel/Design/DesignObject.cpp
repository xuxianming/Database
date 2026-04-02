#include "DataModel/Design/DesignObject.h"
#include "DataModel/Design/BusNet.h"
#include "DataModel/Design/BusPort.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/HNet.h"
#include "DataModel/Design/HPin.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Net.h"
#include "DataModel/Design/Pin.h"
#include "DataModel/Design/Port.h"
#include "Utils/Log.h"
namespace db {

DesignObject* db::DesignObject::GetDesignObject(FullObjectId full_id) {
    DMObjectType type = (DMObjectType)(full_id >> TypeOffset);
    FullObjectId id   = full_id & IdMask;
    switch (type) {
        case DMObjectType::DESIGN: {
            return DesignMemoryMgr::GetDesign(id);
            break;
        }
        case DMObjectType::INST:
            return DesignMemoryMgr::GetInst(id);
            break;
        case DMObjectType::HINST:
            return DesignMemoryMgr::GetHInst(id);
            break;
        case DMObjectType::NET:
            return DesignMemoryMgr::GetNet(id);
            break;
        case DMObjectType::HNET:
            return DesignMemoryMgr::GetHNet(id);
            break;
        case DMObjectType::PIN:
            return DesignMemoryMgr::GetPin(id);
            break;
        case DMObjectType::HPIN:
            return DesignMemoryMgr::GetHPin(id);
            break;
        case DMObjectType::PORT:
            return DesignMemoryMgr::GetPort(id);
            break;
        case DMObjectType::BUS_PORT:
            return DesignMemoryMgr::GetBusPort(id);
        case DMObjectType::BUS_NET:
            return DesignMemoryMgr::GetBusNet(id);
            break;
        default:
            DB_LOG(WARNING)
                << "Invalid type of DesignObject : " << ToString(type);
            return nullptr;
    }
}

Design* DesignObject::GetDesign(ObjectId design_id) {
    return DesignMemoryMgr::GetDesign(design_id);
}
Inst* DesignObject::GetInst(ObjectId inst_id) {
    return DesignMemoryMgr::GetInst(inst_id);
}
HInst* DesignObject::GetHInst(ObjectId hinst_id) {
    return DesignMemoryMgr::GetHInst(hinst_id);
}
Net* DesignObject::GetNet(ObjectId net_id) {
    return DesignMemoryMgr::GetNet(net_id);
}
HNet* DesignObject::GetHNet(ObjectId hnet_id) {
    return DesignMemoryMgr::GetHNet(hnet_id);
}
Pin* DesignObject::GetPin(ObjectId pin_id) {
    return DesignMemoryMgr::GetPin(pin_id);
}
HPin* DesignObject::GetHPin(ObjectId hpin_id) {
    return DesignMemoryMgr::GetHPin(hpin_id);
}
Port* DesignObject::GetPort(ObjectId port_id) {
    return DesignMemoryMgr::GetPort(port_id);
}
BusPort* DesignObject::GetBusPort(ObjectId bus_port_id) {
    return DesignMemoryMgr::GetBusPort(bus_port_id);
}
BusNet* DesignObject::GetBusNet(ObjectId bus_net_id) {
    return DesignMemoryMgr::GetBusNet(bus_net_id);
}
}  // namespace db