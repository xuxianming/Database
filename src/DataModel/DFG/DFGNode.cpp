#include "DataModel/DFG/DFGNode.h"

#include "DataModel/DFG/DFGGraph.h"
#include "DataModel/DFG/DFGPin.h"
#include "DataModel/DFG/DFGPrim.h"
#include "DataModel/DFG/MemoryMgr.h"

namespace db {

size_t DFGNode::GetSize() const { return sizeof(DFGNode); }

DFGLink* DFGNode::GetLink() const {
    if (m_link_id == kInvalidId) {
        return nullptr;
    }
    if (m_link_kind == DFGLinkKind::Graph) {
        return DFGMemoryMgr::GetGraph(m_link_id);
    }
    return DFGMemoryMgr::GetPrim(m_link_id);
}

DFGGraph* DFGNode::GetGraph() const {
    return m_link_kind == DFGLinkKind::Graph ? DFGMemoryMgr::GetGraph(m_link_id) : nullptr;
}

DFGPrim* DFGNode::GetPrim() const {
    return m_link_kind == DFGLinkKind::Prim ? DFGMemoryMgr::GetPrim(m_link_id) : nullptr;
}

void DFGNode::SetGraph(DFGGraph* graph) {
    if (graph == nullptr) {
        m_link_id   = kInvalidId;
        m_link_kind = DFGLinkKind::Graph;
        return;
    }
    m_link_id   = graph->GetObjectId();
    m_link_kind = DFGLinkKind::Graph;
}

void DFGNode::SetPrim(DFGPrim* prim) {
    if (prim == nullptr) {
        m_link_id   = kInvalidId;
        m_link_kind = DFGLinkKind::Prim;
        return;
    }
    m_link_id   = prim->GetObjectId();
    m_link_kind = DFGLinkKind::Prim;
}

void DFGNode::AddPin(DFGPin* pin) {
    if (pin == nullptr) {
        return;
    }
    pin->m_owner_node_id = GetObjectId();
    pin->m_next_pin_id   = m_pin_head_id;
    m_pin_head_id        = pin->GetObjectId();
}

}  // namespace db
