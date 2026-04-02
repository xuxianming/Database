#ifndef DATAMODEL_DESIGN_PINBASE_H_
#define DATAMODEL_DESIGN_PINBASE_H_
#include "DataModel/Design/DesignObject.h"
namespace db {
class PinBase : public DesignObject {
public:
    PinBase()           = default;
    ~PinBase() override = default;

private:
    ObjectId     m_port_id;
    ObjectId     m_net_id;
    FullObjectId m_owner_id;  // Inst Id or Hinst Id
};

}  // namespace db

#endif