#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>
#include "utility/aglParameter.h"
#include "utility/aglParameterIO.h"
#include "utility/aglParameterObj.h"

namespace sead {
class Heap;
}

namespace al {
class GraphicsParamFilePath;
class GraphicsSystemInfo;
class Resource;
class UniformBlock;

/**
 * A light color that is either added to or multiplied with a base color, editable through
 * parameters.
 */
class LightInfo {
public:
    LightInfo(const sead::Color4f& rColor, s32 applyMode, agl::utl::ParameterObj* pParamObj,
              const char* pName, const char* pLabel, const char* pCategoryName);

    void calcApplyColor(sead::Color4f* pOut, const sead::Color4f& rColor) const;

    /// Adds the light color to (apply mode 0) or multiplies it with a base color.
    void applyColor(sead::Color4f* pOut, const sead::Color4f& rColor) const {
        *pOut = rColor;

        if (**mApplyMode == 0) {
            *pOut += **mColor;
        } else {
            *pOut *= **mColor;
        }
    }

private:
    agl::utl::Parameter<sead::Color4f>* mColor = nullptr;
    agl::utl::Parameter<s32>* mApplyMode = nullptr;
    sead::FixedSafeString<128> mColorLabel;
    sead::FixedSafeString<64> mApplyModeLabel;
};

static_assert(sizeof(LightInfo) == 0x100);

/**
 * The light settings of a light category: sky lights, directional light and model light scale.
 */
class CategoryLightInfo {
public:
    CategoryLightInfo(bool isDefault, const char* pName, f32 dirIntensity);

    void calcApplyModelLightColor(sead::Color4f* pOut, const sead::Color4f& rColor) const;

    agl::utl::ParameterObj* getParamObj() const { return mParamObj; }

    const char* getLightName() const { return mLightName->cstr(); }

    sead::FixedSafeString<64>& getLightNameString() { return *mLightName; }

    sead::FixedSafeString<256>& getName() { return mName; }

private:
    sead::FixedSafeString<256> mName;
    agl::utl::ParameterObj* mParamObj;
    const char* mCategoryName;
    LightInfo* mSkyLightInfo = nullptr;
    LightInfo* mSky2LightInfo = nullptr;
    LightInfo* mDirLightInfo;
    agl::utl::Parameter<f32> mIllumiCoef;
    agl::utl::Parameter<f32> mZeroIllumiCoef;
    agl::utl::Parameter<sead::FixedSafeString<64>> mLightName;
    agl::utl::Parameter<sead::Color4f> mModelLightScale;
    agl::utl::Parameter<s32> mUsingModelLightScale;
    bool mIsDefault;
};

static_assert(sizeof(CategoryLightInfo) == 0x240);

/**
 * Holds the default category light info and the named ones loaded from the stage resource.
 */
class CategoryLightInfoHolder : public agl::utl::IParameterIO {
public:
    CategoryLightInfoHolder(const char* pName, f32 dirIntensity);

    void initStageResource(const Resource* pResource);
    const CategoryLightInfo* tryGetLightInfo(const char* pName) const;

private:
    GraphicsParamFilePath* mParamFilePath = nullptr;
    CategoryLightInfo* mDefaultInfo;
    sead::FixedPtrArray<CategoryLightInfo, 64> mInfos;
};

static_assert(sizeof(CategoryLightInfoHolder) == 0x440);

/**
 * Per-view uniform blocks holding the camera and light environment of simple models.
 */
class SimpleModelEnv {
public:
    SimpleModelEnv();
    ~SimpleModelEnv();

    void initialize(s32 bufferNum, const GraphicsSystemInfo* pInfo, sead::Heap* pHeap);
    void updateEnv(s32 index, const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                   const sead::Vector2f& rProjOffset, const sead::Vector2f& rScreenSize, f32 near,
                   f32 far, f32 fovy, f32 aspect, sead::Vector2f orthoSize,
                   const LightInfo* pSkyLightInfo, const LightInfo* pSky2LightInfo,
                   const LightInfo* pDirLightInfo, f32 rate, const char* pDirLightName);
    void swapBuffer();
    void prepareModelDraw(s32 index) const;

private:
    sead::PtrArray<UniformBlock> mUniformBlocks;
    const GraphicsSystemInfo* mGraphicsSystemInfo = nullptr;
};

static_assert(sizeof(SimpleModelEnv) == 0x18);

}  // namespace al
