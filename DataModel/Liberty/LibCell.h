#ifndef DATAMODEL_LIBERTY_LIBCELL_H_
#define DATAMODEL_LIBERTY_LIBCELL_H_
#include <cstdint>
#include <vector>
#include "DataModel/Object.h"
namespace db {
using LibCellPortsArray = IdArray<ObjectId, LibPortsArrayId>;
class LibCellPortIterator;
class LibCell : public Object {
    friend class LibCellPortIterator;

public:
    LibCell()           = default;
    ~LibCell() override = default;
    size_t   GetSize() const override;
    uint32_t PortCount() const { return m_port_count; }
    const LibCellPortsArray& GetPortArray() const;
private:
    ObjectId        m_owner_lib{kInvalidId};
    ObjectId        m_index_in_lib{kInvalidId};
    LogicType       m_function_type{LogicType::COUNT};
    uint32_t        m_port_count{0};
    LibPortsArrayId m_ports;
};

class LibCellPortIterator : public Iterator<LibPort> {
public:
    LibCellPortIterator(LibCell* cell);
    ~LibCellPortIterator() override = default;
    LibPort* Next() override;
    bool     HasNext() const override { return m_index < m_ports_array.Size(); }

private:
    IdArray<ObjectId, LibPortsArrayId> m_ports_array;
    size_t                             m_index{0};
};

}  // namespace db

#endif  // DATAMODEL_LIBERTY_LIBCELL_H_