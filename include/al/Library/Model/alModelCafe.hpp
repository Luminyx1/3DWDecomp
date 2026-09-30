#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace nn::g3d {
class ResModel;
}

namespace al {
class AnimPlayerMat;
class AnimPlayerSkl;
class AnimPlayerVis;
class GpuMemAllocator;
class InitResourceDataAnim;
class JointControllerKeeper;
class Resource;
class SimpleModelG3D;
}  // namespace al

class alModelCafe {
public:
    static alModelCafe* createFromOtherModel(alModelCafe* pOther);

    alModelCafe(bool isCreateBaseMtx);
    alModelCafe();
    ~alModelCafe();

    void initResource(const char* pModelArcName, const char* pAnimArcName, const char* pSuffix);
    void initModel(s32 bufferNum, al::GpuMemAllocator* pAllocator);
    const al::Resource* getAnimResource() const;
    void show();
    bool isHidden() const;
    void hide();
    void update();
    void updatePaused();
    void updateLast();
    void calc(const sead::Matrix34f& rMtx, const sead::Vector3f& rScale);
    void setCameraInfo(const sead::Matrix34f* pViewMtx, const sead::Matrix34f* pInvViewMtx,
                       const sead::Matrix44f* pProjMtx, const sead::Matrix44f* pViewProjMtx);
    const sead::Matrix34f* getWorldMtxPtrByIndex(s32 index) const;
    const nn::g3d::ResModel* getResModel() const;
    void initUpdateBounding();
    void initJointControllerKeeper(s32 num);

    al::SimpleModelG3D* getModelG3D() const { return mModelG3D; }

    bool mIsCreatedFromOther;
    const char* mModelName;
    const char* mFileName;
    al::SimpleModelG3D* mModelG3D;
    al::Resource* mModelRes;
    al::Resource* mTextureRes;
    al::Resource* mAnimRes;
    al::InitResourceDataAnim* mInitResourceDataAnim;
    al::AnimPlayerSkl* mAnimPlayerSkl;
    al::AnimPlayerMat* mAnimPlayerMat1;
    al::AnimPlayerMat* mAnimPlayerMat2;
    al::AnimPlayerMat* mAnimPlayerMat0;
    al::AnimPlayerVis* mAnimPlayerVis;
    al::JointControllerKeeper* mJointControllerKeeper;
    sead::Matrix34f* mBaseMtx;
    const sead::Matrix34f* mViewMtx;
    const sead::Matrix34f* mInvViewMtx;
    const sead::Matrix44f* mProjMtx;
    const sead::Matrix44f* mViewProjMtx = nullptr;
    sead::Matrix34f* mWorldMtxArray = nullptr;

private:
    alModelCafe(const alModelCafe* pOther);
};

static_assert(sizeof(alModelCafe) == 0xa0);
