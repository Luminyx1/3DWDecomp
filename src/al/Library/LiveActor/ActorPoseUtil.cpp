#include "Library/LiveActor/Util/ActorPoseUtil.hpp"

#include "Library/Actor/ActorPoseKeeper.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorAnimUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Project/Camera/Info/SceneCameraInfo.hpp"
#include "Project/Collision/CollisionPartsFilterBase.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace al {
/**
 * Creates a translation-rotation pose keeper and sets the translation.
 * @param pActor The actor.
 * @param rTrans The translation.
 */
void initActorPoseT(LiveActor* pActor, const sead::Vector3f& rTrans) {
    pActor->initPoseKeeper(new ActorPoseKeeperTRSV());
    setTrans(pActor, rTrans);
}

/**
 * Sets the translation of an actor.
 * @param pActor The actor.
 * @param rTrans The translation.
 */
void setTrans(LiveActor* pActor, const sead::Vector3f& rTrans) {
    *getTransPtr(pActor) = rTrans;
}

/**
 * Creates a translation-rotation pose keeper and sets the translation and rotation.
 * @param pActor The actor.
 * @param rTrans The translation.
 * @param rRotate The rotation in degrees.
 */
void initActorPoseTR(LiveActor* pActor, const sead::Vector3f& rTrans,
                     const sead::Vector3f& rRotate) {
    pActor->initPoseKeeper(new ActorPoseKeeperTRSV());
    setTrans(pActor, rTrans);
    setRotate(pActor, rRotate);
}

/**
 * Sets the rotation of an actor.
 * @param pActor The actor.
 * @param rRotate The rotation in degrees.
 */
void setRotate(LiveActor* pActor, const sead::Vector3f& rRotate) {
    getRotatePtr(pActor)->set(rRotate);
}

/**
 * Calculates the scale, rotation and translation matrix of an actor.
 * @param pMtx The resulting matrix.
 * @param pActor The actor.
 */
void makeMtxSRT(sead::Matrix34f* pMtx, const LiveActor* pActor) {
    ActorPoseKeeperBase* poseKeeper = pActor->mActorPoseKeeper;
    poseKeeper->calcBaseMtx(pMtx);
    preScaleMtx(pMtx, poseKeeper->getScale());
}

/**
 * Calculates the rotation and translation matrix of an actor.
 * @param pMtx The resulting matrix.
 * @param pActor The actor.
 */
void makeMtxRT(sead::Matrix34f* pMtx, const LiveActor* pActor) {
    pActor->mActorPoseKeeper->calcBaseMtx(pMtx);
}

/**
 * Sets the base matrix of an actor from a front direction and its gravity, then calculates
 * its animation.
 * @param pActor The actor.
 * @param rFront The front direction.
 */
void calcAnimFrontGravityPos(LiveActor* pActor, const sead::Vector3f& rFront) {
    sead::Vector3f up = -getGravity(pActor);
    if (pActor->getBaseMtx() != nullptr) {
        const sead::Matrix34f* baseMtx = pActor->getBaseMtx();
        sead::Vector3f side;
        side.setCross(rFront, up);
        if (isNearZero(side, 0.001f)) {
            up.setCross(rFront, baseMtx->getBase(0));
        }
    }
    sead::Matrix34f mtx;
    makeMtxFrontUpPos(&mtx, rFront, up, getTrans(pActor));
    setBaseMtxAndCalcAnim(pActor, mtx, getScale(pActor));
}

/**
 * Gets the gravity of an actor.
 * @param pActor The actor.
 * @return The gravity.
 */
const sead::Vector3f& getGravity(const LiveActor* pActor) {
    return pActor->mActorPoseKeeper->getGravity();
}

/**
 * Gets the translation of an actor.
 * @param pActor The actor.
 * @return The translation.
 */
const sead::Vector3f& getTrans(const LiveActor* pActor) {
    return pActor->mActorPoseKeeper->mTranslation;
}

/**
 * Gets the scale of an actor.
 * @param pActor The actor.
 * @return The scale.
 */
const sead::Vector3f& getScale(const LiveActor* pActor) {
    return pActor->mActorPoseKeeper->getScale();
}

/**
 * Copies the pose of another actor.
 * @param pActor The actor.
 * @param pTarget The actor to copy.
 */
void copyPose(LiveActor* pActor, const LiveActor* pTarget) {
    pActor->mActorPoseKeeper->copyPose(pTarget->mActorPoseKeeper);
}

/**
 * Updates the pose of an actor from a rotation.
 * @param pActor The actor.
 * @param rRotate The rotation in degrees.
 */
void updatePoseRotate(LiveActor* pActor, const sead::Vector3f& rRotate) {
    pActor->mActorPoseKeeper->updatePoseRotate(rRotate);
}

/**
 * Updates the pose of an actor from a quaternion.
 * @param pActor The actor.
 * @param rQuat The quaternion.
 */
void updatePoseQuat(LiveActor* pActor, const sead::Quatf& rQuat) {
    pActor->mActorPoseKeeper->updatePoseQuat(rQuat);
}

/**
 * Updates the pose of an actor from a matrix.
 * @param pActor The actor.
 * @param pMtx The matrix.
 */
void updatePoseMtx(LiveActor* pActor, const sead::Matrix34f* pMtx) {
    pActor->mActorPoseKeeper->updatePoseMtx(pMtx);
}

/**
 * Calculates the side direction of an actor.
 * @param pSide The resulting direction.
 * @param pActor The actor.
 */
void calcSideDir(sead::Vector3f* pSide, const LiveActor* pActor) {
    sead::Matrix34f mtx;
    makeMtxRT(&mtx, pActor);
    mtx.getBase(*pSide, 0);
}

/**
 * Calculates the up direction of an actor.
 * @param pUp The resulting direction.
 * @param pActor The actor.
 */
void calcUpDir(sead::Vector3f* pUp, const LiveActor* pActor) {
    sead::Matrix34f mtx;
    makeMtxRT(&mtx, pActor);
    mtx.getBase(*pUp, 1);
}

/**
 * Calculates the front direction of an actor.
 * @param pFront The resulting direction.
 * @param pActor The actor.
 */
void calcFrontDir(sead::Vector3f* pFront, const LiveActor* pActor) {
    sead::Matrix34f mtx;
    makeMtxRT(&mtx, pActor);
    mtx.getBase(*pFront, 2);
}

/**
 * Calculates the rotation of an actor as a quaternion.
 * @param pQuat The resulting quaternion.
 * @param pActor The actor.
 */
void calcQuat(sead::Quatf* pQuat, const LiveActor* pActor) {
    sead::Matrix34f mtx;
    makeMtxRT(&mtx, pActor);
    mtx.toQuat(*pQuat);
}

/**
 * Gets a pointer to the translation of an actor.
 * @param pActor The actor.
 * @return The translation.
 */
sead::Vector3f* getTransPtr(LiveActor* pActor) {
    return &pActor->mActorPoseKeeper->mTranslation;
}

/**
 * Sets the translation of an actor.
 * @param pActor The actor.
 * @param x The x coordinate.
 * @param y The y coordinate.
 * @param z The z coordinate.
 */
void setTrans(LiveActor* pActor, f32 x, f32 y, f32 z) {
    setTrans(pActor, {x, y, z});
}

/**
 * Sets the x coordinate of an actor's translation.
 * @param pActor The actor.
 * @param x The x coordinate.
 */
void setTransX(LiveActor* pActor, f32 x) {
    getTransPtr(pActor)->x = x;
}

/**
 * Sets the y coordinate of an actor's translation.
 * @param pActor The actor.
 * @param y The y coordinate.
 */
void setTransY(LiveActor* pActor, f32 y) {
    getTransPtr(pActor)->y = y;
}

/**
 * Sets the z coordinate of an actor's translation.
 * @param pActor The actor.
 * @param z The z coordinate.
 */
void setTransZ(LiveActor* pActor, f32 z) {
    getTransPtr(pActor)->z = z;
}

/**
 * Gets the rotation of an actor.
 * @param pActor The actor.
 * @return The rotation in degrees.
 */
const sead::Vector3f& getRotate(const LiveActor* pActor) {
    return pActor->mActorPoseKeeper->getRotate();
}

/**
 * Gets a pointer to the rotation of an actor.
 * @param pActor The actor.
 * @return The rotation in degrees.
 */
sead::Vector3f* getRotatePtr(LiveActor* pActor) {
    return pActor->mActorPoseKeeper->getRotatePtr();
}

/**
 * Sets the rotation of an actor.
 * @param pActor The actor.
 * @param x The rotation around the x axis in degrees.
 * @param y The rotation around the y axis in degrees.
 * @param z The rotation around the z axis in degrees.
 */
void setRotate(LiveActor* pActor, f32 x, f32 y, f32 z) {
    setRotate(pActor, {x, y, z});
}

/**
 * Sets the rotation of an actor around the x axis.
 * @param pActor The actor.
 * @param x The rotation in degrees.
 */
void setRotateX(LiveActor* pActor, f32 x) {
    getRotatePtr(pActor)->x = x;
}

/**
 * Sets the rotation of an actor around the y axis.
 * @param pActor The actor.
 * @param y The rotation in degrees.
 */
void setRotateY(LiveActor* pActor, f32 y) {
    getRotatePtr(pActor)->y = y;
}

/**
 * Sets the rotation of an actor around the z axis.
 * @param pActor The actor.
 * @param z The rotation in degrees.
 */
void setRotateZ(LiveActor* pActor, f32 z) {
    getRotatePtr(pActor)->z = z;
}

/**
 * Rotates an actor to a rotation, keeping it at the origin.
 * @param pActor The actor.
 * @param rRotate The rotation in degrees.
 */
void rotateActor(LiveActor* pActor, const sead::Vector3f& rRotate) {
    sead::Matrix34f mtx;
    sead::Vector3f rotate = rRotate * (sead::numbers::pi / 180.0f);
    mtx.makeR(rotate);
    updatePoseMtx(pActor, &mtx);
}

/**
 * Sets the pose of an actor from a rotation and translation.
 * @param pActor The actor.
 * @param rRotate The rotation in degrees.
 * @param rTrans The translation.
 */
void rotateTranslateActor(LiveActor* pActor, const sead::Vector3f& rRotate,
                          const sead::Vector3f& rTrans) {
    sead::Matrix34f mtx;
    sead::Vector3f rotate = rRotate * (sead::numbers::pi / 180.0f);
    mtx.makeRT(rotate, rTrans);
    updatePoseMtx(pActor, &mtx);
}

/**
 * Gets a pointer to the scale of an actor.
 * @param pActor The actor.
 * @return The scale, or nullptr if the pose keeper has none.
 */
sead::Vector3f* tryGetScalePtr(LiveActor* pActor) {
    return pActor->mActorPoseKeeper->getScalePtr();
}

/**
 * Gets the x scale of an actor.
 * @param pActor The actor.
 * @return The x scale.
 */
f32 getScaleX(const LiveActor* pActor) {
    return getScale(pActor).x;
}

/**
 * Gets the y scale of an actor.
 * @param pActor The actor.
 * @return The y scale.
 */
f32 getScaleY(const LiveActor* pActor) {
    return getScale(pActor).y;
}

/**
 * Gets the z scale of an actor.
 * @param pActor The actor.
 * @return The z scale.
 */
f32 getScaleZ(const LiveActor* pActor) {
    return getScale(pActor).z;
}

/**
 * Sets the scale of an actor.
 * @param pActor The actor.
 * @param rScale The scale.
 */
void setScale(LiveActor* pActor, const sead::Vector3f& rScale) {
    *tryGetScalePtr(pActor) = rScale;
}

/**
 * Sets the scale of an actor.
 * @param pActor The actor.
 * @param x The x scale.
 * @param y The y scale.
 * @param z The z scale.
 */
void setScale(LiveActor* pActor, f32 x, f32 y, f32 z) {
    setScale(pActor, {x, y, z});
}

/**
 * Sets the same scale on all axes of an actor.
 * @param pActor The actor.
 * @param scale The scale.
 */
void setScaleAll(LiveActor* pActor, f32 scale) {
    setScale(pActor, {scale, scale, scale});
}

/**
 * Sets the x scale of an actor.
 * @param pActor The actor.
 * @param x The x scale.
 */
void setScaleX(LiveActor* pActor, f32 x) {
    tryGetScalePtr(pActor)->x = x;
}

/**
 * Sets the y scale of an actor.
 * @param pActor The actor.
 * @param y The y scale.
 */
void setScaleY(LiveActor* pActor, f32 y) {
    tryGetScalePtr(pActor)->y = y;
}

/**
 * Sets the z scale of an actor.
 * @param pActor The actor.
 * @param z The z scale.
 */
void setScaleZ(LiveActor* pActor, f32 z) {
    tryGetScalePtr(pActor)->z = z;
}

/**
 * Gets the rotation quaternion of an actor.
 * @param pActor The actor.
 * @return The quaternion.
 */
const sead::Quatf& getQuat(const LiveActor* pActor) {
    return pActor->mActorPoseKeeper->getQuat();
}

/**
 * Gets a pointer to the rotation quaternion of an actor.
 * @param pActor The actor.
 * @return The quaternion.
 */
sead::Quatf* getQuatPtr(LiveActor* pActor) {
    return tryGetQuatPtr(pActor);
}

/**
 * Gets a pointer to the rotation quaternion of an actor.
 * @param pActor The actor.
 * @return The quaternion, or nullptr if the pose keeper has none.
 */
sead::Quatf* tryGetQuatPtr(LiveActor* pActor) {
    return pActor->mActorPoseKeeper->getQuatPtr();
}

/**
 * Sets the rotation quaternion of an actor.
 * @param pActor The actor.
 * @param rQuat The quaternion.
 */
void setQuat(LiveActor* pActor, const sead::Quatf& rQuat) {
    getQuatPtr(pActor)->set(rQuat);
}

/**
 * Sets the gravity of an actor.
 * @param pActor The actor.
 * @param rGravity The gravity.
 */
void setGravity(const LiveActor* pActor, const sead::Vector3f& rGravity) {
    pActor->mActorPoseKeeper->getGravityPtr()->set(rGravity);
}

/**
 * Gets the front direction of an actor.
 * @param pActor The actor.
 * @return The front direction.
 */
const sead::Vector3f& getFront(const LiveActor* pActor) {
    return pActor->mActorPoseKeeper->getFront();
}

/**
 * Gets a pointer to the front direction of an actor.
 * @param pActor The actor.
 * @return The front direction.
 */
sead::Vector3f* getFrontPtr(LiveActor* pActor) {
    return pActor->mActorPoseKeeper->getFrontPtr();
}

/**
 * Sets the front direction of an actor.
 * @param pActor The actor.
 * @param rFront The front direction.
 */
void setFront(LiveActor* pActor, const sead::Vector3f& rFront) {
    getFrontPtr(pActor)->set(rFront);
}

/**
 * Transforms a position by the pose of an actor.
 * @param pOut The resulting position.
 * @param pActor The actor.
 * @param rPos The local position.
 */
void multVecPose(sead::Vector3f* pOut, const LiveActor* pActor, const sead::Vector3f& rPos) {
    sead::Matrix34f mtx;
    makeMtxRT(&mtx, pActor);
    pOut->setMul(mtx, rPos);
}

/**
 * Transforms a position by the inverse pose of an actor.
 * @param pOut The resulting local position.
 * @param pActor The actor.
 * @param rPos The position.
 */
void multVecInvPose(sead::Vector3f* pOut, const LiveActor* pActor, const sead::Vector3f& rPos) {
    sead::Matrix34f mtx;
    makeMtxRT(&mtx, pActor);
    mtx.invert();
    pOut->setMul(mtx, rPos);
}

/**
 * Transforms a position by the inverse translation and rotation quaternion of an actor.
 * @param pOut The resulting local position.
 * @param pActor The actor.
 * @param rPos The position.
 */
void multVecInvQuat(sead::Vector3f* pOut, const LiveActor* pActor, const sead::Vector3f& rPos) {
    ActorPoseKeeperBase* poseKeeper = pActor->mActorPoseKeeper;
    sead::Vector3f v = rPos - poseKeeper->mTranslation;
    const sead::Quatf& q = poseKeeper->getQuat();
    sead::Quatf r;
    r.x = -(q.y * v.z) + (q.z * v.y) + (q.w * v.x);
    r.y = (q.x * v.z) - (q.z * v.x) + (q.w * v.y);
    r.z = -(q.x * v.y) + (q.y * v.x) + (q.w * v.z);
    r.w = (q.x * v.x) + (q.y * v.y) + (q.z * v.z);
    pOut->x = (r.x * q.w) + (r.y * q.z) - (r.z * q.y) + (r.w * q.x);
    pOut->y = -(r.x * q.z) + (r.y * q.w) + (r.z * q.x) + (r.w * q.y);
    pOut->z = (r.x * q.y) - (r.y * q.x) + (r.z * q.w) + (r.w * q.z);
}

/**
 * Transforms a local offset by the pose of an actor.
 * @param pOut The resulting position.
 * @param pActor The actor.
 * @param rOffset The local offset.
 */
void calcTransLocalOffset(sead::Vector3f* pOut, const LiveActor* pActor,
                          const sead::Vector3f& rOffset) {
    sead::Matrix34f mtx;
    makeMtxRT(&mtx, pActor);
    calcTransLocalOffsetByMtx(pOut, mtx, rOffset);
}

/**
 * Calculates the angle from an actor's front to a direction on its horizontal plane.
 * @param pActor The actor.
 * @param rDir The direction.
 * @return The signed angle in degrees.
 */
f32 calcAngleOnPlaneDegreeToDirection(LiveActor* pActor, const sead::Vector3f& rDir) {
    sead::Vector3f front;
    calcFrontDir(&front, pActor);
    sead::Vector3f up = {0.0f, 0.0f, 0.0f};
    calcUpDir(&up, pActor);
    return calcAngleOnPlaneDegree(front, rDir, up);
}

/**
 * Calculates the angle from an actor's front to a target on its horizontal plane.
 * @param pActor The actor.
 * @param rTarget The target position.
 * @return The signed angle in degrees.
 */
f32 calcAngleOnPlaneDegreeToTarget(LiveActor* pActor, const sead::Vector3f& rTarget) {
    sead::Vector3f dir = rTarget - getTrans(pActor);
    normalizeOrDirZ(&dir);
    return calcAngleOnPlaneDegreeToDirection(pActor, dir);
}

/**
 * Calculates the angle from an actor's front to a direction, limited to a maximum.
 * @param pActor The actor.
 * @param rDir The direction.
 * @param maxDegree The maximum angle in degrees.
 * @return The signed, limited angle in degrees.
 */
f32 calcAngleOnPlaneDegreeToDirectionFixed(LiveActor* pActor, const sead::Vector3f& rDir,
                                           f32 maxDegree) {
    f32 angle = calcAngleOnPlaneDegreeToDirection(pActor, rDir);
    if (angle > 0.0f && angle > maxDegree) {
        angle = maxDegree;
    } else if (angle < 0.0f && angle < -maxDegree) {
        angle = -maxDegree;
    }
    return angle;
}

/**
 * Calculates the angle from an actor's front to a target, limited to a maximum.
 * @param pActor The actor.
 * @param rTarget The target position.
 * @param maxDegree The maximum angle in degrees.
 * @return The signed, limited angle in degrees.
 */
f32 calcAngleOnPlaneDegreeToTargetFixed(LiveActor* pActor, const sead::Vector3f& rTarget,
                                        f32 maxDegree) {
    sead::Vector3f dir = rTarget - getTrans(pActor);
    normalizeOrDirZ(&dir);
    return calcAngleOnPlaneDegreeToDirectionFixed(pActor, dir, maxDegree);
}

/**
 * Turns an actor towards a direction by at most an angle.
 * @param pActor The actor.
 * @param rDir The direction.
 * @param maxDegree The maximum angle to turn in degrees.
 * @param endDegree The angle below which the turn counts as done.
 * @return Whether the remaining angle is within the end angle.
 */
bool faceToDirection(LiveActor* pActor, const sead::Vector3f& rDir, f32 maxDegree,
                     f32 endDegree) {
    f32 angle = calcAngleOnPlaneDegreeToDirectionFixed(pActor, rDir, maxDegree);
    sead::Vector3f front;
    calcFrontDir(&front, pActor);
    sead::Vector3f up = {0.0f, 0.0f, 0.0f};
    calcUpDir(&up, pActor);
    rotateVectorDegree(&front, front, up, angle);
    faceToDirection(pActor, front);
    return sead::Mathf::abs(angle) <= endDegree;
}

/**
 * Turns an actor towards a target by at most an angle.
 * @param pActor The actor.
 * @param rTarget The target position.
 * @param maxDegree The maximum angle to turn in degrees.
 * @param endDegree The angle below which the turn counts as done.
 * @return Whether the remaining angle is within the end angle.
 */
bool faceToTarget(LiveActor* pActor, const sead::Vector3f& rTarget, f32 maxDegree,
                  f32 endDegree) {
    sead::Vector3f dir = rTarget - getTrans(pActor);
    normalizeOrDirZ(&dir);
    return faceToDirection(pActor, dir, maxDegree, endDegree);
}
/**
 * Checks whether collision blocks the line of sight from the camera to an actor.
 * @param pActor The actor.
 * @param radius The maximum distance to check, or a negative value for no limit.
 * @param pOffset An offset added to the actor position, or nullptr.
 * @return Whether the actor is obscured.
 */
bool isActorObscured(const LiveActor* pActor, f32 radius, const sead::Vector3f* pOffset) {
    sead::Vector3f cameraPos = pActor->getSceneCameraInfo()->mLookAtCamera->getPos();
    sead::Vector3f pos = pActor->mActorPoseKeeper->mTranslation;
    if (pOffset) {
        pos.add(*pOffset);
    }
    sead::Vector3f dir = pos - cameraPos;
    CollisionPartsFilterActor filter(pActor);
    if (!(radius < 0.0f) && !(dir.squaredLength() < radius * radius)) {
        return false;
    }
    return alCollisionUtil::getStrikeArrowCollisionParts(pActor, nullptr, cameraPos, dir, &filter,
                                                         nullptr) != nullptr;
}
}  // namespace al

namespace alActorPoseFunction {
/**
 * Calculates the base matrix of an actor.
 * @param pMtx The resulting matrix.
 * @param pActor The actor.
 */
void calcBaseMtx(sead::Matrix34f* pMtx, const al::LiveActor* pActor) {
    pActor->mActorPoseKeeper->calcBaseMtx(pMtx);
}

/**
 * Updates the pose of an actor from its own rotation.
 * @param pActor The actor.
 */
void updatePoseTRMSV(al::LiveActor* pActor) {
    al::ActorPoseKeeperBase* poseKeeper = pActor->mActorPoseKeeper;
    poseKeeper->updatePoseRotate(poseKeeper->getRotate());
}
}  // namespace alActorPoseFunction
