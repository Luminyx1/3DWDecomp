#include "Library/Play/Camera/CameraArrowCollider.hpp"

#include <math/seadMathCalcCommon.h>
#include <prim/seadDelegate.h>

#include "Library/Camera/CameraTriangleFilter.hpp"
#include "Library/Collision/CollisionCheckInfo.hpp"
#include "Library/Collision/CollisionPartsKeeperUtil.hpp"
#include "Library/Collision/ICollisionPartsKeeper.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Project/Camera/CameraCollisionPartsFilter.hpp"
#include "Project/Collision/CollisionUtil.hpp"
#include "Project/Collision/HitDb.hpp"

namespace al {

/**
 * Hit result buffer of one arrow.
 */
class CameraArrowCollider::HitResultBuffer : public ArrowHitResultBuffer {
public:
    /**
     * Creates the buffer and allocates its hit infos.
     */
    HitResultBuffer() { allocBuffer(128, nullptr); }
};

}  // namespace al

namespace {
using namespace al;

NERVE_DECL(CameraArrowCollider, Keep)
NERVE_DECL(CameraArrowCollider, Shrink)

NERVES_MAKE_NOSTRUCT(CameraArrowCollider, Keep, Shrink)

const s32 cArrowNum = 4;

const sead::Vector2f sAngleRange(60.0f, 75.0f);
const CameraCollisionPartsFilter sPartsFilter;
const CameraTriangleFilter sTriangleFilter;

/**
 * Sets the length of a vector, leaving zero vectors unchanged.
 * @param pVec Vector to modify.
 * @param length New length.
 */
inline void setVecLength(sead::Vector3f* pVec, f32 length) {
    f32 curLength = pVec->length();

    if (curLength > 0.0f) {
        *pVec *= length / curLength;
    }
}

/**
 * Finds the position of the nearest hit in a hit result buffer.
 * @param pPos Position of the nearest hit, unchanged if there is no hit.
 * @param rBuffer Hit result buffer.
 */
inline void findNearestHitPos(sead::Vector3f* pPos, const ArrowHitResultBuffer& rBuffer) {
    f32 minDist = -1.0f;

    for (s32 i = 0; i < rBuffer.size(); i++) {
        const ArrowHitInfo* hitInfo = rBuffer[i];

        if (minDist < 0.0f || hitInfo->_70 < minDist) {
            pPos->set(rBuffer[i]->mPos);
            minDist = rBuffer[i]->_70;
        }
    }
}

/**
 * Checks the arrows of the camera against the collision and calculates how far the camera has
 * to be pushed towards its look at position.
 * @param pPushLength Calculated push length.
 * @param pBuffers Hit result buffers, one per arrow.
 * @param pCollider Arrow collider.
 * @param rAt Look at position.
 * @param rPos Camera position.
 * @param pArrows Arrows from the look at position.
 * @param rDir Direction from the look at position to the camera position.
 * @param isInvalidThroughPassCollision Whether pass-through collision is ignored.
 * @param minLength Length to keep in front of the hit positions.
 * @return Whether any arrow hit collision.
 */
bool calcPushLength(f32* pPushLength, ArrowHitResultBuffer* pBuffers,
                    const CameraArrowCollider* pCollider, const sead::Vector3f& rAt,
                    const sead::Vector3f& rPos, const sead::Vector3f* pArrows,
                    const sead::Vector3f& rDir, bool isInvalidThroughPassCollision,
                    f32 minLength) {
    const IUseCollision* collision = pCollider;
    bool isHit = false;

    for (s32 i = 0; i < cArrowNum; i++) {
        ArrowCheckInfo checkInfo(rAt, pArrows[i]);
        checkInfo.mPartsFilter = nullptr;
        checkInfo.mTriangleFilter = &sTriangleFilter;
        alCollisionUtil::getCollisionPartsKeeper(collision)->checkStrikeArrow(&pBuffers[i],
                                                                              checkInfo);
        isHit |= pBuffers[i].size() > 0;
    }

    if (!isHit) {
        return false;
    }

    f32 pushDist = -1.0f;

    for (s32 i = 0; i < cArrowNum; i++) {
        const ArrowHitResultBuffer& buffer = pBuffers[i];
        s32 hitNum = buffer.size();

        if (hitNum < 1) {
            continue;
        }

        sead::Vector3f hitPos = rAt + pArrows[i];
        f32 minDist = -1.0f;

        for (s32 j = 0; j < hitNum; j++) {
            if (!isCameraCode("InvalidThrough", buffer[j]->mTriangle)) {
                continue;
            }

            const ArrowHitInfo* hitInfo = buffer[j];

            if (minDist < 0.0f || hitInfo->_70 < minDist) {
                hitPos.set(buffer[j]->mPos);
                minDist = buffer[j]->_70;
            }
        }

        bool isFound = minDist > 0.0f;
        sead::Vector3f startPos = hitPos - rAt;
        setVecLength(&startPos, startPos.length() - minLength);
        startPos += rAt;

        sead::Vector3f nearestPos = {0.0f, 0.0f, 0.0f};
        findNearestHitPos(&nearestPos, buffer);

        s32 strikeNum = 0;

        if (!isInvalidThroughPassCollision &&
            (rAt - nearestPos).squaredLength() < (rAt - startPos).squaredLength()) {
            sead::Vector3f checkDir = nearestPos - startPos;
            strikeNum = alCollisionUtil::checkStrikeArrow(collision, startPos, checkDir, nullptr,
                                                          &sTriangleFilter);
        }

        if (strikeNum < 1) {
            findNearestHitPos(&hitPos, buffer);
        } else {
            s32 nearestIndex = -1;

            for (s32 j = 0; j < strikeNum; j++) {
                if (nearestIndex < 0 ||
                    alCollisionUtil::getStrikeArrowInfo(collision, j)->_70 <
                        alCollisionUtil::getStrikeArrowInfo(collision, nearestIndex)->_70) {
                    nearestIndex = j;
                }
            }

            sead::Vector3f strikePos =
                alCollisionUtil::getStrikeArrowInfo(collision, nearestIndex)->mPos;

            if ((strikePos - startPos).length() < 100.0f) {
                findNearestHitPos(&hitPos, buffer);
            } else {
                for (s32 j = 0; j < buffer.size(); j++) {
                    const ArrowHitInfo* hitInfo = buffer.unsafeAt(j);
                    f32 hitDist = hitInfo->_70 * hitInfo->_70;

                    if ((rAt - hitPos).squaredLength() < hitDist ||
                        hitDist < (rAt - strikePos).squaredLength()) {
                        continue;
                    }

                    hitPos.set(hitInfo->mPos);
                    isFound = true;
                }

                if (!isFound) {
                    continue;
                }
            }
        }

        sead::Vector3f pushVec = rAt - hitPos;
        parallelizeVec(&pushVec, rDir, pushVec);

        if (pushDist < 0.0f || pushVec.length() < pushDist) {
            pushDist = pushVec.length();
        }
    }

    if (!(pushDist > 0.0f)) {
        return false;
    }

    f32 length = (rPos - rAt).length();
    *pPushLength = sead::Mathf::clampMax(length + 50.0f - pushDist, length - 5.0f);
    return true;
}

}  // namespace

namespace al {

/**
 * Creates the arrow collider of a camera.
 * @param pDirector Collision director.
 */
CameraArrowCollider::CameraArrowCollider(CollisionDirector* pDirector)
    : NerveExecutor("カメラの線分コライダー"), mCollisionDirector(pDirector) {
    initNerve(&NrvCameraArrowColliderKeep, 0);
    mArrows = new sead::Vector3f[cArrowNum];

    for (s32 i = 0; i < cArrowNum; i++) {
        mArrows[i] = {0.0f, 0.0f, 0.0f};
    }

    mHitResultBuffers = new HitResultBuffer[cArrowNum];
}

/**
 * Resets the push length and starts colliding.
 */
void CameraArrowCollider::start() {
    mPushLength = 0.0f;
    mTargetPushLength = 0.0f;

    if (mIsInvalidThroughPassCollision) {
        return setNerve(this, &NrvCameraArrowColliderShrink);
    }

    setNerve(this, &NrvCameraArrowColliderKeep);
}

/**
 * Updates the arrows and checks them against the collision.
 * @param rPos Camera position.
 * @param rAt Look at position.
 * @param rUp Up direction of the camera.
 */
void CameraArrowCollider::update(const sead::Vector3f& rPos, const sead::Vector3f& rAt,
                                 const sead::Vector3f& rUp) {
    _688 = -1;
    mPos.set(rPos);
    mAt.set(rAt);
    mDir.set(mPos - mAt);

    if (!tryNormalizeOrZero(&mDir)) {
        return;
    }

    if (isParallelDirection(mDir, rUp, 0.01f)) {
        return;
    }

    f32 angle = sead::Mathf::rad2deg(sead::Mathf::asin(mDir.y));
    verticalizeVec(&mUp, mDir, rUp);

    if (!tryNormalizeOrZero(&mUp)) {
        mUp.set(sead::Vector3f::ey);
    }

    mSide.setCross(mDir, mUp);
    normalize(&mSide);

    f32 rate = normalize(angle, sAngleRange.x, sAngleRange.y);
    f32 sideLength = lerpValue(rate, 75.0f, 100.0f);
    f32 upLength = lerpValue(rate, 30.0f, 75.0f);
    mArrows[0].set(mPos + (upLength - 30.0f) * mUp - mAt);
    mArrows[1].set(mPos + sideLength * mSide - mAt);
    mArrows[2].set(mPos - sideLength * mSide - mAt);
    mArrows[3].set(mPos - upLength * mUp - mAt);
    setVecLength(&mArrows[3], (mPos - mAt).length());

    for (s32 i = 0; i < cArrowNum; i++) {
        setVecLength(&mArrows[i], mArrows[i].length() + 50.0f);
    }

    for (s32 i = 0; i < cArrowNum; i++) {
        mHitResultBuffers[i].clear();
    }

    sead::PtrArray<CollisionParts>* prevPartsArray = nullptr;

    if (!mIsInvalidSearchCollisionParts) {
        mCollisionParts.clear();
        sead::Vector3f center = (mPos + mAt) * 0.5f;
        f32 radius = (center - mAt).length();

        for (s32 i = 0; i < cArrowNum; i++) {
            radius = sead::Mathf::max(radius, (center - (mAt + mArrows[i])).length());
        }

        sead::Delegate1<CameraArrowCollider, CollisionParts*> delegate(
            this, &CameraArrowCollider::pushBackCollisionParts);
        alCollisionUtil::searchCollisionParts(this, center, radius, delegate, &sPartsFilter);
        prevPartsArray = alCollisionUtil::getCollisionPartsPtrArray(this);
        alCollisionUtil::validateCollisionPartsPtrArray(this, &mCollisionParts);
    }

    updateNerve();

    if (mIsInvalidSearchCollisionParts) {
        return;
    }

    if (prevPartsArray != nullptr) {
        alCollisionUtil::validateCollisionPartsPtrArray(this, prevPartsArray);
    } else {
        alCollisionUtil::invalidateCollisionPartsPtrArray(this);
    }
}

/**
 * Adds collision parts to check the arrows against.
 * @param pParts Collision parts.
 */
void CameraArrowCollider::pushBackCollisionParts(CollisionParts* pParts) {
    mCollisionParts.pushBack(pParts);
}

/**
 * Pushes the camera position towards the look at position.
 * @param pCamera Camera pose.
 */
void CameraArrowCollider::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    sead::Vector3f diff = pCamera->getAt() - pCamera->getPos();
    sead::Vector3f dir = {0.0f, 0.0f, 0.0f};

    if (!tryNormalizeOrZero(&dir, diff)) {
        return;
    }

    f32 pushLength = mPushLength;
    f32 maxPushLength = diff.length() - 1.0f;

    if (pushLength > maxPushLength) {
        pushLength = maxPushLength;
    }

    pCamera->setPos(pCamera->getPos() + dir * pushLength);
}

/**
 * Returns the length the camera is pushed towards the look at position.
 * @return Push length.
 */
f32 CameraArrowCollider::getPushLength() const {
    return mPushLength;
}

/**
 * Releases the push length while no arrow hits collision.
 */
void CameraArrowCollider::exeKeep() {
    if (calcPushLength(&mPushLength, mHitResultBuffers, this, mAt, mPos, mArrows, mDir,
                       mIsInvalidThroughPassCollision, 0.0f)) {
        mTargetPushLength = mPushLength;
        setNerve(this, &NrvCameraArrowColliderShrink);
        return;
    }

    mTargetPushLength *= 0.7f;
    mPushLength = lerpValue(0.3f, mPushLength, mTargetPushLength);
}

/**
 * Pushes the camera towards the look at position while an arrow hits collision.
 */
void CameraArrowCollider::exeShrink() {
    f32 pushLength = 0.0f;

    if (calcPushLength(&pushLength, mHitResultBuffers, this, mAt, mPos, mArrows, mDir,
                       mIsInvalidThroughPassCollision, mPushLength)) {
        if (pushLength < mPushLength) {
            pushLength = lerpValue(0.3f, mPushLength, pushLength);
            mPushLength = lerpValue(0.3f, mPushLength, pushLength);
        } else {
            mPushLength = pushLength;
        }

        f32 length = (mAt - mPos).length();

        if (length < mPushLength + 5.0f) {
            mPushLength = length - 5.0f;
        }

        mTargetPushLength = mPushLength;
        return;
    }

    mTargetPushLength *= 0.7f;
    mPushLength = lerpValue(0.3f, mPushLength, mTargetPushLength);
    setNerve(this, &NrvCameraArrowColliderKeep);
}

/**
 * Returns whether the camera is being pushed.
 * @return Whether the camera is being pushed.
 */
bool CameraArrowCollider::isShrink() const {
    return isNerve(this, &NrvCameraArrowColliderShrink);
}

}  // namespace al
