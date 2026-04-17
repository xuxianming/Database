#ifndef DATAMODEL_DESIGN_INST_H_
#define DATAMODEL_DESIGN_INST_H_

#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
namespace db {
class Inst : public Object {
public:
    Inst()           = default;
    ~Inst() override = default;
    size_t  GetSize() const override;
    Module* GetModule() const;
    HInst*  GetHInst() const;

private:
    ObjectId   m_owner_id{kInvalidId};  // module or hinst id
    ObjectId   m_libcell_id{kInvalidId};
    ObjectId   m_macro_id{kInvalidId};
    PinArrayId m_pins_array_id{kInvalidId};
};

}  // namespace db

#endif