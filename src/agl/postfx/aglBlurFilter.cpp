#include "postfx/aglBlurFilter.h"

#include <gfx/seadGraphicsContext.h>
#include <heap/seadHeapMgr.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include "common/aglDrawContext.h"
#include "common/aglGPUMemBlock.h"

namespace agl::pfx {

namespace {

utl::ImageFilter2D::ReduceScale sReduceScale;

}  // namespace

/**
 * Constructs the blur filter with default settings.
 */
BlurFilter::BlurFilter() = default;

/**
 * Frees the GPU memory of both work textures.
 */
BlurFilter::~BlurFilter()
{
    for (auto& rImage : mImage)
    {
        if (rImage.isValid())
        {
            rImage.deleteGPUMemBlock();
            rImage.invalidate();
        }
    }
}

/**
 * Allocates the two work textures for the given frame buffer and reduce scale.
 * @param rFrameBuffer source frame buffer
 * @param rViewport source viewport
 * @param scale reduce scale of the work textures
 */
void BlurFilter::setupRenderTarget(const sead::LogicalFrameBuffer& rFrameBuffer,
                                   const sead::Viewport& rViewport,
                                   utl::ImageFilter2D::ReduceScale scale)
{
    mReduceScale = scale;
    sReduceScale = scale;
    mFrameBuffer = rFrameBuffer;
    mViewport = rViewport;

    sead::Vector2f size;
    rViewport.getOnFrameBufferSize(&size, rFrameBuffer);

    for (s32 i = 0; i < 2; i++)
    {
        s32 width = s32(size.x) >> mReduceScale;
        s32 height = s32(size.y) >> mReduceScale;
        TextureData& rTexture = mTexture[i];
        rTexture.initialize_(TextureType::cTextureType_2D,
                             TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, width, height, 1, 1,
                             TextureAttribute(0), MultiSampleType(0), true);
        u32 size = rTexture.getSurface().mStorageSize;
        sead::Heap* pHeap = sead::HeapMgr::instance()->getCurrentHeap();
        u32 alignment = rTexture.getSurface().mAlignment;
        auto* pBlock = new (pHeap, 8) GPUMemBlock<u8>;
        pBlock->allocBuffer_(size, pHeap, alignment,
                             MemoryAttribute::CompressibleMemory);
        mImage[i] = GPUMemAddr<u8>(*pBlock, 0);
        rTexture.setImagePtr(mImage[i]);
        mSampler[i].applyTextureData(rTexture);
        mTarget[i].applyTextureData(rTexture);
        mRenderBuffer[i].setRenderTargetColor(&mTarget[i]);
        mRenderBuffer[i].setVirtualSize(sead::Vector2f(width, height));
        mRenderBuffer[i].setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, width, height));
    }
}

/**
 * Reduces and blurs the color target of the render buffer and draws the result back.
 * @param pDrawContext draw context
 * @param rRenderBuffer render buffer to blur
 */
void BlurFilter::draw(DrawContext* pDrawContext, const RenderBuffer& rRenderBuffer) const
{
    if (!mIsEnable)
    {
        return;
    }

    if (mReduceScale == utl::ImageFilter2D::cReduceScale_1 && (mIteration == 0 || mBlurType == 0))
    {
        return;
    }

    TextureSampler sampler(*rRenderBuffer.getRenderTargetColor());

    if (mReduceScale != utl::ImageFilter2D::cReduceScale_1)
    {
        const RenderBuffer& rTarget = mRenderBuffer[0];
        rTarget.bind(pDrawContext);
        sead::Viewport viewport(rTarget);
        viewport.apply(pDrawContext, rTarget);
        sead::GraphicsContext graphicsContext;
        graphicsContext.setDepthEnable(false, false);
        graphicsContext.setBlendEnable(false);
        graphicsContext.apply(pDrawContext);

        if (mReduceScale != utl::ImageFilter2D::cReduceScale_2 && mIsUseReduce)
        {
            utl::ImageFilter2D::drawReduce(pDrawContext, sampler, viewport, mReduceScale, 1.0f,
                                           sead::Vector2f::zero);
        }
        else
        {
            utl::ImageFilter2D::drawTexture(pDrawContext, sampler, viewport, sead::Vector2f::ones,
                                            sead::Vector2f::zero);
        }

        sampler.applyTextureData(mTexture[0]);
    }

    if (mBlurType != 0)
    {
        if (mBlurType == 5)
        {
            for (s32 i = 0; i < mIteration * 2; i++)
            {
                s32 index = (i + 1) & 1;
                const RenderBuffer& rTarget = mRenderBuffer[index];
                rTarget.bind(pDrawContext);
                sead::Viewport viewport(rTarget);
                viewport.apply(pDrawContext, rTarget);
                sead::GraphicsContext graphicsContext;
                graphicsContext.setDepthEnable(false, false);
                graphicsContext.setBlendEnable(false);
                graphicsContext.apply(pDrawContext);
                utl::ImageFilter2D::drawBlur(pDrawContext, sampler, viewport,
                                             utl::ImageFilter2D::BlurType(2 | index),
                                             sead::Vector2f::zero, sead::Vector2f::ones,
                                             sead::Vector2f::zero);
                sampler.applyTextureData(mTexture[index]);
            }
        }
        else
        {
            for (s32 i = 0; i < mIteration; i++)
            {
                s32 index = (i + 1) & 1;
                const RenderBuffer& rTarget = mRenderBuffer[index];
                rTarget.bind(pDrawContext);
                sead::Viewport viewport(rTarget);
                viewport.apply(pDrawContext, rTarget);
                sead::GraphicsContext graphicsContext;
                graphicsContext.setDepthEnable(false, false);
                graphicsContext.setBlendEnable(false);
                graphicsContext.apply(pDrawContext);
                utl::ImageFilter2D::drawBlur(pDrawContext, sampler, viewport,
                                             utl::ImageFilter2D::BlurType(mBlurType),
                                             sead::Vector2f::zero, sead::Vector2f::ones,
                                             sead::Vector2f::zero);
                sampler.applyTextureData(mTexture[index]);
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
    sampler.setMagFilter(!mIsPointSampling);
    utl::ImageFilter2D::drawTexture(pDrawContext, sampler, finalViewport, sead::Vector2f::ones,
                                    sead::Vector2f::zero);
}

/**
 * Does nothing.
 * @param pContext host IO context
 */
void BlurFilter::genMessage(sead::hostio::Context* pContext) {}

/**
 * Recreates the work textures when the reduce scale property changes.
 * @param pEvent property event
 */
void BlurFilter::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if (pEvent->getIdValue() == 100000)
    {
        setupRenderTarget(mFrameBuffer, mViewport, sReduceScale);
    }
}

}  // namespace agl::pfx
