#ifndef DATAMODEL_LIBERTY_LIBCELL_H_
#define DATAMODEL_LIBERTY_LIBCELL_H_
#include "DataModel/Liberty/LibertyObject.h"
namespace db {
class LibCell : public LibertyObject {
    uint32_t m_libports_id;
};
}  // namespace db

#endif