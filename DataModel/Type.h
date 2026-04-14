#ifndef DATABASE_TYPE_H_
#define DATABASE_TYPE_H_
#include <cstdint>
#include <string>
namespace db {

using BusBitId                     = uint16_t;
using BusBitRangeType              = int16_t;
using ObjectEnumType               = uint8_t;
constexpr uint64_t kInvalidId      = 0;
constexpr uint8_t  ObjectIndexBits = 7;
#ifdef _UINT_64_
using ObjectId                     = uint64_t;
using FullObjectId                 = uint64_t;
using ObjectNameArrayId            = uint64_t;
using constexpr uint8_t TypeOffset = 56;
constexpr uint8_t       IdWidth    = 56;
constexpr ObjectId      IdMask     = 0x00FFFFFFFFFFFFFF;
#else
using ObjectId                = uint32_t;
using FullObjectId            = uint64_t;
using ObjectNameArrayId       = uint32_t;
constexpr uint8_t  TypeOffset = 56;
constexpr uint8_t  IdWidth    = 32;
constexpr ObjectId IdMask     = 0xFFFFFFFF;
#endif

#define ENUM_ELEMENT(x) x,
#define ENUM_STRING(x) #x,
#define DBENUM(name, base, elements)                            \
    enum name : base { elements(ENUM_ELEMENT) };                \
    inline std::string ToString(name e) {                       \
        static const char* strings[] = {elements(ENUM_STRING)}; \
        return strings[static_cast<base>(e)];                   \
    }

// clang-format off

#define BLOCK(X) \
    X(DESIGN) \
    X(INST) \
    X(NET) \
    X(PIN)  \
    X(PORT) \
    X(BUS_PORT) \
    X(BUS_NET) \
    X(HNET) \
    X(HINST) \
    X(HPIN)  \
    X(DESIGNOBJ_COUNT) \
    X(LIBERTYLIB) \
    X(LIBCELL) \
    X(LIBPORT) \
    X(LIBOBJECT_COUNT) \
    X(UNKNOWN_OBJ)

DBENUM(DMObjectType, ObjectEnumType, BLOCK)
#undef BLOCK

#define BLOCK(x) \
  x(INPUT)  \
  x(INOUT)  \
  x(OUTPUT) \
  x(INTERNAL)\
  x(NO_DIRECTION)
DBENUM(Direction, uint8_t, BLOCK)
#undef BLOCK

#define BLOCK(x) \
  x(COMBINATIONAL) \
  x(COMBINATIONAL_RISE) \
  x(COMBINATIONAL_FALL) \
  x(THREE_STATE_DISABLE) \
  x(THREE_STATE_DISABLE_RISE) \
  x(THREE_STATE_DISABLE_FALL) \
  x(THREE_STATE_ENABLE) \
  x(THREE_STATE_ENABLE_RISE) \
  x(THREE_STATE_ENABLE_FALL) \
  x(RISING_EDGE) \
  x(FALLING_EDGE) \
  x(PRESET) \
  x(CLEAR) \
  x(HOLD_RISING) \
  x(HOLD_FALLING) \
  x(SETUP_RISING) \
  x(SETUP_FALLING) \
  x(RECOVERY_RISING) \
  x(RECOVERY_FALLING) \
  x(SKEW_RISING) \
  x(SKEW_FALLING) \
  x(REMOVAL_RISING) \
  x(REMOVAL_FALLING) \
  x(MIN_PULSE_WIDTH) \
  x(MINIMUM_PERIOD) \
  x(MAX_CLOCK_TREE_PATH) \
  x(MIN_CLOCK_TREE_PATH) \
  x(NON_SEQ_SETUP_RISING) \
  x(NON_SEQ_SETUP_FALLING) \
  x(NON_SEQ_HOLD_RISING) \
  x(NON_SEQ_HOLD_FALLING) \
  x(NOCHANGE_HIGH_HIGH) \
  x(NOCHANGE_HIGH_LOW) \
  x(NOCHANGE_LOW_HIGH) \
  x(NOCHANGE_LOW_LOW) \
  x(UNKOWN_TIMINGTYPE)
DBENUM(TimingType, uint8_t, BLOCK)
#undef BLOCK

#define BLOCK(x) \
  x(CELL_DEGRADATION) \
  x(CELL_FALL) \
  x(CELL_RISE) \
  x(FALL_CONSTRAINT) \
  x(RISE_CONSTRAINT) \
  x(FALL_PROPAGATION) \
  x(RISE_PROPAGATION) \
  x(FALL_TRANSITION) \
  x(RISE_TRANSITION) \
  x(RETAINING_FALL) \
  x(RETAINING_RISE) \
  x(RETAIN_FALL_SLEW) \
  x(RETAIN_RISE_SLEW) \
  x(OCV_STD_DEV_CELL_RISE) \
  x(OCV_STD_DEV_CELL_FALL) \
  x(OCV_STD_DEV_RISE_TRANSITION) \
  x(OCV_STD_DEV_FALL_TRANSITION) \
  x(OCV_STD_DEV_RETAINING_RISE) \
  x(OCV_STD_DEV_RETAINING_FALL) \
  x(OCV_STD_DEV_RETAIN_RISE_SLEW) \
  x(OCV_STD_DEV_RETAIN_FALL_SLEW) \
  x(OCV_STD_DEV_RISE_CONSTRAINT) \
  x(OCV_STD_DEV_FALL_CONSTRAINT)\
  x(OCV_MEAN_SHIFT_CELL_RISE) \
  x(OCV_MEAN_SHIFT_CELL_FALL) \
  x(OCV_MEAN_SHIFT_RISE_TRANSITION) \
  x(OCV_MEAN_SHIFT_FALL_TRANSITION) \
  x(OCV_MEAN_SHIFT_RETAINING_RISE) \
  x(OCV_MEAN_SHIFT_RETAINING_FALL) \
  x(OCV_MEAN_SHIFT_RETAIN_RISE_SLEW) \
  x(OCV_MEAN_SHIFT_RETAIN_FALL_SLEW) \
  x(OCV_MEAN_SHIFT_RISE_CONSTRAINT) \
  x(OCV_MEAN_SHIFT_FALL_CONSTRAINT) \
  x(OCV_SKEWNESS_CELL_RISE) \
  x(OCV_SKEWNESS_CELL_FALL) \
  x(OCV_SKEWNESS_RISE_TRANSITION) \
  x(OCV_SKEWNESS_FALL_TRANSITION) \
  x(OCV_SKEWNESS_RETAINING_RISE) \
  x(OCV_SKEWNESS_RETAINING_FALL) \
  x(OCV_SKEWNESS_RETAIN_RISE_SLEW) \
  x(OCV_SKEWNESS_RETAIN_FALL_SLEW) \
  x(OCV_SKEWNESS_RISE_CONSTRAINT) \
  x(OCV_SKEWNESS_FALL_CONSTRAINT) \
  x(TIMING_GROUP_TYPE_MAX) 
DBENUM(TimingDataGroupType, uint8_t, BLOCK)
#undef BLOCK

#define BLOCK(x) \
  x(UNKNOWN_TREE) \
  x(BEST_CASE_TREE) \
  x(BALANCED_TREE) \
  x(WORST_CASE_TREE)
DBENUM(TreeEnumType, uint8_t, BLOCK)
#undef BLOCK


#define BLOCK(x) \
  x(SYN_SET) \
  x(ASYN_SET) \
  x(SYN_CLEAR) \
  x(ASYN_CLEAR) \
  x(ENABLE) \
  x(CP) \
  x(D) \
  x(Q) \
  x(QN) \
  x(UNKNOWN_FFPINTYPE)
DBENUM(FfPinType, uint8_t, BLOCK)
#undef BLOCK

// clang-format on
}  // namespace db
#endif