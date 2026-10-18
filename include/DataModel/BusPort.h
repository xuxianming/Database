#ifndef DATAMODEL_BUSPORT_H_
#define DATAMODEL_BUSPORT_H_

#include "DataModel/Object.h"
namespace db {
class BusPort : public Object {
public:
    BusPort()           = default;
    ~BusPort() override = default;
    size_t GetSize() const override;

private:
    ObjectId        m_owner_id;
    ObjectId        m_members_id;
    BusBitRangeType m_from_index;
    BusBitRangeType m_to_index;
    // list of busports in owner
    ObjectId m_next_id;
};

}  // namespace db

#endif