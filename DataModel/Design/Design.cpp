#include "DataModel/Design/Design.h"
#include "Design.h"

namespace db {
size_t   Design::GetSize() const { return sizeof(Design); }
uint32_t db::Design::GetMaxFanout() const { return 32; }

Module* Design::GetTopModule() const {
    if (m_top_module_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModule(m_top_module_id);
}

void Design::SetTopModule(ObjectId top_module_id) { m_top_module_id = top_module_id; }

}  // namespace db