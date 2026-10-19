#ifndef DATAMODEL_DFG_PRIM_H_
#define DATAMODEL_DFG_PRIM_H_

#include "DataModel/DFG/DFGLink.h"
#include "DataModel/DFG/Enums.h"

namespace db {

// A primitive is a non-expandable leaf definition (gate, cell, DFF, blackbox).
class DFGPrim : public DFGLink {
    friend class DFGMemoryMgr;

public:
    DFGPrim()           = default;
    ~DFGPrim() override = default;

    size_t GetSize() const override;
    bool   IsExpandable() const override { return false; }

    DFGPrimType    GetPrimType() const { return m_prim_type; }
    DFGOperatorType GetOperatorType() const { return m_operator_type; }
    void SetOperatorType(DFGOperatorType type) { m_operator_type = type; }

private:
    DFGPrimType     m_prim_type{DFGPrimType::Gate};
    DFGOperatorType m_operator_type{DFGOperatorType::Unknown};
};

}  // namespace db

#endif  // DATAMODEL_DFG_PRIM_H_
