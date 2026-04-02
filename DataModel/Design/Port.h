#ifndef DATAMODEL_DESIGN_PORT_H_
#define DATAMODEL_DESIGN_PORT_H_
#include "DataModel/Design/DesignObject.h"
namespace db {
class Port : public DesignObject {
public:
    Port()           = default;
    ~Port() override = default;
    size_t GetSize() const override;

private:
    ObjectId m_design_id{};
    BusBitId m_bit_id{};
};

}  // namespace db

#endif