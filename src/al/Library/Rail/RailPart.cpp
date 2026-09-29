#include "Library/Rail/RailPart.hpp"

#include "Library/Math/MathUtil.hpp"
#include "Library/Rail/BezierCurve.hpp"
#include "Library/Rail/LinearCurve.hpp"

namespace al {

/**
 * @brief Constructs an empty rail part.
 */
RailPart::RailPart() = default;

/**
 * @brief Creates the curve of the part, linear when both handles lie on their end points.
 * @param rStart The start point.
 * @param rStartHandle The control point after the start.
 * @param rEndHandle The control point before the end.
 * @param rEnd The end point.
 */
void RailPart::init(const sead::Vector3f& rStart, const sead::Vector3f& rStartHandle,
                    const sead::Vector3f& rEndHandle, const sead::Vector3f& rEnd) {
    sead::Vector3f startDiff = rStart - rStartHandle;
    sead::Vector3f endDiff = rEnd - rEndHandle;
    if (startDiff.equals({0.0f, 0.0f, 0.0f}, 0.1f) && endDiff.equals({0.0f, 0.0f, 0.0f}, 0.1f)) {
        mLinearCurve = new LinearCurve();
        mLinearCurve->set(rStart, rEnd);
    } else {
        mBezierCurve = new BezierCurve();
        mBezierCurve->set(rStart, rStartHandle, rEndHandle, rEnd);
    }
}

/**
 * @brief Sets the up directions at both ends.
 * @param rUpStart The up direction at the start.
 * @param rUpEnd The up direction at the end.
 */
void RailPart::setUp(sead::Vector3f& rUpStart, sead::Vector3f& rUpEnd) {
    mUpStart = rUpStart;
    mUpEnd = rUpEnd;
}

/**
 * @brief Sets the accelerations at both ends.
 * @param accelStart The acceleration at the start.
 * @param accelEnd The acceleration at the end.
 */
void RailPart::setAccel(f32 accelStart, f32 accelEnd) {
    mAccelStart = accelStart;
    mAccelEnd = accelEnd;
}

/**
 * @brief Gets the accelerations at both ends.
 * @param pAccelStart Where the acceleration at the start is written.
 * @param pAccelEnd Where the acceleration at the end is written.
 */
void RailPart::getAccels(f32* pAccelStart, f32* pAccelEnd) {
    *pAccelStart = mAccelStart;
    *pAccelEnd = mAccelEnd;
}

/**
 * @brief Sets the angle at the start.
 * @param angle The angle.
 */
void RailPart::setAngleS(f32 angle) {
    mAngleStart = angle;
    mIsSetAngleStart = true;
}

/**
 * @brief Sets the angle at the end.
 * @param angle The angle.
 */
void RailPart::setAngleE(f32 angle) {
    mAngleEnd = angle;
    mIsSetAngleEnd = true;
}

/**
 * @brief Gets the angle at the start.
 * @param pAngle Where the angle is written.
 * @return True if the angle was set.
 */
bool RailPart::getAngleS(f32* pAngle) {
    if (!mIsSetAngleStart) {
        return false;
    }

    *pAngle = mAngleStart;
    return true;
}

/**
 * @brief Gets the angle at the end.
 * @param pAngle Where the angle is written.
 * @return True if the angle was set.
 */
bool RailPart::getAngleE(f32* pAngle) {
    if (!mIsSetAngleEnd) {
        return false;
    }

    *pAngle = mAngleEnd;
    return true;
}

/**
 * @brief Computes a point on the part.
 * @param pPos Where the point is written.
 * @param param The curve parameter.
 */
void RailPart::calcPos(sead::Vector3f* pPos, f32 param) const {
    return mBezierCurve ? mBezierCurve->calcPos(pPos, param) : mLinearCurve->calcPos(pPos, param);
}

/**
 * @brief Computes the derivative of the part.
 * @param pVel Where the derivative is written.
 * @param param The curve parameter.
 */
void RailPart::calcVelocity(sead::Vector3f* pVel, f32 param) const {
    return mBezierCurve ? mBezierCurve->calcVelocity(pVel, param) :
                          mLinearCurve->calcVelocity(pVel, param);
}

/**
 * @brief Interpolates the up direction along the part.
 * @param pUp Where the up direction is written.
 * @param length The length from the start of the part.
 */
void RailPart::calcUpDir(sead::Vector3f* pUp, f32 length) const {
    f32 rate = length / getPartLength();
    *pUp = mUpStart * (1.0f - rate) + mUpEnd * rate;
}

/**
 * @brief Gets the length of the part.
 * @return The length.
 */
f32 RailPart::getPartLength() const {
    return mBezierCurve ? mBezierCurve->getLength() : mLinearCurve->getLength();
}

/**
 * @brief Computes the direction of the part, falling back to the chord when the speed is zero.
 * @param pDir Where the direction is written.
 * @param param The curve parameter.
 */
void RailPart::calcDir(sead::Vector3f* pDir, f32 param) const {
    calcVelocity(pDir, param);
    if (!isNearZero(*pDir, 0.001f)) {
        normalize(pDir);
        return;
    }

    sead::Vector3f startPos;
    calcStartPos(&startPos);
    sead::Vector3f endPos;
    calcEndPos(&endPos);

    pDir->x = endPos.x - startPos.x;
    pDir->y = endPos.y - startPos.y;
    pDir->z = endPos.z - startPos.z;
    if (isNearZero(*pDir, 0.001f)) {
        *pDir = {0.0f, 0.0f, 1.0f};
    } else {
        normalize(pDir);
    }
}

/**
 * @brief Gets the start point.
 * @param pPos Where the start point is written.
 */
void RailPart::calcStartPos(sead::Vector3f* pPos) const {
    return mBezierCurve ? mBezierCurve->calcStartPos(pPos) : mLinearCurve->calcStartPos(pPos);
}

/**
 * @brief Gets the end point.
 * @param pPos Where the end point is written.
 */
void RailPart::calcEndPos(sead::Vector3f* pPos) const {
    return mBezierCurve ? mBezierCurve->calcEndPos(pPos) : mLinearCurve->calcEndPos(pPos);
}

/**
 * @brief Computes the length between two parameters.
 * @param startParam The start parameter.
 * @param endParam The end parameter.
 * @param stepCount The number of integration steps for curves.
 * @return The length.
 */
f32 RailPart::calcLength(f32 startParam, f32 endParam, s32 stepCount) const {
    return mBezierCurve ? mBezierCurve->calcLength(startParam, endParam, stepCount) :
                          mLinearCurve->calcLength(startParam, endParam);
}

/**
 * @brief Converts a length along the part to a curve parameter.
 * @param length The length from the start.
 * @return The curve parameter.
 */
f32 RailPart::calcCurveParam(f32 length) const {
    return mBezierCurve ? mBezierCurve->calcCurveParam(length) :
                          mLinearCurve->calcCurveParam(length);
}

/**
 * @brief Computes the parameter nearest to a position.
 * @param rPos The position.
 * @param interval The sampling step for curves.
 * @return The curve parameter.
 */
f32 RailPart::calcNearestParam(const sead::Vector3f& rPos, f32 interval) const {
    return mBezierCurve ? mBezierCurve->calcNearestParam(rPos, interval) :
                          mLinearCurve->calcNearestParam(rPos);
}

/**
 * @brief Computes the point nearest to a position.
 * @param pNearest Where the nearest point is written.
 * @param rPos The position.
 * @param interval The sampling step for curves.
 */
void RailPart::calcNearestPos(sead::Vector3f* pNearest, const sead::Vector3f& rPos,
                              f32 interval) const {
    return mBezierCurve ? mBezierCurve->calcNearestPos(pNearest, rPos, interval) :
                          mLinearCurve->calcNearestPos(pNearest, rPos);
}

/**
 * @brief Computes the length to the point nearest to a position.
 * @param pLength Where the length is written.
 * @param rPos The position.
 * @param maxLength The length of the part.
 * @param interval The sampling step for curves.
 * @return The squared distance to the nearest point.
 */
f32 RailPart::calcNearestLength(f32* pLength, const sead::Vector3f& rPos, f32 maxLength,
                                f32 interval) const {
    return mBezierCurve ? mBezierCurve->calcNearestLength(pLength, rPos, maxLength, interval) :
                          mLinearCurve->calcNearestLength(pLength, rPos, maxLength);
}

}  // namespace al
