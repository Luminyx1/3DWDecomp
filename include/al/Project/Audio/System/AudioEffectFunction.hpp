#pragma once

#include <basis/seadTypes.h>

namespace alAudioEffectFunction {
s32 getBusId(const char* pName);
s32 getBusIndex(const char* pName);
const char* getBusNameFromIndex(s32 index);
s32 getOutDeviceId(const char* pName);
const char* getOutDeviceNameFromIndex(s32 index);
}  // namespace alAudioEffectFunction
