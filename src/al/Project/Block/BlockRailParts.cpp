#include "Project/Block/BlockRailParts.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/LiveActor/LiveActorFunc.hpp"
#include "Library/LiveActor/SubActorKeeper.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Block/BlockRailLink.hpp"

namespace al {
/**
 * Constructs a block rail part.
 * @param pName actor name
 */
BlockRailParts::BlockRailParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the model, rail links and a copy of the placement.
 * @param rInfo actor init info
 */
void BlockRailParts::init(const ActorInitInfo& rInfo) {
    mModelSuffix = nullptr;
    tryGetStringArg(&mModelSuffix, rInfo, "ModelSuffix");
    initActorChangeModelSuffix(this, rInfo, mModelSuffix);
    initRailLink(rInfo);
    mPlacementInfo = new PlacementInfo(*rInfo.mPlacementInfo);
    mInitInfo = new ActorInitInfo();
    mInitInfo->initViewIdSelf(mPlacementInfo, rInfo);
    makeActorAppeared();
}

/**
 * Creates the rail links from the map parts parameters.
 * @param rInfo actor init info
 */
void BlockRailParts::initRailLink(const ActorInitInfo& rInfo) {
    ByamlIter iter(getMapPartsResourceYaml(rInfo, "BlockRailParam"));
    initRailLink(iter);
}

/**
 * Appears with a hit reaction.
 */
void BlockRailParts::appear() {
    LiveActor::appear();
    startHitReactionAppear(this);
}

/**
 * Switches to the far model unless the model is hidden.
 */
void BlockRailParts::startFarLod() {
    if (mIsHideModel) {
        return;
    }

    LiveActor::startFarLod();
}

/**
 * Switches back from the far model unless the model is hidden.
 */
void BlockRailParts::endFarLod() {
    if (mIsHideModel) {
        return;
    }

    LiveActor::endFarLod();
}

/**
 * Calculates the offsets of this part and its links from the base translation.
 * @param rBaseTrans base translation
 */
void BlockRailParts::calcOffset(const sead::Vector3f& rBaseTrans) {
    mOffset = getTrans(this) - rBaseTrans;
    for (s32 i = 0; i < mLinkNum; i++) {
        mLinks[i]->calcOffset(rBaseTrans);
    }
}

/**
 * Moves this part and its links with the base translation.
 * @param rBaseTrans base translation
 */
void BlockRailParts::updateLinkedTrans(const sead::Vector3f& rBaseTrans) {
    alLiveActorFunction::forceUpdateTrans(this, rBaseTrans + mOffset, true);
    for (s32 i = 0; i < mLinkNum; i++) {
        mLinks[i]->updateLinkedTrans(rBaseTrans);
    }
}

/**
 * Gets the init info created for this part.
 * @return init info
 */
ActorInitInfo* BlockRailParts::getInitInfo() const {
    return mInitInfo;
}

/**
 * Creates the rail links from the model parameters.
 */
void BlockRailParts::initRailLink() {
    ByamlIter iter(getModelResourceYaml(this, "BlockRailParam", nullptr));
    initRailLink(iter);
}

/**
 * Creates the rail links and connects the linked pairs.
 * @param rIter rail parameters
 */
void BlockRailParts::initRailLink(const ByamlIter& rIter) {
    ByamlIter shapeListIter;
    if (!rIter.tryGetIterByKey(&shapeListIter, "ShapeList")) {
        return;
    }

    mLinkNum = shapeListIter.isTypeArray() ? shapeListIter.getSize() : 1;
    mLinks = new BlockRailLink*[mLinkNum];
    if (shapeListIter.isTypeArray()) {
        for (s32 i = 0; i < mLinkNum; i++) {
            ByamlIter shapeIter;
            shapeListIter.tryGetIterByIndex(&shapeIter, i);
            mLinks[i] = new BlockRailLink(3);
            mLinks[i]->init(getQuat(this), getTrans(this), shapeIter);
        }
    } else {
        mLinks[0] = new BlockRailLink(2);
        mLinks[0]->init(getQuat(this), getTrans(this), shapeListIter);
    }

    ByamlIter linkListIter;
    if (!rIter.tryGetIterByKey(&linkListIter, "LinkList")) {
        return;
    }

    s32 linkListNum = linkListIter.getSize();
    for (s32 i = 0; i < linkListNum; i++) {
        ByamlIter pairIter;
        if (!linkListIter.tryGetIterByIndex(&pairIter, i)) {
            continue;
        }

        if (pairIter.getSize() != 2) {
            continue;
        }

        s32 indexA;
        if (!pairIter.tryGetIntByIndex(&indexA, 0)) {
            continue;
        }

        if (indexA < 0 || indexA >= mLinkNum) {
            continue;
        }

        s32 indexB;
        if (!pairIter.tryGetIntByIndex(&indexB, 1)) {
            continue;
        }

        if (indexB < 0 || indexB >= mLinkNum) {
            continue;
        }

        if (indexA == indexB) {
            continue;
        }

        BlockRailLink::tryConnect(mLinks[indexA], mLinks[indexB], 10.0f);
    }
}

/**
 * Gets a rail link.
 * @param index link index
 * @return rail link
 */
BlockRailLink* BlockRailParts::getLink(s32 index) const {
    return mLinks[index];
}

/**
 * Connects the rail links of two parts.
 * @param pPartsA first part
 * @param pPartsB second part
 */
void BlockRailParts::tryConnect(BlockRailParts* pPartsA, BlockRailParts* pPartsB) {
    s32 linkNumA = pPartsA->mLinkNum;
    s32 linkNumB = pPartsB->mLinkNum;
    for (s32 i = 0; i < linkNumA; i++) {
        for (s32 j = 0; j < linkNumB; j++) {
            BlockRailLink::tryConnect(pPartsA->mLinks[i], pPartsB->mLinks[j], 10.0f);
        }
    }
}

/**
 * Sets whether the model is hidden, hiding the model and sub actors if so.
 * @param isHide whether the model is hidden
 */
void BlockRailParts::setIsHideModel(bool isHide) {
    mIsHideModel = isHide;
    if (!isHide) {
        return;
    }

    SubActorKeeper* keeper = mSubActorKeeper;
    if (keeper) {
        for (s32 i = 0; i < keeper->mCount; i++) {
            keeper->mInfos[i]->mSyncType |= 4;
        }
    }

    hideModelIfShow(this);
}
}  // namespace al
