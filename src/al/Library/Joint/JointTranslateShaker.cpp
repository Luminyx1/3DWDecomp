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

/**
 * Registers a joint to be shaken along an axis.
 * @param jointIndex Joint index.
 * @param axis Local axis to translate along.
 */
void JointTranslateShaker::append(s32 jointIndex, JointTranslateAxis axis) {
    appendJointId(jointIndex);
    mShakeInfos.emplaceBack(jointIndex, axis);
}

/**
 * Registers a joint by name to be shaken along an axis.
 * @param pJointName Joint name.
 * @param axis Local axis to translate along.
 */
void JointTranslateShaker::append(const char* pJointName, JointTranslateAxis axis) {
    append(getJointIndexActor(pJointName), axis);
}

/**
 * Looks up a joint index in the owning actor's model.
 * @param pJointName Name of the joint.
 * @return Joint index.
 */
s32 JointTranslateShaker::getJointIndexActor(const char* pJointName) {
    return getJointIndex(mActor->getModelKeeper(), pJointName);
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

/**
 * Applies the current shake offset to a registered joint, advancing the shake once per frame.
 * @param jointIndex Index of the joint being calculated.
 * @param pMtx Joint matrix, modified in place.
 */
void JointTranslateShaker::calcJointCallback(s32 jointIndex, sead::Matrix34f* pMtx) {
    if (mStep < 0) {
        return;
    }

    s32 infoNum = mShakeInfos.size();

    if (mShakeInfos(0).jointIndex == jointIndex) {
        mStep++;

        if (mDuration <= mStep) {
            mStep = -1;
            return;
        }
    }

    // NOTE: volatile reproduces the original code keeping the axis on the stack.
    volatile JointTranslateAxis axis = JointTranslateAxis_None;
    bool isFound = false;

    for (s32 i = 0; i < infoNum; i++) {
        if (mShakeInfos(i).jointIndex == jointIndex) {
            JointTranslateAxis infoAxis = mShakeInfos(i).axis;
            axis = infoAxis;
            isFound = infoAxis != JointTranslateAxis_None;
            break;
        }
    }

    if (!isFound) {
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
    *pMtx = *pMtx * transMtx;
}

}  // namespace al
