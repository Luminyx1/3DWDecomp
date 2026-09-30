#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.hpp"

namespace al {
class ParabolicPath;

class ParabolicPathMovement : public HostStateBase<LiveActor> {
public:
    ParabolicPathMovement(LiveActor* pHost, bool isUseVelocity);

    void start(const sead::Vector3f& rTarget, f32 maxHeight, f32 speed);
    void setOrbitParams(const sead::Vector3f& rStart, const sead::Vector3f& rEnd, f32 maxHeight,
                        f32 speed);
    bool isReachedEnd() const;
    bool isOverTheTop() const;
    void exeMove();

private:
    ParabolicPath* mPath;
    sead::Vector3f _28 = sead::Vector3f::zero;
    s32 mMoveTime = 0;
    bool mIsUseVelocity;
};

static_assert(sizeof(ParabolicPathMovement) == 0x40);

}  // namespace al
