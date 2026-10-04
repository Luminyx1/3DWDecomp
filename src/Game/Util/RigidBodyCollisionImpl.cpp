#include "Util/RigidBodyCollisionImpl.hpp"

#include "Project/Collision/CollisionPartsFilterBase.hpp"
#include "Project/Collision/CollisionUtil.hpp"
#include "Project/Collision/HitDb.hpp"

/**
 * Creates the collision interface for a rigid body.
 * @param pActor Owning actor whose collision parts are ignored, or nullptr for no filter.
 */
RigidBodyCollisionImpl::RigidBodyCollisionImpl(const al::LiveActor* pActor) {
    if (pActor != nullptr) {
        mPartsFilter = new al::CollisionPartsFilterActor(pActor);
    }
}

/**
 * Checks a sphere against the scene collision.
 * @param pCollision Collision user to check with.
 * @param rPos Sphere center.
 * @param radius Sphere radius.
 * @return Whether anything was hit.
 */
bool RigidBodyCollisionImpl::checkStrikeSphere(const al::IUseCollision* pCollision,
                                               const sead::Vector3f& rPos, f32 radius) {
    return alCollisionUtil::checkStrikeSphere(pCollision, rPos, radius, mPartsFilter, nullptr) !=
           0;
}

/**
 * @param pCollision Collision user of the last check.
 * @return Number of hits from the last strike-sphere check.
 */
u32 RigidBodyCollisionImpl::getHitNum(const al::IUseCollision* pCollision) const {
    return alCollisionUtil::getStrikeSphereInfoNum(pCollision);
}

/**
 * @param pCollision Collision user of the last check.
 * @param index Hit index.
 * @return Hit position.
 */
const sead::Vector3f& RigidBodyCollisionImpl::getHitPosition(const al::IUseCollision* pCollision,
                                                             u32 index) const {
    return alCollisionUtil::getStrikeSphereHitPos(pCollision, index);
}

/**
 * @param pCollision Collision user of the last check.
 * @param index Hit index.
 * @return Face normal of the hit triangle.
 */
const sead::Vector3f* RigidBodyCollisionImpl::getHitNormal(const al::IUseCollision* pCollision,
                                                           u32 index) const {
    return alCollisionUtil::getStrikeSphereInfo(pCollision, index)->mTriangle.getNormal(0);
}

/**
 * @param pCollision Collision user of the last check.
 * @param index Hit index.
 * @return Penetration depth of the hit.
 */
f32 RigidBodyCollisionImpl::getHitOverlap(const al::IUseCollision* pCollision, u32 index) const {
    return alCollisionUtil::getStrikeSphereInfo(pCollision, index)->_70;
}
