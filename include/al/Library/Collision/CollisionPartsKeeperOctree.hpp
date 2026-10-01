#pragma once

#include <container/seadObjList.h>
#include <container/seadTList.h>
#include <math/seadBoundBox.h>
#include <math/seadVector.h>

#include "Library/Collision/ICollisionPartsKeeper.hpp"
#include "Project/Collision/CollisionParts.hpp"

namespace al {
class CollisionPartsKeeperOctree : public ICollisionPartsKeeper {
public:
    struct OctNode {
        OctNode* parent = nullptr;
        OctNode* children[8] = {};
        sead::BoundBox3f box;
        s32 depth = -1;
        CollisionPartsList partsList;
    };

    using SphereCheckFunc = u32 (*)(SphereHitResultBuffer*, const CollisionPartsList&,
                                    const SphereCheckInfo&, bool, const sead::Vector3f&);

    CollisionPartsKeeperOctree(s32 nodeNumMax, s32 depthMax);

    void endInit() override;
    void insertMovingPartsListToOctNode();
    void addCollisionParts(CollisionParts* pParts) override;
    void connectToCollisionPartsList(CollisionParts* pParts) override;
    OctNode* insertLooseOctree(OctNode* pNode, CollisionParts* pParts);
    void disconnectToCollisionPartsList(CollisionParts* pParts) override;
    s32 checkStrikePoint(HitInfo* pHitInfo,
                         const CollisionCheckInfoBase& rCheckInfo) const override;
    s32 checkStrikeSphere(SphereHitResultBuffer* pBuffer, const SphereCheckInfo& rCheckInfo,
                          bool isCheckNear, const sead::Vector3f& rMoveDir) const override;
    s32 checkStrikeSphereRecursive(SphereHitResultBuffer* pBuffer,
                                   const SphereCheckInfo& rCheckInfo, const OctNode* pNode,
                                   bool isCheckNear, const sead::Vector3f& rMoveDir,
                                   SphereCheckFunc checkFunc) const;
    s32 checkStrikeSphereForPlayer(SphereHitResultBuffer* pBuffer,
                                   const SphereCheckInfo& rCheckInfo) const override;
    s32 checkStrikeDisk(DiskHitResultBuffer* pBuffer,
                        const DiskCheckInfo& rCheckInfo) const override;
    s32 checkStrikeDiskRecursive(DiskHitResultBuffer* pBuffer, const DiskCheckInfo& rCheckInfo,
                                 const OctNode* pNode) const;
    void searchWithSphere(const sead::Vector3f& rPos, f32 radius,
                          CollisionPartsDelegate& rDelegate) const override;
    void searchWithSphereRecursive(const sead::Vector3f& rPos, f32 radius,
                                   CollisionPartsDelegate& rDelegate,
                                   const OctNode* pNode) const;
    void searchWithSphere(const SphereCheckInfo& rCheckInfo,
                          CollisionPartsDelegate& rDelegate) const override;
    void searchWithSphereRecursive(const SphereCheckInfo& rCheckInfo,
                                   CollisionPartsDelegate& rDelegate,
                                   const OctNode* pNode) const;
    s32 checkStrikeArrow(ArrowHitResultBuffer* pBuffer,
                         const ArrowCheckInfo& rCheckInfo) const override;
    s32 checkStrikeArrowRecursive(ArrowHitResultBuffer* pBuffer, const ArrowCheckInfo& rCheckInfo,
                                  const OctNode* pNode) const;
    void movement() override;
    void updateOctNodeCollisionPartsRecursive(OctNode* pNode);
    bool isFitsInBox(const CollisionParts* pParts, const sead::BoundBox3f& rBox) const;
    f32 calcNodeCubeSizeHalf(s32 depth) const;
    bool isSphereFitsInBox(const sead::Vector3f& rPos, f32 radius,
                           const sead::BoundBox3f& rBox) const;

private:
    sead::Vector3f& getPartsMin() { return const_cast<sead::Vector3f&>(mPartsBox.getMin()); }

    sead::Vector3f& getPartsMax() { return const_cast<sead::Vector3f&>(mPartsBox.getMax()); }

    sead::BoundBox3f mPartsBox;
    f32 mCubeSize = 0.0f;
    OctNode mRootNode;
    sead::ObjList<OctNode> mNodeList;
    CollisionPartsList mPartsList;
    s32 mDepthMax;
    bool mIsEndInit = false;
};
}  // namespace al
