#ifndef DATAMODEL_DESIGN_NETBASE_H_
#define DATAMODEL_DESIGN_NETBASE_H_
#include <cstddef>
#include "DataModel/Design/DesignObject.h"
namespace db {
class NetBase : public DesignObject {
public:
    NetBase()           = default;
    ~NetBase() override = default;
    size_t GetSize() const override;

protected:
    FullObjectId m_owner_id;  // Design or HInst
    PinArrayId   m_pins_id{};
};

}  // namespace db

#endif