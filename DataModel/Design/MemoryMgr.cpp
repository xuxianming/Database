#include "DataModel/Design/MemoryMgr.h"
#include <cstdint>
#include "BasePlatfrom/ObjectTable.h"
#include "DataModel/BusPort.h"
#include "DataModel/Design/BusNet.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Design/Net.h"
#include "DataModel/Design/Pin.h"
#include "DataModel/Port.h"
#include "DataModel/Type.h"
#include "MemoryMgr.h"
#include "Utils/Log.h"

namespace db {
ObjectTable<Design, ObjectId, ObjectIndexBits> design_table(
    static_cast<uint8_t>(DMObjectType::DESIGN));
ObjectTable<Module, ObjectId, ObjectIndexBits> module_table(
    static_cast<uint8_t>(DMObjectType::MODULE));
ObjectTable<Inst, ObjectId, ObjectIndexBits>  inst_table(static_cast<uint8_t>(DMObjectType::INST));
ObjectTable<HInst, ObjectId, ObjectIndexBits> hinst_table(
    static_cast<uint8_t>(DMObjectType::HINST));
ObjectTable<Net, ObjectId, ObjectIndexBits>  net_table(static_cast<uint8_t>(DMObjectType::NET));
ObjectTable<Pin, ObjectId, ObjectIndexBits>  pin_table(static_cast<uint8_t>(DMObjectType::PIN));
ObjectTable<Port, ObjectId, ObjectIndexBits> port_table(static_cast<uint8_t>(DMObjectType::PORT));
ObjectTable<BusPort, ObjectId, ObjectIndexBits> bus_port_table(
    static_cast<uint8_t>(DMObjectType::BUS_PORT));
ObjectTable<BusNet, ObjectId, ObjectIndexBits> bus_net_table(
    static_cast<uint8_t>(DMObjectType::BUS_NET));
std::string db_path = ".";

ObjectId DesignMemoryMgr::current_design_id = kInvalidId;

Design* DesignMemoryMgr::CreateDesign() {
    auto  id     = design_table.GetNextObjId();
    auto* design = design_table.Make();
    design->SetObjectId(id);
    return design;
}
Module* DesignMemoryMgr::CreateModule() {
    auto  id     = module_table.GetNextObjId();
    auto* module = module_table.Make();
    module->SetObjectId(id);
    return module;
}
Inst* DesignMemoryMgr::CreateInst() {
    auto  id   = inst_table.GetNextObjId();
    auto* inst = inst_table.Make();
    inst->SetObjectId(id);
    return inst;
}

HInst* DesignMemoryMgr::CreateHInst() {
    auto  id    = hinst_table.GetNextObjId();
    auto* hinst = hinst_table.Make();
    hinst->SetObjectId(id);
    return hinst;
}

Net* DesignMemoryMgr::CreateNet() {
    auto  id  = net_table.GetNextObjId();
    auto* net = net_table.Make();
    net->SetObjectId(id);
    return net;
}

Pin* DesignMemoryMgr::CreatePin() {
    auto  id  = pin_table.GetNextObjId();
    auto* pin = pin_table.Make();
    pin->SetObjectId(id);
    return pin;
}

Port* DesignMemoryMgr::CreatePort() {
    auto  id   = port_table.GetNextObjId();
    auto* port = port_table.Make();
    port->SetObjectId(id);
    return port;
}

BusPort* DesignMemoryMgr::CreateBusPort() {
    auto  id      = bus_port_table.GetNextObjId();
    auto* busport = bus_port_table.Make();
    busport->SetObjectId(id);
    return busport;
}

BusNet* DesignMemoryMgr::CreateBusNet() {
    auto  id     = bus_net_table.GetNextObjId();
    auto* busnet = bus_net_table.Make();
    busnet->SetObjectId(id);
    return busnet;
}

Design*  DesignMemoryMgr::GetDesign(ObjectId id) { return design_table.Pointer(id); }
Module*  DesignMemoryMgr::GetModule(ObjectId id) { return module_table.Pointer(id); }
Inst*    DesignMemoryMgr::GetInst(ObjectId id) { return inst_table.Pointer(id); }
HInst*   DesignMemoryMgr::GetHInst(ObjectId id) { return hinst_table.Pointer(id); }
Net*     DesignMemoryMgr::GetNet(ObjectId id) { return net_table.Pointer(id); }
Pin*     DesignMemoryMgr::GetPin(ObjectId id) { return pin_table.Pointer(id); }
Port*    DesignMemoryMgr::GetPort(ObjectId id) { return port_table.Pointer(id); }
BusPort* DesignMemoryMgr::GetBusPort(ObjectId id) { return bus_port_table.Pointer(id); }
BusNet*  DesignMemoryMgr::GetBusNet(ObjectId id) { return bus_net_table.Pointer(id); }

void DesignMemoryMgr::DestroyDesign(ObjectId id) { design_table.Destroy(id); }
void DesignMemoryMgr::DestroyModule(ObjectId id) { module_table.Destroy(id); }
void DesignMemoryMgr::DestroyInst(ObjectId id) { inst_table.Destroy(id); }
void DesignMemoryMgr::DestroyHInst(ObjectId id) { hinst_table.Destroy(id); }
void DesignMemoryMgr::DestroyNet(ObjectId id) { net_table.Destroy(id); }
void DesignMemoryMgr::DestroyPin(ObjectId id) { pin_table.Destroy(id); }
void DesignMemoryMgr::DestroyPort(ObjectId id) { port_table.Destroy(id); }
void DesignMemoryMgr::DestroyBusPort(ObjectId id) { bus_port_table.Destroy(id); }
void DesignMemoryMgr::DestroyBusNet(ObjectId id) { bus_net_table.Destroy(id); }

void DesignMemoryMgr::SetCurrentDesign(ObjectId designId) {
    Design* design = GetDesign(designId);
    if (design == nullptr) {
        DB_LOG(ERROR) << "invdid  design full_id " << designId << '\n';
        return;
    }
    current_design_id = designId;
}

Design* DesignMemoryMgr::GetCurrentDesign() { return GetDesign(current_design_id); }

bool DesignMemoryMgr::SaveAll() {
    bool success = true;
    success &= design_table.Save();
    success &= inst_table.Save();
    success &= hinst_table.Save();
    success &= net_table.Save();
    success &= pin_table.Save();
    success &= port_table.Save();
    success &= bus_port_table.Save();
    success &= bus_net_table.Save();

    // 保存元数据文件
    std::string   meta_file = db_path + "/metadata.bin";
    std::ofstream file(meta_file, std::ios::binary);
    if (file.is_open()) {
        ObjectId magic = 0x4D455441;  // "META"
        file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
        file.write(reinterpret_cast<const char*>(&current_design_id), sizeof(current_design_id));
        file.close();
    }
    return success;
}

bool DesignMemoryMgr::LoadAll(const std::string& db_pathdir) {
    bool success = true;
    success &= design_table.Load();
    success &= inst_table.Load();
    success &= hinst_table.Load();
    success &= net_table.Load();
    success &= pin_table.Load();
    success &= port_table.Load();
    success &= bus_port_table.Load();
    success &= bus_net_table.Load();

    std::string   meta_file = db_pathdir + "/metadata.bin";
    std::ifstream file(meta_file, std::ios::binary);
    if (file.is_open()) {
        ObjectId magic;
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        if (magic == 0x4D455441) {
            file.read(reinterpret_cast<char*>(&current_design_id), sizeof(current_design_id));
        }
        file.close();
    }
    return true;
}

void DesignMemoryMgr::PrintStats() {}
void DesignMemoryMgr::Clear() {}
}  // namespace db