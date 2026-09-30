#include "Library/LiveActor/Util/ActorMovementUtil.hpp"

#include "Library/Actor/ActorPoseKeeper.hpp"
#include "Library/HitSensor/SensorFunction.hpp"
#include "Library/LiveActor/ActorCollisionFunction.hpp"
#include "Library/LiveActor/ActorParamHolder.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Project/Collision/Collider.hpp"
#include "Project/Collision/CollisionPartsKeeperUtil.hpp"
#include "Project/Collision/CollisionUtil.hpp"
#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Library/System/SystemKit.hpp"

namespace al {
inline void addVelocityInline(LiveActor* pActor, const sead::Vector3f& rVel, f32 force) {
    sead::Vector3f* velocity = getVelocityPtr(pActor);
    velocity->setScaleAdd(force, rVel, *velocity);
}

inline bool calcVelocityClockwiseToDirection(LiveActor* pActor, sead::Vector3f* pDirVelocity, const sead::Vector3f& rDir) {
    sead::Vector3f normDir;

    if (!pDirVelocity || normalizeOrZero(&normDir, rDir))
        return false;

    pDirVelocity->setCross(getGravity(pActor), normDir);
    return true;
}

inline void scaleVelocityParallelVertical(LiveActor* pActor, const sead::Vector3f& rDirection, f32 parallel, f32 vertical) {
    const sead::Vector3f& velocity = getVelocity(pActor);

    f32 speedV = rDirection.dot(velocity);
    sead::Vector3f parallelVec = rDirection * (speedV * parallel);
    sead::Vector3f verticalVec = velocity;
    
    verticalVec.x -= rDirection.x * speedV;
    verticalVec.y -= rDirection.y * speedV;
    verticalVec.z -= rDirection.z * speedV;

    sead::Vector3f* newVelocity = getVelocityPtr(pActor);
    *newVelocity = parallelVec;
    newVelocity->setScaleAdd(vertical, verticalVec, parallelVec);
}

inline bool turnToDirectionAxis(LiveActor* pActor, const sead::Vector3f& rHorizontal, const sead::Vector3f& rVertical, f32 deg) {
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    calcFrontDir(&front, pActor);
    bool result = turnVecToVecCosOnPlane(&front, rHorizontal, rVertical,
                                         sead::Mathf::cos(sead::Mathf::deg2rad(deg)));

    sead::Quatf quat = sead::Quatf::unit;
    makeQuatUpFront(&quat, rVertical, front);
    updatePoseQuat(pActor, quat);
    return result;
}

inline bool walkAndTurnToDirectionFittedGroundGravity(LiveActor* pActor, sead::Vector3f* pFront, const sead::Vector3f& rDir, f32 forceFront, f32 forceGravity, f32 decay, f32 deg, bool turnAlongGround) {
    turnDirection(pActor, pFront, rDir, sead::Mathf::cos(sead::Mathf::deg2rad(deg)));

    if (turnAlongGround)
        turnDirectionAlongGround(pActor, pFront);

    sead::Vector3f velFront;
    normalizeOrZero(&velFront, *pFront);
    addVelocityInline(pActor, velFront, forceFront);

    bool isOnGround = isOnGroundNoVelocity(pActor, 3);

    if (isOnGround)
        addVelocityToGravityFittedGround(pActor, forceGravity, 3);
    else
        addVelocityToGravity(pActor, forceGravity);

    scaleVelocity(pActor, decay);
    return isOnGround;
}

inline bool walkAndTurnToDirectionFittedGroundGravity(LiveActor* pActor, const sead::Vector3f& rDir,
                                               f32 forceFront, f32 forceGravity, f32 decay, f32 deg,
                                               bool turnAlongGround) {
    return walkAndTurnToDirectionFittedGroundGravity(pActor, getFrontPtr(pActor), rDir, forceFront,
                                                     forceGravity, decay, deg, turnAlongGround);
}

inline bool walkAndTurnToTargetFittedGroundGravity(LiveActor* pActor, const sead::Vector3f& rTarget, f32 forceFront, f32 forceGravity, f32 decay, f32 deg, bool turnAlongGround) {
    return walkAndTurnToDirectionFittedGroundGravity(pActor, rTarget - getTrans(pActor), forceFront,
                                                     forceGravity, decay, deg, turnAlongGround);
}

/**
 * Tries to set pos on ground.
 * @param pActor The actor.
 * @return Whether the check succeeded.
 */
bool trySetPosOnGround(LiveActor* pActor) {
    sead::Vector3f pos = getTrans(pActor);
    sead::Vector3f dir = {0.0f, -500.0f, 0.0f};
    pos.y += 200.0f;
    return alCollisionUtil::getFirstPolyOnArrow(pActor, getTransPtr(pActor), nullptr, pos, dir,
                                                nullptr, nullptr);
}

/**
 * Gets velocity.
 * @param pActor The actor.
 * @return The result.
 */
const sead::Vector3f& getVelocity(const LiveActor* pActor) {
    return pActor->mActorPoseKeeper->getVelocity();
}

/**
 * Gets velocity ptr.
 * @param pActor The actor.
 * @return The result.
 */
sead::Vector3f* getVelocityPtr(LiveActor* pActor) {
    return pActor->mActorPoseKeeper->getVelocityPtr();
}

/**
 * Sets velocity.
 * @param pActor The actor.
 * @param rVel The vel.
 */
void setVelocity(LiveActor* pActor, const sead::Vector3f& rVel) {
    getVelocityPtr(pActor)->set(rVel);
}

/**
 * Sets velocity.
 * @param pActor The actor.
 * @param x The x.
 * @param y The y.
 * @param z The z.
 */
void setVelocity(LiveActor* pActor, f32 x, f32 y, f32 z) {
    getVelocityPtr(pActor)->set(x, y, z);
}

/**
 * Sets velocity X.
 * @param pActor The actor.
 * @param x The x.
 */
void setVelocityX(LiveActor* pActor, f32 x) {
    getVelocityPtr(pActor)->x = x;
}

/**
 * Sets velocity Y.
 * @param pActor The actor.
 * @param y The y.
 */
void setVelocityY(LiveActor* pActor, f32 y) {
    getVelocityPtr(pActor)->y = y;
}

/**
 * Sets velocity Z.
 * @param pActor The actor.
 * @param z The z.
 */
void setVelocityZ(LiveActor* pActor, f32 z) {
    getVelocityPtr(pActor)->z = z;
}

/**
 * Sets velocity zero.
 * @param pActor The actor.
 */
void setVelocityZero(LiveActor* pActor) {
    setVelocity(pActor, 0.0f, 0.0f, 0.0f);
}

/**
 * Sets velocity zero X.
 * @param pActor The actor.
 */
void setVelocityZeroX(LiveActor* pActor) {
    setVelocityX(pActor, 0.0f);
}

/**
 * Sets velocity zero Y.
 * @param pActor The actor.
 */
void setVelocityZeroY(LiveActor* pActor) {
    setVelocityY(pActor, 0.0f);
}

/**
 * Sets velocity zero Z.
 * @param pActor The actor.
 */
void setVelocityZeroZ(LiveActor* pActor) {
    setVelocityZ(pActor, 0.0f);
}

/**
 * Sets velocity zero H.
 * @param pActor The actor.
 */
void setVelocityZeroH(LiveActor* pActor) {
    setVelocityZeroH(pActor, getGravity(pActor));
}

/**
 * Sets velocity zero H.
 * @param pActor The actor.
 * @param rGravity The gravity.
 */
void setVelocityZeroH(LiveActor* pActor, const sead::Vector3f& rGravity) {
    sead::Vector3f* velocity = getVelocityPtr(pActor);
    parallelizeVec(velocity, rGravity, *velocity);
}

/**
 * Sets velocity zero V.
 * @param pActor The actor.
 */
void setVelocityZeroV(LiveActor* pActor) {
    setVelocityZeroV(pActor, getGravity(pActor));
}

/**
 * Sets velocity zero V.
 * @param pActor The actor.
 * @param rGravity The gravity.
 */
void setVelocityZeroV(LiveActor* pActor, const sead::Vector3f& rGravity) {
    sead::Vector3f* velocity = getVelocityPtr(pActor);
    verticalizeVec(velocity, rGravity, *velocity);
}

/**
 * Sets velocity jump.
 * @param pActor The actor.
 * @param speed The speed.
 */
void setVelocityJump(LiveActor* pActor, f32 speed) {
    getVelocityPtr(pActor)->setScale(getGravity(pActor), -speed);
}

/**
 * Sets velocity to direction.
 * @param pActor The actor.
 * @param rDir The direction.
 * @param speed The speed.
 */
void setVelocityToDirection(LiveActor* pActor, const sead::Vector3f& rDir, f32 speed) {
    sead::Vector3f normDir;
    normalizeOrZero(&normDir, rDir);
    getVelocityPtr(pActor)->setScale(normDir, speed);
}

/**
 * Sets velocity to gravity.
 * @param pActor The actor.
 * @param speed The speed.
 */
void setVelocityToGravity(LiveActor* pActor, f32 speed) {
    getVelocityPtr(pActor)->setScale(getGravity(pActor), speed);
}

/**
 * Adds velocity.
 * @param pActor The actor.
 * @param rVel The vel.
 */
void addVelocity(LiveActor* pActor, const sead::Vector3f& rVel) {
    getVelocityPtr(pActor)->add(rVel);
}

/**
 * Adds velocity.
 * @param pActor The actor.
 * @param x The x.
 * @param y The y.
 * @param z The z.
 */
void addVelocity(LiveActor* pActor, f32 x, f32 y, f32 z) {
    addVelocity(pActor, {x, y, z});
}

/**
 * Adds velocity X.
 * @param pActor The actor.
 * @param x The x.
 */
void addVelocityX(LiveActor* pActor, f32 x) {
    getVelocityPtr(pActor)->x += x;
}

/**
 * Adds velocity Y.
 * @param pActor The actor.
 * @param y The y.
 */
void addVelocityY(LiveActor* pActor, f32 y) {
    getVelocityPtr(pActor)->y += y;
}

/**
 * Adds velocity Z.
 * @param pActor The actor.
 * @param z The z.
 */
void addVelocityZ(LiveActor* pActor, f32 z) {
    getVelocityPtr(pActor)->z += z;
}

/**
 * Adds velocity to direction.
 * @param pActor The actor.
 * @param rDir The direction.
 * @param force The force.
 */
void addVelocityToDirection(LiveActor* pActor, const sead::Vector3f& rDir, f32 force) {
    sead::Vector3f normDir;
    normalizeOrZero(&normDir, rDir);
    addVelocityInline(pActor, normDir, force);
}

/**
 * Adds velocity to gravity.
 * @param pActor The actor.
 * @param force The force.
 */
void addVelocityToGravity(LiveActor* pActor, f32 force) {
    sead::Vector3f* velocity = getVelocityPtr(pActor);
    velocity->setScaleAdd(force, getGravity(pActor), *velocity);
}

/**
 * Adds velocity to gravity fitted ground.
 * @param pActor The actor.
 * @param force The force.
 * @param maxAirTime The max air time.
 */
void addVelocityToGravityFittedGround(LiveActor* pActor, f32 force, u32 maxAirTime) {
    sead::Vector3f* velocity = getVelocityPtr(pActor);
    const sead::Vector3f& normal = getOnGroundNormal(pActor, maxAirTime);
    
    velocity->x -= normal.x * force;
    velocity->y -= normal.y * force;
    velocity->z -= normal.z * force;
}

/**
 * Adds velocity to target.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param force The force.
 */
void addVelocityToTarget(LiveActor* pActor, const sead::Vector3f& rTarget, f32 force) {
    sead::Vector3f diff = rTarget;
    diff -= getTrans(pActor);
    normalizeOrZero(&diff);
    addVelocityInline(pActor, diff, force);
}

/**
 * Adds velocity to target.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param minForce The min force.
 * @param maxForce The max force.
 * @param minDistance The min distance.
 * @param maxDistance The max distance.
 */
void addVelocityToTarget(LiveActor* pActor, const sead::Vector3f& rTarget, f32 minForce, f32 maxForce, f32 minDistance, f32 maxDistance) {
    sead::Vector3f diff = rTarget;
    diff -= getTrans(pActor);
    f32 distance;
    separateScalarAndDirection(&distance, &diff, diff);
    f32 normDistance = normalize(distance, minDistance, maxDistance);
    sead::Vector3f* velocity = getVelocityPtr(pActor);
    f32 force = lerpValue(minForce, maxForce, normDistance);
    velocity->setScaleAdd(force, diff, *velocity);
}

/**
 * Adds velocity jump.
 * @param pActor The actor.
 * @param force The force.
 */
void addVelocityJump(LiveActor* pActor, f32 force) {
    addVelocity(pActor, getGravity(pActor) * -force);
}

/**
 * Tries to add velocity limit.
 * @param pActor The actor.
 * @param rVelocity The velocity.
 * @param limit The limit.
 */
void tryAddVelocityLimit(LiveActor* pActor, const sead::Vector3f& rVelocity, f32 limit) {
    sead::Vector3f newVelocity = getVelocity(pActor);
    addVectorLimit(&newVelocity, rVelocity, limit);
    setVelocity(pActor, newVelocity);
}

/**
 * Scales velocity.
 * @param pActor The actor.
 * @param factor The factor.
 */
void scaleVelocity(LiveActor* pActor, f32 factor) {
    *getVelocityPtr(pActor) *= factor;
}

/**
 * Scales velocity X.
 * @param pActor The actor.
 * @param factorX The factor x.
 */
void scaleVelocityX(LiveActor* pActor, f32 factorX) {
    getVelocityPtr(pActor)->x *= factorX;
}

/**
 * Scales velocity Y.
 * @param pActor The actor.
 * @param factorY The factor y.
 */
void scaleVelocityY(LiveActor* pActor, f32 factorY) {
    getVelocityPtr(pActor)->y *= factorY;
}

/**
 * Scales velocity Z.
 * @param pActor The actor.
 * @param factorZ The factor z.
 */
void scaleVelocityZ(LiveActor* pActor, f32 factorZ) {
    getVelocityPtr(pActor)->z *= factorZ;
}

/**
 * Scales velocity HV.
 * @param pActor The actor.
 * @param factorH The factor h.
 * @param factorV The factor v.
 */
void scaleVelocityHV(LiveActor* pActor, f32 factorH, f32 factorV) {
    sead::Vector3f* velocity = getVelocityPtr(pActor);
    velocity->x *= factorH;
    velocity->y *= factorV;
    velocity->z *= factorH;
}

/**
 * Scales velocity except direction.
 * @param pActor The actor.
 * @param rDirection The direction.
 * @param factor The factor.
 */
void scaleVelocityExceptDirection(LiveActor* pActor, const sead::Vector3f& rDirection, f32 factor) {
    sead::Vector3f* velocity = getVelocityPtr(pActor);
    scaleVectorExceptDirection(velocity, rDirection, *velocity, factor);
}

/**
 * Limits velocity.
 * @param pActor The actor.
 * @param limit The limit.
 */
void limitVelocity(LiveActor* pActor, f32 limit) {
    if (calcSpeed(pActor) > limit) {
        normalizeOrZero(getVelocityPtr(pActor));
        scaleVelocity(pActor, limit);
    }
}

/**
 * Calculates speed.
 * @param pActor The actor.
 * @return The result.
 */
f32 calcSpeed(const LiveActor* pActor) {
    return getVelocity(pActor).length();
}

/**
 * Limits velocity X.
 * @param pActor The actor.
 * @param limitX The limit x.
 */
void limitVelocityX(LiveActor* pActor, f32 limitX) {
    if (getVelocity(pActor).x > limitX)
        getVelocityPtr(pActor)->x = limitX;
    else if (getVelocity(pActor).x < -limitX)
        getVelocityPtr(pActor)->x = -limitX;
}

/**
 * Limits velocity Y.
 * @param pActor The actor.
 * @param limitY The limit y.
 */
void limitVelocityY(LiveActor* pActor, f32 limitY) {
    if (getVelocity(pActor).y > limitY)
        getVelocityPtr(pActor)->y = limitY;
    else if (getVelocity(pActor).y < -limitY)
        getVelocityPtr(pActor)->y = -limitY;
}

/**
 * Limits velocity Z.
 * @param pActor The actor.
 * @param limitZ The limit z.
 */
void limitVelocityZ(LiveActor* pActor, f32 limitZ) {
    if (getVelocity(pActor).z > limitZ)
        getVelocityPtr(pActor)->z = limitZ;
    else if (getVelocity(pActor).z < -limitZ)
        getVelocityPtr(pActor)->z = -limitZ;
}

/**
 * Rebounds velocity from each collision.
 * @param pActor The actor.
 * @param ground The ground.
 * @param wall The wall.
 * @param ceiling The ceiling.
 * @param threshold The threshold.
 * @return Whether the check succeeded.
 */
bool reboundVelocityFromEachCollision(LiveActor* pActor, f32 ground, f32 wall, f32 ceiling, f32 threshold) {
    if (!isCollided(pActor))
        return false;

    sead::Vector3f normalSum;
    calcCollidedNormalSum(pActor, &normalSum);

    if (isNearZero(normalSum, 0.001f))
        return false;

    normalize(&normalSum);
    const sead::Vector3f& gravity = getGravity(pActor);
    f32 rebound;

    if (isFloorPolygon(normalSum, gravity))
        rebound = ground;
    else if (isWallPolygon(normalSum, gravity))
        rebound = wall;
    else if (isCeilingPolygon(normalSum, gravity))
        rebound = ceiling;
    else
        rebound = 0.0f;

    f32 dot = normalSum.dot(getVelocity(pActor));

    if (dot < -threshold) {
        sead::Vector3f* velocity = getVelocityPtr(pActor);
        f32 mul = (rebound + 1.0f) * dot;
        velocity->setScaleAdd(-mul, normalSum, *velocity);
        return true;
    } else if (dot < 0.0f) {
        sead::Vector3f* velocity = getVelocityPtr(pActor);
        velocity->setScaleAdd(-dot, normalSum, *velocity);
    }

    return false;
}

/**
 * Rebounds velocity from collision.
 * @param pActor The actor.
 * @param reboundStrength The rebound strength.
 * @param reboundMin The rebound min.
 * @param friction The friction.
 * @return Whether the check succeeded.
 */
bool reboundVelocityFromCollision(LiveActor* pActor, f32 reboundStrength, f32 reboundMin, f32 friction) {
    if (!isCollided(pActor))
        return false;

    sead::Vector3f normalSum;
    calcCollidedNormalSum(pActor, &normalSum);

    if (isNearZero(normalSum, 0.001f))
        return false;

    normalize(&normalSum);
    f32 dot = normalSum.dot(getVelocity(pActor));

    if (dot < -reboundMin) {
        *getVelocityPtr(pActor) -= normalSum * dot;
        scaleVelocity(pActor, friction);
        *getVelocityPtr(pActor) -= normalSum * dot * reboundStrength;
        return true;
    } else if (dot < 0.0f) {
        *getVelocityPtr(pActor) -= normalSum * dot;
    }

    return false;
}

/**
 * Calculates velocity separate HV.
 * @param pVelocity The velocity.
 * @param pActor The actor.
 * @param rH The h.
 * @param speedH The speed h.
 * @param speedV The speed v.
 */
void calcVelocitySeparateHV(sead::Vector3f* pVelocity, const LiveActor* pActor, const sead::Vector3f& rH, f32 speedH, f32 speedV) {
    sead::Vector3f dir;
    verticalizeVec(&dir, getGravity(pActor), rH);
    normalizeOrZero(&dir);
    pVelocity->set(dir * speedH - getGravity(pActor) * speedV);
}

/**
 * Sets velocity separate HV.
 * @param pActor The actor.
 * @param rH The h.
 * @param speedH The speed h.
 * @param speedV The speed v.
 */
void setVelocitySeparateHV(LiveActor* pActor, const sead::Vector3f& rH, f32 speedH, f32 speedV) {
    calcVelocitySeparateHV(getVelocityPtr(pActor), pActor, rH, speedH, speedV);
}

/**
 * Limits velocity separate HV.
 * @param pActor The actor.
 * @param rDir The direction.
 * @param horizontal The horizontal.
 * @param vertical The vertical.
 */
void limitVelocitySeparateHV(LiveActor* pActor, const sead::Vector3f& rDir, f32 horizontal, f32 vertical) {
    limitVectorSeparateHV(getVelocityPtr(pActor), rDir, horizontal, vertical);
}

/**
 * Calculates velocity blow attack.
 * @param pVelocity The velocity.
 * @param pActor The actor.
 * @param rTrans The translation.
 * @param speedH The speed h.
 * @param speedV The speed v.
 */
void calcVelocityBlowAttack(sead::Vector3f* pVelocity, const LiveActor* pActor, const sead::Vector3f& rTrans, f32 speedH, f32 speedV) {
    calcVelocitySeparateHV(pVelocity, pActor, getTrans(pActor) - rTrans, speedH, speedV);
}

/**
 * Adds velocity blow attack.
 * @param pActor The actor.
 * @param rTrans The translation.
 * @param speedH The speed h.
 * @param speedV The speed v.
 */
void addVelocityBlowAttack(LiveActor* pActor, const sead::Vector3f& rTrans, f32 speedH, f32 speedV) {
    sead::Vector3f velocity;
    calcVelocityBlowAttack(&velocity, pActor, rTrans, speedH, speedV);
    addVelocity(pActor, velocity);
}

/**
 * Sets velocity blow attack.
 * @param pActor The actor.
 * @param rTrans The translation.
 * @param speedH The speed h.
 * @param speedV The speed v.
 */
void setVelocityBlowAttack(LiveActor* pActor, const sead::Vector3f& rTrans, f32 speedH, f32 speedV) {
    setVelocitySeparateHV(pActor, getTrans(pActor) - rTrans, speedH, speedV);
}

/**
 * Sets velocity blow attack and turn to target.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param speedH The speed h.
 * @param speedV The speed v.
 */
void setVelocityBlowAttackAndTurnToTarget(LiveActor* pActor, const sead::Vector3f& rTarget, f32 speedH, f32 speedV) {
    sead::Vector3f dir = getTrans(pActor) - rTarget;
    bool isValidDir = !normalizeOrZero(&dir);
    setVelocitySeparateHV(pActor, dir, speedH, speedV);

    if (!isValidDir)
        return;
    sead::Quatf quat;
    makeQuatUpFront(&quat, -getGravity(pActor), -dir);
    updatePoseQuat(pActor, quat);
}

/**
 * Checks whether velocity fast.
 * @param pActor The actor.
 * @param threshold The threshold.
 * @return Whether the check succeeded.
 */
bool isVelocityFast(const LiveActor* pActor, f32 threshold) {
    return getVelocity(pActor).squaredLength() > sead::Mathf::square(threshold);
}

/**
 * Checks whether velocity slow.
 * @param pActor The actor.
 * @param threshold The threshold.
 * @return Whether the check succeeded.
 */
bool isVelocitySlow(const LiveActor* pActor, f32 threshold) {
    return getVelocity(pActor).squaredLength() < sead::Mathf::square(threshold);
}

/**
 * Calculates speed H.
 * @param pActor The actor.
 * @return The result.
 */
f32 calcSpeedH(const LiveActor* pActor) {
    sead::Vector3f velocityH;
    verticalizeVec(&velocityH, getGravity(pActor), getVelocity(pActor));
    return velocityH.length();
}

/**
 * Calculates speed V.
 * @param pActor The actor.
 * @return The result.
 */
f32 calcSpeedV(const LiveActor* pActor) {
    return -getVelocity(pActor).dot(getGravity(pActor));
}

/**
 * Checks whether near.
 * @param pActor The actor.
 * @param pTarget The target actor.
 * @param threshold The threshold.
 * @return Whether the check succeeded.
 */
bool isNear(const LiveActor* pActor, const LiveActor* pTarget, f32 threshold) {
    return isNear(pActor, getTrans(pTarget), threshold);
}

/**
 * Checks whether near.
 * @param pActor The actor.
 * @param rTrans The translation.
 * @param threshold The threshold.
 * @return Whether the check succeeded.
 */
bool isNear(const LiveActor* pActor, const sead::Vector3f& rTrans, f32 threshold) {
    return (getTrans(pActor) - rTrans).squaredLength() < sead::Mathf::square(threshold);
}

/**
 * Checks whether far.
 * @param pActor The actor.
 * @param pTarget The target actor.
 * @param threshold The threshold.
 * @return Whether the check succeeded.
 */
bool isFar(const LiveActor* pActor, const LiveActor* pTarget, f32 threshold) {
    return isFar(pActor, getTrans(pTarget), threshold);
}

/**
 * Checks whether far.
 * @param pActor The actor.
 * @param rTrans The translation.
 * @param threshold The threshold.
 * @return Whether the check succeeded.
 */
bool isFar(const LiveActor* pActor, const sead::Vector3f& rTrans, f32 threshold) {
    return (getTrans(pActor) - rTrans).squaredLength() > sead::Mathf::square(threshold);
}

/**
 * Calculates distance.
 * @param pActor The actor.
 * @param pTarget The target actor.
 * @return The result.
 */
f32 calcDistance(const LiveActor* pActor, const LiveActor* pTarget) {
    return calcDistance(pActor, getTrans(pTarget));
}

/**
 * Calculates distance.
 * @param pActor The actor.
 * @param rTrans The translation.
 * @return The result.
 */
f32 calcDistance(const LiveActor* pActor, const sead::Vector3f& rTrans) {
    return (getTrans(pActor) - rTrans).length();
}

/**
 * Calculates distance V.
 * @param pActor The actor.
 * @param pTarget The target actor.
 * @return The result.
 */
f32 calcDistanceV(const LiveActor* pActor, const LiveActor* pTarget) {
    return calcDistanceV(pActor, getTrans(pTarget));
}

/**
 * Calculates distance V.
 * @param pActor The actor.
 * @param rTrans The translation.
 * @return The result.
 */
f32 calcDistanceV(const LiveActor* pActor, const sead::Vector3f& rTrans) {
    const sead::Vector3f& gravity = getGravity(pActor);
    return sead::Mathf::abs((rTrans - getTrans(pActor)).dot(gravity));
}

/**
 * Calculates distance H.
 * @param pActor The actor.
 * @param pTarget The target actor.
 * @return The result.
 */
f32 calcDistanceH(const LiveActor* pActor, const LiveActor* pTarget) {
    return calcDistanceH(pActor, getTrans(pTarget));
}

/**
 * Calculates distance H.
 * @param pActor The actor.
 * @param rTrans The translation.
 * @return The result.
 */
f32 calcDistanceH(const LiveActor* pActor, const sead::Vector3f& rTrans) {
    sead::Vector3f dist;
    verticalizeVec(&dist, getGravity(pActor), rTrans - getTrans(pActor));
    return dist.length();
}

/**
 * Calculates distance H.
 * @param pActor The actor.
 * @param rTrans1 The trans1.
 * @param rTrans2 The trans2.
 * @return The result.
 */
f32 calcDistanceH(const LiveActor* pActor, const sead::Vector3f& rTrans1, const sead::Vector3f& rTrans2) {
    sead::Vector3f dist;
    verticalizeVec(&dist, getGravity(pActor), rTrans2 - rTrans1);
    return dist.length();
}

/**
 * Calculates height.
 * @param pActor The actor.
 * @param rTrans The translation.
 * @return The result.
 */
f32 calcHeight(const LiveActor* pActor, const sead::Vector3f& rTrans) {
    const sead::Vector3f& gravity = getGravity(pActor);
    return -(rTrans - getTrans(pActor)).dot(gravity);
}

/**
 * Calculates height.
 * @param pActor The actor.
 * @param pTarget The target actor.
 * @return The result.
 */
f32 calcHeight(const LiveActor* pActor, const LiveActor* pTarget) {
    return calcHeight(pActor, getTrans(pTarget));
}

/**
 * Adds rotate and repeat Y.
 * @param pActor The actor.
 * @param deg The deg.
 */
void addRotateAndRepeatY(LiveActor* pActor, f32 deg) {
    setRotateY(pActor, wrapAngle(getRotate(pActor).y + deg));
}

/**
 * Calculates quat side.
 * @param pSide The side.
 * @param pActor The actor.
 */
void calcQuatSide(sead::Vector3f* pSide, const LiveActor* pActor) {
    calcQuatSide(pSide, getQuat(pActor));
}

/**
 * Calculates quat up.
 * @param pUp The up direction.
 * @param pActor The actor.
 */
void calcQuatUp(sead::Vector3f* pUp, const LiveActor* pActor) {
    calcQuatUp(pUp, getQuat(pActor));
}

/**
 * Calculates quat front.
 * @param pFront The front direction.
 * @param pActor The actor.
 */
void calcQuatFront(sead::Vector3f* pFront, const LiveActor* pActor) {
    calcQuatFront(pFront, getQuat(pActor));
}

/**
 * Calculates quat local axis.
 * @param pLocal The local.
 * @param pActor The actor.
 * @param axis The axis.
 */
void calcQuatLocalAxis(sead::Vector3f* pLocal, const LiveActor* pActor, s32 axis) {
    calcQuatLocalAxis(pLocal, getQuat(pActor), axis);
}

/**
 * Calculates trans offset front.
 * @param pOffset The offset.
 * @param pActor The actor.
 * @param len The len.
 */
void calcTransOffsetFront(sead::Vector3f* pOffset, const LiveActor* pActor, f32 len) {
    multVecPose(pOffset, pActor, {0.0f, 0.0f, len});
}

/**
 * Calculates trans offset up.
 * @param pOffset The offset.
 * @param pActor The actor.
 * @param len The len.
 */
void calcTransOffsetUp(sead::Vector3f* pOffset, const LiveActor* pActor, f32 len) {
    multVecPose(pOffset, pActor, {0.0f, len, 0.0f});
}

/**
 * Calculates trans offset side.
 * @param pOffset The offset.
 * @param pActor The actor.
 * @param len The len.
 */
void calcTransOffsetSide(sead::Vector3f* pOffset, const LiveActor* pActor, f32 len) {
    multVecPose(pOffset, pActor, {len, 0.0f, 0.0f});
}

/**
 * Sets trans offset local dir.
 * @param pActor The actor.
 * @param rQuat The quaternion.
 * @param rGlobalOffset The global offset.
 * @param localOffset The local offset.
 * @param axis The axis.
 */
void setTransOffsetLocalDir(LiveActor* pActor, const sead::Quatf& rQuat, const sead::Vector3f& rGlobalOffset, f32 localOffset, s32 axis) {
    sead::Vector3f offset;
    calcQuatLocalAxis(&offset, rQuat, axis);
    getTransPtr(pActor)->setScaleAdd(localOffset, offset, rGlobalOffset);
}

/**
 * Adds trans offset local dir.
 * @param pActor The actor.
 * @param localOffset The local offset.
 * @param axis The axis.
 */
void addTransOffsetLocalDir(LiveActor* pActor, f32 localOffset, s32 axis) {
    setTransOffsetLocalDir(pActor, getQuat(pActor), getTrans(pActor), localOffset, axis);
}

/**
 * Rotates quat X dir degree.
 * @param pActor The actor.
 * @param deg The deg.
 */
void rotateQuatXDirDegree(LiveActor* pActor, f32 deg) {
    sead::Quatf* quat = getQuatPtr(pActor);
    rotateQuatXDirDegree(quat, *quat, deg);
}

/**
 * Rotates quat X dir degree.
 * @param pActor The actor.
 * @param rQuat The quaternion.
 * @param deg The deg.
 */
void rotateQuatXDirDegree(LiveActor* pActor, const sead::Quatf& rQuat, f32 deg) {
    rotateQuatXDirDegree(getQuatPtr(pActor), rQuat, deg);
}

/**
 * Rotates quat Y dir degree.
 * @param pActor The actor.
 * @param deg The deg.
 */
void rotateQuatYDirDegree(LiveActor* pActor, f32 deg) {
    sead::Quatf* quat = getQuatPtr(pActor);
    rotateQuatYDirDegree(quat, *quat, deg);
}

/**
 * Rotates quat Y dir degree.
 * @param pActor The actor.
 * @param rQuat The quaternion.
 * @param deg The deg.
 */
void rotateQuatYDirDegree(LiveActor* pActor, const sead::Quatf& rQuat, f32 deg) {
    rotateQuatYDirDegree(getQuatPtr(pActor), rQuat, deg);
}

/**
 * Rotates quat Z dir degree.
 * @param pActor The actor.
 * @param deg The deg.
 */
void rotateQuatZDirDegree(LiveActor* pActor, f32 deg) {
    sead::Quatf* quat = getQuatPtr(pActor);
    rotateQuatZDirDegree(quat, *quat, deg);
}

/**
 * Rotates quat Z dir degree.
 * @param pActor The actor.
 * @param rQuat The quaternion.
 * @param deg The deg.
 */
void rotateQuatZDirDegree(LiveActor* pActor, const sead::Quatf& rQuat, f32 deg) {
    rotateQuatZDirDegree(getQuatPtr(pActor), rQuat, deg);
}

/**
 * Rotates quat local dir degree.
 * @param pActor The actor.
 * @param axis The axis.
 * @param deg The deg.
 */
void rotateQuatLocalDirDegree(LiveActor* pActor, s32 axis, f32 deg) {
    sead::Quatf* quat = getQuatPtr(pActor);
    rotateQuatLocalDirDegree(quat, *quat, axis, deg);
}

/**
 * Rotates quat local dir degree.
 * @param pActor The actor.
 * @param rQuat The quaternion.
 * @param axis The axis.
 * @param deg The deg.
 */
void rotateQuatLocalDirDegree(LiveActor* pActor, const sead::Quatf& rQuat, s32 axis, f32 deg) {
    rotateQuatLocalDirDegree(getQuatPtr(pActor), rQuat, axis, deg);
}

/**
 * Rotates quat Y dir random degree.
 * @param pActor The actor.
 */
void rotateQuatYDirRandomDegree(LiveActor* pActor) {
    sead::Quatf* quat = getQuatPtr(pActor);
    rotateQuatYDirDegree(quat, *quat, getRandomDegree());
}

/**
 * Rotates quat Y dir random degree.
 * @param pActor The actor.
 * @param rQuat The quaternion.
 */
void rotateQuatYDirRandomDegree(LiveActor* pActor, const sead::Quatf& rQuat) {
    rotateQuatYDirDegree(getQuatPtr(pActor), rQuat, getRandomDegree());
}

/**
 * Turns quat front to dir degree H.
 * @param pActor The actor.
 * @param rDir The direction.
 * @param deg The deg.
 * @return Whether the check succeeded.
 */
bool turnQuatFrontToDirDegreeH(LiveActor* pActor, const sead::Vector3f& rDir, f32 deg) {
    return turnQuatFrontToDirDegreeH(getQuatPtr(pActor), rDir, deg);
}

/**
 * Turns quat front to pos degree H.
 * @param pActor The actor.
 * @param rPos The position.
 * @param deg The deg.
 * @return Whether the check succeeded.
 */
bool turnQuatFrontToPosDegreeH(LiveActor* pActor, const sead::Vector3f& rPos, f32 deg) {
    sead::Vector3f dir;
    dir.setSub(rPos, getTrans(pActor));
    return turnQuatFrontToDirDegreeH(pActor, dir, deg);
}

/**
 * Checks whether face to target degree.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param rFace The face.
 * @param threshDeg The thresh deg.
 * @return Whether the check succeeded.
 */
bool isFaceToTargetDegree(const LiveActor* pActor, const sead::Vector3f& rTarget, const sead::Vector3f& rFace, f32 threshDeg) {
    return isNearAngleDegree(rTarget - getTrans(pActor), rFace, threshDeg);
}

/**
 * Checks whether face to target degree.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param threshDeg The thresh deg.
 * @return Whether the check succeeded.
 */
bool isFaceToTargetDegree(const LiveActor* pActor, const sead::Vector3f& rTarget, f32 threshDeg) {
    sead::Vector3f front = getFront(pActor);
    return isNearAngleDegree(rTarget - getTrans(pActor), front, threshDeg);
}

/**
 * Checks whether face to target degree HV.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param rFace The face.
 * @param degH The deg h.
 * @param degV The deg v.
 * @return Whether the check succeeded.
 */
bool isFaceToTargetDegreeHV(const LiveActor* pActor, const sead::Vector3f& rTarget, const sead::Vector3f& rFace, f32 degH, f32 degV) {
    return isNearAngleDegreeHV(rTarget - getTrans(pActor), rFace, getGravity(pActor), degH, degV);
}

/**
 * Checks whether face to target degree H.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param rFace The face.
 * @param degH The deg h.
 * @return Whether the check succeeded.
 */
bool isFaceToTargetDegreeH(const LiveActor* pActor, const sead::Vector3f& rTarget, const sead::Vector3f& rFace, f32 degH) {
    sead::Vector3f diff = rTarget;
    diff -= getTrans(pActor);
    verticalizeVec(&diff, getGravity(pActor), diff);
    sead::Vector3f alignedFace;
    verticalizeVec(&alignedFace, getGravity(pActor), rFace);
    return isNearAngleDegree(diff, alignedFace, degH);
}

/**
 * Checks whether in sight cone.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param rFace The face.
 * @param maxDist The max dist.
 * @param threshDeg The thresh deg.
 * @return Whether the check succeeded.
 */
bool isInSightCone(const LiveActor* pActor, const sead::Vector3f& rTarget, const sead::Vector3f& rFace, f32 maxDist, f32 threshDeg) {
    return (getTrans(pActor) - rTarget).squaredLength() < sead::Mathf::square(maxDist) &&
           isFaceToTargetDegree(pActor, rTarget, rFace, threshDeg);
}

/**
 * Checks whether in sight fan.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param rFace The face.
 * @param maxDist The max dist.
 * @param angleH The angle h.
 * @param angleV The angle v.
 * @return Whether the check succeeded.
 */
bool isInSightFan(const LiveActor* pActor, const sead::Vector3f& rTarget, const sead::Vector3f& rFace, f32 maxDist, f32 angleH, f32 angleV) {
    return (getTrans(pActor) - rTarget).squaredLength() < sead::Mathf::square(maxDist) &&
           isFaceToTargetDegreeHV(pActor, rTarget, rFace, angleH, angleV);
}

/**
 * Turns direction.
 * @param pActor The actor.
 * @param pVec The vec.
 * @param rDir The direction.
 * @param cos The cos.
 * @return Whether the check succeeded.
 */
bool turnDirection(const LiveActor* pActor, sead::Vector3f* pVec, const sead::Vector3f& rDir, f32 cos) {
    return turnVecToVecCosOnPlane(pVec, rDir, getGravity(pActor), cos);
}

/**
 * Turns direction degree.
 * @param pActor The actor.
 * @param pVec The vec.
 * @param rDir The direction.
 * @param deg The deg.
 * @return Whether the check succeeded.
 */
bool turnDirectionDegree(const LiveActor* pActor, sead::Vector3f* pVec, const sead::Vector3f& rDir, f32 deg) {
    f32 cos = sead::Mathf::cos(sead::Mathf::deg2rad(deg));
    return turnDirection(pActor, pVec, rDir, cos);
}

/**
 * Turns direction to target.
 * @param pActor The actor.
 * @param pVec The vec.
 * @param rTarget The target actor.
 * @param cos The cos.
 * @return Whether the check succeeded.
 */
bool turnDirectionToTarget(const LiveActor* pActor, sead::Vector3f* pVec, const sead::Vector3f& rTarget, f32 cos) {
    return turnDirection(pActor, pVec, rTarget - getTrans(pActor), cos);
}

/**
 * Turns direction to target degree.
 * @param pActor The actor.
 * @param pVec The vec.
 * @param rTarget The target actor.
 * @param deg The deg.
 * @return Whether the check succeeded.
 */
bool turnDirectionToTargetDegree(const LiveActor* pActor, sead::Vector3f* pVec, const sead::Vector3f& rTarget, f32 deg) {
    return turnDirectionDegree(pActor, pVec, rTarget - getTrans(pActor), deg);
}

/**
 * Turns direction from target degree.
 * @param pActor The actor.
 * @param pVec The vec.
 * @param rTarget The target actor.
 * @param deg The deg.
 * @return Whether the check succeeded.
 */
bool turnDirectionFromTargetDegree(const LiveActor* pActor, sead::Vector3f* pVec, const sead::Vector3f& rTarget, f32 deg) {
    return turnDirectionDegree(pActor, pVec, getTrans(pActor) - rTarget, deg);
}

/**
 * Turns direction along ground.
 * @param pActor The actor.
 * @param pDir The direction.
 */
void turnDirectionAlongGround(const LiveActor* pActor, sead::Vector3f* pDir) {
    sead::Vector3f down;

    if (isCollidedGround(pActor))
        down = -getOnGroundNormal(pActor, 0);
    else
        down.set(getGravity(pActor));

    verticalizeVec(pDir, down, *pDir);
    normalize(pDir);
}

/**
 * Turns direction along ground.
 * @param pActor The actor.
 */
void turnDirectionAlongGround(LiveActor* pActor) {
    if (tryGetQuatPtr(pActor)) {
        sead::Vector3f ground;
        calcFrontDir(&ground, pActor);

        sead::Vector3f down;

        if (isCollidedGround(pActor))
            down = -getOnGroundNormal(pActor, 0);
        else
            down.set(getGravity(pActor));

        verticalizeVec(&ground, down, ground);

        if (normalizeOrZero(&ground))
            return;

        turnToDirectionAxis(pActor, ground, -down, 180.0f);
    } else if (getFrontPtr(pActor)) {
        turnDirectionAlongGround(pActor, getFrontPtr(pActor));
    }
}

/**
 * Turns to direction.
 * @param pActor The actor.
 * @param rDir The direction.
 * @param deg The deg.
 * @return Whether the check succeeded.
 */
bool turnToDirection(LiveActor* pActor, const sead::Vector3f& rDir, f32 deg) {
    sead::Vector3f vec;
    calcFrontDir(&vec, pActor);
    bool result = turnDirectionDegree(pActor, &vec, rDir, deg);

    sead::Quatf quat;
    makeQuatUpFront(&quat, -getGravity(pActor), vec);
    updatePoseQuat(pActor, quat);
    return result;
}

/**
 * Turns to target.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param deg The deg.
 * @return Whether the check succeeded.
 */
bool turnToTarget(LiveActor* pActor, const sead::Vector3f& rTarget, f32 deg) {
    sead::Vector3f dir = rTarget - getTrans(pActor);

    if (normalizeOrZero(&dir))
        return false;
    return turnToDirection(pActor, dir, deg);
}

/**
 * Turns to target.
 * @param pActor The actor.
 * @param pTarget The target actor.
 * @param deg The deg.
 * @return Whether the check succeeded.
 */
bool turnToTarget(LiveActor* pActor, const LiveActor* pTarget, f32 deg) {
    return turnToTarget(pActor, getTrans(pTarget), deg);
}

/**
 * Faces to direction.
 * @param pActor The actor.
 * @param rDir The direction.
 */
void faceToDirection(LiveActor* pActor, const sead::Vector3f& rDir) {
    if (isParallelDirection(rDir, getGravity(pActor), 0.01f))
        return;

    sead::Quatf quat;
    makeQuatUpFront(&quat, -getGravity(pActor), rDir);
    updatePoseQuat(pActor, quat);
}

/**
 * Faces to target.
 * @param pActor The actor.
 * @param rTarget The target actor.
 */
void faceToTarget(LiveActor* pActor, const sead::Vector3f& rTarget) {
    sead::Vector3f direction = rTarget - getTrans(pActor);

    if (normalizeOrZero(&direction))
        return;
    faceToDirection(pActor, direction);
}

/**
 * Faces to target.
 * @param pActor The actor.
 * @param pTarget The target actor.
 */
void faceToTarget(LiveActor* pActor, const LiveActor* pTarget) {
    faceToTarget(pActor, getTrans(pTarget));
}

/**
 * Faces to velocity.
 * @param pActor The actor.
 */
void faceToVelocity(LiveActor* pActor) {
    sead::Vector3f direction = getVelocity(pActor);

    if (normalizeOrZero(&direction))
        return;
    faceToDirection(pActor, direction);
}

/**
 * Adds velocity clockwise to direction.
 * @param pActor The actor.
 * @param rDir The direction.
 * @param force The force.
 */
void addVelocityClockwiseToDirection(LiveActor* pActor, const sead::Vector3f& rDir, f32 force) {
    sead::Vector3f dirVelocity;

    if (!calcVelocityClockwiseToDirection(pActor, &dirVelocity, rDir))
        return;
    sead::Vector3f normDir;
    normalizeOrZero(&normDir, dirVelocity);
    addVelocityInline(pActor, normDir, force);
}

/**
 * Adds velocity clockwise to target.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param force The force.
 */
void addVelocityClockwiseToTarget(LiveActor* pActor, const sead::Vector3f& rTarget, f32 force) {
    addVelocityClockwiseToDirection(pActor, rTarget - getTrans(pActor), force);
}

/**
 * Calculates dir clockwise to dir.
 * @param pOut The out.
 * @param pActor The actor.
 * @param rDir The direction.
 */
void calcDirClockwiseToDir(sead::Vector3f* pOut, const LiveActor* pActor, const sead::Vector3f& rDir) {
    sead::Vector3f result;
    result.setCross(getGravity(pActor), rDir);
    normalizeOrZero(pOut, result);
}

/**
 * Calculates dir clockwise to pos.
 * @param pOut The out.
 * @param pActor The actor.
 * @param rTarget The target actor.
 */
void calcDirClockwiseToPos(sead::Vector3f* pOut, const LiveActor* pActor, const sead::Vector3f& rTarget) {
    calcDirClockwiseToDir(pOut, pActor, rTarget - getTrans(pActor));
}

/**
 * Calculates dir to actor.
 * @param pDir The direction.
 * @param pActor The actor.
 * @param pTarget The target actor.
 */
void calcDirToActor(sead::Vector3f* pDir, const LiveActor* pActor, const LiveActor* pTarget) {
    pDir->setSub(getTrans(pTarget), getTrans(pActor));
    normalizeOrZero(pDir);
}

/**
 * Calculates angle to target H.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @return The result.
 */
f32 calcAngleToTargetH(const LiveActor* pActor, const sead::Vector3f& rTarget) {
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    sead::Vector3f up = {0.0f, 0.0f, 0.0f};
    sead::Vector3f dir = {0.0f, 0.0f, 0.0f};
    calcFrontDir(&front, pActor);
    calcUpDir(&up, pActor);
    dir.setSub(rTarget, getTrans(pActor));

    if (normalizeOrZero(&dir))
        return 0.0f;
    return calcAngleOnPlaneDegree(front, dir, up);
}

/**
 * Calculates angle to target V.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @return The result.
 */
f32 calcAngleToTargetV(const LiveActor* pActor, const sead::Vector3f& rTarget) {
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    sead::Vector3f side = {0.0f, 0.0f, 0.0f};
    sead::Vector3f dir = {0.0f, 0.0f, 0.0f};
    calcFrontDir(&front, pActor);
    calcSideDir(&side, pActor);
    dir.setSub(rTarget, getTrans(pActor));

    if (normalizeOrZero(&dir))
        return 0.0f;
    return calcAngleOnPlaneDegree(front, dir, side);
}

/**
 * Walks and turn to direction.
 * @param pActor The actor.
 * @param rDir The direction.
 * @param forceFront The force front.
 * @param forceGravity The force gravity.
 * @param decay The decay.
 * @param deg The deg.
 * @param turnAlongGround The turn along ground.
 */
void walkAndTurnToDirection(LiveActor* pActor, const sead::Vector3f& rDir, f32 forceFront, f32 forceGravity, f32 decay, f32 deg, bool turnAlongGround) {
    walkAndTurnToDirection(pActor, getFrontPtr(pActor), rDir, forceFront, forceGravity, decay, deg,
                           turnAlongGround);
}

/**
 * Walks and turn to direction.
 * @param pActor The actor.
 * @param pFront The front direction.
 * @param rDir The direction.
 * @param forceFront The force front.
 * @param forceGravity The force gravity.
 * @param decay The decay.
 * @param deg The deg.
 * @param turnAlongGround The turn along ground.
 */
void walkAndTurnToDirection(LiveActor* pActor, sead::Vector3f* pFront, const sead::Vector3f& rDir, f32 forceFront, f32 forceGravity, f32 decay, f32 deg, bool turnAlongGround) {
    turnDirection(pActor, pFront, rDir, sead::Mathf::cos(sead::Mathf::deg2rad(deg)));

    if (turnAlongGround)
        turnDirectionAlongGround(pActor);

    sead::Vector3f velFront;
    normalizeOrZero(&velFront, *pFront);
    addVelocityInline(pActor, velFront, forceFront);

    if (!isOnGround(pActor, 3, 0.0f))
        addVelocityToGravity(pActor, forceGravity);

    scaleVelocity(pActor, decay);
}

/**
 * Walks and turn to target.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param forceFront The force front.
 * @param forceGravity The force gravity.
 * @param decay The decay.
 * @param deg The deg.
 * @param turnAlongGround The turn along ground.
 */
void walkAndTurnToTarget(LiveActor* pActor, const sead::Vector3f& rTarget, f32 forceFront, f32 forceGravity, f32 decay, f32 deg, bool turnAlongGround) {
    sead::Vector3f dir = rTarget - getTrans(pActor);
    walkAndTurnToDirection(pActor, dir, forceFront, forceGravity, decay, deg, turnAlongGround);
}

/**
 * Flies and turn to direction.
 * @param pActor The actor.
 * @param pFront The front direction.
 * @param rDir The direction.
 * @param forceFront The force front.
 * @param forceGravity The force gravity.
 * @param decay The decay.
 * @param deg The deg.
 */
void flyAndTurnToDirection(LiveActor* pActor, sead::Vector3f* pFront, const sead::Vector3f& rDir, f32 forceFront, f32 forceGravity, f32 decay, f32 deg) {
    sead::Vector3f normFrontH, actorFront, frontH;

    calcFrontDir(&actorFront, pActor);
    turnDirectionDegree(pActor, &actorFront, rDir, deg);
    turnVecToVecDegree(pFront, *pFront, actorFront, deg);

    calcFrontDir(&frontH, pActor);
    verticalizeVec(&frontH, getGravity(pActor), frontH);
    normalize(&frontH);

    normalizeOrZero(&normFrontH, frontH);
    addVelocityInline(pActor, normFrontH, forceFront);
    addVelocityToGravity(pActor, forceGravity);
    scaleVelocity(pActor, decay);
}

/**
 * Flies and turn to direction.
 * @param pActor The actor.
 * @param rDir The direction.
 * @param forceFront The force front.
 * @param forceGravity The force gravity.
 * @param decay The decay.
 * @param deg The deg.
 */
void flyAndTurnToDirection(LiveActor* pActor, const sead::Vector3f& rDir, f32 forceFront, f32 forceGravity, f32 decay, f32 deg) {
    flyAndTurnToDirection(pActor, getFrontPtr(pActor), rDir, forceFront, forceGravity, decay, deg);
}

/**
 * Flies and turn to target.
 * @param pActor The actor.
 * @param rTarget The target actor.
 * @param forceFront The force front.
 * @param forceGravity The force gravity.
 * @param decay The decay.
 * @param deg The deg.
 */
void flyAndTurnToTarget(LiveActor* pActor, const sead::Vector3f& rTarget, f32 forceFront, f32 forceGravity, f32 decay, f32 deg) {
    flyAndTurnToDirection(pActor, rTarget - getTrans(pActor), forceFront, forceGravity, decay, deg);
}

/**
 * Tries to kill by death area.
 * @param pActor The actor.
 * @return Whether the check succeeded.
 */
bool tryKillByDeathArea(LiveActor* pActor) {
    if (!isInDeathArea(pActor, getTrans(pActor)))
        return false;
    pActor->kill();
    return true;
}

/**
 * Calculates spring movement.
 * @param pActor The actor.
 * @param rPos The position.
 * @param springPos The spring pos.
 * @param sinStrength The sin strength.
 * @param rOffset The offset.
 * @param constStrength The const strength.
 * @param sinAmpl The sin ampl.
 */
void calcSpringMovement(LiveActor* pActor, const sead::Vector3f& rPos, f32 springPos, f32 sinStrength, const sead::Vector3f& rOffset, f32 constStrength, f32 sinAmpl) {
    f32 sinPart = sead::Mathf::sin(sead::Mathf::clamp(springPos, 0.0f, 1.0f) * 2 *
                                   sead::Mathf::pi() * sinAmpl) *
                  (1.0f - springPos) * sinStrength;
    f32 constPart = (1.0f - springPos) * constStrength;
    getTransPtr(pActor)->setScaleAdd(sinPart + constPart, rOffset, rPos);
}

/**
 * Adds velocity clockwise to player.
 * @param pActor The actor.
 * @param force The force.
 */
void addVelocityClockwiseToPlayer(LiveActor* pActor, f32 force) {
    addVelocityClockwiseToDirection(pActor, alProjectInterface::getPlayerPos() - getTrans(pActor),
                                    force);
}

/**
 * Calculates dir clockwise to player.
 * @param pDir The direction.
 * @param pActor The actor.
 */
void calcDirClockwiseToPlayer(sead::Vector3f* pDir, const LiveActor* pActor) {
    calcDirClockwiseToDir(pDir, pActor, alProjectInterface::getPlayerPos());
}

/**
 * Walks and turn to player.
 * @param pActor The actor.
 * @param forceFront The force front.
 * @param forceGravity The force gravity.
 * @param decay The decay.
 * @param deg The deg.
 * @param turnAlongGround The turn along ground.
 */
void walkAndTurnToPlayer(LiveActor* pActor, f32 forceFront, f32 forceGravity, f32 decay, f32 deg, bool turnAlongGround) {
    sead::Vector3f dir = alProjectInterface::getPlayerPos() - getTrans(pActor);
    walkAndTurnToDirection(pActor, getFrontPtr(pActor), dir, forceFront, forceGravity, decay, deg,
                           turnAlongGround);
}

/**
 * Flies and turn to player.
 * @param pActor The actor.
 * @param rParam The param.
 */
void flyAndTurnToPlayer(LiveActor* pActor, const ActorParamMove& rParam) {
    sead::Vector3f dir = alProjectInterface::getPlayerPos() - getTrans(pActor);
    flyAndTurnToDirection(pActor, getFrontPtr(pActor), dir, rParam.moveAccel, rParam.gravity,
                          rParam.moveFriction, rParam.turnSpeedDegree);
}

/**
 * Escapes from player.
 * @param pActor The actor.
 * @param forceFront The force front.
 * @param forceGravity The force gravity.
 * @param decay The decay.
 * @param deg The deg.
 */
void escapeFromPlayer(LiveActor* pActor, f32 forceFront, f32 forceGravity, f32 decay, f32 deg) {
    sead::Vector3f dir = getTrans(pActor) - alProjectInterface::getPlayerPos();
    walkAndTurnToDirection(pActor, getFrontPtr(pActor), dir, forceFront, forceGravity, decay, deg,
                           true);
}

/**
 * Escapes from player.
 * @param pActor The actor.
 * @param pFront The front direction.
 * @param forceFront The force front.
 * @param forceGravity The force gravity.
 * @param decay The decay.
 * @param deg The deg.
 */
void escapeFromPlayer(LiveActor* pActor, sead::Vector3f* pFront, f32 forceFront, f32 forceGravity, f32 decay, f32 deg) {
    walkAndTurnToDirection(pActor, pFront, getTrans(pActor) - alProjectInterface::getPlayerPos(),
                           forceFront, forceGravity, decay, deg, true);
}

/**
 * Checks whether in sight cone player.
 * @param pActor The actor.
 * @param maxDist The max dist.
 * @param threshDeg The thresh deg.
 * @return Whether the check succeeded.
 */
bool isInSightConePlayer(const LiveActor* pActor, f32 maxDist, f32 threshDeg) {
    return isInSightCone(pActor, alProjectInterface::getPlayerPos(), getFront(pActor), maxDist,
                         threshDeg);
}
}  // namespace al
