#include "DataModel/DFG/MemoryMgr.h"

#include "BasePlatfrom/ObjectTable.h"
#include "DataModel/DFG/DFGEdge.h"
#include "DataModel/DFG/DFGGraph.h"
#include "DataModel/DFG/DFGNode.h"
#include "DataModel/DFG/DFGPin.h"
#include "DataModel/DFG/DFGPort.h"
#include "DataModel/DFG/DFGPrim.h"
#include "DataModel/Enums.h"

namespace db {

namespace {
ObjectTable<DFGGraph, ObjectId, ObjectIndexBits> dfg_graph_table(
    static_cast<uint8_t>(DMObjectType::DFG_GRAPH));
ObjectTable<DFGPrim, ObjectId, ObjectIndexBits> dfg_prim_table(
    static_cast<uint8_t>(DMObjectType::DFG_PRIM));
ObjectTable<DFGNode, ObjectId, ObjectIndexBits> dfg_node_table(
    static_cast<uint8_t>(DMObjectType::DFG_NODE));
ObjectTable<DFGPort, ObjectId, ObjectIndexBits> dfg_port_table(
    static_cast<uint8_t>(DMObjectType::DFG_PORT));
ObjectTable<DFGPin, ObjectId, ObjectIndexBits> dfg_pin_table(
    static_cast<uint8_t>(DMObjectType::DFG_PIN));
ObjectTable<DFGEdge, ObjectId, ObjectIndexBits> dfg_edge_table(
    static_cast<uint8_t>(DMObjectType::DFG_EDGE));

}

void DFGMemoryMgr::UnlinkGraphNode(DFGGraph* graph, ObjectId node_id) {
    if (graph == nullptr) {
        return;
    }
    if (graph->m_node_head_id == node_id) {
        DFGNode* node = dfg_node_table.Pointer(node_id);
        graph->m_node_head_id = node != nullptr ? node->m_next_node_id : kInvalidId;
        return;
    }
    ObjectId cur = graph->m_node_head_id;
    while (cur != kInvalidId) {
        DFGNode* node = dfg_node_table.Pointer(cur);
        if (node == nullptr) {
            break;
        }
        if (node->m_next_node_id == node_id) {
            DFGNode* target = dfg_node_table.Pointer(node_id);
            node->m_next_node_id = target != nullptr ? target->m_next_node_id : kInvalidId;
            return;
        }
        cur = node->m_next_node_id;
    }
}

void DFGMemoryMgr::UnlinkGraphEdge(DFGGraph* graph, ObjectId edge_id) {
    if (graph == nullptr) {
        return;
    }
    if (graph->m_edge_head_id == edge_id) {
        DFGEdge* edge = dfg_edge_table.Pointer(edge_id);
        graph->m_edge_head_id = edge != nullptr ? edge->m_next_edge_id : kInvalidId;
        return;
    }
    ObjectId cur = graph->m_edge_head_id;
    while (cur != kInvalidId) {
        DFGEdge* edge = dfg_edge_table.Pointer(cur);
        if (edge == nullptr) {
            break;
        }
        if (edge->m_next_edge_id == edge_id) {
            DFGEdge* target = dfg_edge_table.Pointer(edge_id);
            edge->m_next_edge_id = target != nullptr ? target->m_next_edge_id : kInvalidId;
            return;
        }
        cur = edge->m_next_edge_id;
    }
}

void DFGMemoryMgr::UnlinkGraphPort(DFGGraph* graph, ObjectId port_id) {
    if (graph == nullptr) {
        return;
    }
    if (graph->m_port_head_id == port_id) {
        DFGPort* port = dfg_port_table.Pointer(port_id);
        graph->m_port_head_id = port != nullptr ? port->m_next_port_id : kInvalidId;
        return;
    }
    ObjectId cur = graph->m_port_head_id;
    while (cur != kInvalidId) {
        DFGPort* port = dfg_port_table.Pointer(cur);
        if (port == nullptr) {
            break;
        }
        if (port->m_next_port_id == port_id) {
            DFGPort* target = dfg_port_table.Pointer(port_id);
            port->m_next_port_id = target != nullptr ? target->m_next_port_id : kInvalidId;
            return;
        }
        cur = port->m_next_port_id;
    }
}

void DFGMemoryMgr::UnlinkEdgeFromPins(DFGEdge* edge) {
    if (edge == nullptr) {
        return;
    }
    if (DFGPin* src = dfg_pin_table.Pointer(edge->m_src_pin_id); src != nullptr) {
        if (src->m_edge_head_id == edge->GetObjectId()) {
            src->m_edge_head_id = edge->m_next_src_edge_id;
        } else {
            ObjectId cur = src->m_edge_head_id;
            while (cur != kInvalidId) {
                DFGEdge* cur_edge = dfg_edge_table.Pointer(cur);
                if (cur_edge == nullptr) {
                    break;
                }
                if (cur_edge->m_next_src_edge_id == edge->GetObjectId()) {
                    cur_edge->m_next_src_edge_id = edge->m_next_src_edge_id;
                    break;
                }
                cur = cur_edge->m_next_src_edge_id;
            }
        }
    }
    if (DFGPin* dst = dfg_pin_table.Pointer(edge->m_dst_pin_id); dst != nullptr) {
        if (dst->m_edge_head_id == edge->GetObjectId()) {
            dst->m_edge_head_id = edge->m_next_dst_edge_id;
        } else {
            ObjectId cur = dst->m_edge_head_id;
            while (cur != kInvalidId) {
                DFGEdge* cur_edge = dfg_edge_table.Pointer(cur);
                if (cur_edge == nullptr) {
                    break;
                }
                if (cur_edge->m_next_dst_edge_id == edge->GetObjectId()) {
                    cur_edge->m_next_dst_edge_id = edge->m_next_dst_edge_id;
                    break;
                }
                cur = cur_edge->m_next_dst_edge_id;
            }
        }
    }
}

DFGGraph* DFGMemoryMgr::CreateGraph(DFGGraphType type) {
    auto id = dfg_graph_table.GetNextObjId();
    auto* graph = dfg_graph_table.Make();
    graph->SetObjectId(id);
    graph->m_type       = DMObjectType::DFG_GRAPH;
    graph->m_graph_type = type;
    return graph;
}

DFGPrim* DFGMemoryMgr::CreatePrim(DFGPrimType type, DFGOperatorType op) {
    auto id = dfg_prim_table.GetNextObjId();
    auto* prim = dfg_prim_table.Make();
    prim->SetObjectId(id);
    prim->m_type        = DMObjectType::DFG_PRIM;
    prim->m_prim_type   = type;
    prim->m_operator_type = op;
    return prim;
}

DFGNode* DFGMemoryMgr::CreateNode() {
    auto id = dfg_node_table.GetNextObjId();
    auto* node = dfg_node_table.Make();
    node->SetObjectId(id);
    node->m_type = DMObjectType::DFG_NODE;
    return node;
}

DFGPort* DFGMemoryMgr::CreatePort(DFGPinDirection direction, const DFGType& type) {
    auto id = dfg_port_table.GetNextObjId();
    auto* port = dfg_port_table.Make();
    port->SetObjectId(id);
    port->m_type        = DMObjectType::DFG_PORT;
    port->m_direction   = direction;
    port->m_value_type  = type;

    // A DFGPort owns its io pin, analogous to ModulePort::ioPin.
    auto* io_pin = CreatePin(direction, type);
    if (io_pin != nullptr) {
        port->m_io_pin_id      = io_pin->GetObjectId();
        io_pin->m_owner_port_id = port->GetObjectId();
    }
    return port;
}

DFGPin* DFGMemoryMgr::CreatePin(DFGPinDirection direction, const DFGType& type) {
    auto id = dfg_pin_table.GetNextObjId();
    auto* pin = dfg_pin_table.Make();
    pin->SetObjectId(id);
    pin->m_type      = DMObjectType::DFG_PIN;
    pin->m_direction = direction;
    pin->m_value_type = type;
    return pin;
}

DFGEdge* DFGMemoryMgr::CreateEdge(DFGEdgeKind kind, const DFGType& type) {
    auto id = dfg_edge_table.GetNextObjId();
    auto* edge = dfg_edge_table.Make();
    edge->SetObjectId(id);
    edge->m_type = DMObjectType::DFG_EDGE;
    edge->m_kind = kind;
    edge->m_value_type = type;
    return edge;
}

DFGGraph* DFGMemoryMgr::GetGraph(ObjectId id) { return dfg_graph_table.IsLive(id) ? dfg_graph_table.Pointer(id) : nullptr; }
DFGPrim*  DFGMemoryMgr::GetPrim(ObjectId id) { return dfg_prim_table.IsLive(id) ? dfg_prim_table.Pointer(id) : nullptr; }
DFGNode*  DFGMemoryMgr::GetNode(ObjectId id) { return dfg_node_table.IsLive(id) ? dfg_node_table.Pointer(id) : nullptr; }
DFGPort*  DFGMemoryMgr::GetPort(ObjectId id) { return dfg_port_table.IsLive(id) ? dfg_port_table.Pointer(id) : nullptr; }
DFGPin*   DFGMemoryMgr::GetPin(ObjectId id) { return dfg_pin_table.IsLive(id) ? dfg_pin_table.Pointer(id) : nullptr; }
DFGEdge*  DFGMemoryMgr::GetEdge(ObjectId id) { return dfg_edge_table.IsLive(id) ? dfg_edge_table.Pointer(id) : nullptr; }

void DFGMemoryMgr::DestroyGraph(ObjectId id) { dfg_graph_table.Destroy(id); }
void DFGMemoryMgr::DestroyPrim(ObjectId id) { dfg_prim_table.Destroy(id); }

void DFGMemoryMgr::DestroyPin(ObjectId id) {
    DFGPin* pin = dfg_pin_table.Pointer(id);
    if (pin != nullptr) {
        if (DFGNode* node = dfg_node_table.Pointer(pin->m_owner_node_id); node != nullptr) {
            if (node->m_pin_head_id == id) {
                node->m_pin_head_id = pin->m_next_pin_id;
            } else {
                ObjectId cur = node->m_pin_head_id;
                while (cur != kInvalidId) {
                    DFGPin* cur_pin = dfg_pin_table.Pointer(cur);
                    if (cur_pin == nullptr) {
                        break;
                    }
                    if (cur_pin->m_next_pin_id == id) {
                        cur_pin->m_next_pin_id = pin->m_next_pin_id;
                        break;
                    }
                    cur = cur_pin->m_next_pin_id;
                }
            }
        }
    }
    dfg_pin_table.Destroy(id);
}

void DFGMemoryMgr::DestroyPort(ObjectId id) {
    DFGPort* port = dfg_port_table.Pointer(id);
    if (port == nullptr) {
        return;
    }
    if (DFGGraph* graph = dfg_graph_table.Pointer(port->m_owner_graph_id); graph != nullptr) {
        UnlinkGraphPort(graph, id);
    }
    if (port->m_io_pin_id != kInvalidId) {
        DestroyPin(port->m_io_pin_id);
    }
    dfg_port_table.Destroy(id);
}

void DFGMemoryMgr::DestroyNode(ObjectId id) {
    DFGNode* node = dfg_node_table.Pointer(id);
    if (node == nullptr) {
        return;
    }
    if (DFGGraph* graph = dfg_graph_table.Pointer(node->m_parent_graph_id); graph != nullptr) {
        UnlinkGraphNode(graph, id);
    }
    ObjectId pin_id = node->m_pin_head_id;
    while (pin_id != kInvalidId) {
        DFGPin* pin = dfg_pin_table.Pointer(pin_id);
        ObjectId next = pin != nullptr ? pin->m_next_pin_id : kInvalidId;
        DestroyPin(pin_id);
        pin_id = next;
    }
    dfg_node_table.Destroy(id);
}

void DFGMemoryMgr::DestroyEdge(ObjectId id) {
    DFGEdge* edge = dfg_edge_table.Pointer(id);
    if (edge == nullptr) {
        return;
    }
    if (DFGGraph* graph = dfg_graph_table.Pointer(edge->m_parent_graph_id); graph != nullptr) {
        UnlinkGraphEdge(graph, id);
    }
    UnlinkEdgeFromPins(edge);
    dfg_edge_table.Destroy(id);
}

void DFGMemoryMgr::DestroyGraphTree(ObjectId graph_id) {
    DFGGraph* graph = dfg_graph_table.Pointer(graph_id);
    if (graph == nullptr) {
        return;
    }

    // Destroy edges before pins/nodes, then nodes/pins, then the graph.
    ObjectId edge_id = graph->m_edge_head_id;
    while (edge_id != kInvalidId) {
        DFGEdge* edge = dfg_edge_table.Pointer(edge_id);
        ObjectId next = edge != nullptr ? edge->m_next_edge_id : kInvalidId;
        DestroyEdge(edge_id);
        edge_id = next;
    }

    ObjectId node_id = graph->m_node_head_id;
    while (node_id != kInvalidId) {
        DFGNode* node = dfg_node_table.Pointer(node_id);
        ObjectId next = node != nullptr ? node->m_next_node_id : kInvalidId;
        DestroyNode(node_id);
        node_id = next;
    }

    ObjectId port_id = graph->m_port_head_id;
    while (port_id != kInvalidId) {
        DFGPort* port = dfg_port_table.Pointer(port_id);
        ObjectId next = port != nullptr ? port->m_next_port_id : kInvalidId;
        DestroyPort(port_id);
        port_id = next;
    }

    dfg_graph_table.Destroy(graph_id);
}

bool DFGMemoryMgr::SaveAll() {
    bool success = true;
    success &= dfg_graph_table.Save();
    success &= dfg_prim_table.Save();
    success &= dfg_node_table.Save();
    success &= dfg_port_table.Save();
    success &= dfg_pin_table.Save();
    success &= dfg_edge_table.Save();
    return success;
}

bool DFGMemoryMgr::LoadAll() {
    bool success = true;
    success &= dfg_graph_table.Load();
    success &= dfg_prim_table.Load();
    success &= dfg_node_table.Load();
    success &= dfg_port_table.Load();
    success &= dfg_pin_table.Load();
    success &= dfg_edge_table.Load();
    return success;
}

void DFGMemoryMgr::Clear() {
    dfg_edge_table.Clear();
    dfg_port_table.Clear();
    dfg_pin_table.Clear();
    dfg_node_table.Clear();
    dfg_prim_table.Clear();
    dfg_graph_table.Clear();
}

void DFGMemoryMgr::PrintStats() {}

}  // namespace db
