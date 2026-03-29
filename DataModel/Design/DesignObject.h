#ifndef DATAMODEL_LIBERTY_DESIGNOBJECT_H_
#define DATAMODEL_LIBERTY_DESIGNOBJECT_H_

#include "DataModel/Design/MemeryMgr.h"
#include "DataModel/Object.h"
namespace db {
class DesignObject : public Object {
    friend DesignMemoryMgr;

public:
    DesignObject();
};
}  // namespace db

#endif