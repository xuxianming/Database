#ifndef DATAMODEL_DESIGN_PORT_H_
#define DATAMODEL_DESIGN_PORT_H_
#include "DataModel/Design/DesignObject.h"
#include "DataModel/Design/MemeryMgr.h"
namespace db {
class Port : public DesignObject {
    friend class DesignMemoryMgr;

public:
    Port();
    size_t GetSize() const override;

private:
    uint32_t m_design_id{};
    uint32_t m_bit_id{};
};

}  // namespace db

#endif