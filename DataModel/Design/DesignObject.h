#ifndef DATAMODEL_LIBERTY_DESIGNOBJECT_H_
#define DATAMODEL_LIBERTY_DESIGNOBJECT_H_

#include "DataModel/Object.h"
namespace db {
class DesignObject : public Object {
public:
    DesignObject()           = default;
    ~DesignObject() override = default;
};
}  // namespace db

#endif