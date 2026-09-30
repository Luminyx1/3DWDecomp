#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Project/Collision/CollisionPartsTriangle.hpp"

namespace al {
enum class CollisionLocation : u8 {
    None = 0,
    Face = 1,
    Edge1 = 2,
    Edge2 = 3,
    Edge3 = 4,
    Corner1 = 5,
    Corner2 = 6,
    Corner3 = 7,
};

class HitInfo {
public:
    HitInfo();

    bool isCollisionAtFace() const;
    bool isCollisionAtEdge() const;
    bool isCollisionAtCorner() const;

    Triangle mTriangle;
    f32 _70 = 0.0f;
    sead::Vector3f mPos = {0.0f, 0.0f, 0.0f};
    sead::Vector3f _80 = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mMovingReaction = {0.0f, 0.0f, 0.0f};
    CollisionLocation mCollisionLocation = CollisionLocation::None;
};

class ArrowHitInfo : public HitInfo {};

class SphereHitInfo : public HitInfo {
public:
    void calcFixVector(sead::Vector3f* pFix, sead::Vector3f* pFixNormal) const;
    void calcFixVectorNormal(sead::Vector3f* pFix, sead::Vector3f* pFixNormal) const;
};

class DiskHitInfo : public HitInfo {
public:
    void calcFixVector(sead::Vector3f* pFix, sead::Vector3f* pFixNormal) const;
    void calcFixVectorNormal(sead::Vector3f* pFix, sead::Vector3f* pFixNormal) const;
};
}  // namespace al
