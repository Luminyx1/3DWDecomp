#include "Library/Light/DirectionalLightRequester.hpp"

#include "Library/Light/DirectionalLightKeeper.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"

namespace al {
/**
 * Creates the requester with a default directional light parameter.
 * @param pName Actor name.
 */
DirectionalLightRequester::DirectionalLightRequester(const char* pName)
    : LiveActor(pName), mParam(new DirLightParam()) {}

/**
 * Reads the light parameters from the placement arguments.
 * @param rInfo Actor init info.
 */
void DirectionalLightRequester::init(const ActorInitInfo& rInfo) {
    initActorSceneInfo(this, rInfo);
    initActorPoseTRSV(this);
    initActorSRT(this, rInfo);
    initStageSwitch(this, rInfo);
    initExecutorUpdate(this, rInfo, "グラフィックス要求者");
    bool isSwitchAppearOnly = false;
    tryGetArg(&isSwitchAppearOnly, rInfo, "IsSwitchAppearOnly");

    if (isSwitchAppearOnly) {
        tryListenStageSwitchAppear(this);
    } else {
        trySyncStageSwitchAppear(this);
    }

    tryGetArg(&mParam->getColor().r, rInfo, "ColorRed");
    tryGetArg(&mParam->getColor().g, rInfo, "ColorGreen");
    tryGetArg(&mParam->getColor().b, rInfo, "ColorBlue");
    tryGetArg(&mPriority, rInfo, "Priority");
    tryGetArg(&mInterpFrame, rInfo, "InterpFrame");
    tryGetArg(&mParam->getSpcColor().r, rInfo, "SpecularColorRed");
    tryGetArg(&mParam->getSpcColor().g, rInfo, "SpecularColorGreen");
    tryGetArg(&mParam->getSpcColor().b, rInfo, "SpecularColorBlue");
    tryGetArg(&mParam->getSpcPower(), rInfo, "SpecularPower");
    tryGetArg(&mIsUsingSpecularDir, rInfo, "UsingSpecularDir");
    tryGetArg(&mSpecularDirDegree.x, rInfo, "SpecularDirX");
    tryGetArg(&mSpecularDirDegree.y, rInfo, "SpecularDirY");
    tryGetArg(&mSpecularDirDegree.z, rInfo, "SpecularDirZ");
    mParam->getSpcDirection()->syncFromRPYDegree(mSpecularDirDegree);

    if (!isDead(this)) {
        control();
    }
}

/**
 * Updates the light directions from the actor pose and requests the light.
 */
void DirectionalLightRequester::control() {
    sead::Vector3f up;
    calcUpDir(&up, this);
    mParam->getDirection()->syncFromDirection(-up);

    if (mIsUsingSpecularDir) {
        mParam->getSpcDirection()->syncFromRPYDegree(mSpecularDirDegree);
    } else {
        mParam->getSpcDirection()->syncFromDirection(mParam->getDirection()->getDirection());
    }

    DirLightFunction::getDirectionalLightKeeper(this)->requestDirectionalLight(
        mPriority, mInterpFrame, *mParam);
}

}  // namespace al
