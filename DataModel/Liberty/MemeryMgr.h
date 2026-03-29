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

    LibertyLib* GetLibertyLib(uint64_t full_id);
    LibCell*    GetLibCell(uint64_t full_id);
    LibPort*    GetLibPort(uint64_t full_id);
    LibertyLib* GetLibertyLib(uint32_t id);
    LibCell*    GetLibCell(uint32_t id);
    LibPort*    GetLibPort(uint32_t id);

    void DestroyLibertyLib(LibertyLib* obj);
    void DestroyLibCell(LibCell* obj);
    void DestroyLibPort(LibPort* obj);

    void        SetDefaultLibertyLib(uint64_t libertyId);
    LibertyLib* GetDefaultLibertyLib();

    bool SaveAll();
    bool LoadAll(const std::string& db_pathdir);

    void PrintStats() const;
    void Clear();

private:
    uint64_t EncodeId(uint32_t realId, DMObjectType type) {
        return (static_cast<uint64_t>(type) << 56) |
               (realId & 0x00000000FFFFFFFF);
    }

    inline uint32_t DecodeId(uint64_t fullId) {
        return fullId & 0x00000000FFFFFFFF;
    }

    static uint64_t default_liberty_id;
};
}  // namespace db
#endif