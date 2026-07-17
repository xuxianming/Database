#ifndef DATAMODEL_LIBERTY_LIBERTYLIB_H_
#define DATAMODEL_LIBERTY_LIBERTYLIB_H_

#include <cstdint>
#include "DataModel/Object.h"
#include "DataModel/Type.h"

namespace db {
class LibertyLib : public Object {
public:
    LibertyLib()           = default;
    ~LibertyLib() override = default;
    size_t GetSize() const override;

private:
    LibCellsArrayId m_libcells_id{kInvalidId};
    char*           m_file_name{nullptr};
};
}  // namespace db

#endif