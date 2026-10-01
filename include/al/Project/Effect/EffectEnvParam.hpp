#pragma once

#include <math/seadVector.h>

namespace al {
class NatureDirector;

class EffectEnvParam {
public:
    EffectEnvParam();

    sead::Vector3f mWindDir = sead::Vector3f::ez;
    bool mIsEnableRepulsion = false;
    s32 mRepulsionPosNum = 0;
    sead::Vector3f mRepulsionPos[4] = {sead::Vector3f::zero, sead::Vector3f::zero,
                                       sead::Vector3f::zero, sead::Vector3f::zero};
    NatureDirector* mNatureDirector = nullptr;
};

static_assert(sizeof(EffectEnvParam) == 0x50);
}  // namespace al
