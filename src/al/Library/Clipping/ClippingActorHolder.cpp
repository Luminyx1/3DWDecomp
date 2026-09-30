#include "Library/Clipping/ClippingActorInfo.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/LiveActorFlag.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"

namespace al {
/**
 * Creates the clipping actor holder.
 * @param maxActors maximum number of actors
 */
ClippingActorHolder::ClippingActorHolder(s32 maxActors) : mMaxActors(maxActors) {
    mClippingTargets = new ClippingActorInfoList(maxActors);
    mInvalidClippings = new ClippingActorInfoList(mMaxActors);
    mNonClippingTargets = new ClippingActorInfoList(mMaxActors);
    mGroupClippings = new ClippingActorInfoList(mMaxActors);
}

/**
 * Updates the clipping of all clipping targets.
 * @param pJudge clipping judge
 */
void ClippingActorHolder::update(const ClippingJudge* pJudge) {
    for (s32 i = 0; i < mClippingTargets->mNumInfos; i++) {
        mClippingTargets->mInfos[i]->updateClipping(pJudge);
    }
}

/**
 * Registers an actor for clipping without making it a clipping target yet.
 * @param pActor actor to register
 * @return the clipping info of the actor
 */
ClippingActorInfo* ClippingActorHolder::registerActor(LiveActor* pActor) {
    LiveActorFlag* flags = pActor->mActorFlags;
    flags->isInvalidClipping = false;
    flags->_1c = false;
    ClippingActorInfo* info = new ClippingActorInfo(pActor);
    mNonClippingTargets->add(info);
    mNumActors++;
    return info;
}

/**
 * Moves an actor to group clipping and reads its clipping group id.
 * @param pActor actor to move
 * @param rInfo actor init info
 * @return the clipping info of the actor
 */
ClippingActorInfo* ClippingActorHolder::initGroupClipping(LiveActor* pActor,
                                                          const ActorInitInfo& rInfo) {
    ClippingActorInfo* info;
    if (mClippingTargets->isInList(pActor)) {
        info = mClippingTargets->remove(pActor);
        mGroupClippings->add(info);
    } else if (mNonClippingTargets->isInList(pActor)) {
        info = mNonClippingTargets->find(pActor, nullptr);
    } else if (mInvalidClippings->isInList(pActor)) {
        info = mInvalidClippings->remove(pActor);
        mGroupClippings->add(info);
        if (isClipped(pActor)) {
            pActor->endClipped();
        }
    } else {
        info = nullptr;
    }
    info->setGroupClippingId(rInfo);
    return info;
}

/**
 * Enables the clipping of an actor.
 * @param pActor actor to enable clipping for
 */
void ClippingActorHolder::validateClipping(LiveActor* pActor) {
    LiveActorFlag* flags = pActor->mActorFlags;
    flags->isInvalidClipping = false;
    flags->_1c = false;
    ClippingActorInfo* info = mInvalidClippings->remove(pActor);
    if (isDead(pActor)) {
        mNonClippingTargets->add(info);
    } else if (info->isGroupClipping()) {
        mGroupClippings->add(info);
    } else {
        mClippingTargets->add(info);
    }
}

/**
 * Disables the clipping of an actor and ends its clipping.
 * @param pActor actor to disable clipping for
 */
void ClippingActorHolder::invalidateClipping(LiveActor* pActor) {
    LiveActorFlag* flags = pActor->mActorFlags;
    flags->isInvalidClipping = true;
    flags->_1c = false;
    ClippingActorInfoList* list;
    if (isDead(pActor)) {
        list = mNonClippingTargets;
    } else {
        list = mClippingTargets->isInList(pActor) ? mClippingTargets : mGroupClippings;
    }
    ClippingActorInfo* info = list->remove(pActor);
    mInvalidClippings->add(info);
    if (isClipped(pActor)) {
        pActor->endClipped();
    }
}

/**
 * Makes an actor a clipping target.
 * @param pActor actor to add
 */
void ClippingActorHolder::addToClippingTarget(LiveActor* pActor) {
    if (isInvalidClipping(pActor) || mClippingTargets->isInList(pActor) ||
        mGroupClippings->isInList(pActor)) {
        return;
    }
    ClippingActorInfo* info = mNonClippingTargets->remove(pActor);
    if (info->isGroupClipping()) {
        mGroupClippings->add(info);
    } else {
        mClippingTargets->add(info);
    }
}

/**
 * Stops an actor from being a clipping target.
 * @param pActor actor to remove
 */
void ClippingActorHolder::removeFromClippingTarget(LiveActor* pActor) {
    if (isInvalidClipping(pActor) || mNonClippingTargets->isInList(pActor)) {
        return;
    }
    ClippingActorInfoList* list =
        mGroupClippings->isInList(pActor) ? mGroupClippings : mClippingTargets;
    ClippingActorInfo* info = list->remove(pActor);
    mNonClippingTargets->add(info);
}

/**
 * Gets the clipping radius of an actor.
 * @param pActor actor to look for
 * @return the clipping radius
 */
f32 ClippingActorHolder::getClippingRadius(const LiveActor* pActor) {
    return find(pActor)->mClippingRadius;
}

/**
 * Finds the clipping info of an actor in all lists.
 * @param pActor actor to look for
 * @return the clipping info
 */
ClippingActorInfo* ClippingActorHolder::find(const LiveActor* pActor) const {
    ClippingActorInfo* info = mClippingTargets->tryFind(pActor);
    if (info) {
        return info;
    }
    info = mNonClippingTargets->tryFind(pActor);
    if (info) {
        return info;
    }
    info = mGroupClippings->tryFind(pActor);
    if (info) {
        return info;
    }
    return mInvalidClippings->find(pActor, nullptr);
}

/**
 * Gets the clipping center of an actor.
 * @param pActor actor to look for
 * @return pointer to the clipping center
 */
const sead::Vector3f& ClippingActorHolder::getClippingCenterPos(const LiveActor* pActor) {
    return *find(pActor)->mTransPtr;
}

/**
 * Sets the clipping shape of an actor to a sphere.
 * @param pActor actor to change
 * @param radius radius of the sphere
 * @param pPos center of the sphere, or nullptr to use the actor position
 */
void ClippingActorHolder::setTypeToSphere(LiveActor* pActor, f32 radius,
                                          const sead::Vector3f* pPos) {
    find(pActor)->setTypeToSphere(radius, pPos);
}

/**
 * Sets the near clip distance of an actor.
 * @param pActor actor to change
 * @param distance near clip distance
 */
void ClippingActorHolder::setNearClipDistance(LiveActor* pActor, f32 distance) {
    find(pActor)->mNearClipDistance = distance;
}

/**
 * Sets the far clip level of an actor.
 * @param pActor actor to change
 * @param level far clip level
 */
void ClippingActorHolder::setFarClipLevel(LiveActor* pActor, s32 level) {
    find(pActor)->mFarClipLevel = level;
}
}  // namespace al
