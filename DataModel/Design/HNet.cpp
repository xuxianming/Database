#include "DataModel/Design/HNet.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Design/Net.h"

namespace db {
size_t HNet::GetSize() const { return size_t(); }
Net*   HNet::GetNet() const {
    if (m_net_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetNet(m_net_id);
}

HInst* HNet::GetHInst() const {
    if (m_hinst_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHInst(m_hinst_id);
}

Module* HNet::GetModule() const {
    if (m_module_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModule(m_module_id);
}

Design* HNet::GetMasterDesign() const {
    Module* module = GetModule();
    if (module != nullptr) {
        return module->GetMasterDesign();
    }
    return nullptr;
}

void HNet::AttachToNet(Net* net) {
    if (net == nullptr) {
        return;
    }
    AttachToNet(net->GetObjectId());
}
void HNet::AttachToNet(ObjectId net_id) {
    if (net_id == m_net_id) {
        return;
    }
    m_net_id = net_id;
}
}  // namespace db
