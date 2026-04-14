#ifndef DATAMODEL_DESIGN_PINBASE_H_
#define DATAMODEL_DESIGN_PINBASE_H_
#include "DataModel/Design/DesignObject.h"
namespace db {
class PinBase : public DesignObject {
public:
    PinBase()           = default;
    ~PinBase() override = default;

private:
    ObjectId     m_port_id{0};
    ObjectId     m_net_id{0};
    FullObjectId m_owner_id{0};     // Inst Id or Hinst Id
    ObjectId     m_next_pin_id{0};  // double dir list next pin ID
    ObjectId     m_prev_pin_id{0};  // double dir list prev pin ID
    uint32_t     m_is_driving_pin : 1;  // 1 bit for driving flag
    uint32_t     m_is_loading_pin : 1;  // 1 bit for loading flag
    uint32_t     m_is_io_pin   : 1;  // 1 bit for iopin flag
    uint32_t     m_is_hierarchical_pin : 1;  // 1 bit for hierarchical pin flag
};

}  // namespace db

#endif