#ifndef DATAMODEL_LIBERTY_LIBERTYLIB_H_
#define DATAMODEL_LIBERTY_LIBERTYLIB_H_

#include "DataModel/Liberty/LibertyObject.h"
namespace db {
class LibertyLib : public LibertyObject {
    uint32_t m_libcells_id;
};
}  // namespace db

#endif