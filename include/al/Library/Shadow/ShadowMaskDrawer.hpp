#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjArray.h>
#include <container/seadPtrArray.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include <agl/common/aglShaderEnum.h>
#include <agl/utility/aglParameter.h>
#include <agl/utility/aglParameterObj.h>

#include "Library/Execute/IUseExecutor.hpp"
#include "Library/Shadow/ShadowMaskBase.hpp"

namespace agl {
class ShaderProgram;
class TextureData;
class TextureSampler;
}  // namespace agl

namespace al {
class ExecuteDirector;
class GBufferArray;
class LiveActor;
class ShadowMaskBase;
class ShadowMaskKeeper;

class ShadowMaskDrawer : public IUseExecutor {
public:
    ShadowMaskDrawer(ShadowMaskKeeper* pKeeper, s32 drawCategory, ExecuteDirector* pDirector);

    void execute() override;
    void draw() const override;
    void drawToTextureData(const agl::TextureData* pTarget, const agl::TextureData* pLightBuffer,
                           agl::ShaderMode mode) const;
    void drawLightScaleToAlbedoGBuffer(agl::ShaderMode mode) const;

    ShadowMaskKeeper* mKeeper;
    s32 mDrawCategory;
    s32 mViewIndex = 0;
    const sead::Matrix34f* mViewMtx = nullptr;
    const sead::Matrix44f* mProjMtx = nullptr;
    void* _28 = nullptr;
    GBufferArray* mGBufferArray = nullptr;
    agl::TextureData* mLinearDepthTexture = nullptr;
    agl::TextureData* mColorTexture = nullptr;
};

static_assert(sizeof(ShadowMaskDrawer) == 0x48);

struct ShadowMaskPrimInfo {
    s32 mType = 0;
    sead::Vector3f _4 = sead::Vector3f::zero;
    sead::Vector3f _10 = {100.0f, 100.0f, 100.0f};
    sead::Vector3f _1c = sead::Vector3f::zero;
    sead::Matrix34f mMtx;
    sead::Color4f mColor = sead::Color4f::cWhite;
    sead::Vector3f mExp = {2.0f, 2.0f, 2.0f};
    f32 _74 = 0.0f;
    f32 _78 = 0.0f;
    s32 _7c = 0;
    s32 mCategory = 0;
    f32 mIntensity = 0.0f;
    f32 mDistYBase = 0.0f;
    u8 _8c[0xa0 - 0x8c] = {};
    f32 _a0 = 1.0f;
    f32 _a4 = 1.0f;
    s32 _a8 = 1;
    s32 _ac = 0;
};

static_assert(sizeof(ShadowMaskPrimInfo) == 0xb0);

class ShadowMaskParam : public agl::utl::IParameterObj {
public:
    void init();
    bool operator==(const ShadowMaskParam& rOther) const;
    ShadowMaskParam& operator=(const ShadowMaskParam& rOther);
    void interp(const ShadowMaskParam& rParamA, const ShadowMaskParam& rParamB, f32 rate);

    s32 getBlockIntensity() const { return *mBlockIntensity; }

    s32 getItemIntensity() const { return *mItemIntensity; }

    s32 getMapObjIntensity() const { return *mMapObjIntensity; }

    s32 getEnemyIntensity() const { return *mEnemyIntensity; }

    s32 getPlayerIntensity() const { return *mPlayerIntensity; }

private:
    agl::utl::Parameter<s32> mBlockIntensity;
    agl::utl::Parameter<s32> mItemIntensity;
    agl::utl::Parameter<s32> mMapObjIntensity;
    agl::utl::Parameter<s32> mEnemyIntensity;
    agl::utl::Parameter<s32> mPlayerIntensity;
};

static_assert(sizeof(ShadowMaskParam) == 0xd0);

class ShadowMaskKeeper {
public:
    u8 getShadowIntensity(s32 category) const;
    void registerShadowMask(ShadowMaskBase* pMask);
    void removeShadowMask(ShadowMaskBase* pMask);
    void addSphere(const sead::Matrix34f& rMtx, const sead::Color4f& rColor, f32 exp, f32 intensity,
                   s32 category);
    void addCylinder(const sead::Matrix34f& rMtx, const sead::Color4f& rColor, f32 expXZ, f32 expY,
                     f32 intensity, f32 distYBase, s32 category);
    void addCastOvalCylinder(const sead::Matrix34f& rMtx, const sead::Color4f& rColor, f32 expXZ,
                             f32 expY, f32 intensity, f32 distYBase, s32 category);
    void addCube(const sead::Matrix34f& rMtx, const sead::Color4f& rColor,
                 const sead::Vector3f& rExp, f32 intensity, f32 distYBase, s32 category,
                 agl::TextureSampler* pSampler);
    void addCastInterpolateCube(const sead::Matrix34f& rMtx, const sead::Color4f& rColor,
                                const sead::Vector3f& rExp, f32 intensity, f32 distYBase,
                                s32 category);

    void init(ExecuteDirector* pDirector);
    void clear();
    void updateMultiCore(s32 coreIndex);
    void setViewInfo(s32 viewIndex, GBufferArray* pGBufferArray, const sead::Matrix34f* pViewMtx,
                     const sead::Matrix44f* pProjMtx);
    void overrideLinearDepthTexture(agl::TextureData* pTexture);
    void overrideColorTexture(agl::TextureData* pTexture);
    bool isExistDrawShadowMask() const;
    bool isExistDrawShadowMaskCategory(s32 category) const;
    void drawLightScaleToAlbedo(agl::ShaderMode mode) const;

    void declare(s32 type, ShadowMaskDrawCategory category) { mDeclareCount[type][category]++; }

    u8 _0[0x230];
    s32 _230;
    u8 _234[0x23c - 0x234];
    u16 _23c;
    ShadowMaskParam mCurrentParam;
    u8 _310[0x658 - 0x310];
    bool _658;
    u8 _659[0x678 - 0x659];
    sead::PtrArray<ShadowMaskDrawer> mDrawers;
    ShadowMaskArray mEvenTargetMasks;
    ShadowMaskArray mMasks;
    sead::ObjArray<ShadowMaskPrimInfo> mPrimInfoArray[3][17];
    u8 _d08[0x15b0 - 0xd08];
    s32 mDeclareCount[6][17];
    u8 _1748[0x1754 - 0x1748];
    s32 _1754;
    s32 mEvenTargetMaskLastIndex;
    s32 mMaskLastIndex;
};

}  // namespace al

namespace ShadowMaskFunction {
al::ShadowMaskKeeper* getShadowMaskKeeper(const al::LiveActor* pActor);
}
