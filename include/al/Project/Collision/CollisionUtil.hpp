#pragma once

#include <math/seadVector.h>

namespace al {
    class IUseCollision;
    class Triangle;
    class CollisionPartsFilterBase;
    class TriangleFilterBase;
};

namespace alCollisionUtil {
    bool getFirstPolyOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos, al::Triangle* pTriangle, const sead::Vector3f& rStart,
                             const sead::Vector3f& rArrow, const al::CollisionPartsFilterBase* pPartsFilter, const al::TriangleFilterBase* pTriangleFilter);
};
