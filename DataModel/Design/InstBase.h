#ifndef DATAMODEL_DESIGN_INSTBASE_H_
#define DATAMODEL_DESIGN_INSTBASE_H_
#include <cstdint>
#include "DataModel/Design/DesignObject.h"

namespace db {
class InstBase : public DesignObject {
public:
    InstBase()           = default;
    ~InstBase() override = default;

protected:
    FullObjectId m_owner_id;  // Design or HInst
    FullObjectId m_ref_id;    // Design or LibCell
    PinArrayId   m_pins_id;
};

}  // namespace db

#endif