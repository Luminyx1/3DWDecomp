#include "Library/MapObj/GateMapParts.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {
using namespace al;

NERVE_ACTION_IMPL(GateMapParts, Wait)
NERVE_ACTION_IMPL(GateMapParts, Open)
NERVE_ACTION_IMPL(GateMapParts, Bound)
NERVE_ACTION_IMPL(GateMapParts, End)

NERVE_ACTIONS_MAKE_STRUCT(GateMapParts, Wait, Open, Bound, End)
}  // namespace

namespace al {
/**
 * Constructs a gate map part.
 * @param pName actor name
 */
GateMapParts::GateMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the gate and its open pose.
 * @param rInfo actor init info
 */
void GateMapParts::init(const ActorInitInfo& rInfo) {
    initNerveAction(this, "Wait", &NrvGateMapParts.collector, 0);
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, 0);
    mTrans = getTrans(this);
    mQuat = getQuat(this);
    tryGetLinksTrans(&mMoveNextTrans, rInfo, "MoveNext");
    tryGetLinksQuat(&mMoveNextQuat, rInfo, "MoveNext");
    tryGetArg(&mOpenTime, rInfo, "OpenTime");
    tryGetArg(&mBoundRate, rInfo, "BoundRate");
    tryGetArg(&mHitReactionCount, rInfo, "HitReactionCount");
    listenStageSwitchOnStart(
        this, FunctorV0M<GateMapParts*, void (GateMapParts::*)()>(this, &GateMapParts::start));
    makeActorAppeared();
}

/**
 * Starts opening when the start switch turns on.
 */
void GateMapParts::start() {
    if (!isNerve(this, NrvGateMapParts.Wait.data())) {
        return;
    }
    invalidateClipping(this);
    startNerveAction(this, "Open");
}

/**
 * Waits for the start switch.
 */
void GateMapParts::exeWait() {
    if (isFirstStep(this)) {
        validateClipping(this);
    }
}

/**
 * Opens the gate.
 */
void GateMapParts::exeOpen() {
    updatePose(calcNerveSquareInRate(this, mOpenTime - 1));
    if (isGreaterEqualStep(this, mOpenTime - 1)) {
        mCurrentBoundRate = mBoundRate;
        mCurrentBoundSteps = mOpenTime * mBoundRate * 2;
        if (mMaxHitReactions > 0 && mCurrentBoundSteps > 1) {
            startNerveAction(this, "Bound");
            return;
        }
        startNerveAction(this, "End");
        if (mHitReactionCount < 2) {
            startHitReaction(this, "バウンド1回目");
        }
    }
}

/**
 * Interpolates between the closed and open pose.
 * @param rate interpolation rate
 */
void GateMapParts::updatePose(f32 rate) {
    rate = sead::Mathf::clamp(rate, 0.0f, 1.0f);
    lerpVec(getTransPtr(this), mTrans, mMoveNextTrans, rate);
    slerpQuat(getQuatPtr(this), mQuat, mMoveNextQuat, rate);
}

/**
 * Bounces the gate after opening.
 */
void GateMapParts::exeBound() {
    if (isFirstStep(this)) {
        if (mHitReactionCurrent++ < mHitReactionCount) {
            startHitReaction(this, StringTmp<32>("バウンド%d回目", mHitReactionCurrent).cstr());
        }
        tryStartSeWithParam(this, "BoundStart",
                            static_cast<f32>(mMaxHitReactions - mHitReactionCurrent));
    }
    f32 rate = calcNerveRate(this, mCurrentBoundSteps - 1);
    rate = sead::Mathf::square(mCurrentBoundRate * (rate * 2 - 1.0f));
    rate += 1.0f - sead::Mathf::square(mCurrentBoundRate);
    updatePose(rate);
    if (isGreaterEqualStep(this, mCurrentBoundSteps - 1)) {
        mCurrentBoundRate *= mBoundRate;
        mCurrentBoundSteps = mBoundRate * mCurrentBoundSteps;
        if (mMaxHitReactions > mHitReactionCurrent && mCurrentBoundSteps > 1) {
            startNerveAction(this, "Bound");
            return;
        }
        startNerveAction(this, "End");
    }
}

/**
 * Keeps the gate open.
 */
void GateMapParts::exeEnd() {
    if (isFirstStep(this)) {
        validateClipping(this);
        updatePose(1.0f);
    }
}
}  // namespace al
