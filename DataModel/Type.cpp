#include "DataModel/Type.h"
#include <vector>

namespace db {
inline std::string GetTypeName(DMObjectType type) {
    static std::vector<const char*> type_name = {
#define X(name) #name,
        DMOBJTYPE_ENTRIES
#undef X
    };
    return type_name[static_cast<uint8_t>(type)];
}
}  // namespace db