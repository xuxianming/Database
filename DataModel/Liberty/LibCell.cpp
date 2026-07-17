#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/LibPort.h"
#include "LibCell.h"

namespace db {
size_t LibCell::GetSize() const { return sizeof(LibCell); }

const LibCellPortsArray& LibCell::GetPortArray() const {
    static const LibCellPortsArray empty_array;
    if (m_ports == kInvalidId) {
        return empty_array;
    }
    auto* array_mgr = LibertyMemoryMgr::GetIdArrayMgr();
    if (array_mgr == nullptr) {
        return empty_array;
    }
    return array_mgr->Get(m_ports);
}

LibCellPortIterator::LibCellPortIterator(LibCell* cell) {
    if (cell == nullptr) {
        return;
    }
    m_ports_array = LibertyMemoryMgr::GetIdArrayMgr()->Get(cell->m_ports);
}

LibPort* LibCellPortIterator::Next() {
    if (m_index >= m_ports_array.Size()) {
        return nullptr;
    }
    auto port_id = m_ports_array[m_index];
    m_index++;
    return LibertyMemoryMgr::GetLibPort(port_id);
}

}  // namespace db