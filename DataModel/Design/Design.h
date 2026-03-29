#ifndef DATAMODEL_DESIGN_DESIGN_H_
#define DATAMODEL_DESIGN_DESIGN_H_
#include "DataModel/Design/DesignObject.h"
#include "DataModel/Design/MemeryMgr.h"
namespace db {
class Design : public DesignObject {
    friend class DesignMemoryMgr;

public:
    Design();
    size_t GetSize() const override;

private:
    uint32_t m_nets_id{};
    uint32_t m_ports_id{};
    uint32_t m_insts_id{};
    uint32_t m_bus_nets_id{};
    uint32_t m_bus_ports_id{};
};

}  // namespace db

#endif