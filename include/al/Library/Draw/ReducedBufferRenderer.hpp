#pragma once

#include <basis/seadTypes.h>
#include <common/aglRenderTarget.h>
#include <common/aglTextureEnum.h>
#include <common/aglTextureSampler.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace agl {
class DrawContext;
class ShaderProgram;
class TextureData;
}  // namespace agl

namespace al {
class EffectSystem;
class FullScreenTriangle;
class RenderVariables;
class ShaderHolder;

/**
 * Renders effects into reduced (half resolution) color buffers and composes them back onto the
 * full resolution color buffer.
 */
class ReducedBufferRenderer {
public:
    typedef void (*DrawEffectFunc)(const EffectSystem* pEffectSystem,
                                   const sead::Matrix44f& rProjMtx,
                                   const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy);

    ReducedBufferRenderer(const ShaderHolder* pShaderHolder,
                          const FullScreenTriangle* pFullScreenTriangle);
    ~ReducedBufferRenderer();

    void freeReducedBuffer();
    void freeReducedBufferHdr();
    void freeReducedDepthBuffers();
    void createReducedDepthBuffers(agl::DrawContext* pDrawContext, const sead::Vector2i& rSize);
    agl::TextureData* createReducedBuffer(agl::DrawContext* pDrawContext, const char* pName,
                                          agl::TextureFormat format, s32 width, s32 height);
    agl::TextureData* createReducedBufferHdr(agl::DrawContext* pDrawContext, const char* pName,
                                             agl::TextureFormat format, s32 width, s32 height);
    void drawReducedDepth(f32 near, f32 far, const agl::RenderTargetDepth& rSrcDepthTarget,
                          const agl::TextureData* pSrcDepthTexture, bool isOutLinear);
    void drawReducedDepth(f32 near, f32 far, const agl::RenderTargetDepth& rSrcDepthTarget,
                          const agl::TextureData* pSrcDepthTexture,
                          const agl::RenderTargetDepth& rDstDepthTarget,
                          const agl::TextureData* pDstLinearDepth);
    void drawCompose(const agl::TextureData* pTarget, const agl::TextureData* pViewDepth,
                     const agl::TextureData* pHalfViewDepth, f32 near, f32 far, bool isHdr,
                     bool isAdjust);
    void activateAdjustParam(const agl::ShaderProgram* pProgram,
                             const agl::TextureData* pViewDepth,
                             const agl::TextureData* pHalfViewDepth);
    void drawComposeFullScreen(const agl::TextureData* pSource, f32 near, f32 far);
    void RenderEffects(const RenderVariables& rRenderVariables,
                       const agl::TextureData* pViewDepth, const agl::TextureData* pTarget,
                       DrawEffectFunc drawFunc, const sead::Matrix44f& rProjMtx,
                       const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy,
                       bool isUseLinearDepth, bool isHdr);
    bool draw(const RenderVariables& rRenderVariables, const agl::TextureData* pViewDepth,
              const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx, f32 near,
              f32 far, f32 fovy, bool isUseLinearDepth);

    void setEnable(bool isEnable) { mIsEnable = isEnable; }

    void setEnableHdr(bool isEnable) { mIsEnableHdr = isEnable; }

    bool isEnable() const { return mIsEnable; }

    bool isEnableHdr() const { return mIsEnableHdr; }

private:
    agl::ShaderProgram* mQuarterResBufferShader = nullptr;
    agl::ShaderProgram* mQuarterResBufferLinearDepthShader = nullptr;
    agl::ShaderProgram* mQuarterResBufferComposeShader = nullptr;
    agl::ShaderProgram* mHalfResBufferDepthShader = nullptr;
    agl::ShaderProgram* mHalfResBufferComposeShader = nullptr;
    agl::ShaderProgram* mQuarterResBufferComposeShader2 = nullptr;
    agl::ShaderProgram* mFullResBufferComposeShader = nullptr;
    const FullScreenTriangle* mFullScreenTriangle;
    agl::TextureData* mReducedBuffer = nullptr;
    agl::TextureData* mReducedBufferHdr = nullptr;
    agl::RenderTargetDepth mHalfDepthTarget;
    agl::TextureData* mHalfDepthTexture = nullptr;
    agl::TextureSampler mHalfDepthSampler;
    agl::RenderTargetDepth mQuarterDepthTarget;
    agl::TextureData* mQuarterDepthTexture = nullptr;
    agl::TextureSampler mQuarterDepthSampler;
    agl::TextureData* mHalfLinearDepth = nullptr;
    agl::TextureData* mQuarterLinearDepth = nullptr;
    bool mIsEnable = false;
    bool mIsEnableHdr = false;
};

static_assert(sizeof(ReducedBufferRenderer) == 0x648);

}  // namespace al
