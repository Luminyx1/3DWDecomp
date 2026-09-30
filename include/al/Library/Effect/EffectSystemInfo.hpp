#pragma once

#include <basis/seadTypes.h>

namespace al {
class EffectDataBase;
class EffectSystem;
class PtclSystem;

class EffectSystemInfo {
public:
    EffectSystemInfo();

    EffectSystem* getEffectSystem() const;

    s32 _0;
    PtclSystem* mPtclSystem;
    EffectDataBase* mEffectDataBase;
    s32 _18;
};

static_assert(sizeof(EffectSystemInfo) == 0x20);
}  // namespace al
