#ifndef DATAMODEL_DFG_PIN_H_
#define DATAMODEL_DFG_PIN_H_

#include "DataModel/DFG/Enums.h"
#include "DataModel/DFG/Type.h"
#include "DataModel/Object.h"

namespace db {

class DFGEdge;
class DFGMemoryMgr;
class DFGNode;
class DFGPort;

// A pin belongs to a node and is an endpoint for DFG edges.
class DFGPin : public Object {
    friend class DFGEdge;
    friend class DFGMemoryMgr;
    friend class DFGNode;
    friend class DFGPort;

public:
    DFGPin()           = default;
    ~DFGPin() override = default;

    size_t GetSize() const override;

    ObjectId        GetOwnerNodeId() const { return m_owner_node_id; }
    ObjectId        GetOwnerPortId() const { return m_owner_port_id; }
    DFGPinDirection GetDirection() const { return m_direction; }
    DFGType         GetType() const { return m_value_type; }
    ObjectId        GetEdgeHead() const { return m_edge_head_id; }
    ObjectId        GetNextPin() const { return m_next_pin_id; }

    void SetType(const DFGType& type) { m_value_type = type; }
    void SetDirection(DFGPinDirection direction) { m_direction = direction; }

private:
    ObjectId        m_owner_node_id{kInvalidId};
    ObjectId        m_owner_port_id{kInvalidId};
    DFGPinDirection m_direction{DFGPinDirection::Internal};
    DFGType         m_value_type{};
    ObjectId        m_edge_head_id{kInvalidId};
    ObjectId        m_next_pin_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DFG_PIN_H_
