#ifndef DATAMODEL_LIBERTY_LIBCELL_H_
#define DATAMODEL_LIBERTY_LIBCELL_H_
#include <cstdint>
#include "DataModel/Liberty/LibertyObject.h"
namespace db {
class LibCell : public LibertyObject {
public:
    LibCell()           = default;
    ~LibCell() override = default;
    size_t GetSize() const override;

private:
    ObjectId       m_owner_lib;
    LibPortArrayId m_libports_id;
};
}  // namespace db

#endif