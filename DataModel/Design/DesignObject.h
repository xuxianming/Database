#ifndef DATAMODEL_LIBERTY_DESIGNOBJECT_H_
#define DATAMODEL_LIBERTY_DESIGNOBJECT_H_

#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
namespace db {

class DesignObject : public Object {
public:
    DesignObject()           = default;
    ~DesignObject() override = default;
    bool                 IsDesignObject() const { return true; }
    static DesignObject* GetDesignObject(FullObjectId full_id);
    /**
     * GetDesignObjectById retrieves a DesignObject pointer based on the
     * provided object ID and type.
     */
    static Design*  GetDesign(ObjectId design_id);
    static Inst*    GetInst(ObjectId inst_id);
    static HInst*   GetHInst(ObjectId hinst_id);
    static Net*     GetNet(ObjectId net_id);
    static HNet*    GetHNet(ObjectId hnet_id);
    static Pin*     GetPin(ObjectId pin_id);
    static HPin*    GetHPin(ObjectId hpin_id);
    static Port*    GetPort(ObjectId port_id);
    static BusPort* GetBusPort(ObjectId bus_port_id);
    static BusNet*  GetBusNet(ObjectId bus_net_id);

private:
    ObjectId m_master_design_id{};  // For objects that belong to a design,
                                    // store the design ID for quick access
};
}  // namespace db

#endif