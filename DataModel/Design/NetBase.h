#ifndef DATAMODEL_DESIGN_NETBASE_H_
#define DATAMODEL_DESIGN_NETBASE_H_
#include <cstddef>
#include "DataModel/Design/DesignObject.h"
#include "DataModel/Design/MemeryMgr.h"
namespace db {
class NetBase : public DesignObject {
    friend DesignMemoryMgr;

public:
    size_t GetSize() const override;

private:
    uint32_t m_owner_id{};
    uint32_t m_pins_id{};
};

}  // namespace db

#endif