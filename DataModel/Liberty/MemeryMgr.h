#ifndef DATAMODEL_LIBERTY_MEMERTYMGR_H_
#define DATAMODEL_LIBERTY_MEMERTYMGR_H_

#include <cstdint>
#include <string>
#include "DataModel/Type.h"
namespace db {
class LibertyLib;
class LibCell;
class LibPort;
class LibMemeryMgr {
public:
    LibertyLib* CreatLibertyLib();
    LibCell*    CreateLibCell();
    LibPort*    CreateLibPort();

    LibertyLib* GetLibertyLib(FullObjectId full_id);
    LibCell*    GetLibCell(FullObjectId full_id);
    LibPort*    GetLibPort(FullObjectId full_id);
    LibertyLib* GetLibertyLib(ObjectId id);
    LibCell*    GetLibCell(ObjectId id);
    LibPort*    GetLibPort(ObjectId id);

    void DestroyLibertyLib(LibertyLib* obj);
    void DestroyLibCell(LibCell* obj);
    void DestroyLibPort(LibPort* obj);

    void        SetDefaultLibertyLib(ObjectId libertyId);
    LibertyLib* GetDefaultLibertyLib();

    bool SaveAll();
    bool LoadAll(const std::string& db_pathdir);

    void PrintStats() const;
    void Clear();

private:
    FullObjectId EncodeId(ObjectId realId, DMObjectType type) {
        return (static_cast<FullObjectId>(type) << TypeOffset) |
               (realId & IdMask);
    }

    inline ObjectId DecodeId(FullObjectId fullId) { return fullId & IdMask; }

    static ObjectId default_liberty_id;
};
}  // namespace db
#endif