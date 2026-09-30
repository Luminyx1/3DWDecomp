#include "Library/LiveActor/ActorCollisionFunction.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Project/Collision/Collider.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace al {
namespace {
inline bool isOnGroundInline(const LiveActor* pActor, u32 checkFrame, f32 margin) {
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
}  // namespace

/**
 * Gets the material code name of the floor an actor collides with.
 * @param pActor The actor.
 * @return The code name.
 */
const char* getCollidedFloorMaterialCodeName(const LiveActor* pActor) {
    return getCollisionCodeName(pActor->mCollider->mFloor.mTriangle, "MaterialCode");
}

/**
 * Gets the floor code name of the floor an actor collides with.
 * @param pActor The actor.
 * @return The code name.
 */
const char* getCollidedFloorCodeName(const LiveActor* pActor) {
    return getCollisionCodeName(pActor->mCollider->mFloor.mTriangle, "FloorCode");
}

/**
 * Gets the wall code name of the floor an actor collides with.
 * @param pActor The actor.
 * @return The code name.
 */
const char* getCollidedFloorWallCodeName(const LiveActor* pActor) {
    return getCollisionCodeName(pActor->mCollider->mFloor.mTriangle, "WallCode");
}

/**
 * Gets the material code name of the wall an actor collides with.
 * @param pActor The actor.
 * @return The code name.
 */
const char* getCollidedWallMaterialCodeName(const LiveActor* pActor) {
    return getCollisionCodeName(pActor->mCollider->mWall.mTriangle, "MaterialCode");
}

/**
 * Gets the wall code name of the wall an actor collides with.
 * @param pActor The actor.
 * @return The code name.
 */
const char* getCollidedWallCodeName(const LiveActor* pActor) {
    return getCollisionCodeName(pActor->mCollider->mWall.mTriangle, "WallCode");
}

/**
 * Gets the material code name of the ceiling an actor collides with.
 * @param pActor The actor.
 * @return The code name.
 */
const char* getCollidedCeilingMaterialCodeName(const LiveActor* pActor) {
    return getCollisionCodeName(pActor->mCollider->mCeiling.mTriangle, "MaterialCode");
}

/**
 * Checks whether an actor collides with a floor, wall or ceiling.
 * @param pActor The actor.
 * @return Whether the actor collides with anything.
 */
bool isCollided(const LiveActor* pActor) {
    return isCollidedGround(pActor) || isCollidedWall(pActor) || isCollidedCeiling(pActor);
}

/**
 * Checks whether an actor collides with the ground.
 * @param pActor The actor.
 * @return Whether the actor collides with the ground.
 */
bool isCollidedGround(const LiveActor* pActor) {
    return pActor->mCollider->_110 >= 0.0f;
}

/**
 * Checks whether an actor collides with a wall.
 * @param pActor The actor.
 * @return Whether the actor collides with a wall.
 */
bool isCollidedWall(const LiveActor* pActor) {
    return pActor->mCollider->_1b8 >= 0.0f;
}

/**
 * Checks whether an actor collides with a ceiling.
 * @param pActor The actor.
 * @return Whether the actor collides with a ceiling.
 */
bool isCollidedCeiling(const LiveActor* pActor) {
    return pActor->mCollider->_260 >= 0.0f;
}

/**
 * Checks whether an actor collides with a wall at a face.
 * @param pActor The actor.
 * @return Whether the actor collides with a wall face.
 */
bool isCollidedWallFace(const LiveActor* pActor) {
    Collider* collider = pActor->mCollider;
    if (!(collider->_1b8 >= 0.0f)) {
        return false;
    }

    return collider->mIsCollidedWallFace;
}

/**
 * Checks whether an actor collides with something while moving into it.
 * @param pActor The actor.
 * @return Whether the actor collides against its velocity.
 */
bool isCollidedVelocity(const LiveActor* pActor) {
    return isOnGroundInline(pActor, 0, 0.0f) || isCollidedWallVelocity(pActor) ||
           isCollidedCeilingVelocity(pActor);
}

/**
 * Checks whether an actor collides with a wall while moving into it.
 * @param pActor The actor.
 * @return Whether the actor moves into a wall.
 */
bool isCollidedWallVelocity(const LiveActor* pActor) {
    if (!isCollidedWall(pActor)) {
        return false;
    }

    return getVelocity(pActor).dot(getCollidedWallNormal(pActor)) < 0.0f;
}

/**
 * Checks whether an actor collides with a ceiling while moving into it.
 * @param pActor The actor.
 * @return Whether the actor moves into a ceiling.
 */
bool isCollidedCeilingVelocity(const LiveActor* pActor) {
    if (!isCollidedCeiling(pActor)) {
        return false;
    }

    return getVelocity(pActor).dot(getCollidedCeilingNormal(pActor)) < 0.0f;
}

/**
 * Gets the normal of the wall an actor collides with.
 * @param pActor The actor.
 * @return The wall normal.
 */
const sead::Vector3f& getCollidedWallNormal(const LiveActor* pActor) {
    return *pActor->mCollider->mWall.mTriangle.getFaceNormal();
}

/**
 * Gets the normal of the ceiling an actor collides with.
 * @param pActor The actor.
 * @return The ceiling normal.
 */
const sead::Vector3f& getCollidedCeilingNormal(const LiveActor* pActor) {
    return *pActor->mCollider->mCeiling.mTriangle.getFaceNormal();
}

/**
 * Gets the normal of the ground an actor collides with.
 * @param pActor The actor.
 * @return The ground normal.
 */
const sead::Vector3f& getCollidedGroundNormal(const LiveActor* pActor) {
    return *pActor->mCollider->mFloor.mTriangle.getFaceNormal();
}

/**
 * Gets the position where an actor collides with the ground.
 * @param pActor The actor.
 * @return The ground hit position.
 */
const sead::Vector3f& getCollidedGroundPos(const LiveActor* pActor) {
    return pActor->mCollider->mFloor.mPos;
}

/**
 * Gets the position where an actor collides with a wall.
 * @param pActor The actor.
 * @return The wall hit position.
 */
const sead::Vector3f& getCollidedWallPos(const LiveActor* pActor) {
    return pActor->mCollider->mWall.mPos;
}

/**
 * Gets the position where an actor collides with a ceiling.
 * @param pActor The actor.
 * @return The ceiling hit position.
 */
const sead::Vector3f& getCollidedCeilingPos(const LiveActor* pActor) {
    return pActor->mCollider->mCeiling.mPos;
}

/**
 * Gets the collision parts of the ground an actor collides with, if any.
 * @param pActor The actor.
 * @return The collision parts or nullptr.
 */
const CollisionParts* tryGetCollidedGroundCollisionParts(const LiveActor* pActor) {
    if (!isCollidedGround(pActor)) {
        return nullptr;
    }

    return pActor->mCollider->mFloor.mTriangle.mCollisionParts;
}

/**
 * Gets the collision parts of the wall an actor collides with, if any.
 * @param pActor The actor.
 * @return The collision parts or nullptr.
 */
const CollisionParts* tryGetCollidedWallCollisionParts(const LiveActor* pActor) {
    if (!isCollidedWall(pActor)) {
        return nullptr;
    }

    return pActor->mCollider->mWall.mTriangle.mCollisionParts;
}

/**
 * Gets the collision parts of the ceiling an actor collides with, if any.
 * @param pActor The actor.
 * @return The collision parts or nullptr.
 */
const CollisionParts* tryGetCollidedCeilingCollisionParts(const LiveActor* pActor) {
    if (!isCollidedCeiling(pActor)) {
        return nullptr;
    }

    return pActor->mCollider->mCeiling.mTriangle.mCollisionParts;
}

/**
 * Gets the sensor of the ground an actor collides with, if any.
 * @param pActor The actor.
 * @return The sensor or nullptr.
 */
HitSensor* tryGetCollidedGroundSensor(const LiveActor* pActor) {
    if (!isCollidedGround(pActor)) {
        return nullptr;
    }

    return pActor->mCollider->mFloor.mTriangle.mCollisionParts->mSensor;
}

/**
 * Gets the sensor of the wall an actor collides with, if any.
 * @param pActor The actor.
 * @return The sensor or nullptr.
 */
HitSensor* tryGetCollidedWallSensor(const LiveActor* pActor) {
    if (!isCollidedWall(pActor)) {
        return nullptr;
    }

    return pActor->mCollider->mWall.mTriangle.mCollisionParts->mSensor;
}

/**
 * Gets the sensor of the ceiling an actor collides with, if any.
 * @param pActor The actor.
 * @return The sensor or nullptr.
 */
HitSensor* tryGetCollidedCeilingSensor(const LiveActor* pActor) {
    if (!isCollidedCeiling(pActor)) {
        return nullptr;
    }

    return pActor->mCollider->mCeiling.mTriangle.mCollisionParts->mSensor;
}

/**
 * Forces an actor's collision parts to use a scale of one.
 * @param pActor The actor.
 */
void setForceCollisionScaleOne(const LiveActor* pActor) {
    pActor->mCollisionParts->_165 = 2;
}
}  // namespace al
