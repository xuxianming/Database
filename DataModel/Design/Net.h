#ifndef DATAMODEL_DESIGN_NET_H_
#define DATAMODEL_DESIGN_NET_H_

#include <cstdint>
#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
namespace db {
class Net : public Object {
public:
    Net()           = default;
    ~Net() override = default;
    size_t GetSize() const override;

private:
    BusBitId m_bit_id{};
};

}  // namespace db

#endif