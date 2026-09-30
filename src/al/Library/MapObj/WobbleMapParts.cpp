#include "Library/MapObj/WobbleMapParts.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
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

NERVE_ACTION_IMPL(WobbleMapParts, Wait)
NERVE_ACTION_IMPL(WobbleMapParts, Move)
NERVE_ACTION_IMPL(WobbleMapParts, AssistStop)

NERVE_ACTIONS_MAKE_STRUCT(WobbleMapParts, Wait, Move, AssistStop)
}  // namespace

namespace al {
/**
 * Constructs a wobbling map part.
 * @param pName actor name
 */
WobbleMapParts::WobbleMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the map part and its child steps.
 * @param rInfo actor init info
 */
void WobbleMapParts::init(const ActorInitInfo& rInfo) {
    initNerveAction(this, "Wait", &NrvWobbleMapParts.collector, 0);
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, calcChildStepCount(rInfo));
    registerAreaHostMtx(this, rInfo);
    mInitialQuat = getQuat(this);
    mCurrentQuat = mInitialQuat;
    calcQuatUp(&mInitialUp, mInitialQuat);
    mTargetUp.set(mInitialUp);
    tryGetArg(&mMaxRotate, rInfo, "MaxRotate");
    f32 soundScale = -1.0f;

    if (tryGetArg(&soundScale, rInfo, "RotateSoundScale") && soundScale > 0.0f) {
        mRotateSoundScale = soundScale;
    }

    createChildStep(rInfo, this, true);
    trySyncStageSwitchAppear(this);
}

/**
 * Tilts towards actors touching the map part.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool WobbleMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (isMsgTouchAssist(pMsg)) {
        mAssistStopTimer = 45;

        if (!isNerve(this, NrvWobbleMapParts.AssistStop.data())) {
            startNerveAction(this, "AssistStop");
        }

        return true;
    }

    if (isMsgFloorTouch(pMsg)) {
        sead::Vector3f pos;

        if (isMySensor(pSelf, this)) {
            pos.set(getSensorPos(pOther));
        } else {
            pos.set(getActorTrans(pSelf));
        }

        sead::Vector3f horizontal;
        sead::Vector3f up;
        calcQuatUp(&up, mCurrentQuat);
        verticalizeVec(&horizontal, up, pos - getTrans(this));
        f32 distance = horizontal.length();
        f32 rate = normalize(distance, 0.0f, 100.0f);

        if (isNearZero(distance)) {
            horizontal = sead::Vector3f::zero;
        } else {
            horizontal *= sead::Mathf::sin(sead::Mathf::deg2rad(rate * mMaxRotate)) / distance;
        }

        f32 cos = sead::Mathf::cos(sead::Mathf::deg2rad(rate * mMaxRotate));
        mTargetUp.set(cos * mInitialUp + horizontal);
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
 * Waits until the map part starts tilting.
 */
void WobbleMapParts::exeWait() {
    updateMove();

    if (mTiltSpeed * mRotateSoundScale > 0.1f) {
        startNerveAction(this, "Move");
    }
}

/**
 * Updates the tilt of the map part.
 */
void WobbleMapParts::updateMove() {
    sead::Vector3f horizontal;
    sead::Vector3f up;
    calcQuatUp(&up, mCurrentQuat);
    horizontal.setCross(up, mTargetUp);
    horizontal *= 180.0f / sead::Mathf::pi();
    limitLength(&horizontal, horizontal, mMaxRotate * (1.0f / 750.0f));
    mMoment = (mMoment + horizontal) * 0.92f;
    rotateQuatMomentDegree(&mCurrentQuat, mCurrentQuat, mMoment);
    sead::Vector3f currentUp;
    calcQuatUp(&currentUp, mCurrentQuat);
    sead::Vector3f newUp;
    bool isStop = turnVecToVecDegree(&newUp, mInitialUp, currentUp, mMaxRotate);
    turnQuatYDirRate(getQuatPtr(this), mInitialQuat, newUp, 1.0f);

    if (isStop) {
        mCurrentQuat = getQuat(this);
    }

    mTargetUp.set(mInitialUp);
    mTiltSpeed = mMoment.length();

    if (mIsStop != isStop) {
        tryStartSeWithParam(this, "Stop", mTiltSpeed * mRotateSoundScale, nullptr);
    }

    mIsStop = isStop;
}

/**
 * Tilts the map part.
 */
void WobbleMapParts::exeMove() {
    updateMove();
    tryHoldSeWithParam(this, "Rotate", mTiltSpeed * mRotateSoundScale, nullptr);

    if (mTiltSpeed * mRotateSoundScale < 0.1f) {
        startNerveAction(this, "Wait");
    }
}

/**
 * Stops tilting while assisted.
 */
void WobbleMapParts::exeAssistStop() {
    if (--mAssistStopTimer <= 0) {
        mAssistStopTimer = 0;

        if (mTiltSpeed * mRotateSoundScale > 0.1f) {
            startNerveAction(this, "Move");
        } else {
            startNerveAction(this, "Wait");
        }
    }
}
}  // namespace al
