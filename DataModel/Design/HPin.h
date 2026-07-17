#ifndef DATAMODEL_DESIGN_HPIN_H_
#define DATAMODEL_DESIGN_HPIN_H_
#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
namespace db {
class HPin : public Object {
    friend class HInst;
    friend class HInstHPinIterator;

public:
    HPin()           = default;
    ~HPin() override = default;
    size_t GetSize() const override;

    ModulePort* GetModulePort() const;
    Module*     GetModule() const;
    Design*     GetMasterDesign() const;
    HInst*      GetHInst() const;
    HNet*       GetUpperHNet() const;
    HNet*       GetLowerHNet() const;
    Net*        GetNet() const;

    // Connect api
    void ConnectUpperHNet(HNet* hnet);
    void ConnectLowerHNet(HNet* hnet);
    void DisConnectUpperHNet();
    void DisConnectLowerHNet();

private:
    ObjectId m_port_id{kInvalidId};
    ObjectId m_hinst_id{kInvalidId};
    ObjectId m_upper_hnet_id{kInvalidId};
    ObjectId m_lower_hnet_id{kInvalidId};
    ObjectId m_next_hpin_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_HPIN_H_