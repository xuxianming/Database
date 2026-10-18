#include "DataModel/Liberty/MemoryMgr.h"
#include "BasePlatfrom/ObjectTable.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/LibPort.h"
#include "DataModel/Liberty/LibertyLib.h"

namespace db {
ObjectTable<LibertyLib, ObjectId, ObjectIndexBits> lib_table(
    static_cast<uint8_t>(DMObjectType::LIBERTYLIB));
ObjectTable<LibCell, ObjectId, ObjectIndexBits> libcell_table(
    static_cast<uint8_t>(DMObjectType::LIBCELL));
ObjectTable<LibPort, ObjectId, ObjectIndexBits> libport_table(
    static_cast<uint8_t>(DMObjectType::LIBPORT));

ObjectId              LibertyMemoryMgr::default_liberty_id = kInvalidId;
LibertyStringMgr*     LibertyMemoryMgr::m_string_mgr       = nullptr;
LibertyIdArrayMgr*    LibertyMemoryMgr::m_idarray_mgr      = nullptr;
std::vector<LibCell*> LibertyMemoryMgr::logic_libcells;
LibertyLib*           LibertyMemoryMgr::CreateLibertyLib() {
    EnsureManagers();
    auto  id  = lib_table.GetNextObjId();
    auto* lib = lib_table.Make();
    lib->SetObjectId(id);
    return lib;
}

void LibertyMemoryMgr::EnsureManagers() {
    if (m_string_mgr == nullptr) {
        m_string_mgr = new LibertyStringMgr();
    }
    if (m_idarray_mgr == nullptr) {
        m_idarray_mgr = new LibertyIdArrayMgr();
    }
}

LibCell* LibertyMemoryMgr::CreateLibCell() {
    EnsureManagers();
    auto  id   = libcell_table.GetNextObjId();
    auto* cell = libcell_table.Make();
    cell->SetObjectId(id);
    return cell;
}

LibPort* LibertyMemoryMgr::CreateLibPort() {
    EnsureManagers();
    auto  id   = libport_table.GetNextObjId();
    auto* port = libport_table.Make();
    port->SetObjectId(id);
    return port;
}

LibertyLib* LibertyMemoryMgr::GetLibertyLib(ObjectId id) { return lib_table.Pointer(id); }
LibCell*    LibertyMemoryMgr::GetLibCell(ObjectId id) { return libcell_table.Pointer(id); }
LibPort*    LibertyMemoryMgr::GetLibPort(ObjectId id) { return libport_table.Pointer(id); }

void LibertyMemoryMgr::DestroyLibertyLib(ObjectId id) { lib_table.Destroy(lib_table.Pointer(id)); }
LibCell* LibertyMemoryMgr::GetLibCellByLogicType(LogicType type) {
    if (type == LogicType::COUNT) {
        return nullptr;
    }
    return logic_libcells[(uint8_t)type];
}
void LibertyMemoryMgr::DestroyLibCell(ObjectId id) {
    libcell_table.Destroy(libcell_table.Pointer(id));
}
void LibertyMemoryMgr::DestroyLibPort(ObjectId id) {
    libport_table.Destroy(libport_table.Pointer(id));
}

void LibertyMemoryMgr::SetDefaultLibertyLib(ObjectId libertyId) { default_liberty_id = libertyId; }

LibertyLib* LibertyMemoryMgr::GetDefaultLibertyLib() {
    return lib_table.Pointer(default_liberty_id);
}

bool LibertyMemoryMgr::SaveAll() { return false; }

bool LibertyMemoryMgr::LoadAll(const std::string& db_pathdir) { return false; }

void LibertyMemoryMgr::PrintStats() const {}

void LibertyMemoryMgr::Clear() {
    delete m_string_mgr;
    m_string_mgr = nullptr;
    delete m_idarray_mgr;
    m_idarray_mgr = nullptr;

    lib_table.Clear();
    libcell_table.Clear();
    libport_table.Clear();

    logic_libcells.clear();
    default_liberty_id = kInvalidId;
}

}  // namespace db