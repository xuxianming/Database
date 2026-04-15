#ifndef DATAMODEL_DESIGN_BUSNET_H_
#define DATAMODEL_DESIGN_BUSNET_H_
#include "DataModel/Object.h"
namespace db {
class BusNet : public Object {
public:
    BusNet()           = default;
    ~BusNet() override = default;
    size_t GetSize() const override;

private:
    ObjectId        m_design_id;
    ObjectId        m_members_id;
    BusBitRangeType m_from_index;
    BusBitRangeType m_to_index;
};

}  // namespace db

#endif