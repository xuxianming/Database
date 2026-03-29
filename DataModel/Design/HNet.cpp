#ifndef DATAMODEL_DESIGN_DESIGN_H_
#define DATAMODEL_DESIGN_DESIGN_H_
#include "DataModel/Design/HNet.h"
namespace db {
size_t HNet::GetSize() const { return sizeof(HNet); }

}  // namespace db

#endif