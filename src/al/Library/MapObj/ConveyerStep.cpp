#include "Library/MapObj/ConveyerStep.hpp"

#include "Library/LiveActor/ActorCollisionFunction.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/LiveActor/ConveyerKeyKeeper.hpp"

namespace {
using namespace al;

NERVE_DECL(ConveyerStep, Wait)

NERVES_MAKE_NOSTRUCT(ConveyerStep, Wait)
}  // namespace

namespace al {
/**
 * Constructs a conveyer step.
 * @param pName actor name
 */
ConveyerStep::ConveyerStep(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the step as a map part.
 * @param rInfo actor init info
 */
void ConveyerStep::init(const ActorInitInfo& rInfo) {
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, 0);
    initNerve(this, &NrvConveyerStepWait, 0);
    onDrawClipping(this);
    makeActorAppeared();
}

/**
 * Forwards messages to the host.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the host handled the message
 */
bool ConveyerStep::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (mHost != nullptr) {
        return mHost->receiveMsg(pMsg, pOther, pSelf);
    }

    return false;
}

/**
 * Sets the host actor.
 * @param pHost host actor
 */
void ConveyerStep::setHost(LiveActor* pHost) {
    mHost = pHost;
}

/**
 * Sets the conveyer keys and the loop length.
 * @param pConveyerKeyKeeper conveyer keys
 * @param coord loop length
 */
void ConveyerStep::setConveyerKeyKeeper(const ConveyerKeyKeeper* pConveyerKeyKeeper, f32 coord) {
    mConveyerKeyKeeper = pConveyerKeyKeeper;
    mMaxCoord = coord;
}

/**
 * Moves the step to a conveyer coordinate.
 * @param coord coordinate
 * @param isForwards whether the conveyer moves forwards
 */
void ConveyerStep::setTransByCoord(f32 coord, bool isForwards) {
    setTransByCoord(coord, isForwards, false);
}

/**
 * Moves the step to a conveyer coordinate.
 * @param coord coordinate
 * @param isForwards whether the conveyer moves forwards
 * @param isForceReset whether the position is always reset
 */
void ConveyerStep::setTransByCoord(f32 coord, bool isForwards, bool isForceReset) {
    f32 newCoord = wrapValue(coord, mMaxCoord);
    s32 index = -1;
    mConveyerKeyKeeper->calcPosAndQuat(getTransPtr(this), getQuatPtr(this), &index, newCoord);
    const char* keyHitReactionName = nullptr;
    const char* actionName = nullptr;

    if (index > -1) {
        const ConveyerKey& conveyerKey = mConveyerKeyKeeper->getConveyerKey(index);

        if (tryGetStringArg(&keyHitReactionName, *conveyerKey.mPlacementInfo,
                            "KeyHitReactionName") &&
            (mKeyHitReactionName == nullptr || !isEqualString(mKeyHitReactionName, keyHitReactionName))) {
            startHitReaction(this, keyHitReactionName);
        }

        if (tryGetStringArg(&actionName, *conveyerKey.mPlacementInfo, "ActionName") &&
            (mActionName == nullptr || !isEqualString(mActionName, actionName))) {
            startAction(this, actionName);
        }
    }

    mKeyHitReactionName = keyHitReactionName;
    mActionName = actionName;

    if ((isForwards && newCoord < mCurrentCoord) || (!isForwards && newCoord > mCurrentCoord) ||
        isForceReset) {
        resetPosition(this, false);
    }

    f32 totalMoveDistance = mConveyerKeyKeeper->getTotalMoveDistance();
    bool isHide = isHideModel(this);

    if (newCoord > totalMoveDistance) {
        if (!isHide) {
            if (isExistCollisionParts(this)) {
                invalidateCollisionParts(this);
            }

            hideModel(this);
        }
    } else if (isHide) {
        if (isExistCollisionParts(this)) {
            validateCollisionParts(this);
        }

        showModel(this);
    }

    mCurrentCoord = newCoord;
}

/**
 * Moves the step to a conveyer coordinate and resets its position.
 * @param coord coordinate
 */
void ConveyerStep::setTransAndResetByCoord(f32 coord) {
    setTransByCoord(coord, true, true);
}

/**
 * Does nothing.
 */
void ConveyerStep::exeWait() {}
}  // namespace al
