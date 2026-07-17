#ifndef DATABASE_TYPE_H_
#define DATABASE_TYPE_H_
#include <cstdint>
#include <string>

namespace db {

#ifdef _UINT_64_
using ObjectId                     = uint64_t;
ObjectId kInvalidId                = 0;
using FullObjectId                 = uint64_t;
using ObjectNameArrayId            = uint64_t;
using constexpr uint8_t TypeOffset = 56;
constexpr uint8_t       IdWidth    = 56;
constexpr ObjectId      IdMask     = 0x00FFFFFFFFFFFFFF;
#else
using ObjectId                = uint32_t;
constexpr ObjectId kInvalidId = 0;
using FullObjectId            = uint64_t;
using ObjectNameId            = uint32_t;
constexpr uint8_t  TypeOffset = 56;
constexpr uint8_t  IdWidth    = 32;
constexpr ObjectId IdMask     = 0xFFFFFFFF;
#endif
using BusBitId                     = int16_t;
constexpr BusBitId InvalidBusBitId = -1;
using BusBitRangeType              = int16_t;
using ObjectEnumType               = uint8_t;
constexpr uint8_t ObjectIndexBits  = 7;
using ObjArrayIdType               = ObjectId;
using InstPinsArrayId              = ObjArrayIdType;
using LibPortsArrayId              = ObjArrayIdType;
using LibCellsArrayId              = ObjArrayIdType;
template <typename OBJ>
class Iterator {
public:
    virtual ~Iterator() = default;
    virtual OBJ* Next() = 0;
    virtual bool HasNext() const { return false; }
};

// clang-format on
}  // namespace db
#endif