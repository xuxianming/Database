#ifndef DATAMODEL_DESIGN_PIN_H_
#define DATAMODEL_DESIGN_PIN_H_
#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
namespace db {
class Pin : public Object {
    friend class DesignMemoryMgr;
    friend class Net;
    friend class Inst;
    friend class HInst;

public:
    Pin()           = default;
    ~Pin() override = default;
    size_t GetSize() const override;

private:
    ObjectId m_net_id{kInvalidId};
    ObjectId m_port_id{kInvalidId};
    ObjectId m_owner_id{kInvalidId};     // inst or hinst id
    ObjectId m_next_pin_id{kInvalidId};  // for double linked list of pins in net
    ObjectId m_prev_pin_id{kInvalidId};  // for double linked list of pins in net
};

}  // namespace db

#endif