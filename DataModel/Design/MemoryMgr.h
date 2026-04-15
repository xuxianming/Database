#ifndef DATAMODEL_DESIGN_MEMORYMGR_H_
#define DATAMODEL_DESIGN_MEMORYMGR_H_

#include <cstdint>
#include "DataModel/Type.h"
namespace db {
class Design;
class Module;
class Inst;
class HInst;
class Net;
class Pin;
class Port;
class BusPort;
class BusNet;

class DesignMemoryMgr {
public:
    static Design*  CreateDesign();
    static Module*  CreateModule();
    static Inst*    CreateInst();
    static HInst*   CreateHInst();
    static Net*     CreateNet();
    static Pin*     CreatePin();
    static Port*    CreatePort();
    static BusPort* CreateBusPort();
    static BusNet*  CreateBusNet();

    static Design*  GetDesign(ObjectId id);
    static Module*  GetModule(ObjectId id);
    static Inst*    GetInst(ObjectId id);
    static HInst*   GetHInst(ObjectId id);
    static Net*     GetNet(ObjectId id);
    static Pin*     GetPin(ObjectId id);
    static Port*    GetPort(ObjectId id);
    static BusPort* GetBusPort(ObjectId id);
    static BusNet*  GetBusNet(ObjectId id);

    static void DestroyDesign(ObjectId id);
    static void DestroyModule(ObjectId id);
    static void DestroyInst(ObjectId id);
    static void DestroyHInst(ObjectId id);
    static void DestroyNet(ObjectId id);
    static void DestroyPin(ObjectId id);
    static void DestroyPort(ObjectId id);
    static void DestroyBusPort(ObjectId id);
    static void DestroyBusNet(ObjectId id);

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