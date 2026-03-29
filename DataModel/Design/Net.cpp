#include "DataModel/Design/Net.h"

namespace db {

Net::Net() { SetType(DMObjectType::NET); }
size_t Net::GetSize() const { return sizeof(Net); }

}  // namespace db
