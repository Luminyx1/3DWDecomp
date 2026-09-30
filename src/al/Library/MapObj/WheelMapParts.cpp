#include "Library/MapObj/WheelMapParts.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/MapObj/ChildStep.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Rail/RailUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Project/AreaObj/SwitchKeepOnAreaGroup.hpp"
#include "Project/AreaObj/SwitchOnAreaGroup.hpp"

namespace {
using namespace al;

NERVE_ACTION_IMPL(WheelMapParts, Wait)
NERVE_ACTION_IMPL(WheelMapParts, AssistStop)

NERVE_ACTIONS_MAKE_STRUCT(WheelMapParts, Wait, AssistStop)
}  // namespace

namespace al {
/**
 * Constructs a wheel map part.
 * @param pName actor name
 */
WheelMapParts::WheelMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the wheel from its placement parameters.
 * @param rInfo actor init info
 */
void WheelMapParts::init(const ActorInitInfo& rInfo) {
    initNerveAction(this, "Wait", &NrvWheelMapParts.collector, 0);
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, calcChildStepCount(rInfo));
    registerAreaHostMtx(this, rInfo);
    tryGetArg(&mRotateAxis, rInfo, "RotateAxis");
    tryGetArg(&mRotateAccel, rInfo, "RotateAccel");
    tryGetArg(&mMoveEndDegree, rInfo, "MoveEndDegree");
    tryGetArg(&mNoRotateWidth, rInfo, "NoRotateWidth");
    tryGetArg(&mIsResetOnKill, rInfo, "ResetOnKill");
    mInitialQuat = getQuat(this);
    sead::Vector3f rotateAxis = sead::Vector3f::ex;
    calcQuatLocalAxis(&rotateAxis, mInitialQuat, mRotateAxis);
    mMoveDir.setCross(rotateAxis, sead::Vector3f::ey);
    isNearZero(mMoveDir);
    createChildStep(rInfo, this, true);
    if (isExistRail(this)) {
        setSyncRailToNearestPos(this);
        mIsRailPlusDir = isRailPlusPoseFront(this);
        f32 railProgress = getRailCoord(this) / getRailTotalLength(this);
        if (mIsRailPlusDir) {
            mWheelAngle = railProgress * mMoveEndDegree;
        } else {
            mWheelAngle = -(railProgress * mMoveEndDegree);
        }
    }
    mSwitchKeepOnAreaGroup = tryCreateSwitchKeepOnAreaGroup(this, rInfo);
    mSwitchOnAreaGroup = tryCreateSwitchOnAreaGroup(this, rInfo);
    trySyncStageSwitchAppear(this);
    _142 = true;
}

/**
 * Updates the switch areas.
 */
void WheelMapParts::control() {
    if (mSwitchKeepOnAreaGroup) {
        mSwitchKeepOnAreaGroup->update(getTrans(this));
    }
    if (mSwitchOnAreaGroup) {
        mSwitchOnAreaGroup->update(getTrans(this));
    }
}

/**
 * Appears.
 */
void WheelMapParts::appear() {
    LiveActor::appear();
}

/**
 * Kills the wheel, resetting its angle if requested.
 */
void WheelMapParts::kill() {
    LiveActor::kill();
    if (mIsResetOnKill) {
        mWheelAngle = 0.0f;
    }
}

/**
 * Handles assist touches, floor touches and model visibility.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool WheelMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (isMsgTouchAssist(pMsg)) {
        mAssistStopTimer = 45;
        if (!isNerve(this, NrvWheelMapParts.AssistStop.data())) {
            startNerveAction(this, "AssistStop");
        }
        return true;
    }
    if (isMsgFloorTouch(pMsg)) {
        sead::Vector3f pos;
        if (isMySensor(pSelf, this)) {
            pos = getSensorPos(pOther);
        } else {
            pos = getActorTrans(pSelf);
        }
        f32 width = normalizeAbs(mMoveDir.dot(pos - getTrans(this)), mNoRotateWidth,
                                 mNoRotateWidth + 50.0f);
        mRotateWidth += isMsgEnemyFloorTouch(pMsg) ? width * 0.9f : width;
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
 * Rotates the wheel from the accumulated floor touches.
 */
void WheelMapParts::exeWait() {
    mRotateWidth = sead::Mathf::clamp(mRotateWidth, -1.25f, 1.25f);
    mDeltaAngle = (mRotateWidth * mRotateAccel * 0.001f + mDeltaAngle) * 0.97f;
    mWheelAngle = mWheelAngle + mDeltaAngle;
    if (isExistRail(this)) {
        bool isInvertDirection = false;
        f32 railProgress;
        if (mIsRailPlusDir) {
            if (mWheelAngle < 0.0f) {
                mWheelAngle = 0.0f;
                if (mDeltaAngle < 0.0f) {
                    if (mDeltaAngle < mRotateAccel * -0.001f) {
                        isInvertDirection = true;
                    } else {
                        mDeltaAngle = 0.0f;
                    }
                }
            }
            if (mWheelAngle > mMoveEndDegree) {
                mWheelAngle = mMoveEndDegree;
                if (mDeltaAngle > 0.0f) {
                    if (mDeltaAngle > mRotateAccel * 0.001f) {
                        isInvertDirection = true;
                    } else {
                        mDeltaAngle = 0.0f;
                    }
                }
            }
            railProgress = mWheelAngle / mMoveEndDegree;
        } else {
            if (mWheelAngle < -mMoveEndDegree) {
                mWheelAngle = -mMoveEndDegree;
                if (mDeltaAngle < 0.0f) {
                    if (mDeltaAngle < mRotateAccel * -0.001f) {
                        isInvertDirection = true;
                    } else {
                        mDeltaAngle = 0.0f;
                    }
                }
            }
            if (mWheelAngle > 0.0f) {
                mWheelAngle = 0.0f;
                if (mDeltaAngle > 0.0f) {
                    if (mDeltaAngle > mRotateAccel * 0.001f) {
                        isInvertDirection = true;
                    } else {
                        mDeltaAngle = 0.0f;
                    }
                }
            }
            railProgress = -mWheelAngle / mMoveEndDegree;
        }
        if (isInvertDirection) {
            if (!isNearZero(mDeltaAngle, 0.1f)) {
                tryStartSeWithParam(this, "PgStop", mDeltaAngle);
            }
            mDeltaAngle *= -0.2f;
        }
        setSyncRailToCoord(this, railProgress * getRailTotalLength(this));
    } else {
        mWheelAngle = wrapAngle(mWheelAngle);
    }
    if (!isNearZero(mDeltaAngle, 0.001f)) {
        tryHoldSeWithParam(this, "Rotate", mDeltaAngle);
    }
    rotateQuatLocalDirDegree(getQuatPtr(this), mInitialQuat, mRotateAxis, wrapAngle(mWheelAngle));
    mRotateWidth = 0.0f;
}

/**
 * Stops the wheel while it is touched by the assist.
 */
void WheelMapParts::exeAssistStop() {
    mAssistStopTimer--;
    if (mAssistStopTimer <= 0) {
        mAssistStopTimer = 0;
        startNerveAction(this, "Wait");
    }
}
}  // namespace al
