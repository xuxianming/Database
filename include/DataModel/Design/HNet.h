#ifndef DATAMODEL_DESIGN_HNET_H_
#define DATAMODEL_DESIGN_HNET_H_

#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"

namespace db {
/**
 * HNet is used to represent a connectivity in logical hierarchy.
 */
class HNet : public Object {
    friend class ModuleHNetIterator;
    friend class HInstHNetIterator;

public:
    HNet()           = default;
    ~HNet() override = default;
    size_t  GetSize() const override;
    bool    IsBusBit() const { return m_bit_id != InvalidBusBitId; }
    Net*    GetNet() const;
    HInst*  GetHInst() const;
    Module* GetModule() const;
    Design* GetDesign() const;

    void AttachToNet(Net* net);
    void AttachToNet(ObjectId net_id);
    void DetachFromNet();
    // HNet may connect both ordinary pins and hierarchy boundary pins.
    void ConnectPin(Pin* pin);
    void DisConnectPin(Pin* pin);
    void ConnectHPin(HPin* hpin);
    void DisConnectHPin(HPin* hpin);

private:
    ObjectId m_module_id{kInvalidId};
    ObjectId m_design_id{kInvalidId};
    ObjectId m_top_module_id{kInvalidId};
    ObjectId m_net_id{kInvalidId};
    ObjectId m_hinst_id{kInvalidId};
    BusBitId m_bit_id{InvalidBusBitId};
    // double link list pin head, for fast traversal of Pins/HPins connected to this hnet
    ObjectId m_pin_head_id{kInvalidId};
    ObjectId m_hpin_head_id{kInvalidId};
    ObjectId m_next_hnet_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_HNET_H_