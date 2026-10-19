#ifndef DATAMODEL_DFG_EDGE_H_
#define DATAMODEL_DFG_EDGE_H_

#include "DataModel/DFG/Enums.h"
#include "DataModel/DFG/Type.h"
#include "DataModel/Object.h"

namespace db {

class DFGGraph;
class DFGMemoryMgr;
class DFGPin;

// Directed edge between two DFG pins. A net with fanout is represented by
// multiple edges, optionally sharing a Signal node/endpoint.
class DFGEdge : public Object {
    friend class DFGGraph;
    friend class DFGMemoryMgr;

public:
    DFGEdge()           = default;
    ~DFGEdge() override = default;

    size_t GetSize() const override;

    ObjectId    GetSrcPinId() const { return m_src_pin_id; }
    ObjectId    GetDstPinId() const { return m_dst_pin_id; }
    DFGEdgeKind GetKind() const { return m_kind; }
    DFGType     GetType() const { return m_value_type; }
    ObjectId    GetNextEdge() const { return m_next_edge_id; }
    ObjectId    GetParentGraphId() const { return m_parent_graph_id; }

    void SetKind(DFGEdgeKind kind) { m_kind = kind; }
    void SetType(const DFGType& type) { m_value_type = type; }
    void Connect(DFGPin* src, DFGPin* dst);

private:
    ObjectId    m_src_pin_id{kInvalidId};
    ObjectId    m_dst_pin_id{kInvalidId};
    DFGEdgeKind m_kind{DFGEdgeKind::Data};
    DFGType     m_value_type{};
    ObjectId    m_parent_graph_id{kInvalidId};
    ObjectId    m_next_edge_id{kInvalidId};
    ObjectId    m_next_src_edge_id{kInvalidId};
    ObjectId    m_next_dst_edge_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DFG_EDGE_H_
