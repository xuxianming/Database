#ifndef DATAMODEL_DFG_LINK_H_
#define DATAMODEL_DFG_LINK_H_

#include "DataModel/DFG/Type.h"
#include "DataModel/Object.h"

namespace db {

class Design;
class DFGMemoryMgr;

// DFGLink is a reusable definition/reference target. A DFGGraph can be expanded
// into a body; a DFGPrim is a leaf definition with no body.
class DFGLink : public Object {
    friend class Design;
    friend class DFGMemoryMgr;

public:
    DFGLink()           = default;
    ~DFGLink() override = default;

    virtual bool IsExpandable() const = 0;
    ObjectId     GetDesignId() const { return m_design_id; }

protected:
    ObjectId m_design_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DFG_LINK_H_
