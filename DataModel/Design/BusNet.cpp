
#include "DataModel/Design/BusNet.h"
namespace db {
BusNet::BusNet() { SetType(DMObjectType::BUS_NET); }
size_t BusNet::GetSize() const { return sizeof(BusNet); }

}  // namespace db
