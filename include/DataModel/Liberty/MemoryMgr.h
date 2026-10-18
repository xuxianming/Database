#ifndef DATAMODEL_LIBERTY_MEMERTYMGR_H_
#define DATAMODEL_LIBERTY_MEMERTYMGR_H_

#include <cstdint>
#include <string>
#include "DataModel/Enums.h"
#include "DataModel/Liberty/Type.h"
#include "DataModel/Type.h"
namespace db {
class LibertyLib;
class LibCell;
class LibPort;
class LibertyMemoryMgr {
public:
    static LibertyLib* CreateLibertyLib();
    static LibCell*    CreateLibCell();
    static LibPort*    CreateLibPort();

    static LibertyLib* GetLibertyLib(ObjectId id);
    static LibCell*    GetLibCell(ObjectId id);
    static LibPort*    GetLibPort(ObjectId id);

    static LibCell* GetLibCellByLogicType(LogicType type);
    static void DestroyLibertyLib(ObjectId id);
    static void DestroyLibCell(ObjectId id);
    static void DestroyLibPort(ObjectId id);

    static LibertyIdArrayMgr* GetIdArrayMgr() { return m_idarray_mgr; }
    static LibertyStringMgr*  GetStringMgr() { return m_string_mgr; }

    // Idempotently allocates the Liberty string/id-array managers.
    static void EnsureManagers();

    void                      SetDefaultLibertyLib(ObjectId libertyId);
    LibertyLib*               GetDefaultLibertyLib();

    bool SaveAll();
    bool LoadAll(const std::string& db_pathdir);

    void PrintStats() const;
    void Clear();

private:
    FullObjectId EncodeId(ObjectId realId, DMObjectType type) {
        return (static_cast<FullObjectId>(type) << TypeOffset) | (realId & IdMask);
    }

    inline ObjectId DecodeId(FullObjectId fullId) { return fullId & IdMask; }

    static ObjectId              default_liberty_id;
    static LibertyStringMgr*     m_string_mgr;
    static LibertyIdArrayMgr*    m_idarray_mgr;
    static std::vector<LibCell*> logic_libcells;
};
}  // namespace db
#endif