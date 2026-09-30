#include "Library/MapObj/FallMapParts.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/LiveActor/ActorCollisionFunction.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorAnimUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"

namespace {
using namespace al;

NERVE_ACTION_IMPL(FallMapParts, Appear)
NERVE_ACTION_IMPL(FallMapParts, Wait)
NERVE_ACTION_IMPL(FallMapParts, FallSign)
NERVE_ACTION_IMPL(FallMapParts, Fall)
NERVE_ACTION_IMPL(FallMapParts, End)

NERVE_ACTIONS_MAKE_STRUCT(FallMapParts, Appear, Wait, FallSign, Fall, End)
}  // namespace

namespace al {
/**
 * Constructs a falling map part.
 * @param pName actor name
 */
FallMapParts::FallMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the map part.
 * @param rInfo actor init info
 */
void FallMapParts::init(const ActorInitInfo& rInfo) {
    init(rInfo, nullptr);
}

/**
 * Initializes the map part with a model suffix.
 * @param rInfo actor init info
 * @param pSuffix model suffix
 */
void FallMapParts::init(const ActorInitInfo& rInfo, const char* pSuffix) {
    initNerveAction(this, "Wait", &NrvFallMapParts.collector, 0);
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, pSuffix, 0);
    registerAreaHostMtx(this, rInfo);
    mStartTrans = getTrans(this);
    tryGetArg(&mFallTime, rInfo, "FallTime");

    if (listenStageSwitchOnOffAppear(this, FunctorV0M<FallMapParts*, void (FallMapParts::*)()>(
                                               this, &FallMapParts::switchAppear),
                                     FunctorV0M<FallMapParts*, void (FallMapParts::*)()>(
                                         this, &FallMapParts::switchKill))) {
        makeActorDead();
    } else {
        makeActorAppeared();
    }
}

/**
 * Makes the map part appear at its start position.
 */
void FallMapParts::switchAppear() {
    LiveActor::appear();
    showModelIfHide(this);
    setTrans(this, mStartTrans);
    resetPosition(this, false);
    startAction(this, "Wait");
    startNerveAction(this, "Wait");
    validateCollisionParts(this);
}

/**
 * Kills the map part.
 */
void FallMapParts::switchKill() {
    LiveActor::kill();
}

/**
 * Starts falling when something touches the floor.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool FallMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (isMsgFloorTouch(pMsg) && isNerve(this, NrvFallMapParts.Wait.data())) {
        startNerveAction(this, "FallSign");
        invalidateClipping(this);
        return true;
    }

    return false;
}

/**
 * Plays the appear action.
 */
void FallMapParts::exeAppear() {
    if (isFirstStep(this)) {
        validateCollisionParts(this);
        tryStartMclAnimIfExist(this, "Wait");

        if (!tryStartAction(this, "Appear")) {
            startNerveAction(this, "Wait");
            return;
        }
    }

    if (!isExistAction(this) || isActionEnd(this)) {
        startNerveAction(this, "Wait");
    }
}

/**
 * Waits to be touched.
 */
void FallMapParts::exeWait() {
    if (isFirstStep(this)) {
        tryStartAction(this, "Wait");
        showModelIfHide(this);
        validateClipping(this);
    }
}

/**
 * Shakes before falling.
 */
void FallMapParts::exeFallSign() {
    if (isFirstStep(this)) {
        mIsStartFallSignAction = tryStartAction(this, "FallSign");
    }

    if (!mIsStartFallSignAction) {
        f32 offset = sead::Mathf::sin(calcNerveValue(this, 20, 0.0f, sead::Mathf::pi() * 3)) * 3;
        setTrans(this, offset * sead::Vector3f::ey + mStartTrans);
    }

    if (isEndFallSign()) {
        startNerveAction(this, "Fall");
    }
}

/**
 * Checks if the fall sign is over.
 * @return whether the fall sign is over
 */
bool FallMapParts::isEndFallSign() const {
    return mIsStartFallSignAction ? isActionEnd(this) : isGreaterEqualStep(this, 20);
}

/**
 * Falls with gravity.
 */
void FallMapParts::exeFall() {
    if (isFirstStep(this)) {
        tryStartAction(this, "Fall");
        setTrans(this, mStartTrans);
    }

    addVelocityToGravity(this, 0.3f);
    scaleVelocity(this, 0.9f);

    if (isGreaterStep(this, mFallTime)) {
        startNerveAction(this, "End");
    }
}

/**
 * Hides the map part and restarts it after a while.
 */
void FallMapParts::exeEnd() {
    if (isFirstStep(this)) {
        tryStartAction(this, "End");
        hideModel(this);
        invalidateCollisionParts(this);
        setVelocityZero(this);
    }

    if (isGreaterStep(this, 120)) {
        setTrans(this, mStartTrans);
        resetPosition(this, false);
        showModel(this);
        startNerveAction(this, "Appear");
    }
}
}  // namespace al
