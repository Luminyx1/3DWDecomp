#pragma once

#include <container/seadPtrArray.h>

#include "Library/Collision/ICollisionPartsKeeper.hpp"

namespace al {
class CollisionPartsKeeperPtrArray : public ICollisionPartsKeeper {
public:
    CollisionPartsKeeperPtrArray();

    void endInit() override;
    void addCollisionParts(CollisionParts* pParts) override;
    void connectToCollisionPartsList(CollisionParts* pParts) override;
    void disconnectToCollisionPartsList(CollisionParts* pParts) override;
    s32 checkStrikePoint(HitInfo* pHitInfo,
                         const CollisionCheckInfoBase& rCheckInfo) const override;
    s32 checkStrikeSphere(SphereHitResultBuffer* pBuffer, const SphereCheckInfo& rCheckInfo,
                          bool isCheckNear, const sead::Vector3f& rMoveDir) const override;
    s32 checkStrikeArrow(ArrowHitResultBuffer* pBuffer,
                         const ArrowCheckInfo& rCheckInfo) const override;
    s32 checkStrikeSphereForPlayer(SphereHitResultBuffer* pBuffer,
                                   const SphereCheckInfo& rCheckInfo) const override;
    s32 checkStrikeDisk(DiskHitResultBuffer* pBuffer,
                        const DiskCheckInfo& rCheckInfo) const override;
    void searchWithSphere(const sead::Vector3f& rPos, f32 radius,
                          CollisionPartsDelegate& rDelegate) const override;
    void searchWithSphere(const SphereCheckInfo& rCheckInfo,
                          CollisionPartsDelegate& rDelegate) const override;
    void movement() override;

    sead::PtrArray<CollisionParts>* getPartsArray() const { return mPartsArray; }

    void setPartsArray(sead::PtrArray<CollisionParts>* pPartsArray) { mPartsArray = pPartsArray; }

private:
    sead::PtrArray<CollisionParts>* mPartsArray = nullptr;
};
}  // namespace al
