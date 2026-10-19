#include "DataModel/DFG/DFGGraph.h"

#include "DataModel/DFG/DFGEdge.h"
#include "DataModel/DFG/DFGNode.h"
#include "DataModel/DFG/DFGPort.h"

namespace db {

size_t DFGGraph::GetSize() const { return sizeof(DFGGraph); }

void DFGGraph::AddNode(DFGNode* node) {
    if (node == nullptr) {
        return;
    }
    node->m_parent_graph_id = GetObjectId();
    node->m_next_node_id    = m_node_head_id;
    m_node_head_id          = node->GetObjectId();
}

void DFGGraph::AddEdge(DFGEdge* edge) {
    if (edge == nullptr) {
        return;
    }
    edge->m_parent_graph_id = GetObjectId();
    edge->m_next_edge_id    = m_edge_head_id;
    m_edge_head_id          = edge->GetObjectId();
}

void DFGGraph::AddPort(DFGPort* port) {
    if (port == nullptr) {
        return;
    }
    port->m_owner_graph_id = GetObjectId();
    port->m_next_port_id   = m_port_head_id;
    m_port_head_id         = port->GetObjectId();
}

}  // namespace db
