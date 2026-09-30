#include "Project/Block/BlockRailShapeStraight.hpp"

#include <math/seadMathCalcCommon.h>
#include <math/seadMatrix.h>

#include "Library/Math/MathUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace al {
/**
 * Constructs a straight block rail shape.
 * @param pName shape name
 */
BlockRailShapeStraight::BlockRailShapeStraight(const char* pName) : BlockRailShape(pName) {}

/**
 * Initializes the start position, direction, up direction and length.
 * @param rQuat rotation
 * @param rTrans translation
 * @param rIter shape parameters
 */
void BlockRailShapeStraight::init(const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
                                  const ByamlIter& rIter) {
    sead::Vector3f startPos = sead::Vector3f::zero;
    tryGetByamlV3f(&startPos, rIter, "StartPos");
    sead::Vector3f endPos = sead::Vector3f::zero;
    tryGetByamlV3f(&endPos, rIter, "EndPos");
    sead::Vector3f upDir = sead::Vector3f::ey;
    tryGetByamlV3f(&upDir, rIter, "UpDir");

    sead::Matrix34f mtx;
    mtx.makeQT(rQuat, rTrans);
    mStartPos.setMul(mtx, startPos);
    mUpDir.setRotated(rQuat, upDir);
    sead::Vector3f worldEndPos;
    worldEndPos.setMul(mtx, endPos);
    separateScalarAndDirection(&mLength, &mDir, worldEndPos - mStartPos);
    normalizeOrZero(&mUpDir);
}

/**
 * Checks if a movement crosses the shape from above.
 * @param pRate crossing rate
 * @param rPrevPos previous position
 * @param rPos current position
 * @return whether the movement crosses the shape
 */
bool BlockRailShapeStraight::isRide(f32* pRate, const sead::Vector3f& rPrevPos,
                                    const sead::Vector3f& rPos) const {
    f32 prevHeight = (rPrevPos - mStartPos).dot(mUpDir);

    if (prevHeight < 0.0f) {
        return false;
    }

    f32 height = (rPos - mStartPos).dot(mUpDir);

    if (height > 0.0f) {
        return false;
    }

    f32 diff = prevHeight - height;
    f32 rate = isNearZero(diff, 0.001f) ? 0.0f : prevHeight / diff;
    sead::Vector3f crossPos = rPrevPos + (rPos - rPrevPos) * rate;
    f32 coord = (crossPos - mStartPos).dot(mDir);

    if (coord < 0.0f || coord > mLength) {
        return false;
    }

    if ((mStartPos + mDir * coord - crossPos).length() > 10.0f) {
        return false;
    }

    *pRate = coord / mLength;
    return true;
}

/**
 * Calculates the position at a rate.
 * @param pPos output position
 * @param rate rate on the shape
 */
void BlockRailShapeStraight::calcPos(sead::Vector3f* pPos, f32 rate) const {
    f32 coord = sead::Mathf::clamp(rate, 0.0f, 1.0f) * mLength;
    pPos->setScaleAdd(coord, mDir, mStartPos);
}

/**
 * Calculates the direction at a rate.
 * @param pDir output direction
 * @param rate rate on the shape
 */
void BlockRailShapeStraight::calcDir(sead::Vector3f* pDir, f32 rate) const {
    *pDir = mDir;
}

/**
 * Calculates the offset from the base translation.
 * @param rBaseTrans base translation
 */
void BlockRailShapeStraight::calcOffset(const sead::Vector3f& rBaseTrans) {
    mOffset = mStartPos - rBaseTrans;
}

/**
 * Updates the start position from the base translation.
 * @param rBaseTrans base translation
 */
void BlockRailShapeStraight::updateLinkedTrans(const sead::Vector3f& rBaseTrans) {
    mStartPos = rBaseTrans + mOffset;
}

/**
 * Calculates the nearest point on the shape.
 * @param pPos output position
 * @param pRate output rate
 * @param rPos position
 */
void BlockRailShapeStraight::calcNearestParam(sead::Vector3f* pPos, f32* pRate,
                                              const sead::Vector3f& rPos) const {
    // TODO: calcPerpendicFootToLineInside returns the clamped rate in 3DW (MathUtil.hpp declares void)
    calcPerpendicFootToLineInside(pPos, rPos, mStartPos, mDir * mLength + mStartPos);
    *pRate = 0.0f;
}

/**
 * Constructs a curved block rail shape.
 * @param pName shape name
 */
BlockRailShapeCurve::BlockRailShapeCurve(const char* pName) : BlockRailShape(pName) {}

/**
 * Initializes the center, axes, radius and length.
 * @param rQuat rotation
 * @param rTrans translation
 * @param rIter shape parameters
 */
void BlockRailShapeCurve::init(const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
                               const ByamlIter& rIter) {
    sead::Vector2f centerOffset = sead::Vector2f::zero;
    rIter.tryGetFloatByKey(&mRadius, "Radius");
    tryGetByamlV2f(&centerOffset, rIter, "CenterOffset");
    s32 sideAxisIndex = 1;
    s32 upAxisIndex = 2;
    s32 frontAxisIndex = 3;
    rIter.tryGetIntByKey(&sideAxisIndex, "SideAxisIndex");
    rIter.tryGetIntByKey(&upAxisIndex, "UpAxisIndex");
    rIter.tryGetIntByKey(&frontAxisIndex, "FrontAxisIndex");
    calcQuatLocalSignAxis(&mSideAxis, rQuat, sideAxisIndex);
    calcQuatLocalSignAxis(&mUpAxis, rQuat, upAxisIndex);
    calcQuatLocalSignAxis(&mFrontAxis, rQuat, frontAxisIndex);
    mCenter = rTrans;
    mCenter += centerOffset.x * mSideAxis;
    mCenter += centerOffset.y * mFrontAxis;
    mLength = mRadius * (sead::Mathf::pi() / 2);
}

/**
 * Checks if a movement crosses the shape from above.
 * @param pRate crossing rate
 * @param rPrevPos previous position
 * @param rPos current position
 * @return whether the movement crosses the shape
 */
bool BlockRailShapeCurve::isRide(f32* pRate, const sead::Vector3f& rPrevPos,
                                 const sead::Vector3f& rPos) const {
    f32 prevHeight = (rPrevPos - mCenter).dot(mUpAxis);

    if (!(prevHeight > 0.0f)) {
        return false;
    }

    f32 height = (rPos - mCenter).dot(mUpAxis);

    if (height >= 0.0f) {
        return false;
    }

    f32 diff = prevHeight - height;
    f32 rate = isNearZero(diff, 0.001f) ? 0.0f : prevHeight / diff;
    sead::Vector3f crossPos = rPrevPos + (rPos - rPrevPos) * rate;
    sead::Vector3f localPos = crossPos - mCenter;
    f32 side = localPos.dot(mSideAxis);

    if (side < 0.0f) {
        return false;
    }

    f32 front = localPos.dot(mFrontAxis);

    if (front < 0.0f) {
        return false;
    }

    if (sead::Mathf::abs(localPos.length() - mRadius) > 10.0f) {
        return false;
    }

    *pRate = atan2f(front, side) * (2 / sead::Mathf::pi());
    return true;
}

/**
 * Calculates the position at a rate.
 * @param pPos output position
 * @param rate rate on the shape
 */
void BlockRailShapeCurve::calcPos(sead::Vector3f* pPos, f32 rate) const {
    *pPos = mCenter;
    if (rate <= 0.0f) {
        *pPos += mRadius * mSideAxis;
    } else if (rate >= 1.0f) {
        *pPos += mRadius * mFrontAxis;
    } else {
        f32 angle = rate * (sead::Mathf::pi() / 2);
        *pPos += mSideAxis * (cosf(angle) * mRadius);
        *pPos += mFrontAxis * (sinf(angle) * mRadius);
    }
}

/**
 * Calculates the direction at a rate.
 * @param pDir output direction
 * @param rate rate on the shape
 */
void BlockRailShapeCurve::calcDir(sead::Vector3f* pDir, f32 rate) const {
    if (rate <= 0.0f) {
        *pDir = mFrontAxis;
    } else if (rate >= 1.0f) {
        *pDir = -mSideAxis;
    } else {
        f32 angle = rate * (sead::Mathf::pi() / 2);
        *pDir = cosf(angle) * mFrontAxis;
        *pDir -= mSideAxis * sinf(angle);
    }
}

/**
 * Calculates the nearest point on the shape.
 * @param pPos output position
 * @param pRate output rate
 * @param rPos position
 */
void BlockRailShapeCurve::calcNearestParam(sead::Vector3f* pPos, f32* pRate,
                                           const sead::Vector3f& rPos) const {
    sead::Vector3f dir = rPos - mCenter;
    verticalizeVec(&dir, mUpAxis, dir);

    if (normalizeOrZero(&dir)) {
        *pPos = mRadius * mSideAxis + mCenter;
        *pRate = 0.0f;
        return;
    }

    f32 side = dir.dot(mSideAxis);
    f32 front = dir.dot(mFrontAxis);

    if (side > 0.0f && front > 0.0f) {
        *pPos = mCenter + dir * mRadius;
        *pRate = atan2f(front, side) / (sead::Mathf::pi() / 2);
    } else if (side < front) {
        *pPos = mFrontAxis * mRadius + mCenter;
        *pRate = 1.0f;
    } else {
        *pPos = mSideAxis * mRadius + mCenter;
        *pRate = 0.0f;
    }
}

/**
 * Calculates the offset from the base translation.
 * @param rBaseTrans base translation
 */
void BlockRailShapeCurve::calcOffset(const sead::Vector3f& rBaseTrans) {
    mOffset = mCenter - rBaseTrans;
}

/**
 * Updates the center from the base translation.
 * @param rBaseTrans base translation
 */
void BlockRailShapeCurve::updateLinkedTrans(const sead::Vector3f& rBaseTrans) {
    mCenter = rBaseTrans + mOffset;
}
}  // namespace al
