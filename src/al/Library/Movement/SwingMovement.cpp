#include "Library/Movement/SwingMovement.hpp"

#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"

namespace {
using namespace al;

NERVE_DECL(SwingMovement, Move)
NERVE_DECL(SwingMovement, Stop)

NERVES_MAKE_NOSTRUCT(SwingMovement, Move, Stop)
}  // namespace

namespace al {

/**
 * Constructs a swing movement with default parameters.
 */
SwingMovement::SwingMovement() : NerveExecutor("スイング動作計算") {
    initNerve(&NrvSwingMovementMove, 0);
}

/**
 * Constructs a swing movement from placement parameters.
 * @param rInfo Placement information.
 */
SwingMovement::SwingMovement(const ActorInitInfo& rInfo) : NerveExecutor("スイング動作計算") {
    tryGetArg(&mSwingAngle, rInfo, "SwingAngle");
    tryGetArg(&mSwingCycle, rInfo, "SwingCycle");
    tryGetArg(&mDelayRate, rInfo, "DelayRate");
    tryGetArg(&mStopTime, rInfo, "StopTime");
    tryGetArg(&mOffsetRotate, rInfo, "OffsetRotate");

    mFrameInCycle = static_cast<s32>((static_cast<f32>(mDelayRate) - 25.0f) / 100.0f *
                                     static_cast<f32>(mSwingCycle));
    updateRotate();

    initNerve(&NrvSwingMovementMove, 0);
}

/**
 * Updates the current swing angle from the frame in the cycle.
 * @return Whether the swing reached one of its turning points.
 */
bool SwingMovement::updateRotate() {
    f32 degree = static_cast<f32>(mFrameInCycle) * 360.0f / static_cast<f32>(mSwingCycle);

    f32 swingAngle = sead::Mathf::abs(mSwingAngle);

    if (swingAngle < 180.0f) {
        mCurrentAngle =
            sead::Mathf::sin(sead::Mathf::deg2rad(degree)) * mSwingAngle + mOffsetRotate;

        return mFrameInCycle % mSwingCycle == mSwingCycle / 4 ||
               mFrameInCycle % mSwingCycle == 3 * mSwingCycle / 4;
    }

    f32 swingAngleSign = sgn(mSwingAngle);

    if (swingAngle < 360.0f) {
        f32 rad = sead::Mathf::deg2rad(modf(degree + 90.0f + 180.0f, 180.0f) - 90.0f);
        mCurrentAngle = swingAngleSign * sead::Mathf::sin(rad) * 180.0f + mOffsetRotate;
    } else {
        mCurrentAngle = swingAngleSign * wrapAngle(degree * 2) + mOffsetRotate;
    }

    return false;
}

/**
 * Advances the swing and pauses at its turning points.
 */
void SwingMovement::exeMove() {
    if (updateRotate()) {
        setNerve(this, &NrvSwingMovementStop);
    }

    mFrameInCycle = wrapValue(mFrameInCycle + 1, mSwingCycle);
}

/**
 * Waits at a turning point before swinging again.
 */
void SwingMovement::exeStop() {
    if (isGreaterEqualStep(this, mStopTime)) {
        setNerve(this, &NrvSwingMovementMove);
    }
}

/**
 * Checks whether the swing is on its left half.
 * @return Whether the cycle angle is in [90, 270).
 */
bool SwingMovement::isLeft() const {
    f32 angle = static_cast<f32>(mFrameInCycle) * 360.0f / static_cast<f32>(mSwingCycle);

    return 90.0f <= angle && angle < 270.0f;
}

/**
 * Checks whether the swing is paused.
 * @return Whether the swing is in its stop nerve.
 */
bool SwingMovement::isStop() const {
    return isNerve(this, &NrvSwingMovementStop);
}

}  // namespace al
