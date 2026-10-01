#pragma once

#include <basis/seadTypes.h>
#include <container/seadTList.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class ArrowHitResultBuffer;
class CollisionParts;
class DiskHitResultBuffer;
class HitInfo;
class HitSensor;
class LiveActor;
class KCollisionServer;
class SphereHitResultBuffer;
class TriangleFilterBase;

using CollisionPartsList = sead::TList<CollisionParts*>;
using CollisionPartsListNode = sead::TListNode<CollisionParts*>;

enum class ForceCollisionScaleType : u8 {
    None = 0,
    Average = 1,
    One = 2,
};

class CollisionParts {
public:
    CollisionParts(void* pKcl, const void* pAttribute);

    void calcInvMtxScale();
    LiveActor* getConnectedHost() const;
    void initParts(const sead::Matrix34f& rMtx);
    void resetAllMtx(const sead::Matrix34f& rMtx);
    void updateBoundingSphereRange(sead::Vector3f scale);
    void validateByUser();
    void invalidateByUser();
    void validateBySystem();
    void invalidateBySystem();
    void onJoinList();
    f32 makeEqualScale(sead::Matrix34f* pMtx);
    void resetAllMtxPrivate(const sead::Matrix34f& rMtx);
    void resetAllMtx();
    void updateBoundingSphereRange();
    void forceResetAllMtxAndSetUpdateMtxOneTime(const sead::Matrix34f& rMtx);
    void forceResetAllMtxAndSetUpdateMtxOneTime();
    void syncMtx(const sead::Matrix34f& rMtx);
    void syncMtx();
    void updateMtx();
    void updateScale();
    void updateBoundingSphereRangePrivate(f32 scale);
    bool checkBoundingSphereRange(const sead::Vector3f& rPos, f32 radius);
    s32 checkStrikePoint(HitInfo* pHitInfo, const sead::Vector3f& rPos,
                         const TriangleFilterBase* pFilter) const;
    s32 checkStrikeSphere(SphereHitResultBuffer* pBuffer, const sead::Vector3f& rPos, f32 radius,
                          bool isCheckNear, const sead::Vector3f& rMoveDir,
                          const TriangleFilterBase* pFilter) const;
    s32 checkStrikeSphereCore(SphereHitResultBuffer* pBuffer, const sead::Vector3f& rPos,
                              const sead::Vector3f& rLocalPos, const sead::Vector3f& rMoveVec,
                              f32 radius, const TriangleFilterBase* pFilter) const;
    s32 checkStrikeArrow(ArrowHitResultBuffer* pBuffer, const sead::Vector3f& rPos,
                         const sead::Vector3f& rDir, const TriangleFilterBase* pFilter) const;
    s32 checkStrikeSphereForPlayer(SphereHitResultBuffer* pBuffer, const sead::Vector3f& rPos,
                                   f32 radius, const TriangleFilterBase* pFilter) const;
    s32 checkStrikeSphereForPlayerCore(SphereHitResultBuffer* pBuffer, const sead::Vector3f& rPos,
                                       const sead::Vector3f& rLocalPos,
                                       const sead::Vector3f& rMoveVec,
                                       const sead::Vector3f& rUnused, f32 radius,
                                       const TriangleFilterBase* pFilter) const;
    s32 checkStrikeDisk(DiskHitResultBuffer* pBuffer, const sead::Vector3f& rPos, f32 radius,
                        f32 height, const sead::Vector3f& rDir,
                        const TriangleFilterBase* pFilter) const;
    s32 checkStrikeDiskCore(DiskHitResultBuffer* pBuffer, const sead::Vector3f& rPos,
                            const sead::Vector3f& rLocalPos, const sead::Vector3f& rMoveVec,
                            f32 radius, f32 height, const sead::Vector3f& rLocalDir,
                            const TriangleFilterBase* pFilter) const;
    void calcForceMovePower(sead::Vector3f* pPower, const sead::Vector3f& rPos) const;
    void calcForceRotatePower(sead::Quatf* pPower) const;

    static void setCheckBallIgnoreSeparateDir(bool isIgnore);

    CollisionPartsListNode* getListNode() { return &mListNode; }

    const sead::Matrix34f& getBaseMtx() const { return mBaseMtx; }

    const sead::Matrix34f& getBaseInvMtx() const { return mBaseInvMtx; }

    const sead::Matrix34f& getPrevBaseMtx() const { return mPrevBaseMtx; }

    const sead::Matrix34f& getPrevBaseInvMtx() const { return mPrevBaseInvMtx; }

    const sead::Matrix34f* getSyncCollisionMtx() const { return mSyncCollisionMtx; }

    KCollisionServer* getKCollisionServer() const { return mKColServer; }

    HitSensor* getSensor() const { return mSensor; }

    const char* getSpecialPurpose() const { return mSpecialPurpose; }

    void setSyncCollisionMtx(const sead::Matrix34f* pMtx) { mSyncCollisionMtx = pMtx; }

    void setSensor(HitSensor* pSensor) { mSensor = pSensor; }

    void setSpecialPurpose(const char* pName) { mSpecialPurpose = pName; }

    bool isValidCollision() const { return _160 && _161; }

    bool isJustValidated() const { return mIsJustValidated; }

    void resetJustValidated() { mIsJustValidated = false; }

    f32 getBoundingSphereRange() const { return mBoundingSphereRange; }

    ForceCollisionScaleType getForceScaleType() const {
        return static_cast<ForceCollisionScaleType>(_165);
    }

    CollisionPartsListNode mListNode{this};
    const sead::Matrix34f* mSyncCollisionMtx = nullptr;
    sead::Matrix34f mSyncMtx;
    sead::Matrix34f mBaseMtx;
    sead::Matrix34f mBaseInvMtx;
    sead::Matrix34f mPrevBaseMtx;
    sead::Matrix34f mPrevBaseInvMtx;
    sead::Vector3f mMtxScaleVec = {1.0f, 1.0f, 1.0f};
    f32 mMtxScale = 1.0f;
    f32 mInvMtxScale = 1.0f;
    s32 mPriority = -1;
    KCollisionServer* mKColServer = nullptr;
    HitSensor* mSensor = nullptr;
    const char* mSpecialPurpose = nullptr;
    sead::Vector3f mEqualScale = {1.0f, 1.0f, 1.0f};
    s32 _154 = 0;
    f32 mBoundingSphereRange = -1.0f;
    f32 mBaseMtxScale = 1.0f;
    bool _160 = true;
    bool _161 = true;
    bool mIsJustValidated = false;
    bool mIsMoving = true;
    bool mIsUpdateMtxOneTime = false;
    u8 _165 = 0;
    void* _168 = nullptr;
};

static_assert(sizeof(CollisionParts) == 0x170);
}  // namespace al
