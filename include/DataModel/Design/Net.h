#ifndef DATAMODEL_DESIGN_NET_H_
#define DATAMODEL_DESIGN_NET_H_

#include <cstdint>
#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
namespace db {
class Design;
class Module;
class HInst;

/**
 * Defines netlist connectivity and regular-routes for nets containing regular pins.
 */
class Net : public Object {
    friend class ModuleNetIterator;

public:
    Net()           = default;
    ~Net() override = default;
    size_t GetSize() const override;
    bool   IsBusBit() const { return m_bit_id != InvalidBusBitId; }

    Design* GetDesign() const;
    Module* GetModule() const;
    HInst*  GetHInst() const;

    void ConnectPin(Pin* pin);
    void DisConnectPin(Pin* pin);

private:
    ObjectId m_design_id{kInvalidId};
    ObjectId m_top_module_id{kInvalidId};
    ObjectId m_hinst_id{kInvalidId};
    BusBitId m_bit_id{InvalidBusBitId};
    ObjectId m_hnet_list_id{kInvalidId};
    ObjectId m_pin_head_id{kInvalidId};
    ObjectId m_next_net_id{kInvalidId};
    ObjectId m_prev_net_id{kInvalidId};
};

}  // namespace db

#endif