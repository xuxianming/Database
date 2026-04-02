#ifndef DATAMODEL_DESIGN_MEMORYMGR_H_
#define DATAMODEL_DESIGN_MEMORYMGR_H_

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
    static Design*  CreateDesign();
    static Inst*    CreateInst();
    static HInst*   CreateHInst();
    static Net*     CreateNet();
    static HNet*    CreateHNet();
    static Pin*     CreatePin();
    static HPin*    CreateHPin();
    static Port*    CreatePort();
    static BusPort* CreateBusPort();
    static BusNet*  CreateBusNet();

    static Design*  GetDesign(FullObjectId full_id);
    static Inst*    GetInst(FullObjectId full_id);
    static HInst*   GetHInst(FullObjectId full_id);
    static Net*     GetNet(FullObjectId full_id);
    static HNet*    GetHNet(FullObjectId full_id);
    static Pin*     GetPin(FullObjectId full_id);
    static HPin*    GetHPin(FullObjectId full_id);
    static Port*    GetPort(FullObjectId full_id);
    static BusPort* GetBusPort(FullObjectId full_id);
    static BusNet*  GetBusNet(FullObjectId full_id);

    static Design*  GetDesign(ObjectId id);
    static Inst*    GetInst(ObjectId id);
    static HInst*   GetHInst(ObjectId id);
    static Net*     GetNet(ObjectId id);
    static HNet*    GetHNet(ObjectId id);
    static Pin*     GetPin(ObjectId id);
    static HPin*    GetHPin(ObjectId id);
    static Port*    GetPort(ObjectId id);
    static BusPort* GetBusPort(ObjectId id);
    static BusNet*  GetBusNet(ObjectId id);

    static void DestroyDesign(Design* obj);
    static void DestroyInst(Inst* obj);
    static void DestroyHInst(HInst* obj);
    static void DestroyNet(Net* obj);
    static void DestroyHNet(HNet* obj);
    static void DestroyPin(Pin* obj);
    static void DestroyHPin(HPin* obj);
    static void DestroyPort(Port* obj);
    static void DestroyBusPort(BusPort* obj);
    static void DestroyBusNet(BusNet* obj);

    static void    SetCurrentDesign(ObjectId designId);
    static Design* GetCurrentDesign();

    static bool SaveAll();
    static bool LoadAll(const std::string& db_pathdir);

    static void PrintStats();
    static void Clear();

private:
    static ObjectId current_design_id;
};

}  // namespace db

#endif