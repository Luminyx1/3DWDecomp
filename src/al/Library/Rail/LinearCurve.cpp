#include "Library/Rail/LinearCurve.hpp"

#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * @brief Constructs a degenerate curve at the origin.
 */
LinearCurve::LinearCurve() = default;

/**
 * @brief Sets the line from its end points.
 * @param rStart The start point.
 * @param rEnd The end point.
 */
void LinearCurve::set(const sead::Vector3f& rStart, const sead::Vector3f& rEnd) {
    mStart = rStart;
    mDiff.x = rEnd.x - rStart.x;
    mDiff.y = rEnd.y - rStart.y;
    mDiff.z = rEnd.z - rStart.z;
    mLength = sead::Mathf::sqrt(mDiff.x * mDiff.x + mDiff.y * mDiff.y + mDiff.z * mDiff.z);
}

/**
 * @brief Computes a point on the line.
 * @param pPos Where the point is written.
 * @param param The curve parameter in [0, 1].
 */
void LinearCurve::calcPos(sead::Vector3f* pPos, f32 param) const {
    pPos->x = (mDiff.x * param) + mStart.x;
    pPos->y = (mDiff.y * param) + mStart.y;
    pPos->z = (mDiff.z * param) + mStart.z;
}

/**
 * @brief Computes the derivative of the line.
 * @param pVel Where the derivative is written.
 * @param param The curve parameter, unused.
 */
void LinearCurve::calcVelocity(sead::Vector3f* pVel, f32 param) const {
    *pVel = mDiff;
}

/**
 * @brief Computes the length between two parameters.
 * @param startParam The start parameter.
 * @param endParam The end parameter.
 * @return The length.
 */
f32 LinearCurve::calcLength(f32 startParam, f32 endParam) const {
    return mLength * sead::Mathf::abs(endParam - startParam);
}

/**
 * @brief Converts a length along the line to a curve parameter.
 * @param length The length from the start.
 * @return The curve parameter.
 */
f32 LinearCurve::calcCurveParam(f32 length) const {
    if (isNearZero(mLength, 0.001f)) {
        return 0.0f;
    }

    return sead::Mathf::clamp(length, 0.0f, mLength) / mLength;
}

/**
 * @brief Computes the parameter of the point nearest to a position.
 * @param rPos The position.
 * @return The curve parameter.
 */
f32 LinearCurve::calcNearestParam(const sead::Vector3f& rPos) const {
    if (isNearZero(mLength, 0.001f)) {
        return 0.0f;
    }

    f32 dot = (rPos - mStart).dot(mDiff);
    return sead::Mathf::clamp(dot / sead::Mathf::square(mLength), 0.0f, 1.0f);
}

/**
 * @brief Computes the length to the point nearest to a position.
 * @param pLength Where the length along the line is written.
 * @param rPos The position.
 * @param maxLength The length of the line.
 * @return The squared distance between the position and the nearest point.
 */
f32 LinearCurve::calcNearestLength(f32* pLength, const sead::Vector3f& rPos, f32 maxLength) const {
    f32 nearestParam = calcNearestParam(rPos);

    sead::Vector3f nearestPos;
    calcPos(&nearestPos, nearestParam);
    sead::Vector3f diff = nearestPos - rPos;
    f32 distance = diff.squaredLength();

    *pLength = nearestParam * maxLength;
    return distance;
}

/**
 * @brief Computes the point nearest to a position.
 * @param pNearest Where the nearest point is written.
 * @param rPos The position.
 */
void LinearCurve::calcNearestPos(sead::Vector3f* pNearest, const sead::Vector3f& rPos) const {
    calcPos(pNearest, calcNearestParam(rPos));
}

/**
 * @brief Gets the start point.
 * @param pPos Where the start point is written.
 */
void LinearCurve::calcStartPos(sead::Vector3f* pPos) const {
    *pPos = mStart;
}

/**
 * @brief Gets the end point.
 * @param pPos Where the end point is written.
 */
void LinearCurve::calcEndPos(sead::Vector3f* pPos) const {
    pPos->x = mStart.x + mDiff.x;
    pPos->y = mStart.y + mDiff.y;
    pPos->z = mStart.z + mDiff.z;
}

}  // namespace al
