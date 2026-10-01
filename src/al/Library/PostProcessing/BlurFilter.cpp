#include "Library/PostProcessing/BlurFilter.hpp"

#include <gfx/seadGraphicsContext.h>
#include <heap/seadHeapMgr.h>
#include "common/aglDrawContext.h"
#include "common/aglGPUMemBlock.h"

namespace al {
/**
 * Constructs the blur filter with default settings.
 */
BlurFilter::BlurFilter() = default;

/**
 * Destroys the blur filter.
 */
BlurFilter::~BlurFilter() = default;

/**
 * Allocates the two work textures from an explicit source size.
 * @param width source width
 * @param height source height
 * @param rFrameBuffer source frame buffer
 * @param rViewport source viewport
 * @param scale reduce scale of the work textures
 */
void BlurFilter::init(s32 width, s32 height, const sead::LogicalFrameBuffer& rFrameBuffer,
                      const sead::Viewport& rViewport, agl::utl::ImageFilter2D::ReduceScale scale) {
    mReduceScale = scale;
    mFrameBuffer = rFrameBuffer;
    mViewport = rViewport;

    sead::Vector2f size;
    rViewport.getOnFrameBufferSize(&size, rFrameBuffer);

    for (s32 i = 0; i < 2; i++) {
        sead::Vector2f bufferSize;
        rViewport.getOnFrameBufferSize(&bufferSize, rFrameBuffer);
        s32 reducedWidth = width >> mReduceScale;
        s32 reducedHeight = height >> mReduceScale;
        agl::TextureData& rTexture = mTexture[i];
        rTexture.initialize_(agl::TextureType::cTextureType_2D,
                             agl::TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, reducedWidth,
                             reducedHeight, 1, 1, agl::TextureAttribute(0),
                             agl::MultiSampleType(0), true);
        u32 storageSize = rTexture.getSurface().mStorageSize;
        sead::Heap* pHeap = sead::HeapMgr::instance()->getCurrentHeap();
        u32 alignment = rTexture.getSurface().mAlignment;
        auto* pBlock = new (pHeap, 8) agl::GPUMemBlock<u8>;
        pBlock->allocBuffer_(storageSize, pHeap, alignment,
                             agl::MemoryAttribute::CompressibleMemory);
        mImage[i] = agl::GPUMemAddr<u8>(*pBlock, 0);
        rTexture.setImagePtr(mImage[i]);
        mSampler[i].applyTextureData(rTexture);
        mTarget[i].applyTextureData(rTexture);
        mRenderBuffer[i].setRenderTargetColor(&mTarget[i]);
        mRenderBuffer[i].setVirtualSize(sead::Vector2f(reducedWidth, reducedHeight));
        mRenderBuffer[i].setPhysicalArea(
            sead::BoundBox2f(0.0f, 0.0f, reducedWidth, reducedHeight));
    }
}

/**
 * Reduces and blurs the sampled texture and draws the result into the render buffer.
 * @param pDrawContext draw context
 * @param rRenderBuffer render buffer to draw into
 * @param rSampler sampler of the source texture, rebound to the work textures while blurring
 */
void BlurFilter::draw(agl::DrawContext* pDrawContext, const agl::RenderBuffer& rRenderBuffer,
                      agl::TextureSampler& rSampler) {
    if (!mIsEnable) {
        return;
    }

    if (mReduceScale == agl::utl::ImageFilter2D::cReduceScale_1 &&
        (mIteration == 0 || mBlurType == 0)) {
        return;
    }

    if (mReduceScale != agl::utl::ImageFilter2D::cReduceScale_1) {
        const agl::RenderBuffer& rTarget = mRenderBuffer[0];
        rTarget.bind(pDrawContext);
        sead::Viewport viewport(rTarget);
        viewport.apply(pDrawContext, rTarget);
        sead::GraphicsContext graphicsContext;
        graphicsContext.setDepthEnable(false, false);
        graphicsContext.setBlendEnable(false);
        graphicsContext.apply(pDrawContext);

        if (mReduceScale != agl::utl::ImageFilter2D::cReduceScale_2 && mIsUseReduce) {
            agl::utl::ImageFilter2D::drawReduce(pDrawContext, rSampler, viewport, mReduceScale,
                                                1.0f, sead::Vector2f::zero);
        } else {
            agl::utl::ImageFilter2D::drawTexture(pDrawContext, rSampler, viewport,
                                                 sead::Vector2f::ones, sead::Vector2f::zero);
        }

        rSampler.applyTextureData(mTexture[0]);
    }

    if (mBlurType != 0) {
        if (mBlurType == 5) {
            for (s32 i = 0; i < mIteration * 2; i++) {
                s32 index = (i + 1) & 1;
                const agl::RenderBuffer& rTarget = mRenderBuffer[index];
                rTarget.bind(pDrawContext);
                sead::Viewport viewport(rTarget);
                viewport.apply(pDrawContext, rTarget);
                sead::GraphicsContext graphicsContext;
                graphicsContext.setDepthEnable(false, false);
                graphicsContext.setBlendEnable(false);
                graphicsContext.apply(pDrawContext);
                agl::utl::ImageFilter2D::drawBlur(pDrawContext, rSampler, viewport,
                                                  agl::utl::ImageFilter2D::BlurType(2 | index),
                                                  sead::Vector2f::zero, sead::Vector2f::ones,
                                                  sead::Vector2f::zero);
                rSampler.applyTextureData(mTexture[index]);
            }
        } else {
            for (s32 i = 0; i < mIteration; i++) {
                s32 index = (i + 1) & 1;
                const agl::RenderBuffer& rTarget = mRenderBuffer[index];
                rTarget.bind(pDrawContext);
                sead::Viewport viewport(rTarget);
                viewport.apply(pDrawContext, rTarget);
                sead::GraphicsContext graphicsContext;
                graphicsContext.setDepthEnable(false, false);
                graphicsContext.setBlendEnable(false);
                graphicsContext.apply(pDrawContext);
                agl::utl::ImageFilter2D::drawBlur(pDrawContext, rSampler, viewport,
                                                  agl::utl::ImageFilter2D::BlurType(mBlurType),
                                                  sead::Vector2f::zero, sead::Vector2f::ones,
                                                  sead::Vector2f::zero);
                rSampler.applyTextureData(mTexture[index]);
            }
        }
    }

    rRenderBuffer.bind(pDrawContext);
    sead::Viewport finalViewport(rRenderBuffer);
    finalViewport.apply(pDrawContext, rRenderBuffer);
    sead::GraphicsContext finalContext;
    finalContext.setDepthEnable(false, false);
    finalContext.setBlendEnable(false);
    finalContext.apply(pDrawContext);
    rSampler.setMagFilter(!mIsPointSampling);

    sead::Vector2f scale;
    sead::Vector2f size;
    finalViewport.getOnFrameBufferSize(&size, rRenderBuffer);
    const agl::TextureData* pTargetTexture = rRenderBuffer.getRenderTargetColor();
    pTargetTexture->getHeight(0);
    const sead::Vector2f& rVirtualSize = rRenderBuffer.getVirtualSize();
    scale.set(rVirtualSize.x / mTexture[0].getWidth(0), rVirtualSize.y / mTexture[0].getHeight(0));
    agl::utl::ImageFilter2D::drawTexture(pDrawContext, rSampler, finalViewport, scale,
                                         sead::Vector2f::zero);
}
}  // namespace al
