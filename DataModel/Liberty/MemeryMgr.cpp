#include "DataModel/Liberty/MemeryMgr.h"
#include "BasePlatfrom/ObjectTable.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/LibPort.h"
#include "DataModel/Liberty/LibertyLib.h"

namespace db {
ObjectTable<LibertyLib> lib_table(
    static_cast<uint8_t>(DMObjectType::LIBERTYLIB));
ObjectTable<LibCell> libcell_table(static_cast<uint8_t>(DMObjectType::LIBCELL));
ObjectTable<LibPort> libport_table(static_cast<uint8_t>(DMObjectType::LIBPORT));

LibertyLib* LibMemeryMgr::CreatLibertyLib() {
    auto  id  = lib_table.GetNextObjId();
    auto* lib = lib_table.Make();
    lib->SetObjectId(id);
    return lib;
}

LibCell* LibMemeryMgr::CreateLibCell() {
    auto  id   = libcell_table.GetNextObjId();
    auto* cell = libcell_table.Make();
    cell->SetObjectId(id);
    return cell;
}

LibPort* LibMemeryMgr::CreateLibPort() {
    auto  id   = libport_table.GetNextObjId();
    auto* port = libport_table.Make();
    port->SetObjectId(id);
    return port;
}
LibertyLib* LibMemeryMgr::GetLibertyLib(uint64_t full_id) {
    uint64_t real_id = DecodeId(full_id);
    return lib_table.Pointer(real_id);
}
LibCell* LibMemeryMgr::GetLibCell(uint64_t full_id) {
    uint64_t real_id = DecodeId(full_id);
    return libcell_table.Pointer(real_id);
}
LibPort* LibMemeryMgr::GetLibPort(uint64_t full_id) {
    uint64_t real_id = DecodeId(full_id);
    return libport_table.Pointer(real_id);
}

LibertyLib* LibMemeryMgr::GetLibertyLib(uint32_t id) {
    return lib_table.Pointer(id);
}
LibCell* LibMemeryMgr::GetLibCell(uint32_t id) {
    return libcell_table.Pointer(id);
}
LibPort* LibMemeryMgr::GetLibPort(uint32_t id) {
    return libport_table.Pointer(id);
}

void LibMemeryMgr::DestroyLibertyLib(LibertyLib* obj) {
    lib_table.Destroy(obj);
}
void LibMemeryMgr::DestroyLibCell(LibCell* obj) { libcell_table.Destroy(obj); }
void LibMemeryMgr::DestroyLibPort(LibPort* obj) { libport_table.Destroy(obj); }

}  // namespace db