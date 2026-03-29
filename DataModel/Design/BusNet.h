#ifndef DATAMODEL_DESIGN_BUSNET_H_
#define DATAMODEL_DESIGN_BUSNET_H_
#include "DataModel/Design/DesignObject.h"
#include "DataModel/Design/MemeryMgr.h"
namespace db {
class BusNet : public DesignObject {
    friend class DesignMemoryMgr;

public:
    BusNet();
    size_t GetSize() const override;

private:
    uint32_t m_design_id;
    uint32_t m_members_id;
    uint32_t m_from_index;
    uint32_t m_to_index;
};

}  // namespace db

#endif