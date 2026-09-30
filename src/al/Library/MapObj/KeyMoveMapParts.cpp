#include "Library/MapObj/KeyMoveMapParts.hpp"

#include <cstring>

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/KeyPose/KeyPoseKeeper.hpp"
#include "Library/KeyPose/KeyPoseKeeperUtil.hpp"
#include "Library/Light/LppBase.hpp"
#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/MapObj/EffectMtxSetter.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/AreaObj/SwitchKeepOnAreaGroup.hpp"
#include "Project/AreaObj/SwitchOnAreaGroup.hpp"
#include "Project/Collision/CollisionPartsTriangle.hpp"
#include "Project/Collision/CollisionUtil.hpp"
#include "Project/Light/ActorPrePassLightKeeper.hpp"

namespace {
using namespace al;

NERVE_ACTION_IMPL(KeyMoveMapParts, StandBy)
NERVE_ACTION_IMPL(KeyMoveMapParts, Delay)
NERVE_ACTION_IMPL(KeyMoveMapParts, WaitSign)
NERVE_ACTION_IMPL(KeyMoveMapParts, Wait)
NERVE_ACTION_IMPL(KeyMoveMapParts, MoveSign)
NERVE_ACTION_IMPL(KeyMoveMapParts, Move)
NERVE_ACTION_IMPL(KeyMoveMapParts, StopSign)
NERVE_ACTION_IMPL(KeyMoveMapParts, Stop)

NERVE_ACTIONS_MAKE_STRUCT(KeyMoveMapParts, StandBy, Delay, WaitSign, Wait, MoveSign, Move,
                          StopSign, Stop)
}  // namespace

namespace al {
/**
 * Gets the move sound of a key.
 * @param index key index
 * @return sound name, or null
 */
const char* KeyMoveMapParts::getMoveSeName(s32 index) {
    switch (index) {
    case 0:
        return "PgMove0";
    case 1:
        return "PgMove1";
    default:
        return nullptr;
    }
}

/**
 * Constructs a key moving map part.
 * @param pName actor name
 */
KeyMoveMapParts::KeyMoveMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the map part and its key poses.
 * @param rInfo actor init info
 */
void KeyMoveMapParts::init(const ActorInitInfo& rInfo) {
    using KeyMoveMapPartsFunctor = FunctorV0M<KeyMoveMapParts*, void (KeyMoveMapParts::*)()>;

    initNerveAction(this, "Wait", &NrvKeyMoveMapParts.collector, 0);
    initActorPoseTQSV(this);
    mIsSingleMode = rInfo.mActorSceneInfo.isSingleMode;
    const char* suffix = nullptr;
    tryGetStringArg(&suffix, rInfo, "SuffixName");
    initMapPartsActor(this, rInfo, suffix, 0);
    mKeyPoseKeeper = createKeyPoseKeeper(rInfo);
    registerAreaHostMtx(this, rInfo);
    setKeyMoveClippingInfo(this, &mClippingOffset, mKeyPoseKeeper);
    tryGetArg(&mIsFloorTouchStart, rInfo, "IsFloorTouchStart");
    tryGetArg(&mIsStopKill, rInfo, "IsStopKill");
    tryGetArg(&mDelayTime, rInfo, "DelayTime");
    if (mDelayTime >= 1) {
        startNerveAction(this, "Delay");
    }
    tryGetArg(&mIsCancelWithinWaitTime, rInfo, "IsCancelWithinWaitTime");
    tryGetArg(&mIsReverseWhenSwitchOff, rInfo, "IsReverseWhenSwitchOff");
    tryGetArg(&mIsIgnoreFirstWaitTime, rInfo, "IsIgnoreFirstWaitTime");
    if (getKeyPoseCount(mKeyPoseKeeper) < 2 || mIsFloorTouchStart) {
        startNerveAction(this, "StandBy");
    } else if (mIsReverseWhenSwitchOff) {
        if (listenStageSwitchOnOffStart(this,
                                        KeyMoveMapPartsFunctor(this, &KeyMoveMapParts::start),
                                        KeyMoveMapPartsFunctor(this, &KeyMoveMapParts::reverse))) {
            startNerveAction(this, "StandBy");
        }
    } else if (listenStageSwitchOnStart(this,
                                        KeyMoveMapPartsFunctor(this, &KeyMoveMapParts::start))) {
        startNerveAction(this, "StandBy");
    }
    listenStageSwitchOn(this, "KillPrePassLights",
                        KeyMoveMapPartsFunctor(this, &KeyMoveMapParts::killLights));
    mSwitchKeepOnAreaGroup = tryCreateSwitchKeepOnAreaGroup(this, rInfo);
    mSwitchOnAreaGroup = tryCreateSwitchOnAreaGroup(this, rInfo);
    trySyncStageSwitchAppear(this);
    tryListenStageSwitchKill(this);
    if (mIsSingleMode) {
        listenStageSwitchOn(this, "SwitchStop",
                            KeyMoveMapPartsFunctor(this, &KeyMoveMapParts::stop));
        mEffectMtxSetter = tryCreateEffectMtxSetter(this, "EffectMtxSetter");
        if (mEffectMtxSetter) {
            mEffectMtxSetter->setMtxPtr(&mBaseEffectMtx, "BaseEffectMtx");
            mBaseQuat.set(getQuat(this));
        }
        tryGetArg(&mGroundCheckOffset, rInfo, "GroundCheckOffset");
    }
    _142 = true;
    _143 = true;
}

/**
 * Starts moving forwards.
 */
void KeyMoveMapParts::start() {
    if (mIsSingleMode) {
        mIsReversed = false;
        if (!isNerve(this, NrvKeyMoveMapParts.StandBy.data()) ||
            getKeyPoseCount(mKeyPoseKeeper) < 2) {
            return;
        }
        if (!mKeyPoseKeeper->isGoingToEnd()) {
            mKeyPoseKeeper->reverse();
        }
    } else if (!isNerve(this, NrvKeyMoveMapParts.StandBy.data()) ||
               getKeyPoseCount(mKeyPoseKeeper) < 2) {
        return;
    }
    if (isExistAction(this, "Start")) {
        startAction(this, "Start");
    }
    if (mDelayTime >= 1) {
        startNerveAction(this, "Delay");
    } else {
        startNerveAction(this, "Wait");
    }
}

/**
 * Starts moving backwards.
 */
void KeyMoveMapParts::reverse() {
    mIsReversed = true;
    if (!isNerve(this, NrvKeyMoveMapParts.StandBy.data()) ||
        getKeyPoseCount(mKeyPoseKeeper) < 2) {
        return;
    }
    if (mKeyPoseKeeper->isGoingToEnd()) {
        mKeyPoseKeeper->reverse();
    }
    if (isExistAction(this, "StartReverse")) {
        startAction(this, "StartReverse");
    } else {
        tryStartAction(this, "Start");
    }
    if (mDelayTime >= 1) {
        startNerveAction(this, "Delay");
    } else {
        startNerveAction(this, "Wait");
    }
}

/**
 * Kills the pre pass lights whose name contains "Kill".
 */
void KeyMoveMapParts::killLights() {
    ActorPrePassLightKeeper* lightKeeper = mLightKeeper;
    if (!lightKeeper) {
        return;
    }
    for (s32 i = 0; i < lightKeeper->getLightNum(); i++) {
        if (strstr(lightKeeper->getLightBase(i)->mName, "Kill")) {
            lightKeeper->getLightBase(i)->requestKill();
        }
    }
}

/**
 * Stops moving.
 */
void KeyMoveMapParts::stop() {
    if (isNerve(this, NrvKeyMoveMapParts.StopSign.data()) ||
        isNerve(this, NrvKeyMoveMapParts.Stop.data())) {
        return;
    }
    if (isExistAction(this, "StopSign")) {
        startNerveAction(this, "StopSign");
    } else if (!isNerve(this, NrvKeyMoveMapParts.Stop.data())) {
        startNerveAction(this, "Stop");
    }
    if (mSeMoveName) {
        tryStopSe(this, mSeMoveName);
        mSeMoveName = nullptr;
    }
    tryStartSe(this, "MoveEnd");
}

/**
 * Places the base effect matrix on the ground below the map part.
 */
void KeyMoveMapParts::initAfterPlacement() {
    if (!mEffectMtxSetter || !mIsSingleMode) {
        return;
    }
    mBaseEffectMtx.makeQT(mBaseQuat, getGroundPos());
}

/**
 * Finds the ground below the map part.
 * @return ground position, or the check start position if there is no ground
 */
sead::Vector3f KeyMoveMapParts::getGroundPos() {
    sead::Vector3f hitPos = sead::Vector3f::zero;
    Triangle triangle;
    sead::Vector3f startPos = getTrans(this);
    startPos.y += mGroundCheckOffset;
    if (alCollisionUtil::getFirstPolyOnArrow(this, &hitPos, &triangle, startPos,
                                             sead::Vector3f::ey * -10000.0f, nullptr, nullptr)) {
        return hitPos;
    }
    return startPos;
}

/**
 * Starts on floor touch and shows or hides the model on request.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool KeyMoveMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (mIsFloorTouchStart && isMsgFloorTouch(pMsg)) {
        mReverseTimer = 0;
        start();
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
 * Appears and restarts from the current key.
 */
void KeyMoveMapParts::appear() {
    LiveActor::appear();
    if (!mIsSingleMode) {
        return;
    }
    restartKeyPose(mKeyPoseKeeper, getTransPtr(this), getQuatPtr(this));
    resetPosition(this, false);
    if (mDelayTime >= 1) {
        startNerveAction(this, "Delay");
    } else {
        startNerveAction(this, "Wait");
    }
    if (getKeyPoseCount(mKeyPoseKeeper) < 2 || mIsFloorTouchStart) {
        startNerveAction(this, "StandBy");
    }
}

/**
 * Updates the switch areas and the floor touch reverse timer.
 */
void KeyMoveMapParts::control() {
    if (mSwitchKeepOnAreaGroup) {
        mSwitchKeepOnAreaGroup->update(getTrans(this));
    }
    if (mSwitchOnAreaGroup) {
        mSwitchOnAreaGroup->update(getTrans(this));
    }
    if (!mIsSingleMode) {
        return;
    }
    if (mReverseTimer == 20 && !isNerve(this, NrvKeyMoveMapParts.StandBy.data())) {
        reverse();
    }
    if (mReverseTimer != -1) {
        mReverseTimer++;
    }
}

/**
 * Resets to the first key and appears.
 */
void KeyMoveMapParts::appearAndSetStart() {
    resetKeyPose(mKeyPoseKeeper);
    setQuat(this, getCurrentKeyQuat(mKeyPoseKeeper));
    setTrans(this, getCurrentKeyTrans(mKeyPoseKeeper));
    resetPosition(this, false);
    if (mDelayTime >= 1) {
        startNerveAction(this, "Delay");
    } else {
        startNerveAction(this, "Wait");
    }
    appear();
}

/**
 * Called when starting to move up.
 */
void KeyMoveMapParts::_startUp() {}

/**
 * Called when starting to move down.
 */
void KeyMoveMapParts::_startDown() {}

/**
 * Waits for the start trigger.
 */
void KeyMoveMapParts::exeStandBy() {}

/**
 * Waits for the start delay.
 */
void KeyMoveMapParts::exeDelay() {
    if (isGreaterEqualStep(this, mDelayTime - 1)) {
        startNerveAction(this, "Wait");
    }
}

/**
 * Plays the wait sign action.
 */
void KeyMoveMapParts::exeWaitSign() {
    if (isFirstStep(this)) {
        startAction(this, "WaitSign");
    }
    if (isActionEnd(this)) {
        startNerveAction(this, "Wait");
    }
}

/**
 * Waits at a key.
 */
void KeyMoveMapParts::exeWait() {
    if (mIsSingleMode) {
        if (isFirstStep(this)) {
            if (mIsIgnoreFirstWaitTime) {
                mKeyMoveWaitTime = 0;
                mIsIgnoreFirstWaitTime = false;
            } else {
                s32 waitTime = calcKeyMoveWaitTime(mKeyPoseKeeper);
                if (waitTime > -1) {
                    mKeyMoveWaitTime = waitTime;
                }
            }
        }
        if (isGreaterEqualStep(this, mKeyMoveWaitTime)) {
            if (isRestart(mKeyPoseKeeper)) {
                restartKeyPose(mKeyPoseKeeper, getTransPtr(this), getQuatPtr(this));
                resetPosition(this, false);
                startNerveAction(this, "Wait");
                return;
            }
            if (isMoveSignKey(mKeyPoseKeeper) && isExistAction(this, "MoveKeySign")) {
                startNerveAction(this, "MoveSign");
                return;
            }
            startNerveAction(this, "Move");
            return;
        }
        if (mIsCancelWithinWaitTime && mReverseTimer != 0 && mKeyPoseKeeper->isGoingToEnd()) {
            restartKeyPose(mKeyPoseKeeper, getTransPtr(this), getQuatPtr(this));
            mIsReversed = false;
            startNerveAction(this, "StandBy");
        }
    } else {
        if (isFirstStep(this)) {
            s32 waitTime = calcKeyMoveWaitTime(mKeyPoseKeeper);
            if (waitTime > -1) {
                mKeyMoveWaitTime = waitTime;
            }
        }
        if (isGreaterEqualStep(this, mKeyMoveWaitTime)) {
            if (isMoveSignKey(mKeyPoseKeeper) && isExistAction(this, "MoveKeySign")) {
                startNerveAction(this, "MoveSign");
                return;
            }
            startNerveAction(this, "Move");
        }
    }
}

/**
 * Plays the move sign action.
 */
void KeyMoveMapParts::exeMoveSign() {
    if (isFirstStep(this)) {
        startAction(this, "MoveKeySign");
    }
    if (isActionEnd(this)) {
        startNerveAction(this, "Move");
    }
}

/**
 * Moves to the next key.
 */
void KeyMoveMapParts::exeMove() {
    if (mIsSingleMode) {
        if (isFirstStep(this)) {
            if (isExistAction(this, "MoveLoop")) {
                tryStartActionIfNotPlaying(this, "MoveLoop");
            }
            mKeyMoveMoveTime = calcKeyMoveMoveTime(mKeyPoseKeeper);
            s32 keyIndex = mKeyPoseKeeper->getKeyPoseCurrentIdx();
            sead::Vector3f dir;
            calcDirToNextKey(&dir, mKeyPoseKeeper);
            if (dir.y < 0.0f) {
                _startDown();
            } else if (dir.y > 0.0f) {
                _startUp();
            }
            mSeMoveName = getMoveSeName(keyIndex);
            if (mSeMoveName) {
                tryStartSe(this, mSeMoveName);
            }
        }
        f32 rate = calcNerveRate(this, mKeyMoveMoveTime);
        calcLerpKeyTrans(getTransPtr(this), mKeyPoseKeeper, rate);
        calcSlerpKeyQuat(getQuatPtr(this), mKeyPoseKeeper, rate);
        if (isGreaterEqualStep(this, mKeyMoveMoveTime)) {
            nextKeyPose(mKeyPoseKeeper);
            if (isStop(mKeyPoseKeeper)) {
                stop();
                return;
            }
            if (isMoveSignKey(mKeyPoseKeeper)) {
                if (isExistAction(this, "WaitSign")) {
                    startNerveAction(this, "WaitSign");
                } else {
                    tryStartActionNoAnim(this, "WaitSign");
                    startNerveAction(this, "Wait");
                }
            } else {
                startNerveAction(this, "Wait");
            }
            if (mSeMoveName) {
                tryStopSe(this, mSeMoveName);
                mSeMoveName = nullptr;
            }
            tryStartSe(this, "MoveEnd");
        }
    } else {
        if (isFirstStep(this)) {
            if (isExistAction(this, "MoveLoop")) {
                tryStartActionIfNotPlaying(this, "MoveLoop");
            }
            mKeyMoveMoveTime = calcKeyMoveMoveTime(mKeyPoseKeeper);
        }
        f32 rate = calcNerveRate(this, mKeyMoveMoveTime);
        calcLerpKeyTrans(getTransPtr(this), mKeyPoseKeeper, rate);
        calcSlerpKeyQuat(getQuatPtr(this), mKeyPoseKeeper, rate);
        if (isGreaterEqualStep(this, mKeyMoveMoveTime)) {
            nextKeyPose(mKeyPoseKeeper);
            if (isStop(mKeyPoseKeeper)) {
                if (isExistAction(this, "StopSign")) {
                    startNerveAction(this, "StopSign");
                } else {
                    startNerveAction(this, "Stop");
                }
            } else {
                startNerveAction(this, "Wait");
            }
            tryStartSe(this, "MoveEnd");
        }
    }
}

/**
 * Plays the stop sign action.
 */
void KeyMoveMapParts::exeStopSign() {
    if (isFirstStep(this)) {
        startAction(this, "StopSign");
    }
    if (isActionEnd(this)) {
        tryStartAction(this, "Stop");
    }
}

/**
 * Stops at the last key.
 */
void KeyMoveMapParts::exeStop() {
    if (isFirstStep(this)) {
        if (isInvalidClipping(this)) {
            validateClipping(this);
        }
        if (mIsStopKill) {
            kill();
        }
    }
    if (mIsSingleMode && mIsReversed == mKeyPoseKeeper->isGoingToEnd()) {
        startNerveAction(this, "StandBy");
        if (mIsReversed) {
            reverse();
        } else {
            start();
        }
    }
}
}  // namespace al
