#ifndef DATAMODEL_ENUMS_H_
#define DATAMODEL_ENUMS_H_
#include "DataModel/Type.h"
namespace db {

#define ENUM_ELEMENT(x) x,
#define ENUM_STRING(x) #x,
#define DBENUM(name, base, elements)                            \
    enum class name : base { elements(ENUM_ELEMENT) };                \
    inline std::string ToString(name e) {                       \
        static const char* strings[] = {elements(ENUM_STRING)}; \
        return strings[static_cast<base>(e)];                   \
    }

// clang-format off

#define BLOCK(X) \
    X(DESIGN) \
    X(MODULE) \
    X(INST) \
    X(NET) \
    X(PIN)  \
    X(MODULE_PORT) \
    X(BUS_PORT) \
    X(BUS_NET) \
    X(HINST) \
    X(HNET) \
    X(HPIN) \
    X(DESIGNOBJ_BOUNDARY) \
    X(LIBERTYLIB) \
    X(LIBCELL) \
    X(LIBPORT) \
    X(LIBOBJECT_BOUNDARY) \
    X(MACRO) \
    X(PHYSICALOBJ_BOUNDARY) \
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


#define SIMPLE_LOGIC_BLOCK(x) \
  x(NOT) \
  x(POS) \
  x(NEG) \
  x(LOGIC_NOT) \
  x(SLICE) \
  x(LUT) \
  x(SOP) \
  x(AND) \
  x(OR) \
  x(XOR) \
  x(XNOR) \
  x(LOGIC_AND) \
  x(LOGIC_OR) \
  x(CONCAT) \
  x(MUX) \
  x(PMUX) \
  x(LCU) \
  x(TRIBUF) \
  x(LOGIC_0) \
  x(LOGIC_1) \
  x(LOGIC_X) \
  x(LOGIC_Z) \
  x(VirtualTieHigh) \
  x(VirtualTieLow) \
  x(BUF_GATE) \
  x(NOT_GATE) \
  x(AND_GATE) \
  x(NAND_GATE) \
  x(OR_GATE) \
  x(NOR_GATE) \
  x(XOR_GATE) \
  x(XNOR_GATE) \
  x(ANDNOT_GATE) \
  x(ORNOT_GATE) \
  x(MUX_GATE) \
  x(MUX4_GATE) \
  x(MUX8_GATE) \
  x(MUX16_GATE) \
  x(AOI3_GATE) \
  x(OAI3_GATE) \
  x(AOI4_GATE) \
  x(OAI4_GATE) \
  x(TBUF_GATE) \
  x(MUXF7_GATE) \
  x(MUXF8_GATE) \
  x(TRI_STATE) \
  x(LATCH_GATE) \
  x(INV_GNUWA) \
  x(BUFF_GNUWA) \
  x(AN2_GNUWA) \
  x(AN3_GNUWA) \
  x(AN4_GNUWA) \
  x(ND2_GNUWA) \
  x(ND3_GNUWA) \
  x(ND4_GNUWA) \
  x(OR2_GNUWA) \
  x(OR3_GNUWA) \
  x(OR4_GNUWA) \
  x(NR2_GNUWA) \
  x(NR3_GNUWA) \
  x(NR4_GNUWA) \
  x(XOR2_GNUWA) \
  x(XOR3_GNUWA) \
  x(XOR4_GNUWA) \
  x(XNR2_GNUWA) \
  x(XNR3_GNUWA) \
  x(XNR4_GNUWA) \
  x(IND2_GNUWA) \
  x(INR2_GNUWA) \
  x(AO21_GNUWA) \
  x(OA21_GNUWA) \
  x(AO22_GNUWA) \
  x(OA22_GNUWA) \
  x(AOI21_GNUWA) \
  x(OAI21_GNUWA) \
  x(AOI22_GNUWA) \
  x(OAI22_GNUWA) \
  x(AOI222_GNUWA) \
  x(AOI2222_GNUWA) \
  x(AO2222_GNUWA) \
  x(MAOI22_GNUWA) \
  x(MOAI22_GNUWA) \
  x(MUX2_GNUWA) \
  x(MUX2N_GNUWA) \
  x(MUX4_GNUWA) \
  x(MUX4N_GNUWA) \
  x(MUX8_GNUWA) \
  x(MUX8N_GNUWA) \
  x(HA1_GNUWA) \
  x(FA1_GNUWA) \
  x(FA1SN_GNUWA) \
  x(CMPE42_GNUWA) \
  x(XOR2AOI22_GNUWA) \
  x(XNR2AOI22_GNUWA) \
  x(ICG_P_GATE) \
  x(ICG_N_GATE) \
  x(ICG_P_TB_GATE) \
  x(ICG_N_TB_GATE) \
  x(ICG_P_TA_GATE) \
  x(ICG_N_TA_GATE) \
  x(ICG_P_TBO_GATE) \
  x(ICG_N_TBO_GATE) \
  x(ICG_P_TAO_GATE) \
  x(ICG_N_TAO_GATE) \
  x(LATCH) \
  x(COUNT)

DBENUM(LogicType, uint8_t, SIMPLE_LOGIC_BLOCK)
#undef SIMPLE_LOGIC_BLOCK

#define OPERATOR_BLOCK(x) \
  x(REDUCE_AND) \
  x(REDUCE_OR) \
  x(REDUCE_XOR) \
  x(REDUCE_XNOR) \
  x(REDUCE_BOOL) \
  x(SHL) \
  x(SHR) \
  x(SSHL) \
  x(SSHR) \
  x(SHIFT) \
  x(SHIFTX) \
  x(LT) \
  x(LE) \
  x(EQ) \
  x(NE) \
  x(EQX) \
  x(NEX) \
  x(GE) \
  x(GT) \
  x(ADD) \
  x(SUB) \
  x(MUL) \
  x(DIV) \
  x(MOD) \
  x(POW) \
  x(MACC) \
  x(ADDC) \
  x(FA) \
  x(TADD) \
  x(ALU) \
  x(NEWALU) \
  x(ALUCARRY) \
  x(CMPALU) \
  x(ALUMUX) \
  x(DSP) \
  x(RU_LRAM) \
  x(FCICO_GNUWA) \
  x(FCICON_GNUWA) \
  x(COUNT)

DBENUM(OperatorType, uint8_t, OPERATOR_BLOCK)
#undef OPERATOR_BLOCK

#define LOGIC_PORT_BLOCK(x) \
    x(A)                \
    x(B)                \
    x(C)                \
    x(D)                \
    x(Y)                \
    x(S)                \
    x(E)                \
    x(EN)               \
    x(CI)               \
    x(CO)               \
    x(Q)                \
    x(QN)               \
    x(clear)            \
    x(preset)           \
    x(data_in)          \
    x(enable)           \
    x(clock)            \
    x(next_state)       \
    x(synch_enable)     \
    x(synch_clear)      \
    x(synch_preset)     \
    x(synch_toggle)     \
    x(logic_0)          \
    x(logic_1)          \
    x(logic_x)          \
    x(logic_z)\
    x(UNKNOWN_PORT)

DBENUM(LogicPort, uint8_t, LOGIC_PORT_BLOCK)
#undef LOGIC_PORT_BLOCK

}

#endif  // DATAMODEL_ENUMS_H_