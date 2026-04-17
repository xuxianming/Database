#include "DataModel/Design/ArrayMgr.h"
#include <algorithm>
#include <cstring>
#include "Utils/Log.h"

namespace db {

// ArrayMgr Implementation
ArrayMgr::ArrayMgr(ObjectId design_id) : design_id_(design_id) {}

ArrayMgr::~ArrayMgr() {}

}  // namespace db