#include <cstdint>
#ifndef DATAMODEL_DESIGN_ENUMS_H_
#define DATAMODEL_DESIGN_ENUMS_H_
namespace db {
enum class InstType : uint8_t {
    kDefault = 0,
    kStdCell,
    kMacro,
    kConstZero,
    kConstOne,
    kIo,
    kComb,
    kDef,
    kMem,
    kLatch,
    kTotalCount
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_ENUMS_H_