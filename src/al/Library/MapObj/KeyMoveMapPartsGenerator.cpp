#include "Library/MapObj/KeyMoveMapPartsGenerator.hpp"

#include "Library/KeyPose/KeyPoseKeeperUtil.hpp"
#include "Library/LiveActor/Common/LiveActorGroup.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/MapObj/KeyMoveMapParts.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"

namespace {
using namespace al;

NERVE_DECL(KeyMoveMapPartsGenerator, Generate)
NERVE_DECL(KeyMoveMapPartsGenerator, Delay)

NERVES_MAKE_NOSTRUCT(KeyMoveMapPartsGenerator, Generate, Delay)
}  // namespace

namespace al {
/**
 * Constructs a key moving map part generator.
 * @param pName actor name
 */
KeyMoveMapPartsGenerator::KeyMoveMapPartsGenerator(const char* pName) : LiveActor(pName) {}

/**
 * Creates the generated map parts.
 * @param rInfo actor init info
 */
void KeyMoveMapPartsGenerator::init(const ActorInitInfo& rInfo) {
    initActorSceneInfo(this, rInfo);
    initExecutorMapObjMovement(this, rInfo);
    initActorPoseTQSV(this);
    s32 partsCount = 4;
    tryGetArg(&partsCount, rInfo, "PartsCount");
    tryGetArg(&mGenerateInterval, rInfo, "GenerateInterval");
    tryGetArg(&mDelayTime, rInfo, "DelayTime");
    initSubActorKeeperNoFile(this, rInfo, partsCount);
    mKeyMoveMapPartsGroup =
        new DeriveActorGroup<KeyMoveMapParts>("キー移動マップパーツリスト", partsCount);
    if (calcLinkChildNum(rInfo, "Generate") == 0) {
        makeActorDead();
        return;
    }

    for (s32 i = 0; i < partsCount; i++) {
        KeyMoveMapParts* keyMoveMapParts = new KeyMoveMapParts("キー移動マップマップパーツ");
        initLinksActor(keyMoveMapParts, rInfo, "Generate", 0);
        keyMoveMapParts->mIsStopKill = true;
        keyMoveMapParts->makeActorDead();
        mKeyMoveMapPartsGroup->registerActor(keyMoveMapParts);
    }

    KeyMoveMapParts* keyMoveMapParts = mKeyMoveMapPartsGroup->getDeriveActor(0);
    f32 clippingRadius = 0.0f;
    calcKeyMoveClippingInfo(&mClippingTrans, &clippingRadius, keyMoveMapParts->getKeyPoseKeeper(),
                            500.0f);
    initActorClipping(this, rInfo);
    setClippingInfo(this, clippingRadius, &mClippingTrans);
    initGroupClipping(this, rInfo, 64);
    initNerve(this, &NrvKeyMoveMapPartsGeneratorGenerate, 0);

    if (mDelayTime > 0) {
        setNerve(this, &NrvKeyMoveMapPartsGeneratorDelay);
    }

    getNerveKeeper()->update();
    initStageSwitch(this, rInfo);
    makeActorAppeared();
}

/**
 * Waits for the start delay.
 */
void KeyMoveMapPartsGenerator::exeDelay() {
    if (isGreaterEqualStep(this, mDelayTime - 1)) {
        setNerve(this, &NrvKeyMoveMapPartsGeneratorGenerate);
    }
}

/**
 * Starts a dead map part in intervals.
 */
void KeyMoveMapPartsGenerator::exeGenerate() {
    if (isIntervalStep(this, mGenerateInterval, 0)) {
        KeyMoveMapParts* keyMoveMapParts = mKeyMoveMapPartsGroup->tryFindDeadDeriveActor();

        if (keyMoveMapParts != nullptr) {
            keyMoveMapParts->appearAndSetStart();
        }
    }
}
}  // namespace al
