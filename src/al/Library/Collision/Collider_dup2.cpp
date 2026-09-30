#include <gfx/seadColor.h>
#include <math/seadMathCalcCommon.h>

#include "Library/Math/MathUtil.hpp"
#include "Project/Collision/Collider.hpp"

namespace al {
/**
 * Checks whether a polygon is a wall.
 * @param rNormal normal of the polygon
 * @param rGravity gravity direction
 * @return true if the polygon is a wall
 */
bool isWallPolygon(const sead::Vector3f& rNormal, const sead::Vector3f& rGravity) {
    if (isNearZero(rNormal, 0.001f)) {
        return false;
    }
    return sead::Mathf::abs(rNormal.dot(rGravity)) < 0.34202f;
}

/**
 * Checks whether a polygon is a floor.
 * @param rNormal normal of the polygon
 * @param rGravity gravity direction
 * @return true if the polygon is a floor
 */
bool isFloorPolygon(const sead::Vector3f& rNormal, const sead::Vector3f& rGravity) {
    if (isNearZero(rNormal, 0.001f)) {
        return false;
    }
    f32 dot = rNormal.dot(rGravity);
    return !(sead::Mathf::abs(dot) < 0.34202f) && dot < 0.0f;
}

/**
 * Checks whether a polygon is a floor within an angle.
 * @param rNormal normal of the polygon
 * @param rGravity gravity direction
 * @param cos cosine of the maximum floor angle
 * @return true if the polygon is a floor
 */
bool isFloorPolygonCos(const sead::Vector3f& rNormal, const sead::Vector3f& rGravity, f32 cos) {
    if (isNearZero(rNormal, 0.001f)) {
        return false;
    }
    f32 dot = rNormal.dot(rGravity);
    if (-dot < cos) {
        return false;
    }
    return !(sead::Mathf::abs(dot) < 0.34202f) && dot < 0.0f;
}

/**
 * Checks whether a polygon is a ceiling.
 * @param rNormal normal of the polygon
 * @param rGravity gravity direction
 * @return true if the polygon is a ceiling
 */
bool isCeilingPolygon(const sead::Vector3f& rNormal, const sead::Vector3f& rGravity) {
    if (isNearZero(rNormal, 0.001f)) {
        return false;
    }
    f32 dot = rNormal.dot(rGravity);
    if (sead::Mathf::abs(dot) < 0.34202f) {
        return false;
    }
    return dot >= 0.0f;
}

/**
 * Calculates the debug color of a triangle from the angle of its normal.
 * @param pColor output color
 * @param pAngle output angle in degrees, or nullptr
 * @param rNormal normal of the triangle
 */
void calcTriangleColorByAngle(sead::Color4f* pColor, f32* pAngle, const sead::Vector3f& rNormal) {
    f32 angle = calcAngleDegree(rNormal, sead::Vector3f::ey);
    if (angle >= 60.0f && angle < 79.5f) {
        *pColor = sead::Color4f::cCyan;
    } else if (angle >= 79.5f && angle <= 110.0f) {
        *pColor = sead::Color4f::cBlue;
    } else if (angle > 110.0f) {
        *pColor = sead::Color4f::cMagenta;
    } else {
        *pColor = sead::Color4f::cGreen;
    }
    if (pAngle) {
        *pAngle = angle;
    }
}
}  // namespace al
