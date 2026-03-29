#ifndef DATAMODEL_DESIGN_PINBASE_H_
#define DATAMODEL_DESIGN_PINBASE_H_
#include "DataModel/Design/DesignObject.h"
#include "DataModel/Design/MemeryMgr.h"
namespace db {
class PinBase : public DesignObject {
    friend class DesignMemoryMgr;

private:
    uint32_t m_port_id;
    uint32_t m_net_id;
    uint32_t m_owner_id;
};

}  // namespace db

#endif