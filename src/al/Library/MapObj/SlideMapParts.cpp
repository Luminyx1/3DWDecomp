#include "Library/MapObj/SlideMapParts.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/MapObj/EffectMtxSetter.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"

namespace {
using namespace al;

NERVE_ACTION_IMPL(SlideMapParts, StandBy)
NERVE_ACTION_IMPL(SlideMapParts, Delay)
NERVE_ACTION_IMPL(SlideMapParts, Wait)
NERVE_ACTION_IMPL(SlideMapParts, Move)

NERVE_ACTIONS_MAKE_STRUCT(SlideMapParts, StandBy, Delay, Wait, Move)
}  // namespace

namespace al {
/**
 * Constructs a sliding map part.
 * @param pName actor name
 */
SlideMapParts::SlideMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the map part from its placement parameters.
 * @param rInfo actor init info
 */
void SlideMapParts::init(const ActorInitInfo& rInfo) {
    initNerveAction(this, "Move", &NrvSlideMapParts.collector, 0);
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, tryGetMapPartsSuffix(rInfo, "SlideMapParts"), 0);
    registerAreaHostMtx(this, rInfo);
    mTrans = getTrans(this);
    tryGetArg(&mMoveAxis, rInfo, "MoveAxis");
    tryGetArg(&mMoveDistance, rInfo, "MoveDistance");
    tryGetArg(&mMoveSpeed, rInfo, "MoveSpeed");
    tryGetArg(&mWaitTime, rInfo, "WaitTime");
    if (mWaitTime < 0) {
        mWaitTime = 0;
    }

    tryGetArg(&mMoveTime, rInfo, "MoveTime");
    tryGetArg(&mDelayTime, rInfo, "DelayTime");
    f32 surfaceHeight = 0.0f;
    tryGetArg(&surfaceHeight, rInfo, "SurfaceHeight");
    sead::Vector3f axis;
    calcQuatLocalAxis(&axis, getQuat(this), mMoveAxis);
    sead::Vector3f surfaceTrans;
    surfaceTrans.x = surfaceHeight * axis.x + mTrans.x;
    surfaceTrans.y = surfaceHeight * axis.y + mTrans.y;
    surfaceTrans.z = surfaceHeight * axis.z + mTrans.z;
    mSurfaceEffectMtx.makeQT(getQuat(this), surfaceTrans);
    mEffectMtxSetter = tryCreateEffectMtxSetter(this, "EffectMtxSetter");
    if (mEffectMtxSetter) {
        mEffectMtxSetter->setMtxPtr(&mSurfaceEffectMtx, "SurfaceEffectMtx");
    }

    if (listenStageSwitchOnStart(this, FunctorV0M<SlideMapParts*, void (SlideMapParts::*)()>(
                                           this, &SlideMapParts::start))) {
        startNerveAction(this, "StandBy");
    } else if (mDelayTime >= 1) {
        startNerveAction(this, "Delay");
    }

    trySyncStageSwitchAppear(this);
}

/**
 * Starts sliding when the start switch turns on.
 */
void SlideMapParts::start() {
    if (!isNerve(this, NrvSlideMapParts.StandBy.data())) {
        return;
    }

    if (mDelayTime >= 1) {
        startNerveAction(this, "Delay");
        return;
    }

    startNerveAction(this, "Move");
}

/**
 * Shows or hides the model on request.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool SlideMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
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
void SlideMapParts::exeStandBy() {}

/**
 * Waits for the start delay.
 */
void SlideMapParts::exeDelay() {
    if (isGreaterEqualStep(this, mDelayTime)) {
        startNerveAction(this, "Move");
    }
}

/**
 * Waits between moves.
 */
void SlideMapParts::exeWait() {
    if (isGreaterStep(this, mWaitTime)) {
        startNerveAction(this, "Move");
    }
}

/**
 * Slides to the other end.
 */
void SlideMapParts::exeMove() {
    if (isFirstStep(this)) {
        if (mIsMoveForwards) {
            tryStartSe(this, "MoveStart1");
        } else {
            tryStartSe(this, "MoveStart2");
        }
    }

    f32 rate = calcNerveRate(this, calcMoveTime());
    if (!mIsMoveForwards) {
        rate = 1.0f - rate;
    }

    setTransOffsetLocalDir(this, getQuat(this), mTrans, mMoveDistance * rate, mMoveAxis);
    if (isGreaterEqualStep(this, calcMoveTime())) {
        if (mIsMoveForwards) {
            tryStartSe(this, "MoveEnd1");
        } else {
            tryStartSe(this, "MoveEnd2");
        }

        mIsMoveForwards = !mIsMoveForwards;
        tryStartSe(this, "MoveEnd");
        startNerveAction(this, "Wait");
    }
}

/**
 * Calculates the move time.
 * @return move time in steps
 */
s32 SlideMapParts::calcMoveTime() const {
    if (mMoveTime >= 0) {
        return mMoveTime;
    }

    if (mMoveSpeed < 1.0f) {
        return 0;
    }

    return sead::Mathf::abs(mMoveDistance / mMoveSpeed);
}
}  // namespace al
