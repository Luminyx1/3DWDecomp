#include "Library/Fog/FogRequester.hpp"

#include "Library/Collision/PartsConnectorUtil.hpp"
#include "Library/Fog/FogDirector.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"

namespace al {

/**
 * @brief Creates the requester with its own fog and height fog parameters.
 * @param pName Actor name.
 */
FogRequester::FogRequester(const char* pName)
    : LiveActor(pName), mFogParam(new FogParam()), mYFogParam(new YFogParam()) {}

/**
 * @brief Reads the fog parameters from the placement and listens to the stage switches.
 * @param rInfo Actor init info.
 */
void FogRequester::init(const ActorInitInfo& rInfo) {
    initActorSceneInfo(this, rInfo);
    initActorPoseTQSV(this);
    initActorSRT(this, rInfo);
    initStageSwitch(this, rInfo);
    initExecutorUpdate(this, rInfo, "グラフィックス要求者");

    if (listenStageSwitchOnOffAppear(
            this, FunctorV0M<FogRequester*, void (FogRequester::*)()>(
                      this, &FogRequester::appearBySwitch),
            FunctorV0M<LiveActor*, void (LiveActor::*)()>(this, &LiveActor::kill))) {
        makeActorDead();
    } else {
        makeActorAppeared();
    }

    listenStageSwitchOnKill(
        this, FunctorV0M<LiveActor*, void (LiveActor::*)()>(this, &LiveActor::kill));

    mFogParam->init();
    mYFogParam->init();

    tryGetArg(&mFogParam->mColor->r, rInfo, "FogColorRed");
    tryGetArg(&mFogParam->mColor->g, rInfo, "FogColorGreen");
    tryGetArg(&mFogParam->mColor->b, rInfo, "FogColorBlue");
    tryGetArg(&mFogParam->mMulColor->r, rInfo, "FogMulColorRed");
    tryGetArg(&mFogParam->mMulColor->g, rInfo, "FogMulColorGreen");
    tryGetArg(&mFogParam->mMulColor->b, rInfo, "FogMulColorBlue");
    tryGetArg(&mFogParam->mMulColor->a, rInfo, "FogShadowIntensity");
    tryGetArg(&*mFogParam->mIntensityMax, rInfo, "FogIntensity");
    tryGetArg(&*mFogParam->mStart, rInfo, "FogRangeStart");
    tryGetArg(&*mFogParam->mEnd, rInfo, "FogRangeEnd");
    tryGetArg(&mFogPriority, rInfo, "FogPriority");
    tryGetArg(&mFogInterpStep, rInfo, "FogInterp");

    tryGetArg(&mYFogParam->mColor->r, rInfo, "YFogColorRed");
    tryGetArg(&mYFogParam->mColor->g, rInfo, "YFogColorGreen");
    tryGetArg(&mYFogParam->mColor->b, rInfo, "YFogColorBlue");
    tryGetArg(&mYFogParam->mMulColor->r, rInfo, "YFogMulColorRed");
    tryGetArg(&mYFogParam->mMulColor->g, rInfo, "YFogMulColorGreen");
    tryGetArg(&mYFogParam->mMulColor->b, rInfo, "YFogMulColorBlue");
    tryGetArg(&mYFogParam->mMulColor->a, rInfo, "YFogShadowIntensity");
    tryGetArg(&*mYFogParam->mIntensityMax, rInfo, "YFogIntensity");
    tryGetArg(&*mYFogParam->mStart, rInfo, "YFogRangeStart");
    tryGetArg(&*mYFogParam->mEnd, rInfo, "YFogRangeEnd");
    tryGetArg(&mYFogPriority, rInfo, "YFogPriority");
    tryGetArg(&mYFogInterpStep, rInfo, "YFogInterp");
    tryGetArg(&*mYFogParam->mIsFollowCamera, rInfo, "YFogFollowCamera");
    tryGetArg(&mIsYFogPlacementRelative, rInfo, "YFogPlacementRelative");
    tryGetArg(&mIsYFogConnectAndMove, rInfo, "YFogConnectAndMove");

    if (!isDead(this)) {
        control();
    }

    if (mIsYFogConnectAndMove) {
        mMtxConnector = createMtxConnector(this);
    }
}

/**
 * @brief Appears when the appear switch turns on and reattaches to the collision below.
 */
void FogRequester::appearBySwitch() {
    LiveActor::appear();

    if (mMtxConnector != nullptr) {
        attachMtxConnectorToCollision(mMtxConnector, this, false);
    }
}

/**
 * @brief Attaches to the collision below when the height fog moves with it.
 */
void FogRequester::initAfterPlacement() {
    if (mMtxConnector != nullptr) {
        attachMtxConnectorToCollision(mMtxConnector, this, false);
    }
}

/**
 * @brief Requests the placed fog parameters every frame.
 */
void FogRequester::control() {
    if (mMtxConnector != nullptr) {
        connectPoseQT(this, mMtxConnector);
        FogFunction::getFogDirector(this)->getYFogParam().validateCameraYPos();
        FogFunction::getFogDirector(this)->getYFogParam().setCameraYPos(getTrans(this).y);
    }

    if (mFogPriority != -1) {
        FogFunction::getFogDirector(this)->requestFog(mFogPriority, mFogInterpStep, *mFogParam);
    }

    if (mYFogPriority == -1) {
        return;
    }

    if (!mIsYFogPlacementRelative) {
        FogFunction::getFogDirector(this)->requestYFog(mYFogPriority, mYFogInterpStep,
                                                       *mYFogParam);
        return;
    }

    f32 transY = getTrans(this).y;
    YFogParam param = *mYFogParam;
    *param.mStart += transY;
    *param.mEnd += transY;
    FogFunction::getFogDirector(this)->requestYFog(mYFogPriority, mYFogInterpStep, param);
}

}  // namespace al
