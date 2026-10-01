#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Project/Collision/CollisionParts.hpp"

namespace al {
class ArrowCheckInfo;
class ArrowHitResultBuffer;
class CollisionCheckInfoBase;
class DiskCheckInfo;
class DiskHitResultBuffer;
class HitInfo;
class ICollisionPartsKeeper;
class SphereCheckInfo;
class SphereHitResultBuffer;

bool checkStrikePointCore(HitInfo* pHitInfo, const CollisionPartsList& rList,
                          const CollisionCheckInfoBase& rCheckInfo);
s32 checkStrikeSphereCore(SphereHitResultBuffer* pBuffer, const CollisionPartsList& rList,
                          const SphereCheckInfo& rCheckInfo, bool isCheckNear,
                          const sead::Vector3f& rMoveDir);
s32 checkStrikeArrowCore(ArrowHitResultBuffer* pBuffer, const CollisionPartsList& rList,
                         const ArrowCheckInfo& rCheckInfo);
s32 checkStrikeSphereForPlayerCore(SphereHitResultBuffer* pBuffer, const CollisionPartsList& rList,
                                   const SphereCheckInfo& rCheckInfo, bool isCheckNear,
                                   const sead::Vector3f& rMoveDir);
s32 checkStrikeDiskCore(DiskHitResultBuffer* pBuffer, const CollisionPartsList& rList,
                        const DiskCheckInfo& rCheckInfo);
void updateCollisionParts(CollisionParts* pParts);
void updateCollisionPartsList(CollisionPartsList* pList);
void pushBackCollisionParts(CollisionPartsList* pList, CollisionParts* pParts);
ICollisionPartsKeeper* createSimpleCollisionPartsKeeper();
}  // namespace al
