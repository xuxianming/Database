#ifndef DATAMODEL_DESIGN_NETBASE_H_
#define DATAMODEL_DESIGN_NETBASE_H_
#include <cstddef>
#include "DataModel/Design/DesignObject.h"
namespace db {
class NetBase : public DesignObject {
public:
    NetBase()           = default;
    ~NetBase() override = default;
    size_t GetSize() const override;

protected:
    FullObjectId m_owner_id;                // Design or HInst
    ObjectId     m_pins_id;                 // double dir list head pin ID
    uint32_t     m_driving_pin_count : 16;  // number of driving pins
    uint32_t     m_io_pin_count : 16;       // number of loading pins
    uint32_t     m_pin_count;               // total number of pins
};

}  // namespace db

#endif