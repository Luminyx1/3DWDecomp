#include "Library/MapObj/ClockMapParts.hpp"

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
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"

namespace {
using namespace al;

NERVE_ACTION_IMPL(ClockMapParts, StandBy)
NERVE_ACTION_IMPL(ClockMapParts, Delay)
NERVE_ACTION_IMPL(ClockMapParts, RotateSign)
NERVE_ACTION_IMPL(ClockMapParts, Rotate)
NERVE_ACTION_IMPL(ClockMapParts, Wait)
NERVE_ACTION_IMPL(ClockMapParts, AssistStop)
NERVE_ACTION_IMPL(ClockMapParts, AssistStopSync)
NERVE_ACTION_IMPL(ClockMapParts, AssistStopEndWait)

NERVE_ACTIONS_MAKE_STRUCT(ClockMapParts, StandBy, Delay, RotateSign, Rotate, Wait, AssistStop,
                          AssistStopSync, AssistStopEndWait)
}  // namespace

namespace al {
/**
 * Constructs a clock map part.
 * @param pName actor name
 */
ClockMapParts::ClockMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the clock from its placement parameters.
 * @param rInfo actor init info
 */
void ClockMapParts::init(const ActorInitInfo& rInfo) {
    initNerveAction(this, "Rotate", &NrvClockMapParts.collector, 0);
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, calcChildStepCount(rInfo));
    registerAreaHostMtx(this, rInfo);
    registSupportFreezeSyncGroup(this, rInfo);
    mQuat.set(getQuat(this));
    if (isSingleMode(rInfo)) {
        tryGetArg(&mIsNoIntroUpdate, rInfo, "NoIntroUpdate");
    }

    tryGetArg(&mClockAngle, rInfo, "ClockAngle");
    tryGetArg(&mRotateAxis, rInfo, "RotateAxis");
    createChildStep(rInfo, this, true);
    tryGetArg(&mDelayTime, rInfo, "DelayTime");
    tryGetArg(&mWaitTime, rInfo, "WaitTime");
    tryGetArg(&mRotateTime, rInfo, "RotateTime");
    if (isExistAction(this, "MiddleSign")) {
        mRotateSignTime = getActionFrameMax(this, "MiddleSign");
    } else {
        mRotateSignTime = 0;
    }

    mRotateTimer = mRotateSignTime + mRotateTime + 1;
    mActiveTimer = mRotateTimer + mWaitTime + 1;
    if (mDelayTime >= 1) {
        startNerveAction(this, "Delay");
    }

    if (listenStageSwitchOnStart(
            this, FunctorV0M<ClockMapParts*, void (ClockMapParts::*)()>(this, &ClockMapParts::start))) {
        startNerveAction(this, "StandBy");
    }

    trySyncStageSwitchAppear(this);
    _142 = true;
}

/**
 * Starts rotating when the start switch turns on.
 */
void ClockMapParts::start() {
    if (!isNerve(this, NrvClockMapParts.StandBy.data())) {
        return;
    }

    if (mDelayTime >= 1) {
        startNerveAction(this, "Delay");
        return;
    }

    setRotateStartNerve();
}

/**
 * Pauses the start delay while a demo runs.
 * @param demoType demo type
 */
void ClockMapParts::startDemoActor(s32 demoType) {
    if (mIsNoIntroUpdate) {
        mIsDemo = true;
    }
}

/**
 * Resumes the start delay after a demo.
 * @param demoType demo type
 */
void ClockMapParts::endDemoActor(s32 demoType) {
    if (mIsNoIntroUpdate) {
        mIsDemo = false;
    }
}

/**
 * Handles assist touches, model visibility and support freeze syncing.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool ClockMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (isMsgTouchAssist(pMsg)) {
        mAssistStopTimer = 45;
        if (isNerve(this, NrvClockMapParts.AssistStop.data())) {
            return true;
        }

        if (isExistAction(this)) {
            stopAction(this);
        }

        startNerveAction(this, "AssistStop");
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

    if (isMsgIsNerveSupportFreeze(pMsg)) {
        return isNerve(this, NrvClockMapParts.AssistStop.data());
    }

    if (isMsgOnSyncSupportFreeze(pMsg)) {
        if (isNerve(this, NrvClockMapParts.AssistStop.data())) {
            return true;
        }

        if (isExistAction(this)) {
            stopAction(this);
        }

        startNerveAction(this, "AssistStopSync");
        return true;
    }

    if (isMsgOffSyncSupportFreeze(pMsg)) {
        if (!isNerve(this, NrvClockMapParts.AssistStopSync.data())) {
            return true;
        }

        setRestartNerve();
        return true;
    }

    return false;
}

/**
 * Restarts the nerve matching the current timer.
 */
void ClockMapParts::setRestartNerve() {
    if (isExistAction(this)) {
        restartAction(this);
    }

    if (mTimer >= mRotateTimer) {
        startNerveAction(this, "AssistStopEndWait");
    } else if (mTimer >= mRotateSignTime) {
        startNerveAction(this, "Rotate");
    } else {
        startNerveAction(this, "RotateSign");
    }
}

/**
 * Starts the rotation, with the sign action if it exists.
 */
void ClockMapParts::setRotateStartNerve() {
    if (isExistAction(this, "MiddleSign")) {
        startNerveAction(this, "RotateSign");
    } else {
        startNerveAction(this, "Rotate");
    }
}

/**
 * Waits for the start switch.
 */
void ClockMapParts::exeStandBy() {}

/**
 * Waits for the start delay.
 */
void ClockMapParts::exeDelay() {
    if (isGreaterEqualStep(this, mDelayTime - 1) && !mIsDemo) {
        setRotateStartNerve();
    }
}

/**
 * Plays the rotate sign action.
 */
void ClockMapParts::exeRotateSign() {
    if (isFirstStep(this)) {
        startAction(this, "MiddleSign");
    }

    mTimer++;
    if (mTimer >= mRotateSignTime) {
        startNerveAction(this, "Rotate");
    }
}

/**
 * Rotates to the next step.
 */
void ClockMapParts::exeRotate() {
    f32 time = static_cast<f32>(mTimer - mRotateSignTime) /
               static_cast<f32>(mRotateTimer + ~mRotateSignTime);
    f32 angle = wrapAngle((time + mCurrentStep) * mClockAngle);
    rotateQuatLocalDirDegree(this, mQuat, mRotateAxis, angle);
    mTimer++;
    if (mTimer >= mRotateTimer) {
        mCurrentStep++;
        startNerveAction(this, "Wait");
        tryStartSe(this, "RotateEnd");
    }
}

/**
 * Waits between rotations.
 */
void ClockMapParts::exeWait() {
    mTimer++;
    if (mTimer >= mActiveTimer) {
        mTimer -= mActiveTimer;
        setRotateStartNerve();
    }
}

/**
 * Stops while touched by the assist.
 */
void ClockMapParts::exeAssistStop() {
    mAssistStopTimer--;
    if (mAssistStopTimer <= 0) {
        mAssistStopTimer = 0;
        setRestartNerve();
    }
}

/**
 * Stops while a synced support freeze is active.
 */
void ClockMapParts::exeAssistStopSync() {}

/**
 * Waits after an assist stop.
 */
void ClockMapParts::exeAssistStopEndWait() {
    exeWait();
}
}  // namespace al
