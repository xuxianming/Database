#include "DataModel/DFG/DFGEdge.h"

#include "DataModel/DFG/DFGPin.h"

namespace db {

size_t DFGEdge::GetSize() const { return sizeof(DFGEdge); }

void DFGEdge::Connect(DFGPin* src, DFGPin* dst) {
    if (src == nullptr || dst == nullptr) {
        return;
    }
    m_src_pin_id = src->GetObjectId();
    m_dst_pin_id = dst->GetObjectId();

    m_next_src_edge_id      = src->m_edge_head_id;
    src->m_edge_head_id     = GetObjectId();

    m_next_dst_edge_id      = dst->m_edge_head_id;
    dst->m_edge_head_id     = GetObjectId();
}

}  // namespace db
