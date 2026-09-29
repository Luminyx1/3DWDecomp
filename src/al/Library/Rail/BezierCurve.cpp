#include "Library/Rail/BezierCurve.hpp"

#include <algorithm>
#include <cmath>
#include <math/seadMathCalcCommon.h>

namespace al {

/**
 * @brief Constructs a degenerate curve at the origin.
 */
BezierCurve::BezierCurve() = default;

/**
 * @brief Sets the curve from its control points.
 * @param rStart The start point.
 * @param rStartHandle The control point after the start.
 * @param rEndHandle The control point before the end.
 * @param rEnd The end point.
 */
void BezierCurve::set(const sead::Vector3f& rStart, const sead::Vector3f& rStartHandle,
                      const sead::Vector3f& rEndHandle, const sead::Vector3f& rEnd) {
    sead::Vector3f diff1 = rStartHandle - rStart;
    sead::Vector3f diff2 = rEndHandle - rStartHandle;
    sead::Vector3f diff3 = rEnd - rEndHandle;

    sead::Vector3f diffDiff1 = diff2 - diff1;
    sead::Vector3f diffDiff2 = diff3 - diff2;

    sead::Vector3f diffDiffDiff = diffDiff2 - diffDiff1;

    mStart = rStart;
    mCoeff1 = diff1 * 3;
    mCoeff2 = diffDiff1 * 3;
    mCoeff3 = diffDiffDiff;

    mLength = calcLength(0.0f, 1.0f, 10);
}

/**
 * @brief Integrates the length between two parameters with Simpson's rule.
 * @param startParam The start parameter.
 * @param endParam The end parameter.
 * @param stepCount The number of integration steps.
 * @return The length.
 */
f32 BezierCurve::calcLength(f32 startParam, f32 endParam, s32 stepCount) const {
    f32 avgVelocity = (calcDeltaLength(startParam) + calcDeltaLength(endParam)) / 2;
    f32 halfStepSize = (endParam - startParam) * (1.0f / (stepCount * 2.0f));

    f32 sumVelHalfStep = 0.0f;
    f32 sumVelFullStep = 0.0f;
    for (s32 i = 1; i <= stepCount; i++) {
        f32 doubleI = i * 2.0f;

        sumVelHalfStep += calcDeltaLength((halfStepSize * (doubleI - 1)) + startParam);

        if (i != stepCount) {
            sumVelFullStep += calcDeltaLength((halfStepSize * (doubleI)) + startParam);
        }
    }

    return std::floor((halfStepSize * 0.33333f) *
                      (avgVelocity + (sumVelFullStep * 2) + (sumVelHalfStep * 4)) * 1024.0f) /
           1024.0f;
}

/**
 * @brief Computes a point on the curve.
 * @param pPos Where the point is written.
 * @param param The curve parameter in [0, 1].
 */
void BezierCurve::calcPos(sead::Vector3f* pPos, f32 param) const {
    f32 square = param * param;
    f32 cube = square * param;

    pPos->x = (mCoeff1.x * param) + mStart.x;
    pPos->y = (mCoeff1.y * param) + mStart.y;
    pPos->z = (mCoeff1.z * param) + mStart.z;

    pPos->x = (mCoeff2.x * square) + pPos->x;
    pPos->y = (mCoeff2.y * square) + pPos->y;
    pPos->z = (mCoeff2.z * square) + pPos->z;

    pPos->x = (mCoeff3.x * cube) + pPos->x;
    pPos->y = (mCoeff3.y * cube) + pPos->y;
    pPos->z = (mCoeff3.z * cube) + pPos->z;
}

/**
 * @brief Computes the derivative of the curve.
 * @param pVel Where the derivative is written.
 * @param param The curve parameter.
 */
void BezierCurve::calcVelocity(sead::Vector3f* pVel, f32 param) const {
    f32 fac1 = param + param;
    f32 fac2 = 3 * param * param;

    pVel->x = (mCoeff2.x * fac1) + mCoeff1.x;
    pVel->y = (mCoeff2.y * fac1) + mCoeff1.y;
    pVel->z = (mCoeff2.z * fac1) + mCoeff1.z;

    pVel->x = (mCoeff3.x * fac2) + pVel->x;
    pVel->y = (mCoeff3.y * fac2) + pVel->y;
    pVel->z = (mCoeff3.z * fac2) + pVel->z;
}

/**
 * @brief Computes the speed of the curve at a parameter.
 * @param param The curve parameter.
 * @return The length of the derivative.
 */
f32 BezierCurve::calcDeltaLength(f32 param) const {
    sead::Vector3f velocity;
    calcVelocity(&velocity, param);
    return velocity.length();
}

/**
 * @brief Converts a length along the curve to a curve parameter with Newton's method.
 * @param length The length from the start.
 * @return The curve parameter.
 */
f32 BezierCurve::calcCurveParam(f32 length) const {
    f32 param = length / mLength;
    f32 partLength = calcLength(0, param, 10);
    if (sead::Mathf::abs(length - partLength) <= 0.01f) {
        return param;
    }

    for (s32 i = 0; i <= 4; i++) {
        f32 speed = std::max(calcDeltaLength(param), 0.001f);
        f32 newParam = param + ((length - partLength) / speed);

        param = sead::Mathf::clamp(newParam, 0.0f, 1.0f);
        partLength = calcLength(0.0f, param, 10);
        if (sead::Mathf::abs(length - partLength) <= 0.01f) {
            return param;
        }
    }

    if (param > 1.0f || partLength < 0.0f) {
        return sead::Mathf::clamp(param, 0.0f, 1.0f);
    }
    return param;
}

/**
 * @brief Samples the curve to find the parameter nearest to a position.
 * @param rPos The position.
 * @param interval The sampling step.
 * @return The curve parameter.
 */
f32 BezierCurve::calcNearestParam(const sead::Vector3f& rPos, f32 interval) const {
    f32 currentParam = 0.0f;
    f32 bestParam = -1.0f;
    f32 bestDistance = 3.4028e38f;
    do {
        sead::Vector3f nearest;
        calcPos(&nearest, currentParam);
        f32 currentDistance = (nearest - rPos).squaredLength();

        if (currentDistance < bestDistance) {
            bestParam = currentParam;
            bestDistance = currentDistance;
        }
        currentParam = currentParam + interval;
    } while (currentParam <= 1.0f);
    return bestParam;
}

/**
 * @brief Samples the curve by length to find the length nearest to a position.
 * @param pLength Where the nearest length is written.
 * @param rPos The position.
 * @param maxLength The length of the curve.
 * @param interval The sampling step.
 * @return The squared distance to the nearest sample.
 */
f32 BezierCurve::calcNearestLength(f32* pLength, const sead::Vector3f& rPos, f32 maxLength,
                                   f32 interval) const {
    f32 bestLength = -1.0f;
    f32 currentLength = 0.0f;
    f32 bestDistance = 3.4028e38f;
    while (currentLength < maxLength) {
        sead::Vector3f nearest;
        calcPos(&nearest, calcCurveParam(currentLength));
        f32 currentDistance = (nearest - rPos).squaredLength();

        if (currentDistance < bestDistance) {
            bestLength = currentLength;
            bestDistance = currentDistance;
        }
        currentLength = currentLength + interval;
    }
    *pLength = bestLength;
    return bestDistance;
}

/**
 * @brief Computes the point nearest to a position.
 * @param pNearest Where the nearest point is written.
 * @param rPos The position.
 * @param interval The sampling step.
 */
void BezierCurve::calcNearestPos(sead::Vector3f* pNearest, const sead::Vector3f& rPos,
                                 f32 interval) const {
    calcPos(pNearest, calcNearestParam(rPos, interval));
}

/**
 * @brief Gets the start point.
 * @param pPos Where the start point is written.
 */
void BezierCurve::calcStartPos(sead::Vector3f* pPos) const {
    *pPos = mStart;
}

/**
 * @brief Computes the first control point.
 * @param pPos Where the control point is written.
 */
void BezierCurve::calcCtrlPos1(sead::Vector3f* pPos) const {
    pPos->x = mCoeff1.x * 0.333333f;
    pPos->y = mCoeff1.y * 0.333333f;
    pPos->z = mCoeff1.z * 0.333333f;

    pPos->x = pPos->x + mStart.x;
    pPos->y = pPos->y + mStart.y;
    pPos->z = pPos->z + mStart.z;
}

/**
 * @brief Computes the second control point.
 * @param pPos Where the control point is written.
 */
void BezierCurve::calcCtrlPos2(sead::Vector3f* pPos) const {
    pPos->x = mCoeff2.x * 0.333333f;
    pPos->y = mCoeff2.y * 0.333333f;
    pPos->z = mCoeff2.z * 0.333333f;

    pPos->x = (mCoeff1.x * 0.6666667f) + pPos->x;
    pPos->y = (mCoeff1.y * 0.6666667f) + pPos->y;
    pPos->z = (mCoeff1.z * 0.6666667f) + pPos->z;

    pPos->x = pPos->x + mStart.x;
    pPos->y = pPos->y + mStart.y;
    pPos->z = pPos->z + mStart.z;
}

/**
 * @brief Computes the end point.
 * @param pPos Where the end point is written.
 */
void BezierCurve::calcEndPos(sead::Vector3f* pPos) const {
    pPos->x = mStart.x + mCoeff1.x;
    pPos->y = mStart.y + mCoeff1.y;
    pPos->z = mStart.z + mCoeff1.z;

    pPos->x = pPos->x + mCoeff2.x;
    pPos->y = pPos->y + mCoeff2.y;
    pPos->z = pPos->z + mCoeff2.z;

    pPos->x = pPos->x + mCoeff3.x;
    pPos->y = pPos->y + mCoeff3.y;
    pPos->z = pPos->z + mCoeff3.z;
}

}  // namespace al
