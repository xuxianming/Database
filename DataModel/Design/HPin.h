#ifndef DATAMODEL_DESIGN_HPIN_H_
#define DATAMODEL_DESIGN_HPIN_H_

#include "DataModel/Design/PinBase.h"
namespace db {
class HPin : public PinBase {
public:
    HPin()           = default;
    ~HPin() override = default;
    size_t GetSize() const override;

private:
    uint32_t m_lower_net_id;
};

}  // namespace db

#endif