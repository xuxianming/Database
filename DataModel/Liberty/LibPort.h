#ifndef DATAMODEL_LIBERTY_LIBPORT_H_
#define DATAMODEL_LIBERTY_LIBPORT_H_

#include <cstdint>
#include "DataModel/Liberty/LibertyObject.h"

namespace db {

class LibPort : public LibertyObject {
public:
    LibPort()           = default;
    ~LibPort() override = default;
    size_t GetSize() const override;
    enum BoolAttrName {
        kIsClock = 0,
        kIsClockGateClockPin,
        kClockGateEnablePin,
        kClockGateObsPin,
        kClockGateOutPin,
        kClockGateTestPin,
        kIsolationCellEnablePin,
        kIsolationCellDataPin,
        kIsPad,
        kIsAnalog,
        kInvertedOutput,
        kTestOutPutOnly,
        kIsUnBuffered,
        kHasPassGate,
        kIsLevelShifterDataPin,
        kIsLevelShifterEnablePin,
        kIsAliveDuringPowerUp,
        kIsAliveDuringPartialPowerDown,
        kIsTestOutputOnly,
        kClockIsolationCellClockPin,
        kSwitchPin,
        kIsUnConnected,
        kAlwaysOn,
        kIsIsolated,
        kClockGateOutputPin,
        kIsHysteresis,
        kIsPllReferencePin,
        kIsPllFeedbackPin,
        kIsPllOutputPin,
        kLibPinBoolMaxEnum,
    };
    enum FloatAttrName {
        kCapacitance = 0,
        kFallCapacitance,
        kRiseCapacitance,
        kMaxCapacitance,
        kMinCapacitance,
        kMaxFanout,
        kMinFanout,
        kMaxTransition,
        kMinPeriod,
        kMinPulseWidthHigh,
        kMinPulseWidthLow,
        kFanoutLoad,
        kMinTransition,
        kOutputSignalLevelLow,
        kOutputSignalLevelHigh,
        kInputSignalLevelLow,
        kInputSignalLevelHigh,
        kMaxInputDeltaOverdriveHigh,
        kMaxInputDeltaUnderdriveHigh,
        kDriveCurrent,
        kRiseCurrentSlopeBeforeThreshold,
        kRiseTimeBeforeThreshold,
        kRiseCurrentSlopeAfterThreshold,
        kRiseTimeAfterThreshold,
        kFallCurrentSlopeBeforeThreshold,
        kFallTimeBeforeThreshold,
        kFallCurrentSlopeAfterThreshold,
        kFallTimeAfterThreshold,
        kPullingResistance,
        kMinInputNoiseWidth,
        kMaxInputNoiseWidth,
        kLibPinFloatMaxEnum,
    };
    enum StringAttrName {
        kFunction = 0,
        kXFunction,
        kStateFunction,
        kInternalNode,
        kThreeState,
        kDriverType,
        kPowerDownFunction,
        kComplementaryPinName,
        kFaultModel,
        kDriverWaveForm,
        kDriverWaveFormRise,
        kDriverWaveFormFall,
        kInputSignalLevel,
        kOutputSignalLevel,
        kSwitchFunction,
        kIsolationEnableCondition,
        kInputVoltage,
        kOutputVoltage,
        kPullUpFunction,
        kPullDownFunction,
        kBusHoldFunction,
        kOpenDrainFunction,
        kOpenSourceFunction,
        kResistiveFunction,
        kResistive0Function,
        kResistive1Function,
        kDontFault,
        kLibPinStringMaxEnum,
    };
    enum RelatedPinOwnerType {
        kTimingRelatedPin = 0,
        kPowerRelatedPin,
        kElectromigrationRelatedPin,
        kRelatedPinOwnerTypeMax
    };

private:
    uint32_t m_owner_cell;
    uint32_t m_caps;
    uint8_t  m_dir;
};

};  // namespace db

#endif