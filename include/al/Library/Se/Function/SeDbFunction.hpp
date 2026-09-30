#pragma once

#include <basis/seadTypes.h>

namespace al {
class InOutParam;
class SeEmitterInfo;
class SeSoundSourceInfo;
class SeUserInfo;
template <typename T>
class AudioInfoList;

enum SeInputFunctionId : s32 {};
}  // namespace al

namespace alSeDbFunction {
s32 calcIsOneTimeInUserInfo(const al::SeUserInfo* pUserInfo);
al::SeInputFunctionId convertInputFunctionNameToId(const char* pName);
const char* convertInputFunctionIdToName(al::SeInputFunctionId id);
f32 convertSeInputParam(al::SeInputFunctionId id, f32 param);
f32 calcLeapValue(al::InOutParam* pParam, f32 value);
al::SeSoundSourceInfo* createDefaultSoundSourceInfo();
al::AudioInfoList<al::SeEmitterInfo>* createDefaultEmitterInfoList();
const char* createNameAreaAndCopy(const char* pName);
}  // namespace alSeDbFunction
