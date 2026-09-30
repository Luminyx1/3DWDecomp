#pragma once

#include <math/seadVector.h>

namespace al {
class ArrowHitInfo;
class CollisionParts;
class CollisionPartsFilterBase;
class IUseCollision;
class HitInfo;
class Triangle;
class TriangleFilterBase;

const char* getCollisionCodeName(const Triangle& rTriangle, const char* pCategory);
}  // namespace al

namespace alCollisionUtil {
bool isFarAway(const al::CollisionParts& rParts, const sead::Vector3f& rPos, f32 distance);
bool getFirstPolyOnArrow(const al::IUseCollision* pCollision, const al::ArrowHitInfo** ppHitInfo,
                         const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                         const al::CollisionPartsFilterBase* pPartsFilter,
                         const al::TriangleFilterBase* pTriFilter);
bool getFirstPolyOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos,
                         al::Triangle* pTriangle, const sead::Vector3f& rPos,
                         const sead::Vector3f& rDir, const char* pMaterialName);
bool getFirstPolyOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos,
                         al::Triangle* pTriangle, const sead::Vector3f& rPos,
                         const sead::Vector3f& rDir,
                         const al::CollisionPartsFilterBase* pPartsFilter,
                         const al::TriangleFilterBase* pTriFilter);
s32 checkStrikeSphere(const al::IUseCollision* pCollision, const sead::Vector3f& rPos, f32 radius,
                      const al::CollisionPartsFilterBase* pPartsFilter,
                      const al::TriangleFilterBase* pTriFilter);
s32 checkStrikeArrow(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                     const sead::Vector3f& rDir, const al::CollisionPartsFilterBase* pPartsFilter,
                     const al::TriangleFilterBase* pTriFilter);
const al::HitInfo* getStrikeArrowInfo(const al::IUseCollision* pCollision, u32 index);
al::CollisionParts* getStrikeArrowCollisionParts(const al::IUseCollision* pCollision,
                                                 sead::Vector3f* pHitPos, const sead::Vector3f& rPos,
                                                 const sead::Vector3f& rDir,
                                                 const al::CollisionPartsFilterBase* pPartsFilter,
                                                 const al::TriangleFilterBase* pTriFilter);
}  // namespace alCollisionUtil
