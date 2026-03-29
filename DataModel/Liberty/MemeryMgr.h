#ifndef DATAMODEL_LIBERTY_MEMERTYMGR_H_
#define DATAMODEL_LIBERTY_MEMERTYMGR_H_

#include "DataModel/Type.h"
#include <cstdint>
#include <string>
namespace db {
class LibertyLib;
class LibCell;
class LibPort;
class LibMemeryMgr {
public:
  LibertyLib *CreatLibertyLib();
  LibCell *CreateLibCell();
  LibPort *CreateLibPort();

  LibertyLib *GetLibertyLib(uint64_t id);
  LibCell *GetLibCell(uint64_t id);
  LibPort *GetHLibCell(uint64_t id);

  void DestroyLibertyLib(LibertyLib *obj);
  void DestroyLibCell(LibCell *obj);
  void DestroyLibPort(LibPort *obj);

  void SetDefaultLibertyLib(uint64_t libertyId);
  LibertyLib *GetDefaultLibertyLib();

  bool SaveAll();
  bool LoadAll(const std::string &db_pathdir);

  void PrintStats() const;
  void Clear();

private:
  template <typename T> uint64_t EncodeId(uint64_t realId, DMObjectType type) {
    return (static_cast<uint64_t>(type) << 56) | (realId & 0x00FFFFFFFFFFFFFF);
  }

  template <typename T> uint64_t DecodeId(uint64_t fullId) {
    return fullId & 0x00FFFFFFFFFFFFFF;
  }

  static uint64_t default_liberty_id;
};
} // namespace db
#endif