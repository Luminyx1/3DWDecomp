#include "Library/Joint/JointTranslateShaker.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Model/ModelShapeUtil.hpp"

namespace al {

/**
 * Constructs a shaker with room for a number of joints.
 * @param pActor Owning actor.
 * @param maxJoints Maximum number of joints.
 */
JointTranslateShaker::JointTranslateShaker(const LiveActor* pActor, s32 maxJoints)
    : mActor(pActor) {
    mShakeInfos.allocBuffer(maxJoints, nullptr);
}

void JointTranslateShaker::append(s32 jointIndex, JointTranslateAxis axis) {
    appendJointId(jointIndex);
    mShakeInfos.pushBack({jointIndex, axis});
}

void JointTranslateShaker::append(const char* pJointName, JointTranslateAxis axis) {
    append(getJointIndexActor(pJointName), axis);
}

/**
 * Looks up a joint index in the owning actor's model.
 * @param pJointName Name of the joint.
 * @return Joint index.
 */
s32 JointTranslateShaker::getJointIndexActor(const char* pJointName) {
    return getJointIndex(mActor->mModelKeeper, pJointName);
}

/**
 * Starts a shake if the duration is valid.
 * @param amplitude Shake amplitude.
 * @param duration Shake length in steps.
 * @param cycle Vibration cycle.
 * @param attenuation Vibration attenuation.
 */
void JointTranslateShaker::setShake(f32 amplitude, s32 duration, f32 cycle, f32 attenuation) {
    if (duration < 0) {
        return;
    }

    mAmplitude = amplitude;
    mStep = 0;
    mDuration = duration;
    mCycle = cycle;
    mAttenuation = attenuation;
}

void JointTranslateShaker::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    if (mStep < 0) {
        return;
    }

    if (mShakeInfos(0).jointIndex == jointIndex) {
        mStep++;

        if (mDuration <= mStep) {
            mStep = -1;
            return;
        }
    }

    JointTranslateAxis axis = JointTranslateAxis_None;

    for (s32 i = 0; i < mShakeInfos.size(); i++) {
        if (mShakeInfos(i).jointIndex == jointIndex) {
            axis = mShakeInfos(i).axis;
            break;
        }
    }

    if (axis == JointTranslateAxis_None) {
        return;
    }

    f32 value = calcConvergeVibrationValue(static_cast<f32>(mStep) / mDuration, mAmplitude, 0.0f,
                                           mCycle, mAttenuation);
    sead::Vector3f trans;

    if (axis == JointTranslateAxis_X) {
        trans.set(value, 0.0f, 0.0f);
    } else if (axis == JointTranslateAxis_Y) {
        trans.set(0.0f, value, 0.0f);
    } else if (axis == JointTranslateAxis_Z) {
        trans.set(0.0f, 0.0f, value);
    } else {
        return;
    }

    sead::Matrix34f transMtx;
    transMtx.makeT(trans);
    pMtx->setMul(*pMtx, transMtx);
}

}  // namespace al
