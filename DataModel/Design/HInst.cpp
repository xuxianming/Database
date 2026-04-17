#include "DataModel/Design/HInst.h"
#include "HInst.h"

namespace db {

size_t HInst::GetSize() const { return sizeof(HInst); }

Module* HInst::GetModule() const {
    if (m_owner_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModule(m_owner_id);
}

HInst* HInst::GetHInst() const {
    if (m_owner_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHInst(m_owner_id);
}

}  // namespace db