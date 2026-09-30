#include "Library/MapObj/RotateMapParts.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/MapObj/ChildStep.hpp"
#include "Library/MapObj/SupportFreezeSyncGroupHolder.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"

namespace {
using namespace al;

NERVE_ACTION_IMPL(RotateMapParts, StandBy)
NERVE_ACTION_IMPL(RotateMapParts, Rotate)
NERVE_ACTION_IMPL_(RotateMapParts, EffectOnAngle, StandBy)
NERVE_ACTION_IMPL(RotateMapParts, AssistStop)
NERVE_ACTION_IMPL(RotateMapParts, AssistStopSync)

NERVE_ACTIONS_MAKE_STRUCT(RotateMapParts, StandBy, Rotate, EffectOnAngle, AssistStop,
                          AssistStopSync)
}  // namespace

namespace al {
/**
 * Constructs a rotating map part.
 * @param pName actor name
 */
RotateMapParts::RotateMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the map part and its child steps.
 * @param rInfo actor init info
 */
void RotateMapParts::init(const ActorInitInfo& rInfo) {
    initNerveAction(this, "Rotate", &NrvRotateMapParts.collector, 0);
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, calcChildStepCount(rInfo));
    registerAreaHostMtx(this, rInfo);
    if (mHitSensorKeeper) {
        mIsSupportFreezeSync = registSupportFreezeSyncGroup(this, rInfo);
    }

    tryGetArg(&mRotateAxis, rInfo, "RotateAxis");
    tryGetArg(&mRotateSpeed, rInfo, "RotateSpeed");
    createChildStep(rInfo, this, true);
    if (listenStageSwitchOnStart(this, FunctorV0M<RotateMapParts*, void (RotateMapParts::*)()>(
                                           this, &RotateMapParts::start))) {
        startNerveAction(this, "StandBy");
    }

    trySyncStageSwitchAppear(this);
    mIsSingleMode = rInfo.mActorSceneInfo.isSingleMode;
    mStartTrans = getTrans(this);
    mStartQuat = getQuat(this);
    tryGetArg(&mIsTriggerEffectOnAngle, rInfo, "IsTriggerEffectOnAngle");
    if (mIsTriggerEffectOnAngle) {
        tryGetArg(&mEffectTriggerAngle, rInfo, "EffectTriggerAngle");
        mEffectAngle = 0.0f;
    }

    _142 = true;
    _143 = true;
}

/**
 * Starts rotating when the start switch turns on.
 */
void RotateMapParts::start() {
    if (isNerve(this, NrvRotateMapParts.StandBy.data())) {
        startNerveAction(this, "Rotate");
    }
}

/**
 * Makes the map part appear, resetting its pose if requested.
 */
void RotateMapParts::appear() {
    if (mIsSingleMode) {
        setQuat(this, mStartQuat);
        setTrans(this, mStartTrans);
        mEffectAngle = 0.0f;
        if (isNerve(this, NrvRotateMapParts.StandBy.data())) {
            startNerveAction(this, "Rotate");
        }
    }

    LiveActor::appear();
}

/**
 * Kills the map part.
 */
void RotateMapParts::kill() {
    if (mIsSingleMode) {
        startNerveAction(this, "StandBy");
    }

    LiveActor::kill();
}

/**
 * Handles assist, support freeze and model visibility messages.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool RotateMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (isMsgTouchAssist(pMsg)) {
        mAssistTimer = 45;
        return true;
    }

    if (mIsSupportFreezeSync) {
        if (isMsgIsNerveSupportFreeze(pMsg)) {
            return isNerve(this, NrvRotateMapParts.AssistStop.data());
        }

        if (isMsgOnSyncSupportFreeze(pMsg)) {
            if (isNerve(this, NrvRotateMapParts.AssistStop.data())) {
                return true;
            }

            if (isExistAction(this)) {
                stopAction(this);
            }

            startNerveAction(this, "AssistStopSync");
            return true;
        }

        if (isMsgOffSyncSupportFreeze(pMsg)) {
            if (!isNerve(this, NrvRotateMapParts.AssistStopSync.data())) {
                return true;
            }

            if (isExistAction(this)) {
                restartAction(this);
            }

            startNerveAction(this, "Rotate");
            return true;
        }
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
 * Waits for the start switch.
 */
void RotateMapParts::exeStandBy() {}

/**
 * Rotates the map part.
 */
void RotateMapParts::exeRotate() {
    rotateQuatLocalDirDegree(this, mRotateAxis, mRotateSpeed / 100.0f);
    if (mAssistTimer > 0) {
        startNerveAction(this, "AssistStop");
    }

    if (isExistSePlayNameInUserInfo(this, "RotateWithSpeed")) {
        tryHoldSeWithParam(this, "RotateWithSpeed", mRotateSpeed, nullptr);
    }

    if (mIsTriggerEffectOnAngle) {
        f32 speed = mRotateSpeed / 100.0f;
        f32 angle = mEffectAngle + speed;
        if (angle >= 360.0f) {
            angle += -360.0f;
        }

        mEffectAngle = angle;
        if (!_140 && mEffectAngle < mEffectTriggerAngle &&
            mEffectAngle + speed >= mEffectTriggerAngle) {
            tryStartEffectAction(this, "EffectOnAngle");
        }
    }
}

/**
 * Stops rotating while assisted.
 */
void RotateMapParts::exeAssistStop() {
    if (--mAssistTimer <= 0) {
        mAssistTimer = 0;
        startNerveAction(this, "Rotate");
    }

    if (mIsTriggerEffectOnAngle && mEffectAngle < mEffectTriggerAngle &&
        mEffectAngle + mRotateSpeed / 100.0f >= mEffectTriggerAngle) {
        tryStartEffectAction(this, "EffectOnAngle");
    }
}

/**
 * Stops rotating while a synced support freeze is active.
 */
void RotateMapParts::exeAssistStopSync() {}
}  // namespace al
