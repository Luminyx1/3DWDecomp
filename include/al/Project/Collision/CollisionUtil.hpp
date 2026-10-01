#pragma once

#include <container/seadPtrArray.h>
#include <math/seadVector.h>
#include <prim/seadDelegate.h>

#include "Project/Collision/CollisionPartsTriangle.hpp"

namespace al {
class ArrowHitInfo;
class CollisionCheckInfoBase;
class CollisionParts;
class CollisionPartsFilterBase;
class DiskHitInfo;
class HitInfo;
class HitSensor;
class ICollisionPartsKeeper;
class IUseCollision;
class LiveActor;
class SphereHitInfo;
class TriangleFilterBase;

const char* getCollisionCodeName(const Triangle& rTriangle, const char* pCategory);
}  // namespace al

namespace alCollisionUtil {
struct SphereMoveHitInfo {
    static bool compare(const SphereMoveHitInfo& rLhs, const SphereMoveHitInfo& rRhs);

    f32 time;
    sead::Vector3f pos;
    al::Triangle triangle;
};

al::ICollisionPartsKeeper* getCollisionPartsKeeper(const al::IUseCollision* pCollision);
const sead::Vector3f& getCollisionHitPos(const al::HitInfo* pHitInfo);
const sead::Vector3f* getCollisionHitNormal(const al::HitInfo* pHitInfo);
al::HitSensor* getCollisionHitSensor(const al::HitInfo* pHitInfo);
const al::CollisionParts* getCollisionHitParts(const al::HitInfo* pHitInfo);
al::LiveActor* getCollisionHitActor(const al::HitInfo* pHitInfo);
s32 checkStrikeSphere(const al::IUseCollision* pCollision, const sead::Vector3f& rPos, f32 radius,
                      const al::CollisionPartsFilterBase* pPartsFilter,
                      const al::TriangleFilterBase* pTriFilter);
s32 checkStrikeSphereMovingReaction(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                                    f32 radius, const sead::Vector3f& rMoveVec,
                                    const al::CollisionPartsFilterBase* pPartsFilter,
                                    const al::TriangleFilterBase* pTriFilter);
s32 checkStrikeDisk(const al::IUseCollision* pCollision, const sead::Vector3f& rPos, f32 radius,
                    f32 height, const sead::Vector3f& rDir,
                    const al::CollisionPartsFilterBase* pPartsFilter,
                    const al::TriangleFilterBase* pTriFilter);
s32 checkStrikeArrow(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                     const sead::Vector3f& rDir, const al::CollisionPartsFilterBase* pPartsFilter,
                     const al::TriangleFilterBase* pTriFilter);
s32 checkStrikeSphereForPlayer(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                               f32 radius, const al::CollisionPartsFilterBase* pPartsFilter,
                               const al::TriangleFilterBase* pTriFilter);
bool getFirstPolyOnArrow(const al::IUseCollision* pCollision, const al::ArrowHitInfo** ppHitInfo,
                         const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                         const al::CollisionPartsFilterBase* pPartsFilter,
                         const al::TriangleFilterBase* pTriFilter);
const al::ArrowHitInfo* getStrikeArrowInfo(const al::IUseCollision* pCollision, u32 index);
bool getFirstPolyOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos,
                         al::Triangle* pTriangle, const sead::Vector3f& rPos,
                         const sead::Vector3f& rDir, const char* pSpecialPurpose);
bool getFirstPolyOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos,
                         al::Triangle* pTriangle, const sead::Vector3f& rPos,
                         const sead::Vector3f& rDir,
                         const al::CollisionPartsFilterBase* pPartsFilter,
                         const al::TriangleFilterBase* pTriFilter);
bool getHitPosOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos,
                      const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                      const al::CollisionPartsFilterBase* pPartsFilter,
                      const al::TriangleFilterBase* pTriFilter);
bool getHitPosAndNormalOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos,
                               sead::Vector3f* pNormal, const sead::Vector3f& rPos,
                               const sead::Vector3f& rDir,
                               const al::CollisionPartsFilterBase* pPartsFilter,
                               const al::TriangleFilterBase* pTriFilter);
bool getHitPosAndNormalAndSensorOnArrow(const al::IUseCollision* pCollision,
                                        sead::Vector3f* pHitPos, sead::Vector3f* pNormal,
                                        al::HitSensor** ppSensor, const sead::Vector3f& rPos,
                                        const sead::Vector3f& rDir,
                                        const al::CollisionPartsFilterBase* pPartsFilter,
                                        const al::TriangleFilterBase* pTriFilter);
bool getFirstCollisionSensorOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos,
                                    al::HitSensor** ppSensor, const sead::Vector3f& rPos,
                                    const sead::Vector3f& rDir,
                                    const al::CollisionPartsFilterBase* pPartsFilter,
                                    const al::TriangleFilterBase* pTriFilter);
al::CollisionParts* getStrikeArrowCollisionParts(const al::IUseCollision* pCollision,
                                                 sead::Vector3f* pHitPos, const sead::Vector3f& rPos,
                                                 const sead::Vector3f& rDir,
                                                 const al::CollisionPartsFilterBase* pPartsFilter,
                                                 const al::TriangleFilterBase* pTriFilter);
al::HitSensor* tryGetStrikeArrowCollisionSensor(const al::IUseCollision* pCollision,
                                                const sead::Vector3f& rPos,
                                                const sead::Vector3f& rDir,
                                                const al::CollisionPartsFilterBase* pPartsFilter,
                                                const al::TriangleFilterBase* pTriFilter);
u32 getStrikeArrowInfoNum(const al::IUseCollision* pCollision);
const al::SphereHitInfo* getStrikeSphereInfo(const al::IUseCollision* pCollision, u32 index);
u32 getStrikeSphereInfoNum(const al::IUseCollision* pCollision);
const sead::Vector3f& getStrikeSphereHitPos(const al::IUseCollision* pCollision, u32 index);
const al::DiskHitInfo* getStrikeDiskInfo(const al::IUseCollision* pCollision, u32 index);
u32 getStrikeDiskInfoNum(const al::IUseCollision* pCollision);
const sead::Vector3f& getStrikeDiskHitPos(const al::IUseCollision* pCollision, u32 index);
bool isInvalidParts(const al::CollisionParts& rParts, const al::CollisionCheckInfoBase& rCheckInfo);
bool isFarAway(const al::CollisionParts& rParts, const sead::Vector3f& rPos, f32 distance);
s32 checkStrikeSphereMove(const al::IUseCollision* pCollision, SphereMoveHitInfo* pHitInfos,
                          s32 hitInfoNum, const sead::Vector3f& rPos, f32 radius,
                          const sead::Vector3f& rMoveVec,
                          const al::CollisionPartsFilterBase* pPartsFilter,
                          const al::TriangleFilterBase* pTriFilter);
void searchCollisionParts(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                          f32 radius, sead::IDelegate1<al::CollisionParts*>& rDelegate,
                          const al::CollisionPartsFilterBase* pPartsFilter);
void searchCollisionParts(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                          f32 radius, sead::IDelegate1<al::CollisionParts*>& rDelegate);
void validateCollisionPartsPtrArray(const al::IUseCollision* pCollision,
                                    sead::PtrArray<al::CollisionParts>* pPartsArray);
void invalidateCollisionPartsPtrArray(const al::IUseCollision* pCollision);
sead::PtrArray<al::CollisionParts>* getCollisionPartsPtrArray(const al::IUseCollision* pCollision);
}  // namespace alCollisionUtil
