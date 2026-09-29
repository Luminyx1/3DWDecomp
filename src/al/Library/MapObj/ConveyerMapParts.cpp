#include "Library/MapObj/ConveyerMapParts.hpp"

#include "Library/LiveActor/Common/DeriveActorGroup.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/MapObj/ConveyerKeyKeeper.hpp"
#include "Library/MapObj/ConveyerStep.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"

namespace {
    using namespace al;

    NERVE_DECL(ConveyerMapParts, Move)
    NERVE_DECL(ConveyerMapParts, StandBy)
    NERVES_MAKE_NOSTRUCT(ConveyerMapParts, Move, StandBy)
}  // namespace

namespace al {
    /**
     * @brief Creates and registers a step for every slot of a step group.
     * @param pGroup The step group.
     * @param rInfo The actor init info used to create the steps.
     */
    inline void registerConveyerSteps(DeriveActorGroup<ConveyerStep>* pGroup, const ActorInitInfo& rInfo) {
        for (s32 i = 0; i < pGroup->getMaxActorCount(); i++) {
            ConveyerStep* step = new ConveyerStep("コンベア足場");
            initCreateActorWithPlacementInfo(step, rInfo);
            pGroup->registerActor(step);
        }
    }

    /**
     * @brief Constructs a conveyer that moves a group of steps along its keys.
     * @param pName The actor name.
     */
    ConveyerMapParts::ConveyerMapParts(const char* pName) : LiveActor(pName) {}

    /**
     * @brief Reads the conveyer keys and parameters and creates the steps.
     * @param rInfo The actor init info.
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
            isNearZero(totalMoveDistance, 0.001f);
        }

        s32 stepNum = static_cast<s32>(totalMoveDistance / mPartsInterval) + 1;
        mMaxCoord = mPartsInterval * static_cast<f32>(stepNum);

        f32 startRate = 0.0f;
        tryGetArg(&startRate, rInfo, "StartRate");
        mOffsetCoord = wrapValue(mPartsInterval * startRate, mPartsInterval);

        mConveyerStepGroup = new DeriveActorGroup<ConveyerStep>("コンベア足場リスト", stepNum);
        registerConveyerSteps(mConveyerStepGroup, rInfo);

        for (s32 i = 0; i < stepNum; i++) {
            ConveyerStep* step = mConveyerStepGroup->getDeriveActor(i);
            step->setHost(this);
            step->setConveyerKeyKeeper(mConveyerKeyKeeper, mMaxCoord);
            step->setTransAndResetByCoord(static_cast<f32>(i) * mPartsInterval + mOffsetCoord);
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
     * @brief Starts moving when the switch turns on.
     */
    void ConveyerMapParts::start() {
        if (isNerve(this, &NrvConveyerMapPartsStandBy)) {
            setNerve(this, &NrvConveyerMapPartsMove);
        }
    }

    /**
     * @brief Stops moving when the switch turns off.
     */
    void ConveyerMapParts::stop() {
        if (isNerve(this, &NrvConveyerMapPartsMove)) {
            setNerve(this, &NrvConveyerMapPartsStandBy);
        }
    }

    /**
     * @brief Remembers floor touches for ride-only conveyers.
     * @param pMsg The received message.
     * @param pSelf The receiving sensor.
     * @param pOther The sending sensor.
     * @return Whether the message was handled.
     */
    bool ConveyerMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pSelf, HitSensor* pOther) {
        if (isMsgFloorTouch(pMsg)) {
            mAddRideActiveFrames = 2;
            return true;
        }
        return false;
    }

    /**
     * @brief Accelerates while something rides the conveyer and slows down otherwise.
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
     * @brief Starts being clipped and stops drawing the steps' clipping.
     */
    void ConveyerMapParts::startClipped() {
        LiveActor::startClipped();
        for (s32 i = 0; i < mConveyerStepGroup->getActorCount(); i++) {
            offDrawClipping(mConveyerStepGroup->getActor(i));
        }
    }

    /**
     * @brief Ends being clipped and draws the steps' clipping again.
     */
    void ConveyerMapParts::endClipped() {
        LiveActor::endClipped();
        for (s32 i = 0; i < mConveyerStepGroup->getActorCount(); i++) {
            onDrawClipping(mConveyerStepGroup->getActor(i));
        }
    }

    /**
     * @brief Waits for the start switch.
     */
    void ConveyerMapParts::exeStandBy() {}

    /**
     * @brief Moves all steps along the conveyer.
     */
    void ConveyerMapParts::exeMove() {
        if (!mIsRideOnlyMove || mRideActiveFrames >= 1) {
            f32 speedRate =
                mIsRideOnlyMove ? static_cast<f32>(mRideActiveFrames) / static_cast<f32>(mMaxRideActiveFrames) : 1.0f;
            mOffsetCoord = wrapValue(mOffsetCoord + speedRate * mMoveSpeed, mMaxCoord);

            bool isForwards = mMoveSpeed >= 0.0f;
            s32 stepNum = mConveyerStepGroup->getActorCount();
            for (s32 i = 0; i < stepNum; i++) {
                mConveyerStepGroup->getDeriveActor(i)->setTransByCoord(
                    mPartsInterval * static_cast<f32>(i) + mOffsetCoord, isForwards);
            }
        }
    }
}  // namespace al
