#ifndef DATAMODEL_LIBERTY_LIBCELL_H_
#define DATAMODEL_LIBERTY_LIBCELL_H_
#include <cstdint>
#include "DataModel/Liberty/LibertyObject.h"
namespace db {
class LibCell : public LibertyObject {
public:
    LibCell();
    ~LibCell() override;

private:
    uint32_t m_owner_lib;
    uint32_t m_libports_id;
};
}  // namespace db

#endif