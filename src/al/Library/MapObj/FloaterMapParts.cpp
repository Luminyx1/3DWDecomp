#include "Library/MapObj/FloaterMapParts.hpp"

#include "Library/KeyPose/KeyPoseKeeperUtil.hpp"
#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"

namespace {
using namespace al;

NERVE_ACTION_IMPL(FloaterMapParts, Wait)
NERVE_ACTION_IMPL(FloaterMapParts, Sink)
NERVE_ACTION_IMPL(FloaterMapParts, Back)

NERVE_ACTIONS_MAKE_STRUCT(FloaterMapParts, Wait, Sink, Back)
}  // namespace

namespace al {
/**
 * Constructs a floating map part.
 * @param pName actor name
 */
FloaterMapParts::FloaterMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the map part and its sink path.
 * @param rInfo actor init info
 */
void FloaterMapParts::init(const ActorInitInfo& rInfo) {
    initNerveAction(this, "Wait", &NrvFloaterMapParts.collector, 0);
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, 0);
    registerAreaHostMtx(this, rInfo);
    mKeyPoseKeeper = createKeyPoseKeeper(rInfo);
    mMaxCoord = calcDistanceNextKeyTrans(mKeyPoseKeeper);
    tryGetArg(&mSinkSpeed, rInfo, "SinkSpeed");
    tryGetArg(&mBackSpeed, rInfo, "BackSpeed");
    tryGetArg(&mMaxAccelCount, rInfo, "MaxAccelCount");
    tryGetArg(&mSinkKeepTime, rInfo, "SinkKeepTime");
    trySyncStageSwitchAppear(this);
}

/**
 * Sinks when something touches the floor.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool FloaterMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (isMsgFloorTouch(pMsg)) {
        mSinkFrame = 2;
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
 * Moves the map part along its path.
 */
void FloaterMapParts::control() {
    f32 rate = isNearZero(mMaxCoord) ? 0.0f : mCoord / mMaxCoord;
    calcLerpKeyTrans(getTransPtr(this), mKeyPoseKeeper, rate);
    calcSlerpKeyQuat(getQuatPtr(this), mKeyPoseKeeper, rate);

    if (mSinkFrame > 0) {
        mSinkFrame--;
    }
}

/**
 * Waits to be touched.
 */
void FloaterMapParts::exeWait() {
    if (isFirstStep(this)) {
        validateClipping(this);
    }

    if (mSinkFrame > 0) {
        invalidateClipping(this);
        startNerveAction(this, "Sink");
    }
}

/**
 * Sinks while something stands on the map part.
 */
void FloaterMapParts::exeSink() {
    if (isFirstStep(this)) {
        mAccelCount = 0;
    }

    if (mSinkFrame != 0) {
        mCoord += mSinkSpeed * mAccelCount / mMaxAccelCount;

        if (mAccelCount < mMaxAccelCount) {
            mAccelCount++;
        }

        if (mCoord > mMaxCoord) {
            mCoord = mMaxCoord;
        }

        mSinkTime = 0;
    } else {
        mSinkTime++;

        if (mAccelCount > 0) {
            mAccelCount--;
        }
    }

    if (mSinkTime >= mSinkKeepTime) {
        startNerveAction(this, "Back");
    }
}

/**
 * Moves back up to the start position.
 */
void FloaterMapParts::exeBack() {
    if (isFirstStep(this)) {
        mAccelCount = 0;
    }

    mCoord -= mBackSpeed * mAccelCount / mMaxAccelCount;

    if (mAccelCount < mMaxAccelCount) {
        mAccelCount++;
    }

    bool isReachedStart;

    if (mCoord < 0.0f) {
        isReachedStart = true;
        mCoord = 0.0f;
    } else {
        isReachedStart = false;
    }

    if (mSinkFrame >= 1) {
        startNerveAction(this, "Sink");
    } else if (isReachedStart) {
        startNerveAction(this, "Wait");
    }
}
}  // namespace al
