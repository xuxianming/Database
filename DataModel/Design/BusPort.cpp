
#include "DataModel/Design/BusPort.h"
namespace db {
BusPort::BusPort() { SetType(DMObjectType::BUS_PORT); }
size_t BusPort::GetSize() const { return sizeof(BusPort); }

}  // namespace db
