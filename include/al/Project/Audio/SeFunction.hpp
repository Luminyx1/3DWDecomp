#pragma once

#include <prim/seadSafeString.h>

namespace al {
class IUseAudioKeeper;
class MeInfo;

bool startSe(const IUseAudioKeeper*, const sead::SafeString&, MeInfo*);
bool tryHoldSeWithParam(const IUseAudioKeeper*, const sead::SafeString&, f32, MeInfo*);
bool isExistSePlayNameInUserInfo(const IUseAudioKeeper*, const char*);
}  // namespace al
