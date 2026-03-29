#include "DataModel/Design/HInst.h"

namespace db {
HInst::HInst() { SetType(DMObjectType::HINST); }

size_t HInst::GetSize() const { return sizeof(HInst); }

}  // namespace db
