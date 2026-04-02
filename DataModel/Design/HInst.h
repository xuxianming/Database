#ifndef DATAMODEL_DESIGN_HINST_H_
#define DATAMODEL_DESIGN_HINST_H_

#include "DataModel/Design/InstBase.h"

namespace db {
class HInst : public InstBase {
public:
    HInst()           = default;
    ~HInst() override = default;
    size_t GetSize() const override;

private:
    InstArrayId m_insts_id;
    HInstArrayId m_hinsts_id;
};

}  // namespace db

#endif