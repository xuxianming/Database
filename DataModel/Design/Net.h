#ifndef DATAMODEL_DESIGN_NET_H_
#define DATAMODEL_DESIGN_NET_H_

#include <cstdint>
#include "DataModel/Design/NetBase.h"
namespace db {
class Net : public NetBase {
public:
    Net()           = default;
    ~Net() override = default;
    size_t GetSize() const override;

private:
    BusBitId m_bit_id{};
};

}  // namespace db

#endif