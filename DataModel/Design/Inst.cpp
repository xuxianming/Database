
#include "DataModel/Design/Inst.h"

namespace db {
Inst::Inst() { SetType(DMObjectType::INST); }
size_t Inst::GetSize() const { return sizeof(Inst); }

}  // namespace db
