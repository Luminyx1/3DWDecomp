#include "Library/MapObj/ConveyerStep.hpp"

#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/MapObj/ConveyerKeyKeeper.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Play/Placement/PlacementUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Collision/CollisionPartsKeeperUtil.hpp"

namespace {
    using namespace al;

    NERVE_DECL(ConveyerStep, Wait)
    NERVES_MAKE_NOSTRUCT(ConveyerStep, Wait)
}  // namespace

namespace al {
    /**
     * @brief Constructs a step moved along a conveyer.
     * @param pName The actor name.
     */
    ConveyerStep::ConveyerStep(const char* pName) : LiveActor(pName) {}

    /**
     * @brief Initializes the step model and collision.
     * @param rInfo The actor init info.
     */
    void ConveyerStep::init(const ActorInitInfo& rInfo) {
        initActorPoseTQSV(this);
        initMapPartsActor(this, rInfo, nullptr, 0);
        initNerve(this, &NrvConveyerStepWait, 0);
        onDrawClipping(this);
        makeActorAppeared();
    }

    /**
     * @brief Forwards received messages to the host conveyer.
     * @param pMsg The received message.
     * @param pSelf The receiving sensor.
     * @param pOther The sending sensor.
     * @return Whether the host handled the message.
     */
    bool ConveyerStep::receiveMsg(const SensorMsg* pMsg, HitSensor* pSelf, HitSensor* pOther) {
        if (mHost != nullptr) {
            return mHost->receiveMsg(pMsg, pSelf, pOther);
        }
        return false;
    }

    /**
     * @brief Sets the host conveyer.
     * @param pHost The host actor.
     */
    void ConveyerStep::setHost(LiveActor* pHost) {
        mHost = pHost;
    }

    /**
     * @brief Sets the keys the step moves along.
     * @param pKeyKeeper The conveyer key keeper.
     * @param maxCoord The length after which the coordinate wraps around.
     */
    void ConveyerStep::setConveyerKeyKeeper(const ConveyerKeyKeeper* pKeyKeeper, f32 maxCoord) {
        mConveyerKeyKeeper = pKeyKeeper;
        mMaxCoord = maxCoord;
    }

    /**
     * @brief Moves the step to a coordinate along the conveyer.
     * @param coord The coordinate.
     * @param isForwards Whether the conveyer moves forwards.
     */
    void ConveyerStep::setTransByCoord(f32 coord, bool isForwards) {
        setTransByCoord(coord, isForwards, false);
    }

    /**
     * @brief Moves the step to a coordinate, playing key reactions and hiding it past the last key.
     * @param coord The coordinate.
     * @param isForwards Whether the conveyer moves forwards.
     * @param isForceReset Whether to always reset the position interpolation.
     */
    void ConveyerStep::setTransByCoord(f32 coord, bool isForwards, bool isForceReset) {
        f32 newCoord = modf(coord + mMaxCoord, mMaxCoord) + 0.0f;
        s32 keyIndex = -1;
        mConveyerKeyKeeper->calcPosAndQuat(getTransPtr(this), getQuatPtr(this), &keyIndex, newCoord);

        const char* keyHitReactionName = nullptr;
        const char* actionName = nullptr;
        if (keyIndex >= 0) {
            const ConveyerKey& key = mConveyerKeyKeeper->getConveyerKey(keyIndex);

            if (tryGetStringArg(&keyHitReactionName, *key.mPlacementInfo, "KeyHitReactionName") &&
                (mKeyHitReactionName == nullptr || !isEqualString(mKeyHitReactionName, keyHitReactionName))) {
                startHitReaction(this, keyHitReactionName);
            }

            if (tryGetStringArg(&actionName, *key.mPlacementInfo, "ActionName") &&
                (mActionName == nullptr || !isEqualString(mActionName, actionName))) {
                startAction(this, actionName);
            }
        }

        mKeyHitReactionName = keyHitReactionName;
        mActionName = actionName;

        if ((isForwards && newCoord < mCurrentCoord) || (!isForwards && newCoord > mCurrentCoord) || isForceReset) {
            resetPosition(this, false);
        }

        f32 totalMoveDistance = mConveyerKeyKeeper->getTotalMoveDistance();
        bool isHidden = isHideModel(this);
        if (newCoord > totalMoveDistance) {
            if (!isHidden) {
                if (isExistCollisionParts(this)) {
                    invalidateCollisionParts(this);
                }
                hideModel(this);
            }
        } else if (isHidden) {
            if (isExistCollisionParts(this)) {
                validateCollisionParts(this);
            }
            showModel(this);
        }

        mCurrentCoord = newCoord;
    }

    /**
     * @brief Moves the step to a coordinate and resets its position interpolation.
     * @param coord The coordinate.
     */
    void ConveyerStep::setTransAndResetByCoord(f32 coord) {
        setTransByCoord(coord, true, true);
    }

    /**
     * @brief Waits.
     */
    void ConveyerStep::exeWait() {}
}  // namespace al
