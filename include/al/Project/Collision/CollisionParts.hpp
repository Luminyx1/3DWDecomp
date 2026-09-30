#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class ArrowHitResultBuffer;
class HitInfo;
class HitSensor;
class LiveActor;
class KCollisionServer;
class SphereHitResultBuffer;
class TriangleFilterBase;

class CollisionParts {
public:
    CollisionParts(void* pKcl, const void* pAttribute);
    void initParts(const sead::Matrix34f& rMtx);
    void invalidateBySystem();
    void validateByUser();
    void invalidateByUser();
    void resetAllMtx();
    void resetAllMtx(const sead::Matrix34f& rMtx);
    void syncMtx();
    void syncMtx(const sead::Matrix34f& rMtx);
    void forceResetAllMtxAndSetUpdateMtxOneTime(const sead::Matrix34f& rMtx);

    void validateBySystem();
    void calcForceMovePower(sead::Vector3f* pPower, const sead::Vector3f& rPos) const;
    void calcForceRotatePower(sead::Quatf* pPower) const;
    LiveActor* getConnectedHost() const;
    s32 checkStrikePoint(HitInfo* pHitInfo, const sead::Vector3f& rPos,
                         const TriangleFilterBase* pFilter) const;
    s32 checkStrikeSphere(SphereHitResultBuffer* pBuffer, const sead::Vector3f& rPos, f32 radius,
                          bool isCheckNear, const sead::Vector3f& rMoveDir,
                          const TriangleFilterBase* pFilter) const;
    s32 checkStrikeArrow(ArrowHitResultBuffer* pBuffer, const sead::Vector3f& rPos,
                         const sead::Vector3f& rDir, const TriangleFilterBase* pFilter) const;

    u8 _0[0x20];
    const sead::Matrix34f* mSyncCollisionMtx;
    u8 _28[0x30];
    sead::Matrix34f mBaseMtx;
    sead::Matrix34f mBaseInvMtx;
    sead::Matrix34f mPrevBaseMtx;
    sead::Matrix34f _e8;
    sead::Vector3f _118;
    f32 _124;
    f32 _128;
    s32 mPriority;
    KCollisionServer* mKColServer;
    HitSensor* mSensor;
    const char* mSpecialPurpose;
    u32 _148;
    u32 _14c;
    u32 _150;
    u32 _154;
    f32 _158;
    f32 _15c;
    u8 _160;
    u8 _161;
    u8 _162;
    u8 _163;
    u8 _164;
    u8 _165;
    u8 _166[0x170 - 0x166];
};
}  // namespace al
