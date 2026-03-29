#ifndef DATAMODEL_DESIGN_INSTBASE_H_
#define DATAMODEL_DESIGN_INSTBASE_H_
#include <cstdint>
#include "DataModel/Design/DesignObject.h"

namespace db {
class InstBase : public DesignObject {
public:
    InstBase()           = default;
    ~InstBase() override = default;

private:
    uint32_t m_owner_id;  // Design or HInst
    uint32_t m_ref_id;    // Design or LibCell
    uint32_t m_pins_id;
};

}  // namespace db

#endif