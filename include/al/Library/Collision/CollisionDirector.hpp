#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <math/seadVector.h>
#include <prim/seadDelegate.h>

#include "Library/Collision/CollisionCheckInfo.hpp"
#include "Library/Execute/IUseExecutor.hpp"
#include "Library/HostIO/IUseHioNode.hpp"

namespace al {
class CollisionParts;
class CollisionPartsFilterBase;
class CollisionPartsKeeperPtrArray;
class ExecuteDirector;
class ICollisionPartsKeeper;
class TriangleFilterBase;

class CollisionDirector : public HioNode, public IUseExecutor {
public:
    CollisionDirector(ExecuteDirector* pExecuteDirector, s32 threadNum);

    void setPartsKeeper(ICollisionPartsKeeper* pPartsKeeper);
    void endInit();
    void setPartsFilter(const CollisionPartsFilterBase* pFilter);
    void setTriFilter(const TriangleFilterBase* pFilter);
    s32 checkStrikePoint(const sead::Vector3f& rPos, HitInfo* pHitInfo);
    s32 checkStrikeSphere(const sead::Vector3f& rPos, f32 radius, bool isCheckNear,
                          const sead::Vector3f& rMoveDir);
    s32 checkStrikeArrow(const sead::Vector3f& rPos, const sead::Vector3f& rDir);
    s32 checkStrikeSphereForPlayer(const sead::Vector3f& rPos, f32 radius);
    s32 checkStrikeDisk(const sead::Vector3f& rPos, f32 radius, f32 height,
                        const sead::Vector3f& rDir);
    ArrowHitInfo* getStrikeArrowInfo(u32 index);
    u32 getStrikeArrowInfoNum() const;
    SphereHitInfo* getStrikeSphereInfo(u32 index);
    u32 getStrikeSphereInfoNum() const;
    DiskHitInfo* getStrikeDiskInfo(u32 index);
    u32 getStrikeDiskInfoNum() const;
    void getSphereHitInfoArrayForCollider(SphereHitInfo** ppHitInfos, u32* pNum);
    void getDiskHitInfoArrayForCollider(DiskHitInfo** ppHitInfos, u32* pNum);
    void execute() override;
    void searchCollisionPartsWithSphere(const sead::Vector3f& rPos, f32 radius,
                                        sead::IDelegate1<CollisionParts*>& rDelegate) const;
    void validateCollisionPartsPtrArray(sead::PtrArray<CollisionParts>* pPartsArray);
    void invalidateCollisionPartsPtrArray();
    sead::PtrArray<CollisionParts>* getCollisionPartsPtrArray() const;
    void setCheckBallIgnoreSeparateDir(bool isIgnore);
    void searchCollisionPartsWithSphere(const sead::Vector3f& rPos, f32 radius,
                                        sead::IDelegate1<CollisionParts*>& rDelegate,
                                        const CollisionPartsFilterBase* pFilter) const;

    ICollisionPartsKeeper* getActivePartsKeeper() const { return mActivePartsKeeper; }

private:
    ICollisionPartsKeeper* mActivePartsKeeper = nullptr;
    ICollisionPartsKeeper* mRootPartsKeeper = nullptr;
    CollisionPartsKeeperPtrArray* mPtrArrayPartsKeeper;
    const CollisionPartsFilterBase* mPartsFilter = nullptr;
    const TriangleFilterBase* mTriFilter = nullptr;
    ArrowHitResultBuffer* mStrikeArrowHitInfos = nullptr;
    SphereHitResultBuffer* mStrikeSphereHitInfos = nullptr;
    DiskHitResultBuffer* mStrikeDiskHitInfos = nullptr;
    SphereHitInfo* mSphereHitInfosForCollider = nullptr;
    DiskHitInfo* mDiskHitInfosForCollider = nullptr;
};

static_assert(sizeof(CollisionDirector) == 0x58);
}  // namespace al
