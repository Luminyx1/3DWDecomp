#pragma once

#include <basis/seadTypes.h>
#include <common/aglRenderBuffer.h>
#include <common/aglRenderTarget.h>
#include <common/aglTextureSampler.h>
#include <gfx/seadViewport.h>

namespace agl {
class TextureData;

namespace lght {
class LightPrePass;
}  // namespace lght
}  // namespace agl

namespace al {
class GraphicsSystemInfo;
class LiveActorKit;
class SimpleModelEnv;

extern bool hIsFast;

/**
 * Per-view render state: the main color/depth render buffer, its viewport and samplers.
 */
class RenderVariables {
public:
    RenderVariables(GraphicsSystemInfo* pGraphicsSystemInfo, LiveActorKit* pLiveActorKit,
                    const SimpleModelEnv* pSimpleModelEnv, s32 viewIndex, s32 width, s32 height,
                    bool isFixedSize, bool isAllocDepthBuffer);
    ~RenderVariables();

    void allocDepthBuffer();
    void allocColorBuffer(agl::lght::LightPrePass* pLightPrePass, bool isClear);
    void bindRenderBuffer() const;

    s32 getWidth() const { return mWidth; }

    s32 getHeight() const { return mHeight; }

    GraphicsSystemInfo* getGraphicsSystemInfo() const { return mGraphicsSystemInfo; }

    LiveActorKit* getLiveActorKit() const { return mLiveActorKit; }

    const SimpleModelEnv* getSimpleModelEnv() const { return mSimpleModelEnv; }

    s32 getViewIndex() const { return mViewIndex; }

    agl::lght::LightPrePass* getLightPrePass() const { return mLightPrePass; }

    const agl::RenderBuffer& getRenderBuffer() const { return mRenderBuffer; }

    agl::RenderBuffer& getRenderBuffer() { return mRenderBuffer; }

    const sead::Viewport& getViewport() const { return mViewport; }

    const agl::RenderTargetColor& getColorTarget() const { return mColorTarget; }

    const agl::RenderTargetDepth& getDepthTarget() const { return mDepthTarget; }

    const agl::TextureData* getColorTexture() const { return mColorTexture; }

    const agl::TextureData* getDepthTexture() const { return mDepthTexture; }

    const agl::TextureSampler& getDepthSampler() const { return mDepthSampler; }

    bool isFast() const { return mIsFast; }

    bool isAllocDepthBuffer() const { return mIsAllocDepthBuffer; }

    bool isAllocColorBuffer() const { return mIsAllocColorBuffer; }

    s32 mWidth = -1;
    s32 mHeight = -1;
    GraphicsSystemInfo* mGraphicsSystemInfo;
    LiveActorKit* mLiveActorKit;
    const SimpleModelEnv* mSimpleModelEnv;
    s32 mViewIndex;
    void* _28 = nullptr;
    agl::lght::LightPrePass* mLightPrePass = nullptr;
    agl::RenderBuffer mRenderBuffer;
    sead::Viewport mViewport;
    agl::RenderTargetColor mColorTarget;
    agl::TextureData* mColorTexture = nullptr;
    agl::RenderTargetDepth mDepthTarget;
    agl::TextureData* mDepthTexture = nullptr;
    agl::TextureSampler mDepthSampler;
    agl::TextureData* mSubTexture = nullptr;
    agl::TextureSampler mSubSampler;
    void* _6b0 = nullptr;
    void* _6b8 = nullptr;
    bool _6c0 = false;
    bool mIsFast = hIsFast;
    bool mIsAllocDepthBuffer;
    bool mIsAllocColorBuffer = false;
};

static_assert(sizeof(RenderVariables) == 0x6c8);

}  // namespace al
