#ifndef DATAMODEL_DESIGN_HINST_H_
#define DATAMODEL_DESIGN_HINST_H_

#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"

namespace db {
class HInst : public Object {
public:
    HInst()           = default;
    ~HInst() override = default;
    size_t GetSize() const override;
    Module* GetModule() const;
    HInst*  GetHInst() const;
private:
    ObjectId     m_owner_id{kInvalidId};
    InstArrayId  m_insts_id{kInvalidId};
    HInstArrayId m_hinsts_id{kInvalidId};
};

}  // namespace db

#endif