#include "Library/Debug/Render/RenderBufferDepthAttacher.hpp"

#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <common/aglTextureData.h>
#include <gfx/seadViewport.h>
#include <math/seadBoundBox.h>
#include <math/seadVector.h>

#include "Library/Framework/GameFrameworkNx.hpp"

namespace {

/**
 * Sizes a render buffer so that its virtual size and physical area cover a whole texture.
 * @param pRenderBuffer Render buffer to resize.
 * @param rTextureData Texture whose base mip level gives the size.
 */
inline void setRenderBufferSizeFromTexture(agl::RenderBuffer* pRenderBuffer,
                                           const agl::TextureData& rTextureData) {
    pRenderBuffer->setVirtualSize(
        sead::Vector2f(rTextureData.getWidth(0), rTextureData.getHeight(0)));
    pRenderBuffer->setPhysicalArea(
        sead::BoundBox2f(0.0f, 0.0f, rTextureData.getWidth(0), rTextureData.getHeight(0)));
}

}  // namespace

namespace al {

/**
 * Attaches the depth texture with its Z-cull buffer and the color textures to the render buffer,
 * applies a full-size viewport and binds it with the framework's draw context.
 * @param pRenderBuffer Render buffer to attach the targets to.
 * @param pDepth Depth texture to attach.
 * @param zCullBuffer Z-cull buffer of the depth target.
 * @param pColor0 Texture for color target 0, or nullptr. Gives the render buffer size if set,
 * otherwise the depth texture does.
 * @param pColor1 Texture for color target 1, or nullptr.
 * @param pColor2 Texture for color target 2, or nullptr.
 * @param pColor3 Texture for color target 3, or nullptr.
 */
RenderBufferDepthAttacher::RenderBufferDepthAttacher(agl::RenderBuffer* pRenderBuffer,
                                                     const agl::TextureData* pDepth,
                                                     agl::ConstGPUMemVoidAddr zCullBuffer,
                                                     const agl::TextureData* pColor0,
                                                     const agl::TextureData* pColor1,
                                                     const agl::TextureData* pColor2,
                                                     const agl::TextureData* pColor3)
    : mRenderBuffer(pRenderBuffer) {
    mDepthTexture = pDepth;

    setRenderBufferSizeFromTexture(mRenderBuffer, pColor0 != nullptr ? *pColor0 : *pDepth);

    agl::RenderBuffer* renderBuffer = mRenderBuffer;
    renderBuffer->setRenderTargetColorNullAll();
    renderBuffer->setRenderTargetDepth(nullptr);

    const agl::TextureData* colors[4] = {pColor0, pColor1, pColor2, pColor3};

    for (s32 i = 0; i < 4; i++) {
        mColorTextures[i] = colors[i];

        if (colors[i] != nullptr) {
            mColorTargets[i].applyTextureData(*colors[i]);
            mRenderBuffer->setRenderTargetColor(&mColorTargets[i], i);
        }
    }

    mDepthTarget.applyTextureData(*pDepth);
    mDepthTarget.setZCullBuffer(agl::GPUMemVoidAddr(zCullBuffer));
    mRenderBuffer->setRenderTargetDepth(&mDepthTarget);

    sead::Viewport viewport(*mRenderBuffer);
    viewport.apply(GameFrameworkNx::getAglDrawContext(), *mRenderBuffer);
    mRenderBuffer->bind(GameFrameworkNx::getAglDrawContext());
}

/**
 * Detaches all targets from the render buffer and invalidates the GPU cache of the attached
 * color targets and the depth target.
 */
RenderBufferDepthAttacher::~RenderBufferDepthAttacher() {
    agl::RenderBuffer* renderBuffer = mRenderBuffer;
    renderBuffer->setRenderTargetColorNullAll();
    renderBuffer->setRenderTargetDepth(nullptr);

    for (s32 i = 0; i < 4; i++) {
        if (mColorTextures[i] != nullptr) {
            mColorTargets[i].invalidateGPUCache(GameFrameworkNx::getAglDrawContext());
        }
    }

    mDepthTarget.invalidateGPUCache(GameFrameworkNx::getAglDrawContext());
}

}  // namespace al
