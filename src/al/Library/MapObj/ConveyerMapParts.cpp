#include "Library/MapObj/ConveyerMapParts.hpp"

#include "Library/LiveActor/Common/LiveActorGroup.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/MapObj/ConveyerStep.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/LiveActor/ConveyerKeyKeeper.hpp"

namespace {
using namespace al;

NERVE_DECL(ConveyerMapParts, Move)
NERVE_DECL(ConveyerMapParts, StandBy)

NERVES_MAKE_NOSTRUCT(ConveyerMapParts, Move, StandBy)
}  // namespace

namespace al {
static inline void registerConveyerSteps(DeriveActorGroup<ConveyerStep>* pGroup,
                                         const ActorInitInfo& rInfo) {
    for (s32 i = 0; i < pGroup->mMaxActors; i++) {
        ConveyerStep* conveyerStep = new ConveyerStep("コンベア足場");
        initCreateActorWithPlacementInfo(conveyerStep, rInfo);
        pGroup->registerActor(conveyerStep);
    }
}

/**
 * Constructs a conveyer map part.
 * @param pName actor name
 */
ConveyerMapParts::ConveyerMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the conveyer and creates its steps.
 * @param rInfo actor init info
 */
void ConveyerMapParts::init(const ActorInitInfo& rInfo) {
    using ConveyerMapPartsFunctor = FunctorV0M<ConveyerMapParts*, void (ConveyerMapParts::*)()>;

    initActorSceneInfo(this, rInfo);
    initExecutorMapObjMovement(this, rInfo);
    initActorPoseTQSV(this);
    initActorSRT(this, rInfo);
    initActorClipping(this, rInfo);
    initNerve(this, &NrvConveyerMapPartsMove, 0);
    initStageSwitch(this, rInfo);
    mConveyerKeyKeeper = new ConveyerKeyKeeper();
    mConveyerKeyKeeper->init(rInfo);
    tryGetArg(&mMoveSpeed, rInfo, "MoveSpeed");
    tryGetArg(&mPartsInterval, rInfo, "PartsInterval");
    tryGetArg(&mIsRideOnlyMove, rInfo, "IsRideOnlyMove");

    if (mPartsInterval < 10.0f) {
        mPartsInterval = 10.0f;
    }

    f32 totalMoveDistance = mConveyerKeyKeeper->getTotalMoveDistance();

    if (mConveyerKeyKeeper->getConveyerKeyCount() > 1) {
        isNearZero(totalMoveDistance);
    }

    s32 groupCount = static_cast<s32>(totalMoveDistance / mPartsInterval) + 1;
    mMaxCoord = mPartsInterval * groupCount;
    f32 startRate = 0.0f;
    tryGetArg(&startRate, rInfo, "StartRate");
    mOffsetCoord = wrapValue(mPartsInterval * startRate, mPartsInterval);
    mConveyerStepGroup = new DeriveActorGroup<ConveyerStep>("コンベア足場リスト", groupCount);
    registerConveyerSteps(mConveyerStepGroup, rInfo);

    for (s32 i = 0; i < groupCount; i++) {
        ConveyerStep* conveyerStep = mConveyerStepGroup->getDeriveActor(i);
        conveyerStep->setHost(this);
        conveyerStep->setConveyerKeyKeeper(mConveyerKeyKeeper, mMaxCoord);
        conveyerStep->setTransAndResetByCoord(static_cast<f32>(i) * mPartsInterval + mOffsetCoord);
    }

    f32 clippingRadius = 0.0f;
    mConveyerKeyKeeper->calcClippingSphere(&mClippingTrans, &clippingRadius,
                                           getClippingRadius(mConveyerStepGroup->getActor(0)));
    setClippingInfo(this, clippingRadius, &mClippingTrans);

    if (listenStageSwitchOnOffStart(this, ConveyerMapPartsFunctor(this, &ConveyerMapParts::start),
                                    ConveyerMapPartsFunctor(this, &ConveyerMapParts::stop))) {
        setNerve(this, &NrvConveyerMapPartsStandBy);
    }

    makeActorAppeared();
}

/**
 * Starts moving when the switch turns on.
 */
void ConveyerMapParts::start() {
    if (!isNerve(this, &NrvConveyerMapPartsStandBy)) {
        return;
    }

    setNerve(this, &NrvConveyerMapPartsMove);
}

/**
 * Stops moving when the switch turns off.
 */
void ConveyerMapParts::stop() {
    if (!isNerve(this, &NrvConveyerMapPartsMove)) {
        return;
    }

    setNerve(this, &NrvConveyerMapPartsStandBy);
}

/**
 * Registers floor touches for ride only conveyers.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool ConveyerMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (isMsgFloorTouch(pMsg)) {
        mAddRideActiveFrames = 2;
        return true;
    }

    return false;
}

/**
 * Updates the ride activity.
 */
void ConveyerMapParts::control() {
    if (mAddRideActiveFrames > 0) {
        if (mRideActiveFrames < mMaxRideActiveFrames) {
            mRideActiveFrames++;
        } else {
            mRideActiveFrames = mMaxRideActiveFrames;
        }

        mAddRideActiveFrames--;
        return;
    }

    if (mRideActiveFrames != 0) {
        mRideActiveFrames--;
    }
}

/**
 * Disables the draw clipping of the steps while clipped.
 */
void ConveyerMapParts::startClipped() {
    LiveActor::startClipped();

    for (s32 i = 0; i < mConveyerStepGroup->mNumActors; i++) {
        offDrawClipping(mConveyerStepGroup->getActor(i));
    }
}

/**
 * Enables the draw clipping of the steps again.
 */
void ConveyerMapParts::endClipped() {
    LiveActor::endClipped();

    for (s32 i = 0; i < mConveyerStepGroup->mNumActors; i++) {
        onDrawClipping(mConveyerStepGroup->getActor(i));
    }
}

/**
 * Waits for the switch.
 */
void ConveyerMapParts::exeStandBy() {}

/**
 * Moves the steps.
 */
void ConveyerMapParts::exeMove() {
    if (!mIsRideOnlyMove || mRideActiveFrames >= 1) {
        f32 speedFactor = mIsRideOnlyMove ? static_cast<f32>(mRideActiveFrames) /
                                                static_cast<f32>(mMaxRideActiveFrames) :
                                            1.0f;
        mOffsetCoord = wrapValue(mOffsetCoord + speedFactor * mMoveSpeed, mMaxCoord);
        bool isForwards = mMoveSpeed >= 0.0f;
        s32 actorCount = mConveyerStepGroup->mNumActors;

        for (s32 i = 0; i < actorCount; i++) {
            mConveyerStepGroup->getDeriveActor(i)->setTransByCoord(
                mPartsInterval * i + mOffsetCoord, isForwards);
        }
    }
}
}  // namespace al
