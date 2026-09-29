#pragma once

#include <prim/seadSafeString.h>

namespace al {
class IUseAudioKeeper;
class MeInfo;

bool startSe(const IUseAudioKeeper*, const sead::SafeString&, MeInfo*);
}  // namespace al
