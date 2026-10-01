#include "Library/Collision/CollisionDirector.hpp"

#include "Library/Collision/CollisionPartsKeeperArray.hpp"
#include "Library/Collision/CollisionPartsKeeperOctree.hpp"
#include "Library/Collision/CollisionPartsKeeperPtrArray.hpp"
#include "Library/Collision/ICollisionPartsKeeper.hpp"
#include "Library/Execute/ExecuteUtil.hpp"
#include "Project/Collision/CollisionParts.hpp"

namespace al {
ICollisionPartsKeeper* createSimpleCollisionPartsKeeper();

/**
 * Creates the collision director and its parts keeper.
 * @param pExecuteDirector execute director to register with
 * @param threadNum number of octree threads, 0 for the default, negative for a plain array
 */
CollisionDirector::CollisionDirector(ExecuteDirector* pExecuteDirector, s32 threadNum)
    : mPtrArrayPartsKeeper(new CollisionPartsKeeperPtrArray()) {
    mStrikeArrowHitInfos = new ArrowHitResultBuffer();
    mStrikeArrowHitInfos->allocBuffer(0x200, nullptr);
    mStrikeSphereHitInfos = new SphereHitResultBuffer();
    mStrikeSphereHitInfos->allocBuffer(0x200, nullptr);
    mStrikeDiskHitInfos = new DiskHitResultBuffer();
    mStrikeDiskHitInfos->allocBuffer(0x200, nullptr);
    mSphereHitInfosForCollider = new SphereHitInfo[32];
    mDiskHitInfosForCollider = new DiskHitInfo[32];
    registerExecutorUser(this, pExecuteDirector, "コリジョンディレクター");

    s32 num = threadNum == 0 ? 4 : threadNum;

    if (num > 0) {
        setPartsKeeper(new CollisionPartsKeeperOctree(0x100, num));
    } else if (num < 0) {
        setPartsKeeper(new CollisionPartsKeeperArray());
    } else {
        setPartsKeeper(createSimpleCollisionPartsKeeper());
    }
}

/**
 * Sets the parts keeper used for collision checks.
 * @param pPartsKeeper parts keeper
 */
void CollisionDirector::setPartsKeeper(ICollisionPartsKeeper* pPartsKeeper) {
    mRootPartsKeeper = pPartsKeeper;

    if (mActivePartsKeeper != mPtrArrayPartsKeeper) {
        mActivePartsKeeper = pPartsKeeper;
    }
}

/**
 * Finishes initialization of the active parts keeper.
 */
void CollisionDirector::endInit() {
    mActivePartsKeeper->endInit();
}

/**
 * Sets the parts filter used by the next collision check.
 * @param pFilter parts filter
 */
void CollisionDirector::setPartsFilter(const CollisionPartsFilterBase* pFilter) {
    mPartsFilter = pFilter;
}

/**
 * Sets the triangle filter used by the next collision check.
 * @param pFilter triangle filter
 */
void CollisionDirector::setTriFilter(const TriangleFilterBase* pFilter) {
    mTriFilter = pFilter;
}

/**
 * Checks whether a point is inside collision.
 * @param rPos position to check
 * @param pHitInfo output hit info
 * @return the keeper's result
 */
s32 CollisionDirector::checkStrikePoint(const sead::Vector3f& rPos, HitInfo* pHitInfo) {
    CollisionCheckInfoBase checkInfo;
    checkInfo.mPos = &rPos;
    checkInfo.mPartsFilter = mPartsFilter;
    checkInfo.mTriangleFilter = mTriFilter;
    s32 result = mActivePartsKeeper->checkStrikePoint(pHitInfo, checkInfo);
    mPartsFilter = nullptr;
    mTriFilter = nullptr;
    return result;
}

/**
 * Collects the collisions of a sphere.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param isCheckNear whether to only collect near hits
 * @param rMoveDir movement direction of the sphere
 * @return number of hits
 */
s32 CollisionDirector::checkStrikeSphere(const sead::Vector3f& rPos, f32 radius, bool isCheckNear,
                                         const sead::Vector3f& rMoveDir) {
    mStrikeSphereHitInfos->clear();
    SphereCheckInfo checkInfo(rPos, radius);
    checkInfo.mPartsFilter = mPartsFilter;
    checkInfo.mTriangleFilter = mTriFilter;
    s32 result = mActivePartsKeeper->checkStrikeSphere(mStrikeSphereHitInfos, checkInfo,
                                                       isCheckNear, rMoveDir);
    mPartsFilter = nullptr;
    mTriFilter = nullptr;
    return result;
}

/**
 * Collects the collisions of an arrow.
 * @param rPos start of the arrow
 * @param rDir direction and length of the arrow
 * @return number of hits
 */
s32 CollisionDirector::checkStrikeArrow(const sead::Vector3f& rPos, const sead::Vector3f& rDir) {
    mStrikeArrowHitInfos->clear();
    ArrowCheckInfo checkInfo(rPos, rDir);
    checkInfo.mPartsFilter = mPartsFilter;
    checkInfo.mTriangleFilter = mTriFilter;
    s32 result = mActivePartsKeeper->checkStrikeArrow(mStrikeArrowHitInfos, checkInfo);
    mPartsFilter = nullptr;
    mTriFilter = nullptr;
    return result;
}

/**
 * Collects the collisions of a player sphere.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @return number of hits
 */
s32 CollisionDirector::checkStrikeSphereForPlayer(const sead::Vector3f& rPos, f32 radius) {
    mStrikeSphereHitInfos->clear();
    SphereCheckInfo checkInfo(rPos, radius);
    checkInfo.mPartsFilter = mPartsFilter;
    checkInfo.mTriangleFilter = mTriFilter;
    s32 result = mActivePartsKeeper->checkStrikeSphereForPlayer(mStrikeSphereHitInfos, checkInfo);
    mPartsFilter = nullptr;
    mTriFilter = nullptr;
    return result;
}

/**
 * Collects the collisions of a disk.
 * @param rPos center of the disk
 * @param radius radius of the disk
 * @param height height of the disk
 * @param rDir normal of the disk
 * @return number of hits
 */
s32 CollisionDirector::checkStrikeDisk(const sead::Vector3f& rPos, f32 radius, f32 height,
                                       const sead::Vector3f& rDir) {
    mStrikeDiskHitInfos->clear();
    DiskHitResultBuffer* buffer = mStrikeDiskHitInfos;
    DiskCheckInfo checkInfo(rPos, radius, height, rDir);
    checkInfo.mPartsFilter = mPartsFilter;
    checkInfo.mTriangleFilter = mTriFilter;
    s32 result = mActivePartsKeeper->checkStrikeDisk(buffer, checkInfo);
    mPartsFilter = nullptr;
    mTriFilter = nullptr;
    return result;
}

/**
 * Gets an arrow hit of the last arrow check.
 * @param index hit index
 * @return the hit info
 */
ArrowHitInfo* CollisionDirector::getStrikeArrowInfo(u32 index) {
    return mStrikeArrowHitInfos->unsafeAt(index);
}

/**
 * Gets the number of hits of the last arrow check.
 * @return number of hits
 */
u32 CollisionDirector::getStrikeArrowInfoNum() const {
    return mStrikeArrowHitInfos->size();
}

/**
 * Gets a sphere hit of the last sphere check.
 * @param index hit index
 * @return the hit info
 */
SphereHitInfo* CollisionDirector::getStrikeSphereInfo(u32 index) {
    return mStrikeSphereHitInfos->unsafeAt(index);
}

/**
 * Gets the number of hits of the last sphere check.
 * @return number of hits
 */
u32 CollisionDirector::getStrikeSphereInfoNum() const {
    return mStrikeSphereHitInfos->size();
}

/**
 * Gets a disk hit of the last disk check.
 * @param index hit index
 * @return the hit info
 */
DiskHitInfo* CollisionDirector::getStrikeDiskInfo(u32 index) {
    return mStrikeDiskHitInfos->unsafeAt(index);
}

/**
 * Gets the number of hits of the last disk check.
 * @return number of hits
 */
u32 CollisionDirector::getStrikeDiskInfoNum() const {
    return mStrikeDiskHitInfos->size();
}

/**
 * Gets the shared sphere hit buffer used by colliders without their own buffer.
 * @param ppHitInfos output buffer
 * @param pNum output buffer size
 */
void CollisionDirector::getSphereHitInfoArrayForCollider(SphereHitInfo** ppHitInfos, u32* pNum) {
    *ppHitInfos = mSphereHitInfosForCollider;
    *pNum = 32;
}

/**
 * Gets the shared disk hit buffer used by colliders without their own buffer.
 * @param ppHitInfos output buffer
 * @param pNum output buffer size
 */
void CollisionDirector::getDiskHitInfoArrayForCollider(DiskHitInfo** ppHitInfos, u32* pNum) {
    *ppHitInfos = mDiskHitInfosForCollider;
    *pNum = 32;
}

/**
 * Updates the active parts keeper.
 */
void CollisionDirector::execute() {
    if (mActivePartsKeeper != nullptr) {
        mActivePartsKeeper->movement();
    }
}

/**
 * Calls a delegate for every collision part near a sphere.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param rDelegate delegate to call
 */
void CollisionDirector::searchCollisionPartsWithSphere(
    const sead::Vector3f& rPos, f32 radius, sead::IDelegate1<CollisionParts*>& rDelegate) const {
    if (mRootPartsKeeper != nullptr) {
        mRootPartsKeeper->searchWithSphere(rPos, radius, rDelegate);
    }
}

/**
 * Makes collision checks use only the given collision parts.
 * @param pPartsArray collision parts to check against
 */
void CollisionDirector::validateCollisionPartsPtrArray(sead::PtrArray<CollisionParts>* pPartsArray) {
    mPtrArrayPartsKeeper->setPartsArray(pPartsArray);
    mActivePartsKeeper = mPtrArrayPartsKeeper;
}

/**
 * Makes collision checks use the root parts keeper again.
 */
void CollisionDirector::invalidateCollisionPartsPtrArray() {
    mActivePartsKeeper = mRootPartsKeeper;
}

/**
 * Gets the collision parts used while a parts array is validated.
 * @return the parts array, or nullptr if none is validated
 */
sead::PtrArray<CollisionParts>* CollisionDirector::getCollisionPartsPtrArray() const {
    if (mActivePartsKeeper == mRootPartsKeeper) {
        return nullptr;
    }

    return mPtrArrayPartsKeeper->getPartsArray();
}

/**
 * Sets whether ball checks ignore the separate direction.
 * @param isIgnore whether to ignore the separate direction
 */
void CollisionDirector::setCheckBallIgnoreSeparateDir(bool isIgnore) {
    CollisionParts::setCheckBallIgnoreSeparateDir(isIgnore);
}

/**
 * Calls a delegate for every collision part near a sphere that passes a filter.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param rDelegate delegate to call
 * @param pFilter parts filter
 */
void CollisionDirector::searchCollisionPartsWithSphere(const sead::Vector3f& rPos, f32 radius,
                                                       sead::IDelegate1<CollisionParts*>& rDelegate,
                                                       const CollisionPartsFilterBase* pFilter) const {
    SphereCheckInfo checkInfo(rPos, radius);
    checkInfo.mPartsFilter = pFilter;

    if (mRootPartsKeeper != nullptr) {
        mRootPartsKeeper->searchWithSphere(checkInfo, rDelegate);
    }
}
}  // namespace al
