#include "DataModel/Design/ModulePort.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HNet.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Design/Net.h"
#include "DataModel/Design/Pin.h"
namespace db {
size_t db::ModulePort::GetSize() const { return size_t(); }

Module* ModulePort::GetModule() const {
    if (m_module_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModule(m_module_id);
}

Design* ModulePort::GetMasterDesign() const {
    Module* module = GetModule();
    if (module != nullptr) {
        return module->GetMasterDesign();
    }
    return nullptr;
}

Pin* ModulePort::GetIoPin() const {
    if (m_io_pin_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetPin(m_io_pin_id);
}

Net* ModulePort::GetNet() const {
    Pin* io_pin = GetIoPin();
    if (io_pin != nullptr) {
        return io_pin->GetNet();
    }
    return nullptr;
}

HNet* ModulePort::GetHNet() const {
    Pin* io_pin = GetIoPin();
    if (io_pin != nullptr) {
        return io_pin->GetHNet();
    }
    return nullptr;
}

}  // namespace db