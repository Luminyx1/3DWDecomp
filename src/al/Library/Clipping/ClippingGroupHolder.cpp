#include "Library/Clipping/ClippingGroupHolder.hpp"

#include "Library/Clipping/ClippingActorInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Project/Clipping/ClippingInfoGroup.hpp"

namespace al {
/**
 * Constructs an empty clipping group.
 * @param maxInfos capacity of the group
 */
ClippingInfoGroup::ClippingInfoGroup(s32 maxInfos)
    : mMaxInfos(maxInfos), mGroupId(new PlacementId()) {
    mInfos = new ClippingActorInfo*[mMaxInfos];
    for (s32 i = 0; i < mMaxInfos; i++) {
        mInfos[i] = nullptr;
    }
}

/**
 * Adds a clipping info to the group.
 * @param pInfo info to add
 */
void ClippingInfoGroup::registerInfo(ClippingActorInfo* pInfo) {
    mInfos[mNumInfos] = pInfo;
    mNumInfos++;
}

/**
 * Reads the clipping group id of the group.
 * @param rInfo actor init info
 */
void ClippingInfoGroup::setGroupId(const ActorInitInfo& rInfo) {
    alPlacementFunction::getClippingGroupId(mGroupId, rInfo);
}

/**
 * Checks whether an actor belongs to the group.
 * @param rInfo actor init info
 * @return true if the clipping group ids are equal
 */
bool ClippingInfoGroup::isEqualGroupId(const ActorInitInfo& rInfo) const {
    if (!mGroupId->mPlacementID) {
        return false;
    }

    PlacementId groupId;
    if (!alPlacementFunction::getClippingGroupId(&groupId, rInfo)) {
        return false;
    }

    return mGroupId->isEqual(groupId);
}

/**
 * Checks whether all living actors of the group should be clipped.
 * @param pJudge clipping judge
 * @return true if all actors should be clipped
 */
bool ClippingInfoGroup::judgeClippingAll(const ClippingJudge* pJudge) const {
    for (s32 i = 0; i < mNumInfos; i++) {
        if (isDead(mInfos[i]->getLiveActor())) {
            continue;
        }

        if (isInvalidClipping(mInfos[i]->getLiveActor())) {
            return false;
        }

        if (!mInfos[i]->judgeClipping(pJudge)) {
            return false;
        }
    }

    return true;
}

/**
 * Starts the clipping of all living actors of the group.
 */
void ClippingInfoGroup::startClippedAll() {
    mIsClipped = true;
    for (s32 i = 0; i < mNumInfos; i++) {
        if (!isDead(mInfos[i]->getLiveActor()) && !isClipped(mInfos[i]->getLiveActor())) {
            mInfos[i]->getLiveActor()->startClipped();
        }
    }
}

/**
 * Ends the clipping of all living actors of the group.
 */
void ClippingInfoGroup::endClippedAll() {
    mIsClipped = false;
    for (s32 i = 0; i < mNumInfos; i++) {
        if (!isDead(mInfos[i]->getLiveActor()) && isClipped(mInfos[i]->getLiveActor())) {
            mInfos[i]->getLiveActor()->endClipped();
        }
    }
}

/**
 * Constructs an empty clipping group holder.
 */
ClippingGroupHolder::ClippingGroupHolder() {
    mGroups = new ClippingInfoGroup*[64];
    for (s32 i = 0; i < 64; i++) {
        mGroups[i] = nullptr;
    }
}

/**
 * Clips or unclips each group as a whole.
 * @param pJudge clipping judge
 */
void ClippingGroupHolder::update(const ClippingJudge* pJudge) {
    for (s32 i = 0; i < mNumGroups; i++) {
        ClippingInfoGroup* group = mGroups[i];
        if (group->judgeClippingAll(pJudge)) {
            if (!group->mIsClipped) {
                group->startClippedAll();
            }
        } else if (group->mIsClipped) {
            group->endClippedAll();
        }
    }
}

/**
 * Adds a clipping info to the group with its clipping group id, creating the group if needed.
 * @param pInfo info to add
 * @param rInfo actor init info
 * @param maxInfos capacity of a new group
 */
void ClippingGroupHolder::createAndAdd(ClippingActorInfo* pInfo, const ActorInitInfo& rInfo,
                                       s32 maxInfos) {
    ClippingInfoGroup* group = tryFindGroup(rInfo);
    if (!group) {
        group = new ClippingInfoGroup(maxInfos);
        group->setGroupId(rInfo);
        mGroups[mNumGroups] = group;
        mNumGroups++;
    }

    group->registerInfo(pInfo);
}

/**
 * Finds the group with the clipping group id of an actor.
 * @param rInfo actor init info
 * @return the group, or nullptr if it doesn't exist
 */
ClippingInfoGroup* ClippingGroupHolder::tryFindGroup(const ActorInitInfo& rInfo) {
    for (s32 i = 0; i < mNumGroups; i++) {
        if (mGroups[i]->isEqualGroupId(rInfo)) {
            return mGroups[i];
        }
    }

    return nullptr;
}
}  // namespace al
