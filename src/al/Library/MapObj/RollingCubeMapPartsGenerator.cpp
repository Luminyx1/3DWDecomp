#include "Library/MapObj/RollingCubeMapPartsGenerator.hpp"

#include "Library/LiveActor/Common/LiveActorGroup.hpp"
#include "Library/LiveActor/SubActorUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/MapObj/RollingCubeMapParts.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Project/Joint/RollingCubePoseKeeperUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(RollingCubeMapPartsGenerator, Generate)
NERVE_DECL(RollingCubeMapPartsGenerator, Delay)

NERVES_MAKE_NOSTRUCT(RollingCubeMapPartsGenerator, Generate, Delay)
}  // namespace

namespace al {
/**
 * Constructs a rolling cube generator.
 * @param pName actor name
 */
RollingCubeMapPartsGenerator::RollingCubeMapPartsGenerator(const char* pName)
    : LiveActor(pName) {}

/**
 * Creates the generated rolling cubes.
 * @param rInfo actor init info
 */
void RollingCubeMapPartsGenerator::init(const ActorInitInfo& rInfo) {
    initActorSceneInfo(this, rInfo);
    initExecutorMapObjMovement(this, rInfo);
    initActorPoseTQSV(this);
    s32 partsCount = 4;
    tryGetArg(&partsCount, rInfo, "PartsCount");
    tryGetArg(&mGenerateInterval, rInfo, "GenerateInterval");
    tryGetArg(&mDelayTime, rInfo, "DelayTime");
    initSubActorKeeperNoFile(this, rInfo, partsCount);
    mRollingCubeMapPartsGroup = new DeriveActorGroup<RollingCubeMapParts>(
        "ローリングキューブマップパーツリスト", partsCount);
    if (calcLinkChildNum(rInfo, "GenerateRollingCube") == 0) {
        makeActorDead();
        return;
    }

    for (s32 i = 0; i < partsCount; i++) {
        RollingCubeMapParts* rollingCube = new RollingCubeMapParts("ローリングキューブマップパーツ");
        initLinksActor(rollingCube, rInfo, "GenerateRollingCube", 0);
        rollingCube->mIsStoppable = true;
        invalidateClipping(rollingCube);
        rollingCube->makeActorDead();
        mRollingCubeMapPartsGroup->registerActor(rollingCube);
        registerSubActorSyncClipping(this, rollingCube, false);
    }

    RollingCubeMapParts* rollingCube = mRollingCubeMapPartsGroup->getDeriveActor(0);
    f32 clippingRadius = 0.0f;
    calcRollingCubeClippingInfo(&mClippingTrans, &clippingRadius,
                                rollingCube->mRollingCubePoseKeeper, 0.0f);
    initActorClipping(this, rInfo);
    setClippingInfo(this, clippingRadius, &mClippingTrans);
    initGroupClipping(this, rInfo, 64);
    initNerve(this, &NrvRollingCubeMapPartsGeneratorGenerate, 0);

    if (mDelayTime > 0) {
        setNerve(this, &NrvRollingCubeMapPartsGeneratorDelay);
    }

    getNerveKeeper()->update();
    initStageSwitch(this, rInfo);
    makeActorAppeared();
    tryListenStageSwitchKill(this);
}

/**
 * Kills the generator and all generated cubes.
 */
void RollingCubeMapPartsGenerator::kill() {
    LiveActor::kill();
    mRollingCubeMapPartsGroup->killAll();
}

/**
 * Waits for the start delay.
 */
void RollingCubeMapPartsGenerator::exeDelay() {
    if (isGreaterEqualStep(this, mDelayTime - 1)) {
        setNerve(this, &NrvRollingCubeMapPartsGeneratorGenerate);
    }
}

/**
 * Starts a dead cube in intervals.
 */
void RollingCubeMapPartsGenerator::exeGenerate() {
    if (isIntervalStep(this, mGenerateInterval, 0)) {
        RollingCubeMapParts* rollingCube = mRollingCubeMapPartsGroup->tryFindDeadDeriveActor();

        if (rollingCube) {
            rollingCube->appearAndSetStart();
        }
    }
}
}  // namespace al
