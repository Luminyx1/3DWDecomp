#include "Project/Audio/System/AudioEffectFunction.hpp"

#include "Project/Base/StringUtil.hpp"

namespace {
const char* const cBusNames[] = {"AUX_BUS_A", "AUX_BUS_B", "AUX_BUS_C"};
}

namespace alAudioEffectFunction {
/**
 * Gets the id of an effect bus.
 * @param pName Bus name.
 * @return Bus id.
 */
s32 getBusId(const char* pName) {
    if (al::isEqualString(pName, "AUX_BUS_A")) {
        return 0;
    }

    if (al::isEqualString(pName, "AUX_BUS_B")) {
        return 1;
    }

    return al::isEqualString(pName, "AUX_BUS_C") ? 2 : 0;
}

/**
 * Gets the index of an effect bus.
 * @param pName Bus name.
 * @return Bus index, or -1.
 */
s32 getBusIndex(const char* pName) {
    if (al::isEqualString(pName, "AUX_BUS_A")) {
        return 0;
    }

    if (al::isEqualString(pName, "AUX_BUS_B")) {
        return 1;
    }

    return al::isEqualString(pName, "AUX_BUS_C") ? 2 : -1;
}

/**
 * Gets the name of an effect bus.
 * @param index Bus index.
 * @return Bus name, or nullptr.
 */
const char* getBusNameFromIndex(s32 index) {
    if (static_cast<u32>(index) <= 2) {
        return cBusNames[index];
    }

    return nullptr;
}

/**
 * Gets the id of an output device.
 * @param pName Output device name.
 * @return Output device id.
 */
s32 getOutDeviceId(const char* pName) {
    if (al::isEqualString(pName, "OUTPUT_DEVICE_MAIN")) {
        return 0;
    }

    al::isEqualString(pName, "OUTPUT_DEVICE_DRC");
    return 0;
}

/**
 * Gets the name of an output device.
 * @param index Output device index.
 * @return Output device name, or nullptr.
 */
const char* getOutDeviceNameFromIndex(s32 index) {
    if (index == 0) {
        return "OUTPUT_DEVICE_MAIN";
    }

    return index == 1 ? "OUTPUT_DEVICE_DRC" : nullptr;
}
}  // namespace alAudioEffectFunction
