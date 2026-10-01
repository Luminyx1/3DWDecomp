#include "Project/Collision/CollisionParts.hpp"

#include "Library/Collision/CollisionCheckInfo.hpp"
#include "Library/Collision/ICollisionPartsKeeper.hpp"
#include "Library/Collision/KCollisionFunc.hpp"
#include "Library/Collision/KCollisionServer.hpp"
#include "Library/Collision/SphereInterpolator.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Project/Collision/CollisionPartsTriangle.hpp"
#include "Project/Collision/CollisionUtil.hpp"
#include "Project/Collision/HitDb.hpp"
#include "Project/Collision/TriangleFilterBase.hpp"

namespace {
using namespace al;

KCollisionServer::HitInfoBuffer sHitInfoBuffer;
bool sIsCheckBallIgnoreSeparateDir = true;

class SeparateDirTriangleFilter : public TriangleFilterBase {
public:
    SeparateDirTriangleFilter(const sead::Vector3f* pMoveDir, const TriangleFilterBase* pFilter)
        : mMoveDir(pMoveDir), mFilter(pFilter) {}

    bool isInvalidTriangle(const Triangle& rTriangle) const override;

    void setMoveDir(const sead::Vector3f* pMoveDir) { mMoveDir = pMoveDir; }

private:
    const sead::Vector3f* mMoveDir;
    const TriangleFilterBase* mFilter;
};

KCFxyz* toKCFxyz(const sead::Vector3f& rPos) {
    return static_cast<KCFxyz*>(const_cast<sead::Vector3f*>(&rPos));
}

bool isEqualMtx(const sead::Matrix34f& rA, const sead::Matrix34f& rB) {
    return rA.m[0][0] == rB.m[0][0] && rA.m[0][1] == rB.m[0][1] && rA.m[0][2] == rB.m[0][2] &&
           rA.m[0][3] == rB.m[0][3] && rA.m[1][0] == rB.m[1][0] && rA.m[1][1] == rB.m[1][1] &&
           rA.m[1][2] == rB.m[1][2] && rA.m[1][3] == rB.m[1][3] && rA.m[2][0] == rB.m[2][0] &&
           rA.m[2][1] == rB.m[2][1] && rA.m[2][2] == rB.m[2][2] && rA.m[2][3] == rB.m[2][3];
}

ICollisionPartsKeeper* getPartsKeeper(const HitSensor* pSensor) {
    return alCollisionUtil::getCollisionPartsKeeper(getSensorHost(pSensor));
}
}  // namespace

namespace al {
/**
 * Creates collision parts from KCL data.
 * @param pKcl KCL data
 * @param pAttribute attribute data
 */
CollisionParts::CollisionParts(void* pKcl, const void* pAttribute) {
    mKColServer = new KCollisionServer();
    mKColServer->initKCollisionServer(pKcl, pAttribute);
    mPrevBaseMtx.makeIdentity();
    mSyncMtx.makeIdentity();
    mBaseMtx.makeIdentity();
    mPrevBaseInvMtx.makeIdentity();
    mBaseInvMtx.makeIdentity();
    calcInvMtxScale();
}

/**
 * Calculates the scale of the inverse base matrix and its reciprocal.
 */
void CollisionParts::calcInvMtxScale() {
    calcMtxScale(&mMtxScaleVec, mBaseInvMtx);
    mMtxScale = (mMtxScaleVec.x + mMtxScaleVec.y + mMtxScaleVec.z) / 3.0f;
    mInvMtxScale = 1.0f / mMtxScale;
}

/**
 * Gets the actor owning the sensor connected to the collision parts.
 * @return the connected actor
 */
LiveActor* CollisionParts::getConnectedHost() const {
    return getSensorHost(mSensor);
}

/**
 * Initializes the matrices and the bounding sphere.
 * @param rMtx initial matrix
 */
void CollisionParts::initParts(const sead::Matrix34f& rMtx) {
    resetAllMtx(rMtx);
    sead::Vector3f scale(1.0f, 1.0f, 1.0f);
    calcMtxScale(&scale, mBaseMtx);
    mKColServer->calcFarthestVertexDistance();
    updateBoundingSphereRange(scale);
}

/**
 * Resets every matrix to a new matrix if the collision parts can move.
 * @param rMtx new matrix
 */
void CollisionParts::resetAllMtx(const sead::Matrix34f& rMtx) {
    if (!mIsMoving && !mIsUpdateMtxOneTime) {
        return;
    }

    sead::Matrix34f mtx;
    mtx = rMtx;
    makeEqualScale(&mtx);
    resetAllMtxPrivate(mtx);
    sead::Vector3f scale(1.0f, 1.0f, 1.0f);
    calcMtxScale(&scale, mBaseMtx);
    updateBoundingSphereRange(scale);
}

/**
 * Updates the bounding sphere range from a scale.
 * @param scale scale of the base matrix
 */
void CollisionParts::updateBoundingSphereRange(sead::Vector3f scale) {
    mBaseMtxScale = (scale.x + scale.y + scale.z) / 3.0f;
    mBoundingSphereRange = mBaseMtxScale * mKColServer->getFarthestVertexDistance();
}

/**
 * Validates the collision parts by user request.
 */
void CollisionParts::validateByUser() {
    if (_160) {
        return;
    }

    _160 = true;
    mIsJustValidated = isValidCollision();

    if (mIsJustValidated) {
        getPartsKeeper(mSensor)->connectToCollisionPartsList(this);
    }
}

/**
 * Invalidates the collision parts by user request.
 */
void CollisionParts::invalidateByUser() {
    if (!_160) {
        return;
    }

    _160 = false;
    mIsJustValidated = false;

    if (_161) {
        getPartsKeeper(mSensor)->disconnectToCollisionPartsList(this);
    }
}

/**
 * Validates the collision parts by system request.
 */
void CollisionParts::validateBySystem() {
    if (_161) {
        return;
    }

    _161 = true;
    mIsJustValidated = isValidCollision();

    if (mIsJustValidated && mSensor != nullptr) {
        getPartsKeeper(mSensor)->connectToCollisionPartsList(this);
    }
}

/**
 * Invalidates the collision parts by system request.
 */
void CollisionParts::invalidateBySystem() {
    if (!_161) {
        return;
    }

    bool isValidByUser = _160;
    _161 = false;
    mIsJustValidated = false;

    if (isValidByUser && mSensor != nullptr) {
        getPartsKeeper(mSensor)->disconnectToCollisionPartsList(this);
    }
}

/**
 * Called when the collision parts are added to a list.
 */
void CollisionParts::onJoinList() {}

/**
 * Makes the scale of a matrix uniform depending on the forced scale type.
 * @param pMtx matrix to modify
 * @return the resulting uniform scale
 */
f32 CollisionParts::makeEqualScale(sead::Matrix34f* pMtx) {
    sead::Vector3f scale;
    calcMtxScale(&scale, *pMtx);

    if (isNearZero(sead::Vector3f(scale.x - scale.y, scale.y - scale.z, scale.z - scale.x))) {
        return scale.x;
    }

    f32 equalScale;
    sead::Vector3f mulScale;

    if (getForceScaleType() == ForceCollisionScaleType::One) {
        equalScale = 1.0f;
        mulScale.set(1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z);
    } else if (getForceScaleType() == ForceCollisionScaleType::Average) {
        equalScale = (scale.x + scale.y + scale.z) / 3.0f;
        mulScale.set(equalScale / scale.x, equalScale / scale.y, equalScale / scale.z);
    } else {
        equalScale = 1.0f;
    }

    pMtx->scaleBases(mulScale.x, mulScale.y, mulScale.z);
    return equalScale;
}

/**
 * Sets every matrix to a matrix.
 * @param rMtx new matrix
 */
void CollisionParts::resetAllMtxPrivate(const sead::Matrix34f& rMtx) {
    mPrevBaseMtx = rMtx;
    mBaseMtx = rMtx;
    mSyncMtx = rMtx;
    mPrevBaseInvMtx.setInverse(mPrevBaseMtx);
    mBaseInvMtx = mPrevBaseInvMtx;
    calcInvMtxScale();
}

/**
 * Resets every matrix to the synchronized matrix if the collision parts can move.
 */
void CollisionParts::resetAllMtx() {
    if (!mIsMoving && !mIsUpdateMtxOneTime) {
        return;
    }

    sead::Matrix34f mtx;
    mtx = *mSyncCollisionMtx;
    makeEqualScale(&mtx);
    resetAllMtxPrivate(mtx);
    updateBoundingSphereRange();
}

/**
 * Updates the bounding sphere range from the synchronized matrix.
 */
void CollisionParts::updateBoundingSphereRange() {
    sead::Matrix34f mtx;
    mtx = *mSyncCollisionMtx;
    f32 scale = makeEqualScale(&mtx);
    mBaseMtxScale = scale;
    mBoundingSphereRange = scale * mKColServer->getFarthestVertexDistance();
}

/**
 * Resets every matrix to a matrix and forces one matrix update.
 * @param rMtx new matrix
 */
void CollisionParts::forceResetAllMtxAndSetUpdateMtxOneTime(const sead::Matrix34f& rMtx) {
    resetAllMtxPrivate(rMtx);
    mIsUpdateMtxOneTime = true;
}

/**
 * Resets every matrix to the synchronized matrix and forces one matrix update.
 */
void CollisionParts::forceResetAllMtxAndSetUpdateMtxOneTime() {
    sead::Matrix34f mtx;
    mtx = *mSyncCollisionMtx;
    makeEqualScale(&mtx);
    resetAllMtxPrivate(mtx);
    mIsUpdateMtxOneTime = true;
}

/**
 * Sets the synchronized matrix.
 * @param rMtx new matrix
 */
void CollisionParts::syncMtx(const sead::Matrix34f& rMtx) {
    mSyncMtx = rMtx;
    makeEqualScale(&mSyncMtx);
}

/**
 * Copies the synchronized collision matrix.
 */
void CollisionParts::syncMtx() {
    mSyncMtx = *mSyncCollisionMtx;
    makeEqualScale(&mSyncMtx);
}

/**
 * Moves the base matrix to the synchronized matrix.
 */
void CollisionParts::updateMtx() {
    bool isUpdate = mIsMoving || mIsUpdateMtxOneTime;
    bool isSame = isEqualMtx(mSyncMtx, mBaseMtx);

    if (!isUpdate) {
        if (isSame) {
            _154++;
        }

        return;
    }

    if (isSame) {
        _154++;
    } else {
        _154 = mIsUpdateMtxOneTime;
        updateScale();
    }

    mIsUpdateMtxOneTime = false;

    if (_154 > 1) {
        return;
    }

    mPrevBaseMtx = mBaseMtx;
    mBaseMtx = mSyncMtx;
    mPrevBaseInvMtx = mBaseInvMtx;
    mBaseInvMtx.setInverse(mBaseMtx);
    calcInvMtxScale();
}

/**
 * Updates the scale and the bounding sphere range from the synchronized matrix.
 */
void CollisionParts::updateScale() {
    f32 scale = makeEqualScale(&mSyncMtx);
    f32 diff = scale - mBaseMtxScale;
    mEqualScale.set(scale, scale, scale);

    if (!isNearZero(diff)) {
        mBaseMtxScale = scale;
        mBoundingSphereRange = scale * mKColServer->getFarthestVertexDistance();
    }
}

/**
 * Updates the bounding sphere range from a scale.
 * @param scale scale of the base matrix
 */
void CollisionParts::updateBoundingSphereRangePrivate(f32 scale) {
    mBaseMtxScale = scale;
    mBoundingSphereRange = mKColServer->getFarthestVertexDistance() * scale;
}

/**
 * Checks whether a sphere overlaps the bounding sphere.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @return true if the spheres overlap
 */
bool CollisionParts::checkBoundingSphereRange(const sead::Vector3f& rPos, f32 radius) {
    sead::Vector3f trans;
    mBaseMtx.getTranslation(trans);
    sead::Vector3f diff = rPos - trans;
    f32 range = mBoundingSphereRange + radius;
    return !(range * range < diff.squaredLength());
}

/**
 * Checks whether a point is inside the collision.
 * @param pHitInfo output hit info, may be null
 * @param rPos point
 * @param pFilter triangle filter, may be null
 * @return true if the point is inside the collision
 */
s32 CollisionParts::checkStrikePoint(HitInfo* pHitInfo, const sead::Vector3f& rPos,
                                     const TriangleFilterBase* pFilter) const {
    sead::Vector3f localPos;
    localPos.setMul(mBaseInvMtx, rPos);
    sead::Vector3f scale;
    calcMtxScale(&scale, mBaseInvMtx);
    f32 avgScale = (scale.x + scale.y + scale.z) / 3.0f;
    KCFxyz kcPos;
    kcPos.set(localPos);
    f32 dist;
    const KCPrismData* data = mKColServer->checkPoint(&kcPos, avgScale, &dist);

    if (data == nullptr) {
        return false;
    }

    KCPrismData* prism = const_cast<KCPrismData*>(data);

    if (pFilter != nullptr) {
        Triangle triangle;
        triangle.fillData(*this, prism, nullptr);

        if (pFilter->isInvalidTriangle(triangle)) {
            return false;
        }
    }

    if (pHitInfo != nullptr) {
        pHitInfo->mTriangle.fillData(*this, prism, nullptr);
        pHitInfo->_70 = dist / avgScale;
        pHitInfo->mPos = rPos + *pHitInfo->mTriangle.getNormal(0) * pHitInfo->_70;
    }

    return true;
}

/**
 * Collects the collisions of a sphere, interpolating the movement of the collision parts.
 * @param pBuffer output hit buffer
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param isCheckNear whether to interpolate the movement of the collision parts
 * @param rMoveDir movement direction of the sphere
 * @param pFilter triangle filter, may be null
 * @return the number of hits
 */
s32 CollisionParts::checkStrikeSphere(SphereHitResultBuffer* pBuffer, const sead::Vector3f& rPos,
                                      f32 radius, bool isCheckNear,
                                      const sead::Vector3f& rMoveDir,
                                      const TriangleFilterBase* pFilter) const {
    sead::Vector3f localPos;
    localPos.setMul(mBaseInvMtx, rPos);
    f32 localRadius = mMtxScale * radius;
    sead::Vector3f moveVec(0.0f, 0.0f, 0.0f);

    if (!isCheckNear || _154 != 0) {
        return checkStrikeSphereCore(pBuffer, rPos, localPos, sead::Vector3f(0.0f, 0.0f, 0.0f),
                                     localRadius, pFilter);
    }

    sead::Vector3f prevLocalPos;
    prevLocalPos.setMul(mPrevBaseInvMtx, rPos);
    sead::Vector3f localMove = localPos - prevLocalPos;
    sead::Vector3f moveDir;
    moveDir.setRotated(mBaseMtx, localMove);
    moveDir += rMoveDir;

    SphereInterpolator interp;
    interp.startInterp(prevLocalPos, localPos, localRadius, localRadius,
                       sead::Mathf::clampMax(localRadius * 0.9f, 35.0f));
    SeparateDirTriangleFilter filter(&moveDir, pFilter);

    while (!interp.isEnd()) {
        sead::Vector3f pos;
        sead::Vector3f remain;
        f32 size;
        interp.calcInterp(&pos, &size, &remain);
        moveVec.setRotated(mBaseMtx, -remain);
        filter.setMoveDir(interp.getCurrentStep() >= 1.0f ? nullptr : &moveDir);
        s32 hitNum = checkStrikeSphereCore(pBuffer, rPos, pos, moveVec, size, &filter);

        if (hitNum != 0) {
            return hitNum;
        }

        interp.nextStep();
    }

    return 0;
}

/**
 * Collects the collisions of a sphere in local space.
 * @param pBuffer output hit buffer
 * @param rPos center of the sphere in world space
 * @param rLocalPos center of the sphere in local space
 * @param rMoveVec movement of the collision parts
 * @param radius radius of the sphere in local space
 * @param pFilter triangle filter, may be null
 * @return the number of hits
 */
s32 CollisionParts::checkStrikeSphereCore(SphereHitResultBuffer* pBuffer,
                                          const sead::Vector3f& rPos,
                                          const sead::Vector3f& rLocalPos,
                                          const sead::Vector3f& rMoveVec, f32 radius,
                                          const TriangleFilterBase* pFilter) const {
    s32 restNum = pBuffer->capacity() - pBuffer->size();
    sHitInfoBuffer.clear();
    mKColServer->checkSphere(toKCFxyz(rLocalPos), radius, mMtxScale,
                             restNum, &sHitInfoBuffer);
    u32 num = sHitInfoBuffer.size();
    s32 hitNum = 0;

    for (u32 i = 0; i < num; i++) {
        const KCHitInfo* hit = sHitInfoBuffer.get(i);
        sead::Vector3f hitPos;
        alKCollisionFunc::calcSphereHitPos(&hitPos, mKColServer, rLocalPos, *hit->mData,
                                           hit->mHeader, hit->mCollisionLocation);
        SphereHitInfo* info = pBuffer->emplaceBack();
        info->_80 = rPos;
        info->mPos.setMul(mBaseMtx, hitPos);
        info->mTriangle.fillData(*this, const_cast<KCPrismData*>(hit->mData), hit->mHeader);

        if (pFilter != nullptr && pFilter->isInvalidTriangle(info->mTriangle)) {
            pBuffer->erase(pBuffer->size() - 1);
            continue;
        }

        info->_70 = hit->mDist * mInvMtxScale;
        info->mMovingReaction = rMoveVec;
        info->mCollisionLocation = static_cast<CollisionLocation>(hit->mCollisionLocation);
        parallelizeVec(&info->mMovingReaction, *info->mTriangle.getFaceNormal(),
                       info->mMovingReaction);
        hitNum++;
    }

    return hitNum;
}

/**
 * Collects the collisions of an arrow.
 * @param pBuffer output hit buffer
 * @param rPos start of the arrow
 * @param rDir direction and length of the arrow
 * @param pFilter triangle filter, may be null
 * @return the number of hits
 */
s32 CollisionParts::checkStrikeArrow(ArrowHitResultBuffer* pBuffer, const sead::Vector3f& rPos,
                                     const sead::Vector3f& rDir,
                                     const TriangleFilterBase* pFilter) const {
    f32 length = rDir.length();
    sead::Vector3f localPos;
    localPos.setMul(mBaseInvMtx, rPos);
    sead::Vector3f localDir = mBaseInvMtx * (rPos + rDir) - localPos;
    s32 restNum = pBuffer->capacity() - pBuffer->size();
    sHitInfoBuffer.clear();
    u32 arrowHitNum = 0;
    mKColServer->checkArrow(localPos, localDir, &sHitInfoBuffer, &arrowHitNum,
                            restNum);
    u32 num = sHitInfoBuffer.size();
    s32 hitNum = 0;

    for (u32 i = 0; i < num; i++) {
        ArrowHitInfo* info = pBuffer->emplaceBack();
        const KCHitInfo* hit = sHitInfoBuffer.get(i);
        sead::Vector3f localHitPos;
        localHitPos.setScaleAdd(hit->mDist, localDir, localPos);
        sead::Vector3f hitPos = mBaseMtx * localHitPos;
        info->mTriangle.fillData(*this, const_cast<KCPrismData*>(hit->mData), hit->mHeader);

        if (pFilter != nullptr && pFilter->isInvalidTriangle(info->mTriangle)) {
            pBuffer->erase(pBuffer->size() - 1);
            continue;
        }

        info->_70 = length * hit->mDist;
        info->mPos = hitPos;
        info->mCollisionLocation = static_cast<CollisionLocation>(hit->mCollisionLocation);
        hitNum++;
    }

    return hitNum;
}

/**
 * Collects the collisions of a player sphere, interpolating the movement of the collision parts.
 * @param pBuffer output hit buffer
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param pFilter triangle filter, may be null
 * @return the number of hits
 */
s32 CollisionParts::checkStrikeSphereForPlayer(SphereHitResultBuffer* pBuffer,
                                               const sead::Vector3f& rPos, f32 radius,
                                               const TriangleFilterBase* pFilter) const {
    sead::Vector3f localPos;
    localPos.setMul(mBaseInvMtx, rPos);
    f32 localRadius = mMtxScale * radius;
    sead::Vector3f moveVec(0.0f, 0.0f, 0.0f);
    sead::Vector3f unused;

    if (_154 != 0) {
        return checkStrikeSphereForPlayerCore(pBuffer, rPos, localPos, moveVec, unused,
                                              localRadius, pFilter);
    }

    sead::Vector3f prevLocalPos;
    prevLocalPos.setMul(mPrevBaseInvMtx, rPos);
    sead::Vector3f localMove = localPos - prevLocalPos;
    sead::Vector3f moveDir;
    moveDir.setRotated(mBaseMtx, localMove);

    SphereInterpolator interp;
    interp.startInterp(prevLocalPos, localPos, localRadius, localRadius,
                       sead::Mathf::clampMax(localRadius * 0.9f, 35.0f));
    SeparateDirTriangleFilter filter(&moveDir, pFilter);

    while (!interp.isEnd()) {
        sead::Vector3f pos;
        f32 size;
        sead::Vector3f remain;
        interp.calcInterp(&pos, &size, &remain);
        moveVec.setRotated(mBaseMtx, -remain);
        filter.setMoveDir(nullptr);
        s32 hitNum =
            checkStrikeSphereForPlayerCore(pBuffer, rPos, pos, moveVec, unused, size, &filter);

        if (hitNum != 0) {
            return hitNum;
        }

        interp.nextStep();
    }

    return 0;
}

/**
 * Collects the collisions of a player sphere in local space.
 * @param pBuffer output hit buffer
 * @param rPos center of the sphere in world space
 * @param rLocalPos center of the sphere in local space
 * @param rMoveVec movement of the collision parts
 * @param rUnused unused
 * @param radius radius of the sphere in local space
 * @param pFilter triangle filter, may be null
 * @return the number of hits
 */
s32 CollisionParts::checkStrikeSphereForPlayerCore(SphereHitResultBuffer* pBuffer,
                                                   const sead::Vector3f& rPos,
                                                   const sead::Vector3f& rLocalPos,
                                                   const sead::Vector3f& rMoveVec,
                                                   const sead::Vector3f& rUnused, f32 radius,
                                                   const TriangleFilterBase* pFilter) const {
    s32 restNum = pBuffer->capacity() - pBuffer->size();
    sHitInfoBuffer.clear();
    mKColServer->checkSphereForPlayer(toKCFxyz(rLocalPos), radius,
                                      restNum, &sHitInfoBuffer);
    u32 num = sHitInfoBuffer.size();
    s32 hitNum = 0;

    for (u32 i = 0; i < num; i++) {
        const KCHitInfo* hit = sHitInfoBuffer.get(i);
        sead::Vector3f hitPos;
        alKCollisionFunc::calcSphereHitPos(&hitPos, mKColServer, rLocalPos, *hit->mData,
                                           hit->mHeader, hit->mCollisionLocation);
        SphereHitInfo* info = pBuffer->emplaceBack();
        info->_80 = rPos;
        info->mPos.setMul(mBaseMtx, hitPos);
        info->mTriangle.fillData(*this, const_cast<KCPrismData*>(hit->mData), hit->mHeader);

        if (pFilter != nullptr && pFilter->isInvalidTriangle(info->mTriangle)) {
            pBuffer->erase(pBuffer->size() - 1);
            continue;
        }

        info->_70 = hit->mDist * mInvMtxScale;
        info->mMovingReaction = rMoveVec;
        info->mCollisionLocation = static_cast<CollisionLocation>(hit->mCollisionLocation);
        parallelizeVec(&info->mMovingReaction, *info->mTriangle.getFaceNormal(),
                       info->mMovingReaction);
        hitNum++;
    }

    return hitNum;
}

/**
 * Collects the collisions of a disk.
 * @param pBuffer output hit buffer
 * @param rPos center of the disk
 * @param radius radius of the disk
 * @param height height of the disk
 * @param rDir normal of the disk
 * @param pFilter triangle filter, may be null
 * @return the number of hits
 */
s32 CollisionParts::checkStrikeDisk(DiskHitResultBuffer* pBuffer, const sead::Vector3f& rPos,
                                    f32 radius, f32 height, const sead::Vector3f& rDir,
                                    const TriangleFilterBase* pFilter) const {
    sead::Vector3f localPos;
    localPos.setMul(mBaseInvMtx, rPos);
    sead::Vector3f localDir;
    localDir.setRotated(mBaseInvMtx, rDir);
    normalizeOrZero(&localDir);
    f32 localRadius = mMtxScale * radius;
    f32 localHeight = mMtxScale * height;
    sead::Vector3f moveVec(0.0f, 0.0f, 0.0f);
    return checkStrikeDiskCore(pBuffer, rPos, localPos, moveVec, localRadius, localHeight,
                               localDir, pFilter);
}

/**
 * Collects the collisions of a disk in local space.
 * @param pBuffer output hit buffer
 * @param rPos center of the disk in world space
 * @param rLocalPos center of the disk in local space
 * @param rMoveVec movement of the collision parts
 * @param radius radius of the disk in local space
 * @param height height of the disk in local space
 * @param rLocalDir normal of the disk in local space
 * @param pFilter triangle filter, may be null
 * @return the number of hits
 */
s32 CollisionParts::checkStrikeDiskCore(DiskHitResultBuffer* pBuffer, const sead::Vector3f& rPos,
                                        const sead::Vector3f& rLocalPos,
                                        const sead::Vector3f& rMoveVec, f32 radius, f32 height,
                                        const sead::Vector3f& rLocalDir,
                                        const TriangleFilterBase* pFilter) const {
    s32 restNum = pBuffer->capacity() - pBuffer->size();
    sHitInfoBuffer.clear();
    mKColServer->checkDisk(toKCFxyz(rLocalPos), radius, height, rLocalDir, mMtxScale,
                           restNum, &sHitInfoBuffer);
    u32 num = sHitInfoBuffer.size();
    s32 hitNum = 0;

    for (u32 i = 0; i < num; i++) {
        const KCHitInfo* hit = sHitInfoBuffer.get(i);
        sead::Vector3f hitPos;
        alKCollisionFunc::calcDiskHitPos(&hitPos, mKColServer, rLocalPos, height, rLocalDir,
                                         *hit->mData, hit->mHeader, hit->mCollisionLocation);
        DiskHitInfo* info = pBuffer->emplaceBack();
        info->_80 = rPos;
        info->mPos.setMul(mBaseMtx, hitPos);
        info->mTriangle.fillData(*this, const_cast<KCPrismData*>(hit->mData), hit->mHeader);

        if (pFilter != nullptr && pFilter->isInvalidTriangle(info->mTriangle)) {
            pBuffer->erase(pBuffer->size() - 1);
            continue;
        }

        info->_70 = hit->mDist * mInvMtxScale;
        info->mMovingReaction = rMoveVec;
        info->mCollisionLocation = static_cast<CollisionLocation>(hit->mCollisionLocation);
        parallelizeVec(&info->mMovingReaction, *info->mTriangle.getFaceNormal(),
                       info->mMovingReaction);
        hitNum++;
    }

    return hitNum;
}

/**
 * Calculates how far a point was moved by the collision parts since the last update.
 * @param pPower output movement
 * @param rPos point
 */
void CollisionParts::calcForceMovePower(sead::Vector3f* pPower, const sead::Vector3f& rPos) const {
    sead::Vector3f localPos = mPrevBaseInvMtx * rPos;
    sead::Vector3f pos = mBaseMtx * localPos;
    pPower->setSub(pos, rPos);
}

/**
 * Calculates how far the collision parts rotated since the last update.
 * @param pPower output rotation
 */
void CollisionParts::calcForceRotatePower(sead::Quatf* pPower) const {
    sead::Quatf prevQuat;
    mPrevBaseInvMtx.toQuat(prevQuat);
    sead::Quatf quat;
    mBaseMtx.toQuat(quat);
    pPower->setMul(prevQuat, quat);
}

/**
 * Sets whether sphere checks ignore polygons facing away from the movement.
 * @param isIgnore whether to ignore them
 */
void CollisionParts::setCheckBallIgnoreSeparateDir(bool isIgnore) {
    sIsCheckBallIgnoreSeparateDir = isIgnore;
}
}  // namespace al

namespace {
/**
 * Checks whether a triangle is filtered out or faces away from the movement direction.
 * @param rTriangle triangle
 * @return true if the triangle is filtered out
 */
bool SeparateDirTriangleFilter::isInvalidTriangle(const Triangle& rTriangle) const {
    if (mFilter != nullptr && mFilter->isInvalidTriangle(rTriangle)) {
        return true;
    }

    if (sIsCheckBallIgnoreSeparateDir && mMoveDir != nullptr &&
        mMoveDir->dot(*rTriangle.getFaceNormal()) > 0.0f) {
        return true;
    }

    return false;
}
}  // namespace
