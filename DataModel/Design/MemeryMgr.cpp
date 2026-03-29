#include "DataModel/Design/MemeryMgr.h"
#include <cstdint>
#include "BasePlatfrom/ObjectTable.h"
#include "DataModel/Design/BusNet.h"
#include "DataModel/Design/BusPort.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/HNet.h"
#include "DataModel/Design/HPin.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/Net.h"
#include "DataModel/Design/Pin.h"
#include "DataModel/Design/Port.h"
#include "DataModel/Type.h"
#include "Utils/Log.h"

namespace db {
ObjectTable<Design>  design_table(static_cast<uint8_t>(DMObjectType::DESIGN));
ObjectTable<Inst>    inst_table(static_cast<uint8_t>(DMObjectType::INST));
ObjectTable<HInst>   hinst_table(static_cast<uint8_t>(DMObjectType::HINST));
ObjectTable<Net>     net_table(static_cast<uint8_t>(DMObjectType::NET));
ObjectTable<HNet>    hnet_table(static_cast<uint8_t>(DMObjectType::HNET));
ObjectTable<Pin>     pin_table(static_cast<uint8_t>(DMObjectType::PIN));
ObjectTable<HPin>    hpin_table(static_cast<uint8_t>(DMObjectType::HPIN));
ObjectTable<Port>    port_table(static_cast<uint8_t>(DMObjectType::PORT));
ObjectTable<BusPort> bus_port_table(
    static_cast<uint8_t>(DMObjectType::BUS_PORT));
ObjectTable<BusNet> bus_net_table(static_cast<uint8_t>(DMObjectType::BUS_NET));
std::string         db_path = ".";

uint64_t DesignMemoryMgr::current_design_id = 0;

Design* DesignMemoryMgr::CreateDesign() {
    Design* design = design_table.Make();
    design->m_obj_id =
        EncodeId<Design>(design_table.ObjectId(design), DMObjectType::DESIGN);
    return design;
}

Inst* DesignMemoryMgr::CreateInst() {
    Inst* inst = inst_table.Make();
    inst->m_obj_id =
        EncodeId<Inst>(inst_table.ObjectId(inst), DMObjectType::INST);
    return inst;
}

HInst* DesignMemoryMgr::CreateHInst() {
    HInst* hinst = hinst_table.Make();
    hinst->m_obj_id =
        EncodeId<HInst>(hinst_table.ObjectId(hinst), DMObjectType::HINST);
    return hinst;
}

Net* DesignMemoryMgr::CreateNet() {
    Net* net      = net_table.Make();
    net->m_obj_id = EncodeId<Net>(net_table.ObjectId(net), DMObjectType::NET);
    return net;
}

HNet* DesignMemoryMgr::CreateHNet() {
    HNet* hnet = hnet_table.Make();
    hnet->m_obj_id =
        EncodeId<HNet>(hnet_table.ObjectId(hnet), DMObjectType::HNET);
    return hnet;
}

Pin* DesignMemoryMgr::CreatePin() {
    Pin* pin      = pin_table.Make();
    pin->m_obj_id = EncodeId<Pin>(pin_table.ObjectId(pin), DMObjectType::PIN);
    return pin;
}

HPin* DesignMemoryMgr::CreateHPin() {
    HPin* hpin = hpin_table.Make();
    hpin->m_obj_id =
        EncodeId<HPin>(hpin_table.ObjectId(hpin), DMObjectType::HPIN);
    return hpin;
}

Port* DesignMemoryMgr::CreatePort() {
    Port* port = port_table.Make();
    port->m_obj_id =
        EncodeId<Port>(port_table.ObjectId(port), DMObjectType::PORT);
    return port;
}

BusPort* DesignMemoryMgr::CreateBusPort() {
    BusPort* bus_port  = bus_port_table.Make();
    bus_port->m_obj_id = EncodeId<BusPort>(bus_port_table.ObjectId(bus_port),
                                           DMObjectType::BUS_PORT);
    return bus_port;
}

BusNet* DesignMemoryMgr::CreateBusNet() {
    BusNet* bus_net   = bus_net_table.Make();
    bus_net->m_obj_id = EncodeId<BusNet>(bus_net_table.ObjectId(bus_net),
                                         DMObjectType::BUS_NET);
    return bus_net;
}

Design* DesignMemoryMgr::GetDesign(uint64_t id) {
    uint64_t real_id = DecodeId<Design>(id);
    return design_table.Pointer(real_id);
}

Inst* DesignMemoryMgr::GetInst(uint64_t id) {
    uint64_t real_id = DecodeId<Inst>(id);
    return inst_table.Pointer(real_id);
}

HInst* DesignMemoryMgr::GetHInst(uint64_t id) {
    uint64_t real_id = DecodeId<HInst>(id);
    return hinst_table.Pointer(real_id);
}

Net* DesignMemoryMgr::GetNet(uint64_t id) {
    uint64_t real_id = DecodeId<Net>(id);
    return net_table.Pointer(real_id);
}

HNet* DesignMemoryMgr::GetHNet(uint64_t id) {
    uint64_t real_id = DecodeId<HNet>(id);
    return hnet_table.Pointer(real_id);
}

Pin* DesignMemoryMgr::GetPin(uint64_t id) {
    uint64_t real_id = DecodeId<Pin>(id);
    return pin_table.Pointer(real_id);
}

HPin* DesignMemoryMgr::GetHPin(uint64_t id) {
    uint64_t real_id = DecodeId<HPin>(id);
    return hpin_table.Pointer(real_id);
}

Port* DesignMemoryMgr::GetPort(uint64_t id) {
    uint64_t real_id = DecodeId<Port>(id);
    return port_table.Pointer(real_id);
}

BusPort* DesignMemoryMgr::GetBusPort(uint64_t id) {
    uint64_t real_id = DecodeId<BusPort>(id);
    return bus_port_table.Pointer(real_id);
}

BusNet* DesignMemoryMgr::GetBusNet(uint64_t id) {
    uint64_t real_id = DecodeId<BusNet>(id);
    return bus_net_table.Pointer(real_id);
}

void DesignMemoryMgr::DestroyDesign(Design* obj) { design_table.Destroy(obj); }

void DesignMemoryMgr::DestroyInst(Inst* obj) { inst_table.Destroy(obj); }
void DesignMemoryMgr::DestroyHInst(HInst* obj) { hinst_table.Destroy(obj); }
void DesignMemoryMgr::DestroyNet(Net* obj) { net_table.Destroy(obj); }
void DesignMemoryMgr::DestroyHNet(HNet* obj) { hnet_table.Destroy(obj); }
void DesignMemoryMgr::DestroyPin(Pin* obj) { pin_table.Destroy(obj); }
void DesignMemoryMgr::DestroyHPin(HPin* obj) { hpin_table.Destroy(obj); }
void DesignMemoryMgr::DestroyPort(Port* obj) { port_table.Destroy(obj); }
void DesignMemoryMgr::DestroyBusPort(BusPort* obj) {
    bus_port_table.Destroy(obj);
}

void DesignMemoryMgr::DestroyBusNet(BusNet* obj) { bus_net_table.Destroy(obj); }

void DesignMemoryMgr::SetCurrentDesign(uint64_t designId) {
    Design* design = GetDesign(designId);
    if (design == nullptr) {
        DB_LOG(ERROR) << "invdid  design id " << designId << '\n';
        return;
    }
    current_design_id = designId;
}

Design* DesignMemoryMgr::GetCurrentDesign() {
    return GetDesign(current_design_id);
}

bool DesignMemoryMgr::SaveAll() {
    bool success = true;
    success &= design_table.Save();
    success &= inst_table.Save();
    success &= hinst_table.Save();
    success &= net_table.Save();
    success &= hnet_table.Save();
    success &= pin_table.Save();
    success &= hpin_table.Save();
    success &= port_table.Save();
    success &= bus_port_table.Save();
    success &= bus_net_table.Save();

    // 保存元数据文件
    std::string   meta_file = db_path + "/metadata.bin";
    std::ofstream file(meta_file, std::ios::binary);
    if (file.is_open()) {
        uint32_t magic = 0x4D455441;  // "META"
        file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
        file.write(reinterpret_cast<const char*>(&current_design_id),
                   sizeof(current_design_id));
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
    success &= hnet_table.Load();
    success &= pin_table.Load();
    success &= hpin_table.Load();
    success &= port_table.Load();
    success &= bus_port_table.Load();
    success &= bus_net_table.Load();

    std::string   meta_file = db_pathdir + "/metadata.bin";
    std::ifstream file(meta_file, std::ios::binary);
    if (file.is_open()) {
        uint32_t magic;
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        if (magic == 0x4D455441) {
            file.read(reinterpret_cast<char*>(&current_design_id),
                      sizeof(current_design_id));
        }
        file.close();
    }
    return true;
}

}  // namespace db