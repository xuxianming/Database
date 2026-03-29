#ifndef DATAMODEL_DESIGN_INSTBASE_H_
#define DATAMODEL_DESIGN_INSTBASE_H_
#include "DataModel/Design/DesignObject.h"
#include "DataModel/Design/MemeryMgr.h"
namespace db {
class InstBase : public DesignObject {
    friend class DesignMemoryMgr;

public:
private:
    uint32_t m_ref_id{};
    uint32_t m_pins_id{};
};

}  // namespace db

#endif