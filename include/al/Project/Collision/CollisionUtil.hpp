#pragma once

#include <math/seadVector.h>

namespace al {
class CollisionParts;
class CollisionPartsFilterBase;
class IUseCollision;
class TriangleFilterBase;
}  // namespace al

namespace alCollisionUtil {
al::CollisionParts* getStrikeArrowCollisionParts(const al::IUseCollision*, sead::Vector3f*,
                                                 const sead::Vector3f&, const sead::Vector3f&,
                                                 const al::CollisionPartsFilterBase*,
                                                 const al::TriangleFilterBase*);
}  // namespace alCollisionUtil
