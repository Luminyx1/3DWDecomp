#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class IUseCamera_RS;
class LiveActor;

class CameraPoserFixActor : public CameraPoser_RS {
public:
    CameraPoserFixActor(const LiveActor* pActor);
    CameraPoserFixActor(const char* pName);

    void init() override;
    void loadParam(const ByamlIter& rIter) override;
    void start(const CameraStartInfo& rInfo) override;
    void reset() override;
    void storeCamera(const sead::Vector3f& rCameraPos, const sead::Vector3f& rLookAtPos);
    void setDirectAngle();
    void setDirectAngle(sead::Vector3f& rDir);
    void update() override;
    void exeFollow();
    void exeGoIn();
    void exeGoOut();
    void updateReturnCamTarget(sead::Vector3f& rTarget);
    void setReturn(const IUseCamera_RS* pCamera, s32 step, bool isKeepDir);
    void setReturnGoalItemAppear(const IUseCamera_RS* pCamera, s32 step, bool isKeepDir);
    void setReturn(const IUseCamera_RS* pCamera, s32 step, const sead::Vector3f& rCameraPos,
                   const sead::Vector3f& rLookAtPos);
    void setReturn(s32 step, const sead::Vector3f& rCameraPos, const sead::Vector3f& rLookAtPos);
    void setReturnKeepDir(s32 step, const sead::Vector3f& rCameraPos, sead::Vector3f dir);
    void setReturnSetDir(s32 step, const sead::Vector3f& rCameraPos,
                         const sead::Vector3f& rLookAtPos, const sead::Vector3f& rStartLookAtPos,
                         sead::Vector3f dir, f32 angleH, f32 angleV, f32 dirRate,
                         f32 distanceRate, f32 distance);

    void setTargetActor(const LiveActor* pActor) { mTargetActor = pActor; }

    void setOffset(const sead::Vector3f& rOffset) { mOffset.set(rOffset); }

    void setDistance(f32 distance) { mDistance = distance; }

    void setAngleH(f32 angle) { mAngleH = angle; }

    void setAngleV(f32 angle) { mAngleV = angle; }

private:
    void updateReturnStep();
    void updateReturnLerpDistance(f32 rate);

public:
    const LiveActor* mTargetActor;
    void* _150;
    sead::Vector3f mOffset;
    f32 mDistance;
    f32 mAngleH;
    f32 mAngleV;
    bool _170;
    bool mIsCalcNearestAtFromPreAt;
    bool mIsReturn;
    bool mIsReturnEnd;
    bool mIsDirectAngle;
    bool mIsReturnToDir;
    bool mIsReturnLerpDir;
    bool mIsReturnGoalItemAppear;
    sead::Vector3f mStoredCameraPos;
    sead::Vector3f mStoredLookAtPos;
    sead::Vector3f mReturnStartCameraPos;
    sead::Vector3f mReturnLookAtPos;
    sead::Vector3f mReturnGoalLookAtPos;
    s32 mReturnStepMax;
    s32 mReturnStep;
    f32 mReturnOffsetY;
    sead::Vector3f mDirectAngleDir;
    f32 mStoredDistance;
    sead::Vector3f mStoredDir;
    f32 mReturnDistance;
    sead::Vector3f mReturnDir;
    f32 mReturnDirRate;
    f32 mReturnDistanceRate;
    sead::Vector3f mDefaultOffset;
    f32 mDefaultDistance;
    f32 mDefaultAngleH;
    f32 mDefaultAngleV;
};

static_assert(sizeof(CameraPoserFixActor) == 0x210);

class CameraPoserFixTalk : public CameraPoserFixActor {
public:
    CameraPoserFixTalk(const LiveActor* pActor);

    void start(const CameraStartInfo& rInfo) override;

    f32 mTalkAngleH = 0.0f;
};

static_assert(sizeof(CameraPoserFixTalk) == 0x210);

class CameraPoserFixFishing : public CameraPoserFixActor {
public:
    CameraPoserFixFishing(const LiveActor* pActor);

    void initParam(f32 angleH, const sead::Vector3f& rOffset, const sead::Vector3f& rOffsetRev);
    void start(const CameraStartInfo& rInfo) override;

    f32 mFishingAngleH = 0.0f;
    sead::Vector3f mFishingOffset = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mFishingOffsetRev = {0.0f, 0.0f, 0.0f};
};

static_assert(sizeof(CameraPoserFixFishing) == 0x228);

}  // namespace al
