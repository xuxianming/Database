#ifndef DATAMODEL_DESIGN_PINBASE_H_
#define DATAMODEL_DESIGN_PINBASE_H_
#include "DataModel/Design/DesignObject.h"
namespace db {
class PinBase : public DesignObject {
public:
    PinBase()           = default;
    ~PinBase() override = default;

private:
    uint32_t m_port_id;
    uint32_t m_net_id;
    uint32_t m_owner_id;
};

}  // namespace db

#endif