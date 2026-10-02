#include "Library/Debug/Render/RenderBufferAttacher.hpp"

#include <attributes.h>
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
 * Sizes the render buffer from the first color texture and attaches the color and depth targets.
 * @param pColor0 Texture for color target 0.
 * @param pColor1 Texture for color target 1, or nullptr.
 * @param pColor2 Texture for color target 2, or nullptr.
 * @param pColor3 Texture for color target 3, or nullptr.
 * @param pDepth Depth target to attach, or nullptr.
 */
ALWAYS_INLINE inline void
RenderBufferAttacher::attach_(const agl::TextureData* pColor0, const agl::TextureData* pColor1,
                              const agl::TextureData* pColor2, const agl::TextureData* pColor3,
                              const agl::RenderTargetDepth* pDepth) {
    const agl::TextureData* colors[4] = {pColor0, pColor1, pColor2, pColor3};

    setRenderBufferSizeFromTexture(mRenderBuffer, *pColor0);

    agl::RenderBuffer* renderBuffer = mRenderBuffer;
    renderBuffer->setRenderTargetColorNullAll();
    renderBuffer->setRenderTargetDepth(nullptr);

    for (s32 i = 0; i < 4; i++) {
        mColorTextures[i] = colors[i];

        if (colors[i] != nullptr) {
            mColorTargets[i].applyTextureData(*colors[i]);
            mRenderBuffer->setRenderTargetColor(&mColorTargets[i], i);
        }
    }

    if (pDepth != nullptr) {
        mRenderBuffer->setRenderTargetDepth(const_cast<agl::RenderTargetDepth*>(pDepth));
    }
}

/**
 * Attaches the targets to the render buffer and binds it with the framework's draw context.
 * @param pRenderBuffer Render buffer to attach the targets to.
 * @param pColor0 Texture for color target 0, also giving the render buffer size.
 * @param pColor1 Texture for color target 1, or nullptr.
 * @param pColor2 Texture for color target 2, or nullptr.
 * @param pColor3 Texture for color target 3, or nullptr.
 * @param pDepth Depth target to attach, or nullptr.
 */
RenderBufferAttacher::RenderBufferAttacher(agl::RenderBuffer* pRenderBuffer,
                                           const agl::TextureData* pColor0,
                                           const agl::TextureData* pColor1,
                                           const agl::TextureData* pColor2,
                                           const agl::TextureData* pColor3,
                                           const agl::RenderTargetDepth* pDepth)
    : RenderBufferAttacher(GameFrameworkNx::getAglDrawContext(), pRenderBuffer, pColor0, pColor1,
                           pColor2, pColor3, pDepth) {}

/**
 * Attaches the targets to the render buffer, applies a full-size viewport and binds it.
 * @param pDrawContext Draw context used to apply the viewport and bind the render buffer.
 * @param pRenderBuffer Render buffer to attach the targets to.
 * @param pColor0 Texture for color target 0, also giving the render buffer size.
 * @param pColor1 Texture for color target 1, or nullptr.
 * @param pColor2 Texture for color target 2, or nullptr.
 * @param pColor3 Texture for color target 3, or nullptr.
 * @param pDepth Depth target to attach, or nullptr.
 */
RenderBufferAttacher::RenderBufferAttacher(agl::DrawContext* pDrawContext,
                                           agl::RenderBuffer* pRenderBuffer,
                                           const agl::TextureData* pColor0,
                                           const agl::TextureData* pColor1,
                                           const agl::TextureData* pColor2,
                                           const agl::TextureData* pColor3,
                                           const agl::RenderTargetDepth* pDepth)
    : mRenderBuffer(pRenderBuffer) {
    attach_(pColor0, pColor1, pColor2, pColor3, pDepth);

    sead::Viewport viewport(*mRenderBuffer);
    viewport.apply(pDrawContext, *mRenderBuffer);
    mRenderBuffer->bind(pDrawContext);
}

/**
 * Attaches the targets to the render buffer, applies the given viewport and binds it with the
 * framework's draw context.
 * @note The original code never stores pRenderBuffer, so mRenderBuffer stays uninitialized.
 * @param pRenderBuffer Render buffer to attach the targets to (unused).
 * @param pViewport Viewport to apply, or nullptr to keep the current one.
 * @param pColor0 Texture for color target 0, also giving the render buffer size.
 * @param pColor1 Texture for color target 1, or nullptr.
 * @param pColor2 Texture for color target 2, or nullptr.
 * @param pColor3 Texture for color target 3, or nullptr.
 * @param pDepth Depth target to attach, or nullptr.
 */
RenderBufferAttacher::RenderBufferAttacher(agl::RenderBuffer* pRenderBuffer,
                                           const sead::Viewport* pViewport,
                                           const agl::TextureData* pColor0,
                                           const agl::TextureData* pColor1,
                                           const agl::TextureData* pColor2,
                                           const agl::TextureData* pColor3,
                                           const agl::RenderTargetDepth* pDepth) {
    attach_(pColor0, pColor1, pColor2, pColor3, pDepth);

    if (pViewport != nullptr) {
        pViewport->apply(GameFrameworkNx::getAglDrawContext(), *mRenderBuffer);
    }

    mRenderBuffer->bind(GameFrameworkNx::getAglDrawContext());
}

/**
 * Attaches the given slice of the textures to the render buffer and binds it with the
 * framework's draw context.
 * @param pRenderBuffer Render buffer to attach the targets to.
 * @param slice Texture slice to render into.
 * @param pColor0 Texture for color target 0, also giving the render buffer size.
 * @param pColor1 Texture for color target 1, or nullptr.
 * @param pColor2 Texture for color target 2, or nullptr.
 * @param pColor3 Texture for color target 3, or nullptr.
 * @param pDepth Depth target to attach, or nullptr.
 */
RenderBufferAttacher::RenderBufferAttacher(agl::RenderBuffer* pRenderBuffer, s32 slice,
                                           const agl::TextureData* pColor0,
                                           const agl::TextureData* pColor1,
                                           const agl::TextureData* pColor2,
                                           const agl::TextureData* pColor3,
                                           const agl::RenderTargetDepth* pDepth)
    : mRenderBuffer(pRenderBuffer) {
    const agl::TextureData* colors[4] = {pColor0, pColor1, pColor2, pColor3};

    setRenderBufferSizeFromTexture(mRenderBuffer, *pColor0);

    agl::RenderBuffer* renderBuffer = mRenderBuffer;
    renderBuffer->setRenderTargetColorNullAll();
    renderBuffer->setRenderTargetDepth(nullptr);

#pragma clang loop unroll(full)
    for (s32 i = 0; i < 4; i++) {
        mColorTextures[i] = colors[i];

        if (colors[i] != nullptr) {
            mColorTargets[i].applyTextureData(*colors[i]);
            mColorTargets[i].setSlice(slice);
            mColorTargets[i].setMipLevel(0);
            mRenderBuffer->setRenderTargetColor(&mColorTargets[i], i);
        }
    }

    if (pDepth != nullptr) {
        mRenderBuffer->setRenderTargetDepth(const_cast<agl::RenderTargetDepth*>(pDepth));
    }

    sead::Viewport viewport(*mRenderBuffer);
    viewport.apply(GameFrameworkNx::getAglDrawContext(), *mRenderBuffer);
    mRenderBuffer->bind(GameFrameworkNx::getAglDrawContext());
}

/**
 * Detaches all targets from the render buffer and invalidates the GPU cache of the attached
 * color targets.
 */
RenderBufferAttacher::~RenderBufferAttacher() {
    agl::RenderBuffer* renderBuffer = mRenderBuffer;
    renderBuffer->setRenderTargetColorNullAll();
    renderBuffer->setRenderTargetDepth(nullptr);

    for (s32 i = 0; i < 4; i++) {
        if (mColorTextures[i] != nullptr) {
            mColorTargets[i].invalidateGPUCache(GameFrameworkNx::getAglDrawContext());
        }
    }
}

}  // namespace al
