#pragma once

#include <math/seadVector.h>

namespace al {
    class CollisionParts;
    class CollisionPartsFilterBase;
    class IUseCollision;
    class Triangle;
    class TriangleFilterBase;
};

namespace alCollisionUtil {
    bool getFirstPolyOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos, al::Triangle* pTriangle, const sead::Vector3f& rStart,
                             const sead::Vector3f& rArrow, const al::CollisionPartsFilterBase* pPartsFilter, const al::TriangleFilterBase* pTriangleFilter);
    al::CollisionParts* getStrikeArrowCollisionParts(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos, const sead::Vector3f& rStart,
                                                     const sead::Vector3f& rArrow, const al::CollisionPartsFilterBase* pPartsFilter,
                                                     const al::TriangleFilterBase* pTriangleFilter);
};
