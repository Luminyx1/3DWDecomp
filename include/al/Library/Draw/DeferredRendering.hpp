#pragma once

#include <basis/seadTypes.h>
#include <common/aglShaderEnum.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace agl {
class RenderBuffer;
class RenderTargetDepth;
class ShaderProgram;

namespace sdw {
class ShadowPrePass;
}  // namespace sdw
}  // namespace agl

namespace sead {
class Viewport;
}  // namespace sead

namespace al {
class DepthShadowDrawer;
class FullScreenQuadModel;
class GBufferArray;
class GraphicsSystemInfo;
class LiveActorKit;
class SimpleModelEnv;
class UniformBlock;

enum SkyFillType {
    SkyFillType_Fill = 0,
};

/**
 * Deferred shading of the G-buffers: the shadow pre-pass, primitive occlusion, SSAO, the
 * deferred shading pass itself and the sky fill.
 */
class DeferredRendering {
public:
    DeferredRendering(GraphicsSystemInfo* pGraphicsSystemInfo, s32 viewNum);
    ~DeferredRendering();

    bool isUsingDepthShadowMap() const;
    void prepareRenderGBuffer(s32 width, s32 height, GBufferArray* pGBufferArray, s32 viewIndex,
                              const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                              f32 near, f32 far, f32 fovy, f32 aspect,
                              const sead::Vector2f& rOffset, LiveActorKit* pLiveActorKit,
                              DepthShadowDrawer* pDepthShadowDrawer);
    void endRenderGBuffer();
    agl::ShaderMode drawPrePass(LiveActorKit* pLiveActorKit, GBufferArray* pGBufferArray,
                                s32 viewIndex, const agl::RenderBuffer& rRenderBuffer,
                                const agl::RenderTargetDepth& rDepthTarget,
                                agl::ShaderMode shaderMode);
    agl::ShaderMode fillSky(GBufferArray* pGBufferArray, agl::ShaderMode shaderMode) const;
    agl::ShaderMode drawSSAO(s32 viewIndex, GBufferArray* pGBufferArray,
                             const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                             agl::ShaderMode shaderMode);
    agl::ShaderMode drawDeferredShading(LiveActorKit* pLiveActorKit,
                                        const agl::RenderBuffer& rRenderBuffer,
                                        const sead::Viewport& rViewport,
                                        DepthShadowDrawer* pDepthShadowDrawer,
                                        const sead::Matrix34f& rViewMtx,
                                        const sead::Matrix44f& rProjMtx,
                                        GBufferArray* pGBufferArray, s32 viewIndex,
                                        SkyFillType skyFillType, agl::ShaderMode shaderMode,
                                        const SimpleModelEnv* pSimpleModelEnv, bool isFast);
    agl::ShaderMode prepareDeferredShading(LiveActorKit* pLiveActorKit,
                                           const sead::Matrix34f& rViewMtx,
                                           const sead::Matrix44f& rProjMtx,
                                           GBufferArray* pGBufferArray, s32 viewIndex,
                                           DepthShadowDrawer* pDepthShadowDrawer,
                                           agl::ShaderMode shaderMode,
                                           const SimpleModelEnv* pSimpleModelEnv, bool isFast);
    void preDrawGraphics();

private:
    GraphicsSystemInfo* mGraphicsSystemInfo;
    UniformBlock* mSceneUbo = nullptr;
    UniformBlock* mMakeHalfTextureUbo = nullptr;
    const agl::ShaderProgram* mDeferredShadingShader = nullptr;
    const agl::ShaderProgram* mFillDeferredSkyShader = nullptr;
    const agl::ShaderProgram* mMakeHalfDepthTextureShader;
    FullScreenQuadModel* mFullScreenQuadModel;
    agl::sdw::ShadowPrePass* mShadowPrePass = nullptr;
    f32 mShadowDensityScale = 4.0f;
    bool mIsUsingDepthShadowMap = false;
};

static_assert(sizeof(DeferredRendering) == 0x48);

}  // namespace al
