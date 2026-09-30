#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <prim/seadBitFlag.h>

#include "common/aglGPUMemAddr.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureData.h"
#include "common/aglTextureEnum.h"
#include "common/aglTextureSampler.h"

namespace sead {
class Heap;
}

namespace agl {
class DrawContext;
}

namespace agl::env {

class CubeMap : public TextureSampler {
public:
    CubeMap();
    ~CubeMap();

    void destroyBuffer();
    void initialize(sead::Heap* pHeap, TextureFormat format, u32 width, u32 mipLevelNum);
    void initialize(GPUMemVoidAddr addr, u32 size, TextureFormat format, u32 width,
                    u32 mipLevelNum);
    void initialize(sead::Heap* pHeap, TextureFormat format, u32 width, u32 arrayNum,
                    u32 mipLevelNum);
    void initialize(GPUMemVoidAddr addr, u32 size, TextureFormat format, u32 width, u32 arrayNum,
                    u32 mipLevelNum);
    void initialize(const TextureData& rTextureData);

    bool begin(DrawContext* pDrawContext, bool useColorBuffer, bool useDepthBuffer);
    bool preDraw(DrawContext* pDrawContext, u32 face, u32 mipLevel);
    bool preDraw(DrawContext* pDrawContext, u32 slice, u32 face, u32 mipLevel);
    void bindRenderBuffer(DrawContext* pDrawContext, const TextureData* pColor, u32 mipLevel,
                          u32 slice, const TextureData* pDepth, GPUMemVoidAddr zcullBuffer,
                          bool invalidate);
    void postDraw(DrawContext* pDrawContext, bool flip);
    void clear(DrawContext* pDrawContext, u32 slice, const sead::Color4f& rColor);
    void end(DrawContext* pDrawContext);

    void renderToMipMapUnit(DrawContext* pDrawContext, const TextureSampler& rSrc,
                            const TextureSampler& rWork, u32 srcSlice, u32 srcMipLevel,
                            u32 dstSlice, u32 dstMipLevel, f32 sigma);
    void renderToMipMapImpl(DrawContext* pDrawContext, const TextureSampler& rSrc, u32 srcSlice,
                            u32 srcMipLevel, u32 dstSlice, u32 dstMipLevel, f32 sigma,
                            u32 count);
    void generateMipMapImpl(DrawContext* pDrawContext, u32 slice, u32 startMipLevel,
                            u32 endMipLevel, u32 count, f32 sigma);
    void renderToMipMap(DrawContext* pDrawContext, const TextureSampler& rSrc, u32 srcMipLevel,
                        u32 dstMipLevel, u32 count, f32 sigma);
    void renderToMipMap(DrawContext* pDrawContext, const TextureSampler& rSrc, u32 srcSlice,
                        u32 srcMipLevel, u32 dstSlice, u32 dstMipLevel, u32 count, f32 sigma);
    void generateMipMap(DrawContext* pDrawContext, u32 startMipLevel, u32 endMipLevel, u32 count,
                        f32 sigma, bool seamless);
    void generateMipMapArray(DrawContext* pDrawContext, u32 slice, u32 startMipLevel,
                             u32 endMipLevel, u32 count, f32 sigma, bool seamless);
    void renderIrradiance(DrawContext* pDrawContext, u32 slice, u32 srcMipLevel,
                          u32 dstMipLevel);

    const TextureData& getCubeTextureData() const { return mTextureData; }
    const RenderBuffer& getRenderBuffer() const { return mRenderBuffer; }

    static const sead::Matrix34f scDrawCubeMapScale[2];

private:
    void initialize_();

    TextureData mTextureData;
    GPUMemVoidAddr mImageAddr;
    GPUMemVoidAddr mMipAddr;
    TextureData* mColorBuffer = nullptr;
    TextureData mColorTexture;
    TextureData* mDepthBuffer = nullptr;
    GPUMemVoidAddr mZCullBuffer;
    TextureData mDepthTexture;
    s32 mCurrentSlice = -1;
    s32 mCurrentFace = -1;
    s32 mCurrentMipLevel = -1;
    s32 mCurrentSize = -1;
    RenderBuffer mRenderBuffer;
    RenderTargetColor mRenderTargetColor;
    RenderTargetDepth mRenderTargetDepth;
    sead::BitFlag8 mFlag;
};
static_assert(sizeof(CubeMap) == 0x8b0);

}  // namespace agl::env
