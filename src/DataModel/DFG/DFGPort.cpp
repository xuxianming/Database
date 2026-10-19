#include "DataModel/DFG/DFGPort.h"

#include "DataModel/DFG/DFGPin.h"
#include "DataModel/DFG/MemoryMgr.h"

namespace db {

size_t DFGPort::GetSize() const { return sizeof(DFGPort); }

DFGPin* DFGPort::GetIoPin() const {
    return m_io_pin_id == kInvalidId ? nullptr : DFGMemoryMgr::GetPin(m_io_pin_id);
}

void DFGPort::SetIoPin(DFGPin* pin) {
    if (pin == nullptr) {
        return;
    }
    m_io_pin_id      = pin->GetObjectId();
    pin->m_owner_port_id = GetObjectId();
}

}  // namespace db
