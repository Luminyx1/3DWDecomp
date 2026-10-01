#include "Library/Collision/Collider.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Collision/CollisionDirector.hpp"
#include "Library/Collision/SphereInterpolator.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace al {
namespace {
const sead::Vector3f cUpDir = {0.0f, 1.0f, 0.0f};

inline void debugFirstReaction(Collider* pCollider) {}

inline void updateMinMax(f32* pMax, f32* pMin, f32 value) {
    if (*pMax < value) {
        *pMax = value;
    } else if (value < *pMin) {
        *pMin = value;
    }
}

inline void updateMinMax(sead::Vector3f* pMax, sead::Vector3f* pMin, const sead::Vector3f& rValue) {
    updateMinMax(&pMax->x, &pMin->x, rValue.x);
    updateMinMax(&pMax->y, &pMin->y, rValue.y);
    updateMinMax(&pMax->z, &pMin->z, rValue.z);
}
}  // namespace

/**
 * Creates a sphere collider.
 * @param pDirector collision director
 * @param pBaseMtx base matrix of the owner, or nullptr
 * @param pTrans position of the owner
 * @param pGravity gravity direction of the owner
 * @param radius radius of the sphere
 * @param offsetY vertical offset of the sphere center
 * @param planeNum size of the own hit buffer, 0 to use the director's
 */
Collider::Collider(CollisionDirector* pDirector, const sead::Matrix34f* pBaseMtx,
                   const sead::Vector3f* pTrans, const sead::Vector3f* pGravity, f32 radius,
                   f32 offsetY, u32 planeNum)
    : mCollisionDirector(pDirector), mBaseMtx(pBaseMtx), mTrans(pTrans), mGravity(pGravity),
      mRadius(radius), mOffsetY(offsetY), mPlaneNum(planeNum), mCurrentTrans(*pTrans),
      mCurrentRadius(radius) {
    if (mPlaneNum == 0) {
        mPlanes = nullptr;
    } else {
        mPlanes = new HitInfo[mPlaneNum];
    }

    clear();
    mIsReactMovePower = true;
    mIsCheckMovingReaction = true;
    mIsRotateCheckOffset = false;
    mIsCollidedFloorFace = false;
    mIsCollidedWallFace = false;
    mIsCollidedCeilingFace = false;
}

/**
 * Clears the stored planes and the contact state.
 */
void Collider::clear() {
    mStoredPlaneNum = 0;
    clearContactPlane();
}

/**
 * Sets the triangle filter used for collision checks.
 * @param pFilter triangle filter
 */
void Collider::setTriangleFilter(const TriangleFilterBase* pFilter) {
    mTriFilterBase = pFilter;
}

/**
 * Sets the collision parts filter used for collision checks.
 * @param pFilter collision parts filter
 */
void Collider::setCollisionPartsFilter(const CollisionPartsFilterBase* pFilter) {
    mColFilterBase = pFilter;
}

/**
 * Updates the last ground normal and the number of frames without ground.
 */
void Collider::updateRecentOnGroundInfo() {
    if (!isCollidedFloor()) {
        if (_264 != 0xffffffff) {
            _264++;
        }

        return;
    }

    _264 = 0;
    mRecentOnGroundNormal = *mFloor.mTriangle.getFaceNormal();
}

/**
 * Clears the number of stored planes.
 */
void Collider::clearStoredPlaneNum() {
    mStoredPlaneNum = 0;
}

/**
 * Clears the floor, wall and ceiling contacts.
 */
void Collider::clearContactPlane() {
    _110 = -99999.0f;
    _1b8 = -99999.0f;
    _260 = -99999.0f;
    mFixReaction = {0.0f, 0.0f, 0.0f};
    mMovePower = {0.0f, 0.0f, 0.0f};
}

/**
 * Resets the collider to the current position of the owner.
 */
void Collider::onInvalidate() {
    clear();
    _264 = 0xffffffff;
    calcCheckPos(&mCurrentTrans);
    mCurrentRadius = mRadius;
}

/**
 * Calculates the center of the collision sphere.
 * @param pPos output position
 */
void Collider::calcCheckPos(sead::Vector3f* pPos) const {
    *pPos = *mTrans;

    if (mCheckOffset != nullptr) {
        if (mIsRotateCheckOffset && mBaseMtx != nullptr) {
            pPos->x += mBaseMtx->m[0][0] * mCheckOffset->x;
            pPos->x += mBaseMtx->m[0][1] * mCheckOffset->y;
            pPos->x += mBaseMtx->m[0][2] * mCheckOffset->z;
            pPos->y += mBaseMtx->m[1][0] * mCheckOffset->x;
            pPos->y += mBaseMtx->m[1][1] * mCheckOffset->y;
            pPos->y += mBaseMtx->m[1][2] * mCheckOffset->z;
            pPos->z += mBaseMtx->m[2][0] * mCheckOffset->x;
            pPos->z += mBaseMtx->m[2][1] * mCheckOffset->y;
            pPos->z += mBaseMtx->m[2][2] * mCheckOffset->z;
        } else {
            pPos->x += mCheckOffset->x;
            pPos->y += mCheckOffset->y;
            pPos->z += mCheckOffset->z;
        }
    } else if (mBaseMtx != nullptr) {
        pPos->x += mBaseMtx->m[0][1] * mOffsetY;
        pPos->y += mBaseMtx->m[1][1] * mOffsetY;
        pPos->z += mBaseMtx->m[2][1] * mOffsetY;
    } else {
        pPos->y += mOffsetY;
    }
}

/**
 * Gets the normal of the ground, or of the last ground within a number of frames.
 * @param index number of frames the last ground normal stays valid
 * @return the ground normal
 */
const sead::Vector3f& Collider::getRecentOnGroundNormal(u32 index) const {
    if (isCollidedFloor()) {
        return *mFloor.mTriangle.getFaceNormal();
    }

    return _264 > index ? cUpDir : mRecentOnGroundNormal;
}

/**
 * Gets a stored plane.
 * @param index plane index
 * @return the plane
 */
HitInfo* Collider::getPlane(s32 index) const {
    return &mPlanes[index];
}

/**
 * Calculates the movement given by a moving floor.
 * @param pMovePower output movement
 * @param rCheckPos current sphere center
 * @return whether the floor is moving
 */
bool Collider::calcMovePowerByContact(sead::Vector3f* pMovePower, const sead::Vector3f& rCheckPos) {
    if (_110 < 0.0f) {
        return false;
    }

    if (!mFloor.mTriangle.isHostMoved()) {
        return false;
    }

    mFloor.mTriangle.calcForceMovePower(pMovePower, rCheckPos);
    const sead::Vector3f* normal = mFloor.mTriangle.getFaceNormal();

    if (normal->dot(*pMovePower) > 0.0f) {
        verticalizeVec(pMovePower, *normal, *pMovePower);
    }

    return true;
}

/**
 * Copies the hits of the last sphere check into a buffer.
 * @param pHitInfos output buffer
 * @param maxNum size of the buffer
 * @return number of copied hits
 */
u32 Collider::storeCurrentHitInfo(SphereHitInfo* pHitInfos, u32 maxNum) {
    u32 hitNum = alCollisionUtil::getStrikeSphereInfoNum(this);
    u32 storedNum = mStoredPlaneNum;
    u32 i = 0;

    for (; i < hitNum; i++) {
        if (storedNum + i >= maxNum) {
            i = maxNum - storedNum;
            break;
        }

        pHitInfos[mStoredPlaneNum + i] = *alCollisionUtil::getStrikeSphereInfo(this, i);
        storedNum = mStoredPlaneNum;
    }

    mStoredPlaneNum = storedNum + i;
    return i;
}

/**
 * Calculates the reaction that pushes the sphere out of the stored planes.
 * @param pHitInfos stored planes
 * @param pFixReaction output reaction
 * @param pFixNormalReaction output reaction along the plane normals, or nullptr
 * @param isFirst whether this is the first reaction of the frame
 * @param startIndex first plane to use
 */
void Collider::obtainMomentFixReaction(SphereHitInfo* pHitInfos, sead::Vector3f* pFixReaction,
                                       sead::Vector3f* pFixNormalReaction, bool isFirst,
                                       u32 startIndex) {
    if (isFirst) {
        debugFirstReaction(this);
    }

    mIsCollidedFloorFace = false;
    mIsCollidedWallFace = false;
    mIsCollidedCeilingFace = false;

    for (u32 i = startIndex; i < mStoredPlaneNum; i++) {
        const SphereHitInfo& hitInfo = pHitInfos[i];
        const sead::Vector3f* normal = hitInfo.mTriangle.getFaceNormal();

        if (isFloorPolygon(*normal, *mGravity)) {
            if (hitInfo.isCollisionAtFace()) {
                mIsCollidedFloorFace = true;
            }
        } else if (isWallPolygon(*normal, *mGravity)) {
            if (hitInfo.isCollisionAtFace()) {
                mIsCollidedWallFace = true;
            }
        } else if (hitInfo.isCollisionAtFace()) {
            mIsCollidedCeilingFace = true;
        }
    }

    sead::Vector3f minFix = {0.0f, 0.0f, 0.0f};
    sead::Vector3f maxFix = {0.0f, 0.0f, 0.0f};
    sead::Vector3f maxFixNormal = {0.0f, 0.0f, 0.0f};
    sead::Vector3f minFixNormal = {0.0f, 0.0f, 0.0f};

    for (u32 i = startIndex; i < mStoredPlaneNum; i++) {
        const SphereHitInfo& hitInfo = pHitInfos[i];
        const sead::Vector3f* normal = hitInfo.mTriangle.getFaceNormal();
        sead::Vector3f fix;
        sead::Vector3f fixNormal;

        if (isFloorPolygon(*normal, *mGravity)) {
            if (mIsCollidedFloorFace) {
                hitInfo.calcFixVectorNormal(&fix, &fixNormal);
            } else {
                hitInfo.calcFixVector(&fix, &fixNormal);
            }
        } else if (isWallPolygon(*normal, *mGravity)) {
            if (mIsCollidedWallFace) {
                hitInfo.calcFixVectorNormal(&fix, &fixNormal);
            } else {
                hitInfo.calcFixVector(&fix, &fixNormal);
            }
        } else if (!mIsCollidedCeilingFace) {
            hitInfo.calcFixVector(&fix, &fixNormal);
        } else {
            hitInfo.calcFixVectorNormal(&fix, &fixNormal);
        }

        updateMinMax(&maxFix, &minFix, fix);

        if (pFixNormalReaction != nullptr) {
            updateMinMax(&maxFixNormal, &minFixNormal, fixNormal);
        }

        if (mIsCheckMovingReaction && hitInfo.mTriangle.isHostMoved() &&
            !(fix.dot(hitInfo.mMovingReaction) < 0.0f)) {
            updateMinMax(&maxFix, &minFix, hitInfo.mMovingReaction);

            if (pFixNormalReaction != nullptr) {
                updateMinMax(&maxFixNormal, &minFixNormal, hitInfo.mMovingReaction);
            }
        }
    }

    *pFixReaction = maxFix + minFix;

    if (pFixNormalReaction != nullptr) {
        *pFixNormalReaction = maxFixNormal + minFixNormal;
    }
}

/**
 * Stores the closest floor, wall and ceiling of the stored planes as contacts.
 * @param pHitInfos stored planes
 */
void Collider::storeContactPlane(SphereHitInfo* pHitInfos) {
    for (u32 i = 0; i < mStoredPlaneNum; i++) {
        const SphereHitInfo& hitInfo = pHitInfos[i];
        const sead::Vector3f* normal = hitInfo.mTriangle.getNormal(0);

        if (isFloorPolygon(*normal, *mGravity)) {
            if (_110 < hitInfo._70) {
                mFloor = hitInfo;
                _110 = hitInfo._70;
            }
        } else if (isWallPolygon(*normal, *mGravity)) {
            if (_1b8 < hitInfo._70) {
                mWall = hitInfo;
                _1b8 = hitInfo._70;
            }
        } else if (_260 < hitInfo._70) {
            mCeiling = hitInfo;
            _260 = hitInfo._70;
        }
    }
}

/**
 * Moves the sphere and resolves collisions.
 * @param rMove movement of the owner
 * @return the movement after collision
 */
sead::Vector3f Collider::collide(const sead::Vector3f& rMove) {
    u32 hitInfoNum;
    f32 radius;
    SphereHitInfo* hitInfos;

    if (mPlaneNum != 0) {
        hitInfos = static_cast<SphereHitInfo*>(mPlanes);
        hitInfoNum = mPlaneNum;
    } else {
        getCollisionDirector()->getSphereHitInfoArrayForCollider(&hitInfos, &hitInfoNum);
    }

    sead::Vector3f checkPos = {0.0f, 0.0f, 0.0f};
    calcCheckPos(&checkPos);
    sead::Vector3f pos = mCurrentTrans;
    radius = mCurrentRadius;
    f32 step = sead::Mathf::clampMax(sead::Mathf::min(mRadius * 0.9f, mCurrentRadius * 0.9f), 35.0f);
    sead::Vector3f movePower = {0.0f, 0.0f, 0.0f};

    if (mIsReactMovePower) {
        calcMovePowerByContact(&movePower, checkPos);
    }

    clear();
    mMovePower = movePower;

    SphereInterpolator interp;
    interp.startInterp(mCurrentTrans, checkPos + movePower, mCurrentRadius, mRadius, step);
    s32 hitNum = 0;

    sead::Vector3f preMove = checkPos + movePower - mCurrentTrans;

    if (!isNearZero(preMove, 0.001f) || !isNearZero(mCurrentRadius - mRadius, 0.001f)) {
        preCollide(&interp, &pos, &radius, preMove, hitInfos, hitInfoNum);
    }

    step = sead::Mathf::clampMax(mCurrentRadius * 0.9f, 35.0f);
    interp.startInterp(pos, pos + rMove, mRadius, mRadius, step);
    hitNum = 0;

    sead::Vector3f result;

    if (!findCollidePos(&hitNum, &interp, hitInfos, hitInfoNum) && interp.isEnd()) {
        result = pos - checkPos + rMove;
    } else {
        sead::Vector3f fixReaction = {0.0f, 0.0f, 0.0f};
        u32 startIndex = 0;
        bool isFirst = true;
        bool isContinue;

        do {
            sead::Vector3f fix = {0.0f, 0.0f, 0.0f};
            sead::Vector3f fixNormal = {0.0f, 0.0f, 0.0f};
            obtainMomentFixReaction(hitInfos, &fix, &fixNormal, isFirst, startIndex);
            isFirst = false;
            startIndex += hitNum;

            sead::Vector3f remainMove;
            interp.calcInterp(&pos, &radius, &remainMove);
            pos += fix;
            fixReaction += fix;
            sead::Vector3f fixDir = fix;

            if (isNearZero(fixDir, 0.001f)) {
                fixDir.set(fixNormal);
            }

            normalizeOrZero(&fixDir);
            f32 dot = remainMove.dot(fixDir);

            if (dot < 0.0f) {
                remainMove.setScaleAdd(-dot, fixDir, remainMove);
            }

            isContinue = false;

            if (!(rMove.dot(remainMove) < 0.0f)) {
                interp.startInterp(pos, pos + remainMove, radius, mRadius, step);
                interp.nextStep();
                hitNum = 0;
                bool isHit = findCollidePos(&hitNum, &interp, hitInfos, hitInfoNum);

                if (isHit && !(interp.getCurrentStep() >= 1.0f)) {
                    isContinue = true;
                } else {
                    interp.calcInterpPos(&pos);

                    if (isHit && hitNum >= 1) {
                        obtainMomentFixReaction(hitInfos, &fix, nullptr, false, startIndex);
                        pos += fix;
                        startIndex += hitNum;
                    }
                }
            }
        } while (isContinue);

        storeContactPlane(hitInfos);
        mFixReaction = fixReaction;
        result = pos - checkPos;
    }

    mCurrentTrans.setAdd(checkPos, result);
    mCurrentRadius = mRadius;
    updateRecentOnGroundInfo();
    return result;
}

/**
 * Moves the sphere from its last position to the current position of the owner.
 * @param pInterp interpolator set up for the movement
 * @param pPos output sphere center
 * @param pRadius output sphere radius
 * @param rMove movement of the sphere
 * @param pHitInfos hit buffer
 * @param maxNum size of the hit buffer
 * @return whether anything was hit
 */
bool Collider::preCollide(SphereInterpolator* pInterp, sead::Vector3f* pPos, f32* pRadius,
                          const sead::Vector3f& rMove, SphereHitInfo* pHitInfos, u32 maxNum) {
    const TriangleFilterBase* triFilter = mTriFilterBase;
    const CollisionPartsFilterBase* partsFilter = mColFilterBase;
    sead::Vector3f totalFix = {0.0f, 0.0f, 0.0f};
    bool isHit = false;

    while (!pInterp->isEnd()) {
        sead::Vector3f pos;
        f32 radius;
        pInterp->calcInterp(&pos, &radius, nullptr);
        s32 hitNum;

        if (mIsCheckMovingReaction) {
            hitNum = alCollisionUtil::checkStrikeSphereMovingReaction(
                this, totalFix + pos, radius, rMove, partsFilter, triFilter);
        } else {
            hitNum = alCollisionUtil::checkStrikeSphere(this, pos, radius, partsFilter, triFilter);
        }

        if (hitNum != 0) {
            storeCurrentHitInfo(pHitInfos, maxNum);
            sead::Vector3f fix = {0.0f, 0.0f, 0.0f};
            obtainMomentFixReaction(pHitInfos, &fix, nullptr, false, 0);
            totalFix += fix;
            isHit = true;

            if (pInterp->getCurrentStep() >= 1.0f) {
                break;
            }
        }

        pInterp->nextStep();
    }

    pInterp->calcInterp(pPos, pRadius, nullptr);
    *pPos += totalFix;

    if (isHit) {
        storeContactPlane(pHitInfos);
        clearStoredPlaneNum();
    }

    return isHit;
}

/**
 * Advances an interpolation until the sphere hits something.
 * @param pHitNum output number of stored hits, or nullptr
 * @param pInterp interpolator set up for the movement
 * @param pHitInfos hit buffer
 * @param maxNum size of the hit buffer
 * @return whether anything was hit
 */
bool Collider::findCollidePos(s32* pHitNum, SphereInterpolator* pInterp,
                              SphereHitInfo* pHitInfos, u32 maxNum) {
    const TriangleFilterBase* triFilter = mTriFilterBase;
    const CollisionPartsFilterBase* partsFilter = mColFilterBase;

    while (!pInterp->isEnd()) {
        sead::Vector3f pos;
        sead::Vector3f remainMove;
        f32 radius;
        pInterp->calcInterp(&pos, &radius, &remainMove);
        s32 hitNum;

        if (mIsCheckMovingReaction) {
            hitNum = alCollisionUtil::checkStrikeSphereMovingReaction(
                this, pos, radius, remainMove, partsFilter, triFilter);
        } else {
            hitNum = alCollisionUtil::checkStrikeSphere(this, pos, radius, partsFilter, triFilter);
        }

        if (hitNum != 0) {
            u32 storedNum = storeCurrentHitInfo(pHitInfos, maxNum);

            if (pHitNum != nullptr) {
                *pHitNum = storedNum;
            }

            return true;
        }

        pInterp->nextStep();
    }

    return false;
}

/**
 * Gets the collision director.
 * @return the collision director
 */
CollisionDirector* Collider::getCollisionDirector() const {
    return mCollisionDirector;
}
}  // namespace al
