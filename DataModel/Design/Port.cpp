
#include "DataModel/Design/Port.h"
namespace db {
Port::Port() { SetType(DMObjectType::PORT); }
size_t Port::GetSize() const { return sizeof(Port); }

}  // namespace db
