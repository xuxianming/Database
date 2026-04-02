#include "DataModel/Liberty/MemeryMgr.h"
#include "BasePlatfrom/ObjectTable.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/LibPort.h"
#include "DataModel/Liberty/LibertyLib.h"
#include "MemeryMgr.h"

namespace db {
ObjectTable<LibertyLib, ObjectId, ObjectIndexBits> lib_table(
    static_cast<uint8_t>(DMObjectType::LIBERTYLIB));
ObjectTable<LibCell, ObjectId, ObjectIndexBits> libcell_table(
    static_cast<uint8_t>(DMObjectType::LIBCELL));
ObjectTable<LibPort, ObjectId, ObjectIndexBits> libport_table(
    static_cast<uint8_t>(DMObjectType::LIBPORT));

ObjectId    LibMemeryMgr::default_liberty_id = kInvalidId;
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
LibertyLib* LibMemeryMgr::GetLibertyLib(FullObjectId full_id) {
    FullObjectId real_id = DecodeId(full_id);
    return lib_table.Pointer(real_id);
}
LibCell* LibMemeryMgr::GetLibCell(FullObjectId full_id) {
    FullObjectId real_id = DecodeId(full_id);
    return libcell_table.Pointer(real_id);
}
LibPort* LibMemeryMgr::GetLibPort(FullObjectId full_id) {
    FullObjectId real_id = DecodeId(full_id);
    return libport_table.Pointer(real_id);
}

LibertyLib* LibMemeryMgr::GetLibertyLib(ObjectId id) {
    return lib_table.Pointer(id);
}
LibCell* LibMemeryMgr::GetLibCell(ObjectId id) {
    return libcell_table.Pointer(id);
}
LibPort* LibMemeryMgr::GetLibPort(ObjectId id) {
    return libport_table.Pointer(id);
}

void LibMemeryMgr::DestroyLibertyLib(LibertyLib* obj) {
    lib_table.Destroy(obj);
}
void LibMemeryMgr::DestroyLibCell(LibCell* obj) { libcell_table.Destroy(obj); }
void LibMemeryMgr::DestroyLibPort(LibPort* obj) { libport_table.Destroy(obj); }

void LibMemeryMgr::SetDefaultLibertyLib(ObjectId libertyId) {
    default_liberty_id = libertyId;
}

LibertyLib* LibMemeryMgr::GetDefaultLibertyLib() {
    return lib_table.Pointer(default_liberty_id);
}

bool LibMemeryMgr::SaveAll() { return false; }

bool LibMemeryMgr::LoadAll(const std::string& db_pathdir) { return false; }

void LibMemeryMgr::PrintStats() const {}

void LibMemeryMgr::Clear() {}

}  // namespace db