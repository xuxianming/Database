#include "DataModel/Design/Design.h"
#include "Design.h"

namespace db {
size_t   Design::GetSize() const { return sizeof(Design); }
uint32_t db::Design::GetMaxFanout() const { return 32; }
}  // namespace db