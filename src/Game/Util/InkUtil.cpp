#include "Util/InkUtil.hpp"
#include "Library/Collision/Collider.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Project/Collision/CollisionPartsFilterBase.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace InkUtil {
/**
 * Checks whether an actor's collider sphere is inside an ink limit collision, by casting a
 * vertical arrow through the collider.
 * @param pActor actor to check
 * @return true if the arrow hits an ink limit collision
 */
bool isInInkLimitArrow(const al::LiveActor* pActor) {
    al::CollisionPartsFilterOnlySpecialPurpose filter("InkLimit");
    f32 radius = pActor->getCollider()->getRadius();
    const al::IUseCollision* pCollision = pActor;
    const sead::Vector3f& rTrans = al::getTrans(pActor);
    sead::Vector3f start = rTrans + sead::Vector3f::ey * radius;
    sead::Vector3f dir = sead::Vector3f::ey * radius * -2.0f;
    return alCollisionUtil::checkStrikeArrow(pCollision, start, dir, &filter, nullptr) != 0;
}

/**
 * Checks whether a sphere is inside an ink limit collision, by casting a vertical arrow from the
 * top to the bottom of the sphere.
 * @param pCollision collision user
 * @param rPos sphere center
 * @param radius sphere radius
 * @return true if the arrow hits an ink limit collision
 */
bool isInInkLimitArrow(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                       f32 radius) {
    al::CollisionPartsFilterOnlySpecialPurpose filter("InkLimit");
    sead::Vector3f offset = sead::Vector3f::ey * radius;
    sead::Vector3f start = rPos + offset;
    sead::Vector3f dir = offset * -2.0f;
    return alCollisionUtil::checkStrikeArrow(pCollision, start, dir, &filter, nullptr) != 0;
}

/**
 * Checks whether an arrow hits an ink limit collision.
 * @param pCollision collision user
 * @param rPos arrow start position
 * @param rDir arrow direction (and length)
 * @return true if the arrow hits an ink limit collision
 */
bool isInInkLimitArrow(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                       const sead::Vector3f& rDir) {
    al::CollisionPartsFilterOnlySpecialPurpose filter("InkLimit");
    return alCollisionUtil::checkStrikeArrow(pCollision, rPos, rDir, &filter, nullptr) != 0;
}

/**
 * Checks whether an actor's collider sphere touches an ink limit collision.
 * @param pActor actor to check
 * @return true if the sphere touches an ink limit collision
 */
bool isInInkLimitSphere(const al::LiveActor* pActor) {
    al::CollisionPartsFilterOnlySpecialPurpose filter("InkLimit");
    f32 radius = pActor->getCollider()->getRadius();
    return alCollisionUtil::checkStrikeSphere(pActor, al::getTrans(pActor), radius, &filter,
                                              nullptr) != 0;
}

/**
 * Checks whether a sphere touches an ink limit collision.
 * @param pCollision collision user
 * @param rPos sphere center
 * @param radius sphere radius
 * @return true if the sphere touches an ink limit collision
 */
bool isInInkLimitSphere(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                        f32 radius) {
    al::CollisionPartsFilterOnlySpecialPurpose filter("InkLimit");
    return alCollisionUtil::checkStrikeSphere(pCollision, rPos, radius, &filter, nullptr) != 0;
}
};  // namespace InkUtil
