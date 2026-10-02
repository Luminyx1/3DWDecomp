#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <nn/g3d/g3d_ResCameraAnim.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace nn::g3d {
class ResFile;
}  // namespace nn::g3d

namespace al {
class Resource;

class CameraPoserAnim_RS : public CameraPoser_RS {
public:
    CameraPoserAnim_RS();
    void initAnimResource(const Resource* pResource, const sead::Matrix34f* pBaseMtx);
    void setAnim(const char* pName, s32 startStep, s32 endStep, s32 playStep);
    bool isExistAnim(const char* pName) const;
    void start(const CameraStartInfo& rInfo) override;
    void update() override;
    void setAnimEnd();
    s32 calcStepMax(const char* pName) const;
    bool isAnimPlaying(const char* pName) const;
    bool isAnimEnd() const;

public:
    const nn::g3d::ResCameraAnim* mCameraAnim = nullptr;
    nn::g3d::ResFile* mResFile = nullptr;
    nn::g3d::CameraAnimResult mAnimResult;
    const char* mAnimName = nullptr;
    s32 mStepMax = 0;
    s32 mStep = -1;
    s32 mStartStep = 0;
    s32 mEndStep = 0;
    s32 mPlayStep = 0;
    const sead::Matrix34f* mBaseMtxPtr = nullptr;
    s32 _1b0 = 0;
    bool mIsValidZoom = false;
    bool mIsRotateBaseUp = false;
    bool mIsCheckRange = false;
    sead::Vector3f mLookAtOffset = sead::Vector3f::zero;
};

static_assert(sizeof(CameraPoserAnim_RS) == 0x1c8);

}  // namespace al
