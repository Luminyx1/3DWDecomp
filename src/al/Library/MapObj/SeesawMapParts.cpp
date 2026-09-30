#include "Library/MapObj/SeesawMapParts.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/MapObj/ChildStep.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Se/Function/SeFunction.hpp"

namespace {
using namespace al;

NERVE_ACTION_IMPL(SeesawMapParts, Wait)

NERVE_ACTIONS_MAKE_STRUCT(SeesawMapParts, Wait)

inline bool isGreaterThanOrEqualToZero(f32 value) {
    return value >= 0.0f;
}
}  // namespace

namespace al {
/**
 * Constructs a seesaw map part.
 * @param pName actor name
 */
SeesawMapParts::SeesawMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the seesaw and its child steps.
 * @param rInfo actor init info
 */
void SeesawMapParts::init(const ActorInitInfo& rInfo) {
    initNerveAction(this, "Wait", &NrvSeesawMapParts.collector, 0);
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, calcChildStepCount(rInfo));
    registerAreaHostMtx(this, rInfo);
    mStartQuat = getQuat(this);
    calcQuatSide(&mSide, mStartQuat);
    calcQuatFront(&mFront, mStartQuat);
    tryGetArg(&mMaxDegree, rInfo, "MaxDegree");
    tryGetArg(&mRotateAccelOn, rInfo, "RotateAccelOn");
    tryGetArg(&mRotateAccelOff, rInfo, "RotateAccelOff");
    createChildStep(rInfo, this, true);
    trySyncStageSwitchAppear(this);
}

/**
 * Adds the weight of actors touching the seesaw.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool SeesawMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (isMsgFloorTouch(pMsg)) {
        sead::Vector3f pos;

        if (isMySensor(pSelf, this)) {
            pos.set(getSensorPos(pOther));
        } else {
            pos.set(getActorTrans(pSelf));
        }

        f32 weight = isMsgEnemyFloorTouch(pMsg) ? 0.9f : 1.0f;

        if (!isGreaterThanOrEqualToZero((pos - getTrans(this)).dot(mFront))) {
            weight = -weight;
        }

        mWeight += weight;
        return true;
    }

    if (isMsgShowModel(pMsg)) {
        showModelIfHide(this);
        return true;
    }

    if (isMsgHideModel(pMsg)) {
        hideModelIfShow(this);
        return true;
    }

    return false;
}

/**
 * Tilts the seesaw towards the weight on it.
 */
void SeesawMapParts::exeWait() {
    if (mWeight > 0.0f) {
        mRemainingAccelOnFrames = mRemainingAccelOnFrames > 59 ? 60 : mRemainingAccelOnFrames + 1;
    } else if (mWeight < 0.0f) {
        mRemainingAccelOnFrames =
            mRemainingAccelOnFrames < -59 ? -60 : mRemainingAccelOnFrames - 1;
    } else if (mRemainingAccelOnFrames > 0) {
        mRemainingAccelOnFrames--;
    } else if (mRemainingAccelOnFrames < 0) {
        mRemainingAccelOnFrames++;
    }

    mWeight = 0.0f;

    if (mRemainingAccelOnFrames > 0) {
        mRotateSpeed += mRotateAccelOn;
    } else if (mRemainingAccelOnFrames < 0) {
        mRotateSpeed -= mRotateAccelOn;
    } else if (mRotateDegree >= 0.0f) {
        mRotateSpeed -= mRotateAccelOff;
    } else {
        mRotateSpeed += mRotateAccelOff;
    }

    mRotateSpeed *= 0.95f;
    mRotateDegree += mRotateSpeed;
    f32 rotateSpeed = sead::Mathf::abs(mRotateSpeed);

    if (sead::Mathf::abs(mRotateDegree) > mMaxDegree) {
        if (isSameSign(mRotateSpeed, mRotateDegree)) {
            mRotateSpeed *= -0.5f;
        }

        mRotateDegree = sead::Mathf::clamp(mRotateDegree, -mMaxDegree, mMaxDegree);

        if (rotateSpeed > 0.2f) {
            tryStartSeWithParam(this, "Stop", rotateSpeed, nullptr);
        }
    }

    if (rotateSpeed > 0.1f) {
        tryHoldSeWithParam(this, "Rotate", rotateSpeed, nullptr);
    }

    rotateQuatRadian(getQuatPtr(this), mStartQuat, mSide, sead::Mathf::deg2rad(mRotateDegree));
}
}  // namespace al
