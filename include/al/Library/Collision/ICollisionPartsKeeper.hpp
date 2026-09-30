#pragma once

#include <math/seadVector.h>
#include <prim/seadDelegate.h>

#include "Library/HostIO/IUseHioNode.hpp"

namespace al {
class ArrowCheckInfo;
class ArrowHitResultBuffer;
class CollisionCheckInfoBase;
class CollisionParts;
class DiskCheckInfo;
class DiskHitResultBuffer;
class HitInfo;
class SphereCheckInfo;
class SphereHitResultBuffer;

class ICollisionPartsKeeper : public IUseHioNode {
public:
    virtual void endInit() = 0;
    virtual void addCollisionParts(CollisionParts* pParts) = 0;
    virtual void connectToCollisionPartsList(CollisionParts* pParts) = 0;
    virtual void disconnectToCollisionPartsList(CollisionParts* pParts) = 0;
    virtual s32 checkStrikePoint(HitInfo* pHitInfo,
                                 const CollisionCheckInfoBase& rCheckInfo) const = 0;
    virtual s32 checkStrikeSphere(SphereHitResultBuffer* pBuffer,
                                  const SphereCheckInfo& rCheckInfo, bool isCheckNear,
                                  const sead::Vector3f& rMoveDir) const = 0;
    virtual s32 checkStrikeArrow(ArrowHitResultBuffer* pBuffer,
                                 const ArrowCheckInfo& rCheckInfo) const = 0;
    virtual s32 checkStrikeSphereForPlayer(SphereHitResultBuffer* pBuffer,
                                           const SphereCheckInfo& rCheckInfo) const = 0;
    virtual s32 checkStrikeDisk(DiskHitResultBuffer* pBuffer,
                                const DiskCheckInfo& rCheckInfo) const = 0;
    virtual void searchWithSphere(const sead::Vector3f& rPos, f32 radius,
                                  sead::IDelegate1<CollisionParts*>& rDelegate) const = 0;
    virtual void searchWithSphere(const SphereCheckInfo& rCheckInfo,
                                  sead::IDelegate1<CollisionParts*>& rDelegate) const = 0;
    virtual void movement() = 0;
};
}  // namespace al
