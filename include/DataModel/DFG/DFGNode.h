#ifndef DATAMODEL_DFG_NODE_H_
#define DATAMODEL_DFG_NODE_H_

#include "DataModel/DFG/Enums.h"
#include "DataModel/DFG/Type.h"
#include "DataModel/Object.h"

namespace db {

class DFGLink;
class DFGGraph;
class DFGPin;
class DFGPrim;

// A node is one occurrence in a graph. Its semantic type comes entirely from
// its DFGLink (DFGGraph or DFGPrim); there is no node-type field.
class DFGNode : public Object {
    friend class DFGMemoryMgr;
    friend class DFGGraph;

public:
    DFGNode()           = default;
    ~DFGNode() override = default;

    size_t GetSize() const override;

    ObjectId    GetLinkId() const { return m_link_id; }
    DFGLinkKind GetLinkKind() const { return m_link_kind; }
    DFGLink*    GetLink() const;
    DFGGraph*   GetGraph() const;
    DFGPrim*    GetPrim() const;
    ObjectId    GetParentGraphId() const { return m_parent_graph_id; }
    ObjectId    GetPinHead() const { return m_pin_head_id; }
    ObjectId    GetNextNode() const { return m_next_node_id; }

    void SetLink(ObjectId link_id, DFGLinkKind kind) {
        m_link_id   = link_id;
        m_link_kind = kind;
    }
    void SetGraph(DFGGraph* graph);
    void SetPrim(DFGPrim* prim);
    void AddPin(DFGPin* pin);

private:
    ObjectId    m_link_id{kInvalidId};
    DFGLinkKind m_link_kind{DFGLinkKind::Graph};
    ObjectId    m_parent_graph_id{kInvalidId};
    ObjectId    m_pin_head_id{kInvalidId};
    ObjectId    m_next_node_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DFG_NODE_H_
