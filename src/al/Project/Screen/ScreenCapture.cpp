#include "Project/Screen/ScreenCapture.hpp"

#include <common/aglDrawContext.h>
#include <common/aglGPUMemBlock.h>
#include <common/aglRenderBuffer.h>
#include <common/aglTextureData.h>
#include <common/aglTextureSampler.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>

#include <nvn/nvn_FuncPtrInline.h>
#include <utility/aglImageFilter2D.h>

#include "Library/Draw/BlurFilter.hpp"
#include "Library/Memory/Util.hpp"

namespace al {
/**
 * Creates a screen capture with its own texture.
 * @param width texture width
 * @param height texture height
 */
ScreenCapture::ScreenCapture(s32 width, s32 height) {
    mTextureData = new agl::TextureData();
    mTextureData->initialize_(agl::TextureType::cTextureType_2D, static_cast<agl::TextureFormat>(0x22), width,
                              height, 1, 1, static_cast<agl::TextureAttribute>(0),
                              static_cast<agl::MultiSampleType>(0), true);
    u32 size = mTextureData->getImageByteSize();
    sead::Heap* heap = getCurrentHeap();
    s32 alignment = mTextureData->getAlignment();
    auto* memBlock = new (heap, 8) agl::GPUMemBlock<u8>;
    memBlock->allocBuffer_(size, heap, alignment, static_cast<agl::MemoryAttribute>(0));
    mImageAddr = agl::GPUMemVoidAddr(*memBlock, 0);
    mTextureData->setImagePtr(mImageAddr);
}

/**
 * Initializes the blur applied when drawing the capture.
 * @param rFrameBuffer frame buffer
 * @param rViewport viewport
 * @param scale reduce scale of the blur
 */
void ScreenCapture::initBlur(const sead::LogicalFrameBuffer& rFrameBuffer,
                             const sead::Viewport& rViewport,
                             agl::utl::ImageFilter2D::ReduceScale scale) {
    mBlurFilter = new BlurFilter();
    mBlurFilter->init(mTextureData->getWidth(0), mTextureData->getHeight(0), rFrameBuffer,
                      rViewport, scale);
    mBlurFilter->setBlurType(1, 1);
    mBlurFilter->setIteration(10);
}

/**
 * Enables or disables the blur.
 * @param isEnable whether the blur is enabled
 */
void ScreenCapture::enableBlur(bool isEnable) {
    if (mBlurFilter) {
        mBlurFilter->setEnable(isEnable);
    }
}

/**
 * Copies the color target of a render buffer into the capture texture.
 * @param pDrawContext draw context
 * @param pRenderBuffer render buffer to capture
 */
void ScreenCapture::copyImageFromFrameBuffer(agl::DrawContext* pDrawContext,
                                             const agl::RenderBuffer* pRenderBuffer) {
    const agl::TextureData* src = pRenderBuffer->getRenderTargetColor();
    s32 srcWidth = src->getMipWidth(0);
    s32 srcHeight = src->getMipHeight(0);
    const agl::TextureData* dst = mTextureData;
    s32 dstWidth = dst->getMipWidth(0);
    s32 dstHeight = dst->getMipHeight(0);
    NVNcopyRegion srcRegion = {0, 0, 0, srcWidth, srcHeight, 1};
    NVNcopyRegion dstRegion = {0, 0, 0, dstWidth, dstHeight, 1};

    NVNtextureView srcView;
    nvnTextureViewSetDefaults(&srcView);
    nvnTextureViewSetLevels(&srcView, 0, 1);

    NVNtextureView dstView;
    nvnTextureViewSetDefaults(&dstView);
    nvnTextureViewSetLevels(&dstView, 0, 1);

    nvnCommandBufferCopyTextureToTexture(pDrawContext->getNvnCommandBuffer(),
                                         src->getTexture().getTexture(), &srcView, &srcRegion,
                                         mTextureData->getTexture().getTexture(), &dstView,
                                         &dstRegion, 1);
}

/**
 * Draws the capture texture to a render buffer, blurred if the blur is enabled.
 * @param pDrawContext draw context
 * @param pRenderBuffer render buffer to draw to
 */
void ScreenCapture::drawCaptureImage(agl::DrawContext* pDrawContext,
                                     const agl::RenderBuffer* pRenderBuffer) const {
    pRenderBuffer->bind(pDrawContext);
    sead::Viewport viewport(*pRenderBuffer);
    viewport.apply(pDrawContext, *pRenderBuffer);

    sead::GraphicsContext graphicsContext;
    graphicsContext.setAlphaTestEnable(false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.apply(pDrawContext);

    agl::TextureSampler sampler;
    pRenderBuffer->getRenderTargetColor()->getMipHeight(0);
    const agl::TextureData* texture = mTextureData;
    sead::Vector2f scale(pRenderBuffer->getVirtualSize().x / texture->getWidth(0),
                         pRenderBuffer->getVirtualSize().y / texture->getHeight(0));
    sampler.applyTextureData(*mTextureData);
    if (mBlurFilter && mBlurFilter->isEnable()) {
        mBlurFilter->draw(pDrawContext, *pRenderBuffer, sampler);
    } else {
        agl::utl::ImageFilter2D::drawTexture(pDrawContext, sampler, viewport, scale,
                                             sead::Vector2f::zero);
    }
}

/**
 * Destroys the capture and its texture.
 */
ScreenCapture::~ScreenCapture() {
    if (mBlurFilter) {
        delete mBlurFilter;
        mBlurFilter = nullptr;
    }

    if (mImageAddr.getMemoryPool()) {
        mImageAddr.deleteGPUMemBlock();
        mImageAddr.invalidate();
    }

    if (mTextureData) {
        delete mTextureData;
        mTextureData = nullptr;
    }
}
}  // namespace al
