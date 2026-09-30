#include "Library/Screen/ScreenPointTarget.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Screen/ScreenPointCheckGroup.hpp"
#include "Library/Screen/ScreenPointKeeper.hpp"
#include "Library/Screen/ScreenPointer.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Creates a screen point target.
 * @param pHost actor owning the target
 * @param pName target name
 * @param radius target radius
 * @param pFollowPos position to follow, or nullptr
 * @param pFollowMtx matrix to follow, or nullptr
 * @param rOffset offset from the followed position or matrix
 */
ScreenPointTarget::ScreenPointTarget(LiveActor* pHost, const char* pName, f32 radius,
                                     const sead::Vector3f* pFollowPos,
                                     const sead::Matrix34f* pFollowMtx,
                                     const sead::Vector3f& rOffset)
    : mName(pName), mRadius(radius), mFollowPos(pFollowPos), mFollowMtx(pFollowMtx),
      mOffset(rOffset), mHost(pHost) {}

/**
 * Updates the target position from the followed position or matrix.
 */
void ScreenPointTarget::update() {
    if (mFollowMtx) {
        mPos.setMul(*mFollowMtx, mOffset);
        return;
    }

    if (!mFollowPos) {
        return;
    }

    const sead::Matrix34f* baseMtx = mHost->getBaseMtx();
    if (baseMtx) {
        mPos.setRotated(*baseMtx, mOffset);
        mPos.add(*mFollowPos);
    } else {
        mPos.setAdd(*mFollowPos, mOffset);
    }
}

/**
 * Validates the target.
 */
void ScreenPointTarget::validate() {
    if (mIsValid) {
        return;
    }

    mIsValid = true;
    if (mIsValidBySystem) {
        mCheckGroup->setValid(this);
    }
}

/**
 * Invalidates the target.
 */
void ScreenPointTarget::invalidate() {
    if (!mIsValid) {
        return;
    }

    mIsValid = false;
    if (mIsValidBySystem) {
        mCheckGroup->setInvalid(this);
    }
}

/**
 * Validates the target on behalf of the system.
 */
void ScreenPointTarget::validateBySystem() {
    if (mIsValidBySystem) {
        return;
    }

    if (mIsValid) {
        mCheckGroup->setValid(this);
    }

    mIsValidBySystem = true;
}

/**
 * Invalidates the target on behalf of the system.
 */
void ScreenPointTarget::invalidateBySystem() {
    if (!mIsValidBySystem) {
        return;
    }

    if (mIsValid) {
        mCheckGroup->setInvalid(this);
    }

    mIsValidBySystem = false;
}

/**
 * Returns the hit position of the last target hit by a pointer.
 * @param pPointer screen pointer
 * @return hit position
 */
const sead::Vector3f& getHitScreenPointTargetPos(const ScreenPointer* pPointer) {
    return pPointer->getHitPos();
}

/**
 * Returns the hit normal of the last target hit by a pointer.
 * @param pPointer screen pointer
 * @return hit normal
 */
const sead::Vector3f& getHitScreenPointTargetNormal(const ScreenPointer* pPointer) {
    return pPointer->getHitNormal();
}

/**
 * Returns a screen point target of an actor by name.
 * @param pActor actor
 * @param pName target name
 * @return the target
 */
ScreenPointTarget* getScreenPointTarget(LiveActor* pActor, const char* pName) {
    return pActor->mScreenPointKeeper->getTarget(pName);
}

/**
 * Returns a screen point target of an actor by index.
 * @param pActor actor
 * @param index target index
 * @return the target
 */
ScreenPointTarget* getScreenPointTarget(LiveActor* pActor, s32 index) {
    return pActor->mScreenPointKeeper->getTarget(index);
}

/**
 * Returns the radius of a screen point target of an actor.
 * @param pActor actor
 * @param pName target name
 * @return target radius
 */
f32 getScreenPointTargetRadius(LiveActor* pActor, const char* pName) {
    return getScreenPointTarget(pActor, pName)->getRadius();
}

/**
 * Returns the position of a screen point target of an actor.
 * @param pActor actor
 * @param pName target name
 * @return target position
 */
const sead::Vector3f& getScreenPointTargetPos(LiveActor* pActor, const char* pName) {
    return getScreenPointTarget(pActor, pName)->getPos();
}

/**
 * Returns the position of a screen point target.
 * @param pTarget target
 * @return target position
 */
const sead::Vector3f& getScreenPointTargetPos(const ScreenPointTarget* pTarget) {
    return pTarget->getPos();
}

/**
 * Returns the radius of a screen point target.
 * @param pTarget target
 * @return target radius
 */
f32 getScreenPointTargetRadius(const ScreenPointTarget* pTarget) {
    return pTarget->getRadius();
}

/**
 * Returns the actor owning a screen point target.
 * @param pTarget target
 * @return owning actor
 */
LiveActor* getScreenPointTargetHost(ScreenPointTarget* pTarget) {
    return pTarget->getHost();
}

/**
 * Returns the offset of a screen point target of an actor.
 * @param pActor actor
 * @param pName target name
 * @return target offset
 */
const sead::Vector3f& getScreenPointTargetOffset(LiveActor* pActor, const char* pName) {
    return getScreenPointTarget(pActor, pName)->getOffset();
}

/**
 * Sets the radius of a screen point target of an actor.
 * @param pActor actor
 * @param pName target name
 * @param radius new radius
 */
void setScreenPointTargetRadius(LiveActor* pActor, const char* pName, f32 radius) {
    getScreenPointTarget(pActor, pName)->setRadius(radius);
}

/**
 * Sets the offset of a screen point target of an actor.
 * @param pActor actor
 * @param pName target name
 * @param rOffset new offset
 */
void setScreenPointTargetOffset(LiveActor* pActor, const char* pName,
                                const sead::Vector3f& rOffset) {
    getScreenPointTarget(pActor, pName)->setOffset(rOffset);
}

/**
 * Validates every screen point target of an actor.
 * @param pActor actor
 */
void validateScreenPointTargetAll(LiveActor* pActor) {
    if (pActor->mScreenPointKeeper) {
        pActor->mScreenPointKeeper->validate();
    }
}

/**
 * Invalidates every screen point target of an actor.
 * @param pActor actor
 */
void invalidateScreenPointTargetAll(LiveActor* pActor) {
    if (pActor->mScreenPointKeeper) {
        pActor->mScreenPointKeeper->invalidate();
    }
}

/**
 * Checks the name of a screen point target.
 * @param pTarget target
 * @param pName name to compare with
 * @return whether the names are equal
 */
bool isScreenPointTargetName(const ScreenPointTarget* pTarget, const char* pName) {
    return isEqualString(pTarget->getName(), pName);
}

/**
 * Checks whether a screen point target is valid.
 * @param pTarget target
 * @return whether the target is valid
 */
bool isScreenPointTargetValid(const ScreenPointTarget* pTarget) {
    return pTarget->isValid();
}

/**
 * Validates a screen point target of an actor.
 * @param pActor actor
 * @param pName target name
 */
void validateScreenPointTarget(LiveActor* pActor, const char* pName) {
    getScreenPointTarget(pActor, pName)->validate();
}

/**
 * Invalidates a screen point target of an actor.
 * @param pActor actor
 * @param pName target name
 */
void invalidateScreenPointTarget(LiveActor* pActor, const char* pName) {
    getScreenPointTarget(pActor, pName)->invalidate();
}
}  // namespace al
