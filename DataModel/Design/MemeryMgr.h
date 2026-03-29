#ifndef DATAMODEL_DESIGN_MEMERYMGR_H_
#define DATAMODEL_DESIGN_MEMERYMGR_H_

#include <cstdint>
#include "DataModel/Type.h"
namespace db {
class Design;
class Inst;
class HInst;
class Net;
class HNet;
class Pin;
class HPin;
class Port;
class BusPort;
class BusNet;

class DesignMemoryMgr {
public:
    Design*  CreateDesign();
    Inst*    CreateInst();
    HInst*   CreateHInst();
    Net*     CreateNet();
    HNet*    CreateHNet();
    Pin*     CreatePin();
    HPin*    CreateHPin();
    Port*    CreatePort();
    BusPort* CreateBusPort();
    BusNet*  CreateBusNet();

    Design*  GetDesign(uint64_t full_id);
    Inst*    GetInst(uint64_t full_id);
    HInst*   GetHInst(uint64_t full_id);
    Net*     GetNet(uint64_t full_id);
    HNet*    GetHNet(uint64_t full_id);
    Pin*     GetPin(uint64_t full_id);
    HPin*    GetHPin(uint64_t full_id);
    Port*    GetPort(uint64_t full_id);
    BusPort* GetBusPort(uint64_t full_id);
    BusNet*  GetBusNet(uint64_t full_id);

    Design*  GetDesign(uint32_t id);
    Inst*    GetInst(uint32_t id);
    HInst*   GetHInst(uint32_t id);
    Net*     GetNet(uint32_t id);
    HNet*    GetHNet(uint32_t id);
    Pin*     GetPin(uint32_t id);
    HPin*    GetHPin(uint32_t id);
    Port*    GetPort(uint32_t id);
    BusPort* GetBusPort(uint32_t id);
    BusNet*  GetBusNet(uint32_t id);

    void DestroyDesign(Design* obj);
    void DestroyInst(Inst* obj);
    void DestroyHInst(HInst* obj);
    void DestroyNet(Net* obj);
    void DestroyHNet(HNet* obj);
    void DestroyPin(Pin* obj);
    void DestroyHPin(HPin* obj);
    void DestroyPort(Port* obj);
    void DestroyBusPort(BusPort* obj);
    void DestroyBusNet(BusNet* obj);

    void    SetCurrentDesign(uint32_t designId);
    Design* GetCurrentDesign();

    bool SaveAll();
    bool LoadAll(const std::string& db_pathdir);

    void PrintStats() const;
    void Clear();

private:
    template <typename T>
    uint64_t EncodeId(uint64_t realId, DMObjectType type) {
        return (static_cast<uint64_t>(type) << 56) |
               (realId & 0x00FFFFFFFFFFFFFF);
    }

    template <typename T>
    uint64_t DecodeId(uint64_t fullId) {
        return fullId & 0x00FFFFFFFFFFFFFF;
    }

    static uint64_t current_design_id;
};

}  // namespace db

#endif