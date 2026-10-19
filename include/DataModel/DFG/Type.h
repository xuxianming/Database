#ifndef DATAMODEL_DFG_TYPE_H_
#define DATAMODEL_DFG_TYPE_H_

#include <cstdint>

#include "DataModel/Type.h"

namespace db {

// DFGObjectId uses the same stable ObjectId space as the rest of DataModel.
using DFGObjectId = ObjectId;

// Value type carried by DFG pins and edges. This is separate from the DFG
// object-kind enums (DFGGraphType/DFGPrimType).
struct DFGType {
    uint32_t width{1};
    bool     isSigned{false};
    bool     isFourState{false};

    bool operator==(const DFGType& other) const {
        return width == other.width && isSigned == other.isSigned &&
               isFourState == other.isFourState;
    }
};

}  // namespace db

#endif  // DATAMODEL_DFG_TYPE_H_
