#ifndef DATAMODEL_DESIGN_HNET_H_
#define DATAMODEL_DESIGN_HNET_H_
#include <cstdint>
#include "DataModel/Design/NetBase.h"
namespace db {
class HNet : public NetBase {


public:
    HNet()           = default;
    ~HNet() override = default;
    size_t GetSize() const override;

private:
    uint32_t m_bit_id;
    uint32_t m_hpins_id;
};

}  // namespace db

#endif