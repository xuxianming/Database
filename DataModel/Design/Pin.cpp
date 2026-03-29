
#include "DataModel/Design/Pin.h"

namespace db {
Pin::Pin() { SetType(DMObjectType::PIN); }
size_t Pin::GetSize() const { return sizeof(Pin); }

}  // namespace db
