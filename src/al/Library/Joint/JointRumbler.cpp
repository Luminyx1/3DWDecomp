#include "Library/Joint/JointRumbler.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Model/ModelShapeUtil.hpp"

namespace al {

/**
 * Constructs a rumbler that scales one joint with a decaying oscillation.
 * @param pActor Owning actor.
 * @param pJointName Name of the joint.
 * @param cycle Oscillation cycles over the duration.
 * @param power Oscillation strength.
 * @param duration Rumble length in steps.
 * @param startStep Step at which the rumble starts.
 */
JointRumbler::JointRumbler(const LiveActor* pActor, const char* pJointName, f32 cycle, f32 power,
                           s32 duration, s32 startStep)
    : mActor(pActor), mCycle(cycle), mPower(power), mDuration(duration), mStartStep(startStep),
      mStep(duration + startStep) {
    mJointIndex = getJointIndex(pActor->mModelKeeper, pJointName);
    appendJointId(mJointIndex);
    for (s32 i = 0; i < 3; i++) {
        mDetails[i].delay = 0;
        mDetails[i].rate = 1.0f;
    }
}

/**
 * Sets the per-axis delay and strength.
 * @param axis Axis to configure.
 * @param delay Delay in steps.
 * @param rate Strength multiplier.
 */
void JointRumbler::initDetails(EAxis axis, s32 delay, f32 rate) {
    mDetails[axis].delay = delay;
    mDetails[axis].rate = rate;
    if (mMaxDelay < delay) {
        mMaxDelay = delay;
    }
}

/**
 * Restarts the rumble.
 */
void JointRumbler::start() {
    mStep = 0;
    mScale.set(1.0f, 1.0f, 1.0f);
}

/**
 * Advances the rumble by one step and updates the scale.
 */
void JointRumbler::update() {
    if (!isActive()) {
        mScale.set(1.0f, 1.0f, 1.0f);
        return;
    }

    updateEach(&mScale.x, EAxis_X);
    updateEach(&mScale.y, EAxis_Y);
    updateEach(&mScale.z, EAxis_Z);
    mStep++;
}

/**
 * Calculates the scale factor for one axis.
 * @param pOut Receives the scale factor.
 * @param axis Axis to calculate.
 */
void JointRumbler::updateEach(f32* pOut, EAxis axis) {
    s32 step = mStep - mDetails[axis].delay;
    f32 value = 1.0f;
    if (step >= mStartStep && step < mDuration + mStartStep) {
        f32 rate = static_cast<f32>(step - mStartStep) / mDuration;
        value = (1.0f - rate) *
                    sead::Mathf::cos(rate * sead::Mathf::pi2() * mCycle + sead::Mathf::piHalf()) *
                    mPower * mDetails[axis].rate +
                1.0f;
    }
    *pOut = value;
}

/**
 * Stops the rumble and resets the scale.
 */
void JointRumbler::reset() {
    mStep = mDuration + mStartStep;
    mScale.set(1.0f, 1.0f, 1.0f);
}

/**
 * Applies the rumble scale to the controlled joint while active.
 * @param jointIndex Index of the joint being calculated.
 * @param pMtx Joint matrix to modify.
 */
void JointRumbler::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    if (!isActive() || mJointIndex != jointIndex) {
        return;
    }

    sead::Matrix34f scaleMtx;
    scaleMtx.makeS(mScale);
    pMtx->setMul(*pMtx, scaleMtx);
}

}  // namespace al
