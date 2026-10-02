#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.hpp"

namespace al {
class CameraPoser_RS;

class CameraVerticalAbsorber2DGalaxy : public NerveExecutor {
public:
    CameraVerticalAbsorber2DGalaxy();

    void start(const CameraPoser_RS* pPoser);
    void update(const CameraPoser_RS* pPoser);
    void applyLimit(sead::Vector3f* pPos) const;
    void exeNone();
    void exeGround();
    void exeLimit();
    void exeLimitOver();
    void exeLimitAfter();

private:
    void updateTargetInfo(const CameraPoser_RS* pPoser);

    sead::Vector3f mTargetTrans = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mTargetGravity = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mTargetUp = {0.0f, 0.0f, 0.0f};
    bool mIsTargetCollideGround = false;
    sead::Vector3f mPrevTargetTrans = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mPrevTargetGravity = {0.0f, 0.0f, 0.0f};
    f32 mLimitHeight = 0.0f;
    sead::Vector3f mLimitDir = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mLimitStartUp = {0.0f, 0.0f, 0.0f};
    f32 mLimitTargetHeight = 0.0f;
};

static_assert(sizeof(CameraVerticalAbsorber2DGalaxy) == 0x70);

}  // namespace al
