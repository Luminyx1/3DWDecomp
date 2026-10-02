#pragma once

#include <math/seadMatrix.h>
#include <nn/g3d/g3d_ResCameraAnim.h>

#include "Project/Camera/CameraPoser.hpp"

namespace nn::g3d {
class ResFile;
}  // namespace nn::g3d

namespace al {
class ActorInitInfo;
class CameraSwitcher;
class Resource;

class CameraPoserAnim : public CameraPoser {
public:
    CameraPoserAnim(CameraSwitcher* pSwitcher);
    void initAnim(const Resource* pResource, const PlacementId& rId, const ActorInitInfo& rInfo);
    void setAnimAndBaseMtx(const char* pName, const sead::Matrix34f* pBaseMtx);
    s32 getMaxFrame(const char* pName) const;
    void start() override;
    void update() override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    bool isCameraCurrent() const;
    bool isEndAnim() const;
    bool isExistAnim(const char* pName) const;

public:
    const nn::g3d::ResCameraAnim* mCameraAnim = nullptr;
    nn::g3d::ResFile* mResFile = nullptr;
    nn::g3d::CameraAnimResult mAnimResult;
    CameraSwitcher* mSwitcher;
    s32 mMaxFrame = 0;
    s32 mFrame = 0;
    const sead::Matrix34f* mBaseMtx = nullptr;
    bool mIsInitAnim = false;
    bool mIsSetAnim = false;
    bool mIsApplyAnimFovyAndTwist = false;
};

static_assert(sizeof(CameraPoserAnim) == 0x108);

}  // namespace al
