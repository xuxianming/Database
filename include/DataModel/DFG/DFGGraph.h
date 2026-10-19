#ifndef DATAMODEL_DFG_GRAPH_H_
#define DATAMODEL_DFG_GRAPH_H_

#include "DataModel/DFG/DFGLink.h"
#include "DataModel/DFG/Enums.h"

namespace db {

class DFGEdge;
class DFGNode;
class DFGPort;

// A graph is a reusable, expandable definition. The attached Design root must
// be a graph with DFGGraphType::Module.
class DFGGraph : public DFGLink {
    friend class Design;
    friend class DFGMemoryMgr;

public:
    DFGGraph()           = default;
    ~DFGGraph() override = default;

    size_t GetSize() const override;
    bool   IsExpandable() const override { return true; }

    DFGGraphType GetGraphType() const { return m_graph_type; }
    ObjectId     GetNodeHead() const { return m_node_head_id; }
    ObjectId     GetEdgeHead() const { return m_edge_head_id; }
    ObjectId     GetPortHead() const { return m_port_head_id; }

    void AddNode(DFGNode* node);
    void AddEdge(DFGEdge* edge);
    void AddPort(DFGPort* port);

private:
    DFGGraphType m_graph_type{DFGGraphType::Module};
    ObjectId     m_node_head_id{kInvalidId};
    ObjectId     m_edge_head_id{kInvalidId};
    ObjectId     m_port_head_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DFG_GRAPH_H_
