#ifndef DATAMODEL_DESIGN_MEMORYMGR_H_
#define DATAMODEL_DESIGN_MEMORYMGR_H_

#include <cstdint>
#include "DataModel/Type.h"
namespace db {
class Design;
class Module;
class ModulePort;
class Inst;
class HInst;
class Net;
class Pin;
class HNet;
class HPin;
class Port;
class BusPort;
class BusNet;

class DesignMemoryMgr {
public:
    static Design*     CreateDesign();
    static Module*     CreateModule();
    static ModulePort* CreateModulePort();
    static Inst*       CreateInst();
    static HInst*      CreateHInst();
    static Net*        CreateNet();
    static HNet*       CreateHNet();
    static HPin*       CreateHPin();
    static Pin*        CreatePin();
    static BusPort*    CreateBusPort();
    static BusNet*     CreateBusNet();

    static Design*     GetDesign(ObjectId id);
    static Module*     GetModule(ObjectId id);
    static ModulePort* GetModulePort(ObjectId id);
    static Inst*       GetInst(ObjectId id);
    static HInst*      GetHInst(ObjectId id);
    static Net*        GetNet(ObjectId id);
    static HNet*       GetHNet(ObjectId id);
    static HPin*       GetHPin(ObjectId id);
    static Pin*        GetPin(ObjectId id);
    static BusPort*    GetBusPort(ObjectId id);
    static BusNet*     GetBusNet(ObjectId id);

    static void DestroyDesign(ObjectId id);
    static void DestroyModule(ObjectId id);
    static void DestroyModulePort(ObjectId id);
    static void DestroyInst(ObjectId id);
    static void DestroyHInst(ObjectId id);
    static void DestroyNet(ObjectId id);
    static void DestroyHNet(ObjectId id);
    static void DestroyHPin(ObjectId id);
    static void DestroyPin(ObjectId id);
    static void DestroyBusPort(ObjectId id);
    static void DestroyBusNet(ObjectId id);

    static bool SaveAll();
    static bool LoadAll(const std::string& db_pathdir);

    static void PrintStats();
    static void Clear();

private:
};

}  // namespace db

#endif