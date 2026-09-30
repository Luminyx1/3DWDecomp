#include "Project/Block/BlockRail.hpp"

#include <math/seadMatrix.h>

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/LiveActor/Util/ActorAnimUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Block/BlockRailLink.hpp"

namespace al {
/**
 * Constructs a block rail.
 * @param pName actor name
 */
BlockRail::BlockRail(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the rail link, end model and rail color.
 * @param rInfo actor init info
 */
void BlockRail::init(const ActorInitInfo& rInfo) {
    initMapPartsActor(this, rInfo, nullptr, 0);
    ByamlIter iter(getMapPartsResourceYaml(rInfo, "BlockRailParam"));
    mRailLink = new BlockRailLink(2);
    mRailLink->init(rInfo, iter);
    iter.tryGetStringByKey(&mEndModelName, "EndModel");
    tryGetArg(&mRailColor, rInfo, "RailColor");
    registerBlockRail(this);
    trySyncStageSwitchAppearAndKill(this);
    if (isExistModel(this) && tryStartMclAnimIfExist(this, "RailColor")) {
        setMclAnimFrameAndStop(this, mRailColor);
    }
}

/**
 * Appears with the rail ends and enables riding.
 */
void BlockRail::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    mRailLink->validateRide();
    if (mStartRailEnd) {
        mStartRailEnd->makeActorAppeared();
    }
    if (mEndRailEnd) {
        mEndRailEnd->makeActorAppeared();
    }
}

/**
 * Dies with the rail ends and disables riding.
 */
void BlockRail::makeActorDead() {
    LiveActor::makeActorDead();
    mRailLink->invalidateRide();
    if (mStartRailEnd) {
        mStartRailEnd->makeActorDead();
    }
    if (mEndRailEnd) {
        mEndRailEnd->makeActorDead();
    }
}

/**
 * Connects two rails whose ends are close to each other.
 * @param pRailA first rail
 * @param pRailB second rail
 */
void BlockRail::tryConnect(BlockRail* pRailA, BlockRail* pRailB) {
    BlockRailLink::tryConnect(pRailA->mRailLink, pRailB->mRailLink, 10.0f);
}

/**
 * Creates end models at the rail ends that aren't connected.
 * @param rInfo actor init info
 */
void BlockRail::tryCreateRailEnd(const ActorInitInfo& rInfo) {
    if (mRailLink->isTerminate()) {
        return;
    }
    if (mRailLink->getPrevLinkNum() == 0) {
        sead::Vector3f pos;
        sead::Vector3f dir;
        mRailLink->calcPosAndDir(&pos, &dir, 0.0f);
        createRailEnd(rInfo, pos, -dir, &mStartRailEnd);
    }
    if (mRailLink->getNextLinkNum() == 0) {
        sead::Vector3f pos;
        sead::Vector3f dir;
        mRailLink->calcPosAndDir(&pos, &dir, 1.0f);
        createRailEnd(rInfo, pos, dir, &mEndRailEnd);
    }
}

/**
 * Creates an end model.
 * @param rInfo actor init info
 * @param rPos end position
 * @param rDir end direction
 * @param pRailEnd output end model
 */
void BlockRail::createRailEnd(const ActorInitInfo& rInfo, const sead::Vector3f& rPos,
                              const sead::Vector3f& rDir, LiveActor** pRailEnd) {
    if (!mEndModelName) {
        return;
    }
    ActorInitInfo info;
    info.initViewIdHostActor(rInfo, this);
    LiveActor* railEnd = new LiveActor("ブロックレール終端");
    if (rInfo.mActorSceneInfo.isSingleMode) {
        addToHostActorClipping(railEnd, this);
    }
    initActorWithArchiveName(railEnd, info, mEndModelName, nullptr);
    sead::Vector3f up;
    calcUpDir(&up, this);
    sead::Matrix34f mtx;
    makeMtxUpFrontPos(&mtx, up, rDir, rPos);
    updatePoseMtx(railEnd, &mtx);
    if (isExistModel(this) && tryStartMclAnimIfExist(railEnd, "RailColor")) {
        setMclAnimFrameAndStop(railEnd, mRailColor);
    }
    if (isAlive(this)) {
        railEnd->appear();
    } else {
        railEnd->kill();
    }
    if (pRailEnd) {
        *pRailEnd = railEnd;
    }
}
}  // namespace al
