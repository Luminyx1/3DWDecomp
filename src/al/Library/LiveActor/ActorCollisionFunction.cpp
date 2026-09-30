#include "Library/LiveActor/ActorCollisionFunction.hpp"

#include "Library/Collision/ActorCollisionController.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/SubActorKeeper.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Project/Collision/Collider.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/CollisionPartsFilterBase.hpp"

namespace al {
namespace {
inline void resetCollisionPartsMtx(LiveActor* pActor, CollisionParts* pCollisionParts) {
    if (pCollisionParts->mSyncCollisionMtx) {
        pCollisionParts->resetAllMtx();
    } else {
        sead::Matrix34f mtx;
        makeMtxSRT(&mtx, pActor);
        pCollisionParts->resetAllMtx(mtx);
    }
}
}  // namespace

/**
 * Gets the collider of an actor.
 * @param pActor The actor.
 * @return The collider.
 */
Collider* getActorCollider(const LiveActor* pActor) {
    return pActor->mCollider;
}

/**
 * Checks whether an actor has a collider.
 * @param pActor The actor.
 * @return Whether the collider exists.
 */
bool isExistActorCollider(const LiveActor* pActor) {
    return pActor->mCollider != nullptr;
}

/**
 * Gets the sensor connected to an actor's collision parts.
 * @param pActor The actor.
 * @return The sensor.
 */
const HitSensor* getActorCollisionPartsSensor(const LiveActor* pActor) {
    return pActor->mCollisionParts->mSensor;
}

/**
 * Checks whether an actor has collision parts.
 * @param pActor The actor.
 * @return Whether the collision parts exist.
 */
bool isExistCollisionParts(const LiveActor* pActor) {
    return pActor->mCollisionParts != nullptr;
}

/**
 * Resets the collision matrix of an actor's collision parts and validates them by user.
 * @param pActor The actor.
 */
void validateCollisionParts(LiveActor* pActor) {
    CollisionParts* collisionParts = pActor->mCollisionParts;
    resetCollisionPartsMtx(pActor, collisionParts);
    collisionParts->validateByUser();
}

/**
 * Invalidates an actor's collision parts by user.
 * @param pActor The actor.
 */
void invalidateCollisionParts(LiveActor* pActor) {
    pActor->mCollisionParts->invalidateByUser();
}

/**
 * Validates an actor's collision parts by system and resets their matrix.
 * @param pActor The actor.
 */
void validateCollisionPartsBySystem(LiveActor* pActor) {
    CollisionParts* collisionParts = pActor->mCollisionParts;
    collisionParts->validateBySystem();
    resetCollisionPartsMtx(pActor, collisionParts);
}

/**
 * Invalidates an actor's collision parts by system.
 * @param pActor The actor.
 */
void invalidateCollisionPartsBySystem(LiveActor* pActor) {
    pActor->mCollisionParts->invalidateBySystem();
}

/**
 * Validates the collision parts of an actor and all of its sub actors by user.
 * @param pActor The actor.
 */
void validateAllCollisionParts(LiveActor* pActor) {
    if (pActor->mCollisionParts) {
        pActor->mCollisionParts->validateByUser();
    }
    SubActorKeeper* keeper = pActor->mSubActorKeeper;
    if (!keeper) {
        return;
    }
    for (s32 i = 0; i < keeper->mCount; i++) {
        LiveActor* subActor = keeper->mInfos[i]->mSubActor;
        if (subActor) {
            validateAllCollisionParts(subActor);
        }
    }
}

/**
 * Invalidates the collision parts of an actor and all of its sub actors by user.
 * @param pActor The actor.
 */
void invalidateAllCollisionParts(LiveActor* pActor) {
    if (pActor->mCollisionParts) {
        pActor->mCollisionParts->invalidateByUser();
    }
    SubActorKeeper* keeper = pActor->mSubActorKeeper;
    if (!keeper) {
        return;
    }
    for (s32 i = 0; i < keeper->mCount; i++) {
        LiveActor* subActor = keeper->mInfos[i]->mSubActor;
        if (subActor) {
            invalidateAllCollisionParts(subActor);
        }
    }
}

/**
 * Invalidates the collision parts of an actor and all of its sub actors by system.
 * @param pActor The actor.
 */
void disableAllCollisionParts(LiveActor* pActor) {
    if (pActor->mCollisionParts) {
        pActor->mCollisionParts->invalidateBySystem();
    }
    SubActorKeeper* keeper = pActor->mSubActorKeeper;
    if (!keeper) {
        return;
    }
    for (s32 i = 0; i < keeper->mCount; i++) {
        LiveActor* subActor = keeper->mInfos[i]->mSubActor;
        if (subActor) {
            disableAllCollisionParts(subActor);
        }
    }
}

/**
 * Validates the collision parts of an actor and all of its sub actors by system.
 * @param pActor The actor.
 */
void enableAllCollisionParts(LiveActor* pActor) {
    if (pActor->mCollisionParts) {
        pActor->mCollisionParts->validateBySystem();
    }
    SubActorKeeper* keeper = pActor->mSubActorKeeper;
    if (!keeper) {
        return;
    }
    for (s32 i = 0; i < keeper->mCount; i++) {
        LiveActor* subActor = keeper->mInfos[i]->mSubActor;
        if (subActor) {
            enableAllCollisionParts(subActor);
        }
    }
}

/**
 * Checks whether an actor's collision parts are valid by both system and user.
 * @param pActor The actor.
 * @return Whether the collision parts are valid.
 */
bool isValidCollisionParts(const LiveActor* pActor) {
    CollisionParts* collisionParts = pActor->mCollisionParts;
    return collisionParts->_160 && collisionParts->_161;
}

/**
 * Sets the special purpose name of an actor's collision parts.
 * @param pActor The actor.
 * @param pName The special purpose name.
 */
void setCollisionPartsSpecialPurposeName(LiveActor* pActor, const char* pName) {
    pActor->mCollisionParts->mSpecialPurpose = pName;
}

/**
 * Resets all matrices of an actor's collision parts.
 * @param pActor The actor.
 */
void resetAllCollisionMtx(LiveActor* pActor) {
    resetCollisionPartsMtx(pActor, pActor->mCollisionParts);
}

/**
 * Syncs an actor's collision parts matrix.
 * @param pActor The actor.
 * @param pMtx The matrix to sync to, or nullptr to use the actor's SRT matrix.
 */
void syncCollisionMtx(LiveActor* pActor, const sead::Matrix34f* pMtx) {
    syncCollisionMtx(pActor, pActor->mCollisionParts, pMtx);
}

/**
 * Syncs the matrix of the given collision parts.
 * @param pActor The actor.
 * @param pCollisionParts The collision parts.
 * @param pMtx The matrix to sync to, or nullptr to use the actor's SRT matrix.
 */
void syncCollisionMtx(LiveActor* pActor, CollisionParts* pCollisionParts,
                      const sead::Matrix34f* pMtx) {
    if (!pCollisionParts->_160 || !pCollisionParts->_161) {
        return;
    }
    if (pCollisionParts->mSyncCollisionMtx) {
        pCollisionParts->syncMtx();
    } else if (pMtx) {
        pCollisionParts->syncMtx(*pMtx);
    } else {
        sead::Matrix34f mtx;
        makeMtxSRT(&mtx, pActor);
        pCollisionParts->syncMtx(mtx);
    }
}

/**
 * Sets the matrix pointer an actor's collision parts sync to.
 * @param pActor The actor.
 * @param pMtx The matrix pointer.
 */
void setSyncCollisionMtxPtr(LiveActor* pActor, const sead::Matrix34f* pMtx) {
    pActor->mCollisionParts->mSyncCollisionMtx = pMtx;
}

/**
 * Checks whether an actor is on the ground.
 * @param pActor The actor.
 * @param checkFrame The number of frames after leaving the ground that still count as grounded.
 * @param margin The maximum velocity along the ground normal.
 * @return Whether the actor is on the ground.
 */
bool isOnGround(const LiveActor* pActor, u32 checkFrame, f32 margin) {
    Collider* collider = pActor->mCollider;
    if (collider) {
        if (!(collider->_110 >= 0.0f) && collider->_264 > checkFrame) {
            return false;
        }
        return !(getVelocity(pActor).dot(collider->getRecentOnGroundNormal(checkFrame)) > margin);
    }
    if (getTrans(pActor).y <= 0.0f && getVelocity(pActor).y < 0.0f) {
        return true;
    }
    return false;
}

/**
 * Checks whether an actor is on the ground and touches the ground at a face.
 * @param pActor The actor.
 * @return Whether the actor is on a ground face.
 */
bool isOnGroundFace(const LiveActor* pActor) {
    return isOnGround(pActor, 0, 0.0f) && !isCollidedGroundEdgeOrCorner(pActor);
}

/**
 * Checks whether an actor touches the ground at an edge or corner.
 * @param pActor The actor.
 * @return Whether the ground is touched at an edge or corner.
 */
bool isCollidedGroundEdgeOrCorner(const LiveActor* pActor) {
    Collider* collider = pActor->mCollider;
    return collider->_110 >= 0.0f && !collider->mFloor.isCollisionAtFace();
}

/**
 * Checks whether an actor is on the ground, ignoring its velocity.
 * @param pActor The actor.
 * @param checkFrame The number of frames after leaving the ground that still count as grounded.
 * @return Whether the actor is on the ground.
 */
bool isOnGroundNoVelocity(const LiveActor* pActor, u32 checkFrame) {
    Collider* collider = pActor->mCollider;
    if (!collider) {
        return getTrans(pActor).y <= 0.0f;
    }
    if (collider->_110 >= 0.0f) {
        return true;
    }
    return collider->_264 <= checkFrame;
}

/**
 * Gets a recent ground normal of an actor.
 * @param pActor The actor.
 * @param offset How many frames back to look.
 * @return The ground normal.
 */
const sead::Vector3f& getOnGroundNormal(const LiveActor* pActor, u32 offset) {
    return pActor->mCollider->getRecentOnGroundNormal(offset);
}

/**
 * Sets the radius of an actor's collider.
 * @param pActor The actor.
 * @param radius The radius.
 */
void setColliderRadius(LiveActor* pActor, f32 radius) {
    pActor->mCollider->mRadius = radius;
}

/**
 * Sets the Y offset of an actor's collider.
 * @param pActor The actor.
 * @param offsetY The Y offset.
 */
void setColliderOffsetY(LiveActor* pActor, f32 offsetY) {
    pActor->mCollider->mOffsetY = offsetY;
}

/**
 * Gets the radius of an actor's collider.
 * @param pActor The actor.
 * @return The radius.
 */
f32 getColliderRadius(const LiveActor* pActor) {
    return pActor->mCollider->mRadius;
}

/**
 * Gets the Y offset of an actor's collider.
 * @param pActor The actor.
 * @return The Y offset.
 */
f32 getColliderOffsetY(const LiveActor* pActor) {
    return pActor->mCollider->mOffsetY;
}

/**
 * Calculates the check position of an actor's collider.
 * @param pActor The actor.
 * @param pPos The output position.
 */
void calcColliderPos(const LiveActor* pActor, sead::Vector3f* pPos) {
    pActor->mCollider->calcCheckPos(pPos);
}

/**
 * Sets whether an actor's collider reacts to the move power of collision parts.
 * @param pActor The actor.
 * @param isEnabled Whether to react.
 */
void setColliderReactMovePower(LiveActor* pActor, bool isEnabled) {
    pActor->mCollider->mIsReactMovePower = isEnabled;
}

/**
 * Calculates the rotation power of the floor under an actor's collider.
 * @param pActor The actor.
 * @param pQuat The output rotation power.
 */
void calcColliderFloorRotatePower(LiveActor* pActor, sead::Quatf* pQuat) {
    pActor->mCollider->mFloor.mTriangle.calcForceRotatePower(pQuat);
}

/**
 * Calculates the average of the floor, wall and ceiling normals an actor collides with.
 * @param pActor The actor.
 * @param pOutNormal The output normal.
 */
void calcCollidedNormalSum(const LiveActor* pActor, sead::Vector3f* pOutNormal) {
    pOutNormal->set(0.0f, 0.0f, 0.0f);
    Collider* collider = pActor->mCollider;
    if (collider->_110 >= 0.0f) {
        *pOutNormal += *collider->mFloor.mTriangle.getFaceNormal();
    }
    if (collider->_1b8 >= 0.0f) {
        *pOutNormal += *collider->mWall.mTriangle.getFaceNormal();
    }
    if (collider->_260 >= 0.0f) {
        *pOutNormal += *collider->mCeiling.mTriangle.getFaceNormal();
    }
    *pOutNormal *= 1.0f / 3.0f;
}

/**
 * Sets the triangle filter of an actor's collider.
 * @param pActor The actor.
 * @param pTriangleFilter The triangle filter.
 */
void setColliderFilterTriangle(LiveActor* pActor, const TriangleFilterBase* pTriangleFilter) {
    pActor->mCollider->setTriangleFilter(pTriangleFilter);
}

/**
 * Sets the collision parts filter of an actor's collider.
 * @param pActor The actor.
 * @param pCollisionPartsFilter The collision parts filter.
 */
void setColliderFilterCollisionParts(LiveActor* pActor,
                                     const CollisionPartsFilterBase* pCollisionPartsFilter) {
    pActor->mCollider->setCollisionPartsFilter(pCollisionPartsFilter);
}

/**
 * Makes an actor's collider ignore the actor's own collision parts.
 * @param pActor The actor.
 */
void createAndSetColliderFilterExistActor(LiveActor* pActor) {
    createAndSetColliderFilterExistActor(pActor, pActor);
}

/**
 * Makes an actor's collider ignore another actor's collision parts.
 * @param pActor The actor.
 * @param pFilterActor The actor whose collision parts are ignored.
 */
void createAndSetColliderFilterExistActor(LiveActor* pActor, LiveActor* pFilterActor) {
    setColliderFilterCollisionParts(pActor, new CollisionPartsFilterActor(pFilterActor));
}

/**
 * Makes an actor's collider ignore collision parts with a special purpose.
 * @param pActor The actor.
 * @param pName The special purpose name.
 */
void createAndSetColliderSpecialPurpose(LiveActor* pActor, const char* pName) {
    setColliderFilterCollisionParts(pActor, new CollisionPartsFilterSpecialPurpose(pName));
}

/**
 * Makes an actor's collider ignore special purpose collision parts and its own collision parts.
 * @param pActor The actor.
 * @param pName The special purpose name.
 */
void createAndSetColliderSpecialPurposeForCollisionActor(LiveActor* pActor, const char* pName) {
    setColliderFilterCollisionParts(
        pActor, new CollisionPartsFilterMergePair(new CollisionPartsFilterSpecialPurpose(pName),
                                                  new CollisionPartsFilterActor(pActor)));
}

/**
 * Makes an actor's collider ignore collision parts with either of two special purposes.
 * @param pActor The actor.
 * @param pName The first special purpose name.
 * @param pMultipleName The second special purpose name.
 */
void createAndSetColliderSpecialPurposeForCollisionMultiple(LiveActor* pActor, const char* pName,
                                                            const char* pMultipleName) {
    setColliderFilterCollisionParts(
        pActor,
        new CollisionPartsFilterMergePair(new CollisionPartsFilterSpecialPurpose(pName),
                                          new CollisionPartsFilterSpecialPurpose(pMultipleName)));
}

/**
 * Creates an actor collision controller.
 * @param pActor The actor.
 * @return The controller.
 */
ActorCollisionController* createActorCollisionController(LiveActor* pActor) {
    return new ActorCollisionController(pActor);
}

/**
 * Sets the collider radius of an actor collision controller.
 * @param pCollisionController The controller.
 * @param radius The radius.
 */
void setColliderRadius(ActorCollisionController* pCollisionController, f32 radius) {
    pCollisionController->setColliderRadius(radius);
}

/**
 * Sets the collider Y offset of an actor collision controller.
 * @param pCollisionController The controller.
 * @param offsetY The Y offset.
 */
void setColliderOffsetY(ActorCollisionController* pCollisionController, f32 offsetY) {
    pCollisionController->setColliderOffsetY(offsetY);
}

/**
 * Resets an actor collision controller to its original collider values.
 * @param pCollisionController The controller.
 * @param delay The delay in frames.
 */
void resetActorCollisionController(ActorCollisionController* pCollisionController, s32 delay) {
    pCollisionController->resetToOrigin(delay);
}

/**
 * Updates an actor collision controller.
 * @param pCollisionController The controller.
 */
void updateActorCollisionController(ActorCollisionController* pCollisionController) {
    pCollisionController->update();
}
}  // namespace al
