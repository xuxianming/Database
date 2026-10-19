#ifndef DATAMODEL_DFG_PORT_H_
#define DATAMODEL_DFG_PORT_H_

#include "DataModel/DFG/Enums.h"
#include "DataModel/DFG/Type.h"
#include "DataModel/Object.h"

namespace db {

class DFGGraph;
class DFGMemoryMgr;
class DFGPin;

// A graph-level port definition, analogous to ModulePort. It owns an io pin
// used by edges. Node pins are separate DFGPin objects.
class DFGPort : public Object {
    friend class DFGGraph;
    friend class DFGMemoryMgr;

public:
    DFGPort()           = default;
    ~DFGPort() override = default;

    size_t GetSize() const override;

    ObjectId        GetOwnerGraphId() const { return m_owner_graph_id; }
    ObjectId        GetIoPinId() const { return m_io_pin_id; }
    DFGPin*         GetIoPin() const;
    DFGPinDirection GetDirection() const { return m_direction; }
    DFGType         GetType() const { return m_value_type; }
    ObjectId        GetNextPort() const { return m_next_port_id; }

    void SetIoPin(DFGPin* pin);
    void SetType(const DFGType& type) { m_value_type = type; }
    void SetDirection(DFGPinDirection direction) { m_direction = direction; }

private:
    ObjectId        m_owner_graph_id{kInvalidId};
    ObjectId        m_io_pin_id{kInvalidId};
    DFGPinDirection m_direction{DFGPinDirection::Internal};
    DFGType         m_value_type{};
    ObjectId        m_next_port_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DFG_PORT_H_
