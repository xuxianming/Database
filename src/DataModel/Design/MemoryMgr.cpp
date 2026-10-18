#include "DataModel/Design/MemoryMgr.h"
#include <cstdint>
#include <fstream>
#include "BasePlatfrom/ObjectTable.h"
#include "DataModel/BusPort.h"
#include "DataModel/Design/BusNet.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/HNet.h"
#include "DataModel/Design/HPin.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Design/ModulePort.h"
#include "DataModel/Design/Net.h"
#include "DataModel/Design/Pin.h"
#include "DataModel/Enums.h"
#include "DataModel/Port.h"
#include "Utils/Log.h"

namespace db {
ObjectTable<Design, ObjectId, ObjectIndexBits> design_table(
    static_cast<uint8_t>(DMObjectType::DESIGN));
ObjectTable<Module, ObjectId, ObjectIndexBits> module_table(
    static_cast<uint8_t>(DMObjectType::MODULE));
ObjectTable<ModulePort, ObjectId, ObjectIndexBits> module_port_table(
    static_cast<uint8_t>(DMObjectType::MODULE_PORT));
ObjectTable<Inst, ObjectId, ObjectIndexBits>  inst_table(static_cast<uint8_t>(DMObjectType::INST));
ObjectTable<HInst, ObjectId, ObjectIndexBits> hinst_table(
    static_cast<uint8_t>(DMObjectType::HINST));
ObjectTable<Net, ObjectId, ObjectIndexBits>  net_table(static_cast<uint8_t>(DMObjectType::NET));
ObjectTable<HNet, ObjectId, ObjectIndexBits> hnet_table(static_cast<uint8_t>(DMObjectType::HNET));
ObjectTable<HPin, ObjectId, ObjectIndexBits> hpin_table(static_cast<uint8_t>(DMObjectType::HPIN));
ObjectTable<Pin, ObjectId, ObjectIndexBits>  pin_table(static_cast<uint8_t>(DMObjectType::PIN));
ObjectTable<BusPort, ObjectId, ObjectIndexBits> bus_port_table(
    static_cast<uint8_t>(DMObjectType::BUS_PORT));
ObjectTable<BusNet, ObjectId, ObjectIndexBits> bus_net_table(
    static_cast<uint8_t>(DMObjectType::BUS_NET));
std::string db_path = ".";

namespace {
// Releases a design-pool name from a destroy path; a double release must not
// abort destruction, so it is logged instead of propagated.
void ReleaseName(Design* design, ObjectNameId name_id) {
    if (design == nullptr || design->GetStringMgr() == nullptr || name_id == 0) {
        return;
    }
    try {
        design->GetStringMgr()->ReleaseString(name_id);
    } catch (const std::exception& e) {
        DB_LOG(WARNING) << "Destroy: unable to release name id " << name_id << ": " << e.what()
                        << "\n";
    }
}
}  // namespace

Design* DesignMemoryMgr::CreateDesign() {
    auto  id     = design_table.GetNextObjId();
    auto* design = design_table.Make();
    design->SetObjectId(id);
    design->m_type = DMObjectType::DESIGN;
    design->SetStringMgr(new DesignStringMgr());
    design->SetIdArrayMgr(new DesignIdArrayMgr());
    return design;
}
Module* DesignMemoryMgr::CreateModule() {
    auto  id     = module_table.GetNextObjId();
    auto* module = module_table.Make();
    module->SetObjectId(id);
    module->m_type = DMObjectType::MODULE;
    return module;
}
ModulePort* DesignMemoryMgr::CreateModulePort() {
    auto  id          = module_port_table.GetNextObjId();
    auto* module_port = module_port_table.Make();
    module_port->SetObjectId(id);
    module_port->m_type = DMObjectType::MODULE_PORT;
    return module_port;
}
Inst* DesignMemoryMgr::CreateInst() {
    auto  id   = inst_table.GetNextObjId();
    auto* inst = inst_table.Make();
    inst->SetObjectId(id);
    inst->m_type = DMObjectType::INST;
    return inst;
}

HInst* DesignMemoryMgr::CreateHInst() {
    auto  id    = hinst_table.GetNextObjId();
    auto* hinst = hinst_table.Make();
    hinst->SetObjectId(id);
    hinst->m_type = DMObjectType::HINST;
    return hinst;
}

Net* DesignMemoryMgr::CreateNet() {
    auto  id  = net_table.GetNextObjId();
    auto* net = net_table.Make();
    net->SetObjectId(id);
    net->m_type = DMObjectType::NET;
    return net;
}

HNet* DesignMemoryMgr::CreateHNet() {
    auto  id   = hnet_table.GetNextObjId();
    auto* hnet = hnet_table.Make();
    hnet->SetObjectId(id);
    hnet->m_type = DMObjectType::HNET;
    return hnet;
}

HPin* DesignMemoryMgr::CreateHPin() {
    auto  id   = hpin_table.GetNextObjId();
    auto* hpin = hpin_table.Make();
    hpin->SetObjectId(id);
    hpin->m_type = DMObjectType::HPIN;
    return hpin;
}

Pin* DesignMemoryMgr::CreatePin() {
    auto  id  = pin_table.GetNextObjId();
    auto* pin = pin_table.Make();
    pin->SetObjectId(id);
    pin->m_type = DMObjectType::PIN;
    return pin;
}

BusPort* DesignMemoryMgr::CreateBusPort() {
    auto  id      = bus_port_table.GetNextObjId();
    auto* busport = bus_port_table.Make();
    busport->SetObjectId(id);
    busport->m_type = DMObjectType::BUS_PORT;
    return busport;
}

BusNet* DesignMemoryMgr::CreateBusNet() {
    auto  id     = bus_net_table.GetNextObjId();
    auto* busnet = bus_net_table.Make();
    busnet->SetObjectId(id);
    busnet->m_type = DMObjectType::BUS_NET;
    return busnet;
}

Design*     DesignMemoryMgr::GetDesign(ObjectId id) { return design_table.Pointer(id); }
Module*     DesignMemoryMgr::GetModule(ObjectId id) { return module_table.Pointer(id); }
Inst*       DesignMemoryMgr::GetInst(ObjectId id) { return inst_table.Pointer(id); }
HInst*      DesignMemoryMgr::GetHInst(ObjectId id) { return hinst_table.Pointer(id); }
Net*        DesignMemoryMgr::GetNet(ObjectId id) { return net_table.Pointer(id); }
HNet*       DesignMemoryMgr::GetHNet(ObjectId id) { return hnet_table.Pointer(id); }
HPin*       DesignMemoryMgr::GetHPin(ObjectId id) { return hpin_table.Pointer(id); }
Pin*        DesignMemoryMgr::GetPin(ObjectId id) { return pin_table.Pointer(id); }
ModulePort* DesignMemoryMgr::GetModulePort(ObjectId id) { return module_port_table.Pointer(id); }
BusPort*    DesignMemoryMgr::GetBusPort(ObjectId id) { return bus_port_table.Pointer(id); }
BusNet*     DesignMemoryMgr::GetBusNet(ObjectId id) { return bus_net_table.Pointer(id); }

void DesignMemoryMgr::DestroyDesign(ObjectId id) {
    Design* design = design_table.Pointer(id);
    if (design != nullptr) {
        delete design->m_idarray_mgr;
        design->SetIdArrayMgr(nullptr);
        delete design->m_string_mgr;
        design->SetStringMgr(nullptr);
    }
    ClearModuleNameIndexes();
    design_table.Destroy(id);
}
void DesignMemoryMgr::DestroyModule(ObjectId id) {
    Module* module = module_table.Pointer(id);
    if (module != nullptr) {
        ReleaseName(module->GetDesign(), module->GetNameId());
    }
    InvalidateModuleNameIndex(id);
    module_table.Destroy(id);
}
void DesignMemoryMgr::DestroyInst(ObjectId id) {
    Inst* inst = inst_table.Pointer(id);
    if (inst != nullptr) {
        if (Module* module = GetModule(inst->m_module_id); module != nullptr) {
            module->UnlinkInst(inst);
            InvalidateModuleNameIndex(module->GetObjectId());
            ReleaseName(module->GetDesign(), inst->GetNameId());
        }
    }
    inst_table.Destroy(id);
}
void DesignMemoryMgr::DestroyHInst(ObjectId id) {
    HInst* hinst = hinst_table.Pointer(id);
    if (hinst != nullptr) {
        Design* design = hinst->GetDesign();
        if (hinst->m_upper_hinst_id != kInvalidId) {
            if (HInst* parent = GetHInst(hinst->m_upper_hinst_id); parent != nullptr) {
                if (parent->m_child_hinst_head_id == id) {
                    parent->m_child_hinst_head_id = hinst->m_next_hinst_id;
                } else {
                    ObjectId cur = parent->m_child_hinst_head_id;
                    while (cur != kInvalidId) {
                        HInst* node = GetHInst(cur);
                        if (node == nullptr) {
                            break;
                        }
                        if (node->m_next_hinst_id == id) {
                            node->m_next_hinst_id = hinst->m_next_hinst_id;
                            break;
                        }
                        cur = node->m_next_hinst_id;
                    }
                }
            }
        } else if (design != nullptr) {
            if (design->m_top_hinst_head_id == id) {
                design->m_top_hinst_head_id = hinst->m_next_hinst_id;
            } else {
                ObjectId cur = design->m_top_hinst_head_id;
                while (cur != kInvalidId) {
                    HInst* node = GetHInst(cur);
                    if (node == nullptr) {
                        break;
                    }
                    if (node->m_next_hinst_id == id) {
                        node->m_next_hinst_id = hinst->m_next_hinst_id;
                        break;
                    }
                    cur = node->m_next_hinst_id;
                }
            }
        }
        ReleaseName(design, hinst->GetNameId());
    }
    hinst_table.Destroy(id);
}
void DesignMemoryMgr::DestroyNet(ObjectId id) { net_table.Destroy(id); }
void DesignMemoryMgr::DestroyHNet(ObjectId id) { hnet_table.Destroy(id); }
void DesignMemoryMgr::DestroyHPin(ObjectId id) { hpin_table.Destroy(id); }
void DesignMemoryMgr::DestroyPin(ObjectId id) { pin_table.Destroy(id); }
void DesignMemoryMgr::DestroyModulePort(ObjectId id) { module_port_table.Destroy(id); }
void DesignMemoryMgr::DestroyBusPort(ObjectId id) { bus_port_table.Destroy(id); }
void DesignMemoryMgr::DestroyBusNet(ObjectId id) { bus_net_table.Destroy(id); }

bool DesignMemoryMgr::SaveAll() {
    bool success = true;

    // 保存当前设计的字符串池（命名表），必须在对象表之前落盘
    Design* design = Design::GetCurrentDesign();
    if (design == nullptr || design->GetStringMgr() == nullptr) {
        DB_LOG(WARNING) << "SaveAll: no current design or string pool, skip strings.db\n";
    } else {
        std::string   str_file = db_path + "/strings.db";
        std::ofstream str_stream(str_file, std::ios::binary);
        if (!str_stream.is_open()) {
            DB_LOG(ERROR) << "Failed to open file for writing: " << str_file << "\n";
            success = false;
        } else if (!design->GetStringMgr()->Save(str_stream)) {
            DB_LOG(ERROR) << "Failed to save string pool: " << str_file << "\n";
            success = false;
        }
    }

    // 保存当前设计的 IdArrayMgr（数组内容），同样在对象表之前
    if (design == nullptr || design->GetIdArrayMgr() == nullptr) {
        DB_LOG(WARNING) << "SaveAll: no current design or id array manager, skip id_arrays.db\n";
    } else {
        std::string   arr_file = db_path + "/id_arrays.db";
        std::ofstream arr_stream(arr_file, std::ios::binary);
        if (!arr_stream.is_open()) {
            DB_LOG(ERROR) << "Failed to open file for writing: " << arr_file << "\n";
            success = false;
        } else if (!design->GetIdArrayMgr()->Save(arr_stream)) {
            DB_LOG(ERROR) << "Failed to save id arrays: " << arr_file << "\n";
            success = false;
        }
    }

    success &= design_table.Save();
    success &= module_table.Save();
    success &= inst_table.Save();
    success &= hinst_table.Save();
    success &= net_table.Save();
    success &= hnet_table.Save();
    success &= hpin_table.Save();
    success &= pin_table.Save();
    success &= module_port_table.Save();
    success &= bus_port_table.Save();
    success &= bus_net_table.Save();

    // 保存元数据文件
    std::string   meta_file = db_path + "/metadata.bin";
    std::ofstream file(meta_file, std::ios::binary);
    if (file.is_open()) {
        ObjectId magic             = 0x4D455441;  // "META"
        ObjectId current_design_id = design == nullptr ? kInvalidId : design->GetObjectId();
        file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
        file.write(reinterpret_cast<const char*>(&current_design_id), sizeof(current_design_id));
        file.close();
    }
    return success;
}

bool DesignMemoryMgr::LoadAll(const std::string& db_pathdir) {
    bool success = true;

    // Lazy name indexes are rebuilt from the raw-loaded linked lists.
    ClearModuleNameIndexes();

    // 1. 先读取元数据，确定当前设计
    ObjectId      current_design_id = kInvalidId;
    std::string   meta_file         = db_pathdir + "/metadata.bin";
    std::ifstream meta_stream(meta_file, std::ios::binary);
    if (!meta_stream.is_open()) {
        DB_LOG(WARNING) << "LoadAll: metadata not found, no current design: " << meta_file
                        << "\n";
    } else {
        ObjectId magic = 0;
        meta_stream.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        if (meta_stream.good() && magic == 0x4D455441) {
            meta_stream.read(reinterpret_cast<char*>(&current_design_id),
                             sizeof(current_design_id));
            if (!meta_stream.good()) {
                DB_LOG(ERROR) << "LoadAll: failed to read current design id: " << meta_file
                              << "\n";
                current_design_id = kInvalidId;
            }
        } else {
            DB_LOG(ERROR) << "LoadAll: invalid metadata: " << meta_file << "\n";
            current_design_id = kInvalidId;
        }
    }
    // 1b. 即将 Clear 旧设计表：先释放旧当前设计的字符串池与数组管理器，避免 LoadAll 泄漏。
    //     其它设计（非当前）不在 P0 处理范围内。
    if (Design* previous = Design::GetCurrentDesign(); previous != nullptr) {
        delete previous->m_idarray_mgr;
        previous->SetIdArrayMgr(nullptr);
        delete previous->m_string_mgr;
        previous->SetStringMgr(nullptr);
    }
    Design::SetCurrentDesign(current_design_id);

    // 2. 设计表先加载，当前设计对象才可解析
    success &= design_table.Load();

    // 3. 为当前设计重建字符串池（表中保存的裸指针已失效）
    if (current_design_id == kInvalidId) {
        DB_LOG(WARNING) << "LoadAll: no current design, skip string pool load\n";
    } else {
        Design* design = Design::GetCurrentDesign();
        if (design == nullptr) {
            DB_LOG(ERROR) << "LoadAll: current design " << current_design_id << " not found\n";
            success = false;
        } else {
            design->SetStringMgr(new DesignStringMgr());
            // 数组内容单独持久化，下面加载到新 manager 中
            design->SetIdArrayMgr(new DesignIdArrayMgr());

            std::string   str_file = db_pathdir + "/strings.db";
            std::ifstream str_stream(str_file, std::ios::binary);
            if (!str_stream.is_open()) {
                DB_LOG(ERROR) << "LoadAll: failed to open string pool: " << str_file << "\n";
                success = false;
            } else if (!design->GetStringMgr()->Load(str_stream)) {
                DB_LOG(ERROR) << "LoadAll: failed to load string pool: " << str_file << "\n";
                success = false;
            }

            std::string   arr_file = db_pathdir + "/id_arrays.db";
            std::ifstream arr_stream(arr_file, std::ios::binary);
            if (!arr_stream.is_open()) {
                DB_LOG(ERROR) << "LoadAll: failed to open id arrays: " << arr_file << "\n";
                success = false;
            } else if (!design->GetIdArrayMgr()->Load(arr_stream)) {
                DB_LOG(ERROR) << "LoadAll: failed to load id arrays: " << arr_file << "\n";
                success = false;
            }
        }
    }

    // 4. 加载其余对象表
    success &= module_table.Load();
    success &= inst_table.Load();
    success &= hinst_table.Load();
    success &= net_table.Load();
    success &= hnet_table.Load();
    success &= hpin_table.Load();
    success &= pin_table.Load();
    success &= module_port_table.Load();
    success &= bus_port_table.Load();
    success &= bus_net_table.Load();

    return success;
}

void DesignMemoryMgr::PrintStats() {}

void DesignMemoryMgr::Clear() {
    // Drop the current design's owned pools before clearing the raw tables.
    if (Design* design = Design::GetCurrentDesign(); design != nullptr) {
        delete design->m_idarray_mgr;
        design->SetIdArrayMgr(nullptr);
        delete design->m_string_mgr;
        design->SetStringMgr(nullptr);
    }

    ClearModuleNameIndexes();

    design_table.Clear();
    module_table.Clear();
    inst_table.Clear();
    hinst_table.Clear();
    net_table.Clear();
    hnet_table.Clear();
    hpin_table.Clear();
    pin_table.Clear();
    module_port_table.Clear();
    bus_port_table.Clear();
    bus_net_table.Clear();

    Design::SetCurrentDesign(kInvalidId);
}
}  // namespace db