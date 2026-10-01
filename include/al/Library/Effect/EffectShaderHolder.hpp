#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "common/aglTextureSampler.h"

namespace agl {
class DrawContext;
class TextureData;
}  // namespace agl

namespace nn::vfx {
class RenderStateSetArg;
struct EmitterDrawArg;
struct EmitterPostCalculateArg;
}  // namespace nn::vfx

namespace al {
class CubeMapDirector;
class EffectEnvParam;
class GraphicsSystemInfo;
class PrePassLightKeeper;
class PtclSystem;
class UniformBlock;
struct UniformBlockLayout;

extern UniformBlockLayout cEffectSceneEffectUboLayout[23];
extern UniformBlockLayout cEffectDitherUboLayout[1];
extern s32 gAreaLoopRepNum[4];

/**
 * @brief Point light settings of an emitter's custom action.
 */
struct CustomActionDataPointLight {
    s32 fadeInFrame;
    s32 fadeOutFrame;
    f32 radius;
    sead::Color4f color;
    u8 colorAnim[0xb0 - 0x1c];
    u8 alphaAnim[0x144 - 0xb0];
    u32 flags;
};

class EffectShaderHolder {
public:
    EffectShaderHolder(PtclSystem* pPtclSystem, agl::DrawContext* pDrawContext,
                       EffectEnvParam* pEnvParam);

    virtual void updateShaderParam(const sead::Matrix34f& rViewMtx,
                                   const sead::Vector2f& rScreenSize);
    virtual void swapUbo();
    virtual void bindCustomShaderUbo(agl::DrawContext* pDrawContext);
    virtual void updateLight() {}
    virtual void renderDeferred(nn::vfx::RenderStateSetArg& rArg) const;

    void setGraphicsSystemInfo(const GraphicsSystemInfo* pInfo);
    void setupTextureColor(const agl::TextureData* pTexture);
    void setupTextureDepth(const agl::TextureData* pTexture);
    void setupTextureWaterDepth(const agl::TextureData* pTexture);
    void setupTextureLight(const agl::TextureData* pTexture);
    void setupTextureDither(const agl::TextureData* pTexture, const sead::Vector2f& rScale);
    void setupTextureExposure(const agl::TextureData* pTexture);
    void setupTextureCubeMap(const CubeMapDirector* pDirector);

    static void CustomShaderRenderStateSetCallbackShared(nn::vfx::RenderStateSetArg& rArg,
                                                         u64 flag);
    static bool CustomShaderRenderStateSetCallbackCommon(nn::vfx::RenderStateSetArg& rArg);
    static bool StandardRenderStateCallback(nn::vfx::RenderStateSetArg& rArg);
    static bool CustomShaderRenderStateSetCallbackNormalMap(nn::vfx::RenderStateSetArg& rArg);
    static bool CustomShaderRenderStateSetCallbackLensFlare(nn::vfx::RenderStateSetArg& rArg);
    static void DrawPathRenderStateSetCallback(nn::vfx::RenderStateSetArg& rArg);
    static bool CustomActionEmitterDrawOverrideCallback(nn::vfx::EmitterDrawArg& rArg);
    static void
    CustomActionEmitterPostCalcCallbackPointLightEmitter(nn::vfx::EmitterPostCalculateArg& rArg);
    static void
    CustomActionEmitterPostCalcCallbackPointLightPerticle(nn::vfx::EmitterPostCalculateArg& rArg);
    static bool AreaLoopDrawOverrideCallback(nn::vfx::EmitterDrawArg& rArg);
    static bool AreaLoopRenderStateSetCallback(nn::vfx::RenderStateSetArg& rArg);

    void setDrawPathRenderStateSetCallbackMRT() { mIsDrawPathMRT = true; }

    void setDrawPathRenderStateSetCallbackSRT(bool isDrawDepthShadow) {
        mIsDrawPathDepthShadow = isDrawDepthShadow;
        mIsDrawPathMRT = false;
    }

    PtclSystem* getPtclSystem() const { return mPtclSystem; }

    agl::DrawContext* getDrawContext() const { return mDrawContext; }

    UniformBlock* getUbo() const { return mUbo; }

    const GraphicsSystemInfo* getGraphicsSystemInfo() const { return mGraphicsSystemInfo; }

protected:
    PtclSystem* mPtclSystem;
    agl::DrawContext* mDrawContext;
    UniformBlock* mUbo = nullptr;
    const GraphicsSystemInfo* mGraphicsSystemInfo = nullptr;

public:
    bool mIsDrawPathMRT = false;
    bool mIsDrawPathDepthShadow = false;

protected:
    sead::Vector2f mDitherScale;
    agl::TextureSampler mColorSampler;
    agl::TextureSampler mDepthSampler;
    agl::TextureSampler mWaterDepthSampler;
    agl::TextureSampler mLightSampler;
    agl::TextureSampler mExposureSampler;
    agl::TextureSampler mDitherSampler;
    agl::TextureSampler mIrradianceSampler[2];
    agl::TextureSampler mCubeMapMirrorSampler[2];
};

static_assert(sizeof(EffectShaderHolder) == 0xe98);

f32 GetSeadRand(u32* pSeed);
void SetAreaLoopRepeatNum(s32 index, s32 num);
s32 GetAreaLoopRepeatNum(s32 index);
}  // namespace al
