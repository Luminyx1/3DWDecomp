#include "Library/Screen/ScreenPointTarget.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Screen/ScreenPointCheckGroup.hpp"
#include "Library/Screen/ScreenPointKeeper.hpp"
#include "Library/Screen/ScreenPointer.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * @brief Constructs a target that pointers can hit.
 * @param pActor The owning actor.
 * @param pName The target name.
 * @param radius The target radius.
 * @param pTrans The position followed when no joint is given.
 * @param pJointMtx The joint matrix to follow, may be null.
 * @param rOffset The offset from the followed position.
 */
ScreenPointTarget::ScreenPointTarget(LiveActor* pActor, const char* pName, f32 radius,
                                     const sead::Vector3f* pTrans, const sead::Matrix34f* pJointMtx,
                                     const sead::Vector3f& rOffset)
    : mName(pName), mRadius(radius), mTrans(pTrans), mJointMtx(pJointMtx), mOffset(rOffset),
      mActor(pActor) {}

/**
 * @brief Updates the world position of the target.
 */
void ScreenPointTarget::update() {
    if (mJointMtx) {
        mPos.setMul(*mJointMtx, mOffset);
    } else if (mTrans) {
        const sead::Matrix34f* baseMtx = mActor->getBaseMtx();
        if (baseMtx) {
            mPos.setRotated(*baseMtx, mOffset);
            mPos += *mTrans;
        } else {
            mPos.setAdd(*mTrans, mOffset);
        }
    }
}

/**
 * @brief Validates the target.
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
 * @brief Invalidates the target.
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
 * @brief Validates the target on behalf of the system.
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
 * @brief Invalidates the target on behalf of the system.
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
 * @brief Gets the position the pointer hit.
 * @param pPointer The pointer.
 * @return The hit position.
 */
const sead::Vector3f& getHitScreenPointTargetPos(const ScreenPointer* pPointer) {
    return pPointer->mHitPos;
}

/**
 * @brief Gets the normal at the position the pointer hit.
 * @param pPointer The pointer.
 * @return The hit normal.
 */
const sead::Vector3f& getHitScreenPointTargetNormal(const ScreenPointer* pPointer) {
    return pPointer->mHitNormal;
}

/**
 * @brief Gets a target of an actor by name.
 * @param pActor The actor.
 * @param pName The target name.
 * @return The target.
 */
ScreenPointTarget* getScreenPointTarget(LiveActor* pActor, const char* pName) {
    return pActor->mScreenPointKeeper->getTarget(pName);
}

/**
 * @brief Gets a target of an actor by index.
 * @param pActor The actor.
 * @param index The index.
 * @return The target.
 */
ScreenPointTarget* getScreenPointTarget(LiveActor* pActor, s32 index) {
    return pActor->mScreenPointKeeper->getTarget(index);
}

/**
 * @brief Gets the radius of a target of an actor.
 * @param pActor The actor.
 * @param pName The target name.
 * @return The radius.
 */
f32 getScreenPointTargetRadius(LiveActor* pActor, const char* pName) {
    return getScreenPointTarget(pActor, pName)->mRadius;
}

/**
 * @brief Gets the world position of a target of an actor.
 * @param pActor The actor.
 * @param pName The target name.
 * @return The position.
 */
const sead::Vector3f& getScreenPointTargetPos(LiveActor* pActor, const char* pName) {
    return getScreenPointTarget(pActor, pName)->mPos;
}

/**
 * @brief Gets the world position of a target.
 * @param pTarget The target.
 * @return The position.
 */
const sead::Vector3f& getScreenPointTargetPos(const ScreenPointTarget* pTarget) {
    return pTarget->mPos;
}

/**
 * @brief Gets the radius of a target.
 * @param pTarget The target.
 * @return The radius.
 */
f32 getScreenPointTargetRadius(const ScreenPointTarget* pTarget) {
    return pTarget->mRadius;
}

/**
 * @brief Gets the actor that owns a target.
 * @param pTarget The target.
 * @return The owning actor.
 */
LiveActor* getScreenPointTargetHost(ScreenPointTarget* pTarget) {
    return pTarget->mActor;
}

/**
 * @brief Gets the offset of a target of an actor.
 * @param pActor The actor.
 * @param pName The target name.
 * @return The offset.
 */
const sead::Vector3f& getScreenPointTargetOffset(LiveActor* pActor, const char* pName) {
    return getScreenPointTarget(pActor, pName)->mOffset;
}

/**
 * @brief Sets the radius of a target of an actor.
 * @param pActor The actor.
 * @param pName The target name.
 * @param radius The new radius.
 */
void setScreenPointTargetRadius(LiveActor* pActor, const char* pName, f32 radius) {
    getScreenPointTarget(pActor, pName)->mRadius = radius;
}

/**
 * @brief Sets the offset of a target of an actor.
 * @param pActor The actor.
 * @param pName The target name.
 * @param rOffset The new offset.
 */
void setScreenPointTargetOffset(LiveActor* pActor, const char* pName, const sead::Vector3f& rOffset) {
    getScreenPointTarget(pActor, pName)->mOffset = rOffset;
}

/**
 * @brief Validates all targets of an actor, if it has any.
 * @param pActor The actor.
 */
void validateScreenPointTargetAll(LiveActor* pActor) {
    if (pActor->mScreenPointKeeper) {
        pActor->mScreenPointKeeper->validate();
    }
}

/**
 * @brief Invalidates all targets of an actor, if it has any.
 * @param pActor The actor.
 */
void invalidateScreenPointTargetAll(LiveActor* pActor) {
    if (pActor->mScreenPointKeeper) {
        pActor->mScreenPointKeeper->invalidate();
    }
}

/**
 * @brief Checks the name of a target.
 * @param pTarget The target.
 * @param pName The name to compare with.
 * @return True if the names are equal.
 */
bool isScreenPointTargetName(const ScreenPointTarget* pTarget, const char* pName) {
    return isEqualString(pTarget->mName, pName);
}

/**
 * @brief Checks whether a target is valid both by itself and by the system.
 * @param pTarget The target.
 * @return True if the target is valid.
 */
bool isScreenPointTargetValid(const ScreenPointTarget* pTarget) {
    return pTarget->mIsValid && pTarget->mIsValidBySystem;
}

/**
 * @brief Validates a target of an actor.
 * @param pActor The actor.
 * @param pName The target name.
 */
void validateScreenPointTarget(LiveActor* pActor, const char* pName) {
    getScreenPointTarget(pActor, pName)->validate();
}

/**
 * @brief Invalidates a target of an actor.
 * @param pActor The actor.
 * @param pName The target name.
 */
void invalidateScreenPointTarget(LiveActor* pActor, const char* pName) {
    getScreenPointTarget(pActor, pName)->invalidate();
}

}  // namespace al
