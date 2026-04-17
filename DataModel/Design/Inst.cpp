
#include "DataModel/Design/Inst.h"
#include "Inst.h"

namespace db {
size_t  Inst::GetSize() const { return sizeof(Inst); }
Module* Inst::GetModule() const {
    if (m_owner_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModule(m_owner_id);
}

HInst* Inst::GetHInst() const {
    if (m_owner_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHInst(m_owner_id);
}

}  // namespace db