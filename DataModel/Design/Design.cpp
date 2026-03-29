#include "DataModel/Design/Design.h"

namespace db {
Design::Design() { SetType(DMObjectType::DESIGN); }
size_t Design::GetSize() const { return sizeof(Design); }
}  // namespace db