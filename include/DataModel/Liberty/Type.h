#include <cstdint>
#ifndef DATABASE_LIBERTY_TYPE_H_
#define DATABASE_LIBERTY_TYPE_H_
#include "BasePlatfrom/IdArrayMgr.h"
#include "BasePlatfrom/StringMgr.h"
#include "DataModel/Type.h"
namespace db {
using LibertyStringMgr  = StringMgr<ObjectNameId, 127, 128, 1>;
using LibertyIdArrayMgr = IdArrayMgr<ObjectId, ObjectId, 15>;
using LibPortCapsArrayId   = uint32_t;

}  // namespace db

#endif  // DATABASE_LIBERTY_TYPE_H_