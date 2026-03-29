

#include "DataModel/Design/HPin.h"
namespace db {
HPin::HPin() { SetType(DMObjectType::HPIN); }
size_t HPin::GetSize() const { return sizeof(HPin); }

}  // namespace db
