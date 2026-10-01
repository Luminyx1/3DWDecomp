#pragma once

#include <math/seadVector.h>

#include "Library/Collision/ICollisionPartsKeeper.hpp"

namespace al {
class CollisionPartsKeeperArray : public ICollisionPartsKeeper {
public:
    struct PartsInfo {
        sead::Vector3f trans;
        f32 range;
        CollisionParts* parts;
    };

    CollisionPartsKeeperArray();

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

private:
    s32 findPartsIndex(const CollisionParts* pParts) const {
        for (s32 i = 0; i < mPartsNum; i++) {
            if (mPartsInfos[i].parts == pParts) {
                return i;
            }
        }

        return -1;
    }

    PartsInfo* mPartsInfos;
    s32 mPartsNum;
    s32 mPartsNumMax;
};
}  // namespace al
