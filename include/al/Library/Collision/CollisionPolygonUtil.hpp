#pragma once

#include <gfx/seadColor.h>
#include <math/seadVector.h>

namespace al {
bool isWallPolygon(const sead::Vector3f& rNormal, const sead::Vector3f& rGravity);
bool isFloorPolygon(const sead::Vector3f& rNormal, const sead::Vector3f& rGravity);
bool isFloorPolygonCos(const sead::Vector3f& rNormal, const sead::Vector3f& rGravity, f32 cos);
bool isCeilingPolygon(const sead::Vector3f& rNormal, const sead::Vector3f& rGravity);
void calcTriangleColorByAngle(sead::Color4f* pColor, f32* pAngle, const sead::Vector3f& rNormal);
}  // namespace al
