#include "layer/aglRenderDisplay.h"

#include <gfx/seadFrameBuffer.h>
#include <gfx/seadGraphicsContext.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <nvn/nvn_FuncPtrInline.h>
#include <random/seadGlobalRandom.h>

#include "common/aglDrawContext.h"
#include "common/aglTextureSampler.h"
#include "detail/aglMemoryPoolHeap.h"
#include "detail/aglRootNode.h"
#include "driver/aglGraphicsDriverMgr.h"
#include "layer/aglDrawMethod.h"
#include "layer/aglLayer.h"
#include "layer/aglRenderDLBuffer.h"
#include "layer/aglRenderInfo.h"
#include "utility/aglDevTools.h"
#include "utility/aglDynamicTextureAllocator.h"

namespace agl::lyr {

namespace {

s32 compareRandom(const Layer* pA, const Layer* pB)
{
    return s32(sead::GlobalRandom::instance()->getU32() & 2) - 1;
}

}  // namespace

/**
 * Constructs an uninitialized render display.
 */
RenderDisplay::RenderDisplay()
    : mDisplayIndex(0), _9(0), mFrameBuffer(nullptr), mColorTexture(nullptr),
      mDepthTexture(nullptr), mResolveTexture(nullptr), mFrameBufferSize(0.0f, 0.0f),
      mDLBuffer(nullptr), mLogicalFrameBuffer(nullptr), mFlag(0xa88), mClearFlag(3),
      mClearColor(0.3f, 0.3f, 1.0f, 0.0f), mClearDepth(1.0f), mClearStencil(0), mIsClear(false),
      mDLTotalSize(0), mDLMaxSize(0), mBeginDrawMethod(nullptr), mEndDrawMethod(nullptr),
      mRenderStepDrawMethod(nullptr)
{
    detail::RootNode::setNodeMeta(this, "Icon = SCREEN");
    mFlag.reset(cFlag_11);
    _9 = 1;
}

/**
 * Frees the frame buffers and all buffers owned by the display.
 */
RenderDisplay::~RenderDisplay()
{
    freeFrameBuffer();
    mLayer.freeBuffer();
    mJob.freeBuffer();
    mRenderDLPtr.freeBuffer();
    mRenderDL.freeBuffer();
    mRenderDLSorted.freeBuffer();

    if (mFrameBuffer != nullptr)
    {
        delete mFrameBuffer;
        mFrameBuffer = nullptr;
    }

    for (auto& rDisplayList : mDisplayList)
    {
        if (rDisplayList.getBuffer().isValid())
        {
            rDisplayList.getBuffer().deleteGPUMemBlock();
        }

        if (rDisplayList.isUserControlMemory() && rDisplayList.getControlMemory() != nullptr)
        {
            delete static_cast<u8*>(rDisplayList.getControlMemory());
        }
    }
}

/**
 * Frees the dynamically allocated frame buffer textures.
 */
void RenderDisplay::freeFrameBuffer()
{
    if (mFlag.isOn(cFlag_FrameBufferAllocated))
    {
        utl::DynamicTextureAllocator* pAllocator = utl::DynamicTextureAllocator::instance();

        if (mColorTexture != nullptr)
        {
            pAllocator->free(mColorTexture);
            mColorTexture = nullptr;
        }

        if (mDepthTexture != nullptr)
        {
            pAllocator->free(mDepthTexture);
            mDepthTexture = nullptr;
        }

        if (mResolveTexture != nullptr)
        {
            pAllocator->free(mResolveTexture);
            mResolveTexture = nullptr;
        }

        mFlag.reset(cFlag_FrameBufferAllocated);
    }

    mColorTexture = nullptr;
    mDepthTexture = nullptr;
}

/**
 * Initializes the display.
 * @param displayIndex index of the display
 * @param rName name of the display
 * @param rVirtualSize virtual size of the display
 * @param rPhysicalSize physical size of the frame buffer
 * @param pLogicalFrameBuffer logical frame buffer of the display
 * @param layerNum maximum number of layers
 * @param renderDLNum number of render display lists per layer
 * @param createFrameBuffer whether a frame buffer is created
 * @param pHeap heap to allocate from
 */
void RenderDisplay::initialize(s32 displayIndex, const sead::SafeString& rName,
                               const sead::Vector2i& rVirtualSize,
                               const sead::Vector2i& rPhysicalSize,
                               sead::LogicalFrameBuffer* pLogicalFrameBuffer, s32 layerNum,
                               s32 renderDLNum, bool createFrameBuffer, sead::Heap* pHeap)
{
    mDisplayIndex = displayIndex;
    mLogicalFrameBuffer = pLogicalFrameBuffer;
    mLayer.allocBuffer(layerNum, pHeap);
    mJob.tryAllocBuffer(2, pHeap);
    mRenderDLPtr.allocBuffer(layerNum * renderDLNum + 8, pHeap);
    mRenderDL.tryAllocBuffer(mRenderDLPtr.capacity(), pHeap);
    mRenderDLSorted.allocBuffer(mRenderDLPtr.capacity(), pHeap);

    {
        sead::FormatFixedSafeString<256> nodeName("%s画面", rName.cstr());
    }

    mLogicalFrameBuffer->setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, 0.0f, 0.0f));
    mLogicalFrameBuffer->setVirtualSize(sead::Vector2f(rVirtualSize.x, rVirtualSize.y));
    resetFrameBufferSize_(sead::Vector2f(rPhysicalSize.x, rPhysicalSize.y));

    mScanOutBuffer.setRenderTargetColor(&mScanOutTarget);
    mJob(0).initialize(LayerJob::cType_Begin, nullptr, pHeap);
    mJob[1].initialize(LayerJob::cType_End, nullptr, pHeap);

    for (auto& rDisplayList : mDisplayList)
    {
        auto* pBlock = new (pHeap) GPUMemBlockU8();
        pBlock->allocBuffer(0x10, pHeap, 4, MemoryAttribute::_00);
        rDisplayList.setBuffer(GPUMemAddr<u8>(*pBlock, 0), 0x10);
        rDisplayList.setControlMemory(new (pHeap, 8) u8[0x100], 0x100);
    }

    if (createFrameBuffer)
    {
        mFrameBuffer = new (pHeap) RenderBuffer();
    }
}

/**
 * Resizes the frame buffer and resets the viewports.
 * @param rSize new frame buffer size
 */
void RenderDisplay::resetFrameBufferSize_(const sead::Vector2f& rSize)
{
    if (rSize.x != mLogicalFrameBuffer->getPhysicalArea().getSizeX() ||
        rSize.y != mLogicalFrameBuffer->getPhysicalArea().getSizeY())
    {
        mFlag.set(cFlag_FrameBufferSizeChanged);
        mLogicalFrameBuffer->setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, rSize.x, rSize.y));
        mFrameBufferSize = rSize;
    }

    mViewport.setByFrameBuffer(*mLogicalFrameBuffer);
    mViewportOrigin = mViewport;

    if (mFrameBuffer != nullptr)
    {
        mFrameBuffer->setVirtualSize(mLogicalFrameBuffer->getVirtualSize());
        mFrameBuffer->setPhysicalArea(mLogicalFrameBuffer->getPhysicalArea());
        mIsClear = true;
    }
}

/**
 * Applies a pending frame buffer resize.
 */
void RenderDisplay::calc()
{
    if (mFlag.isOn(cFlag_ResetFrameBufferSize))
    {
        resetFrameBufferSize_(mFrameBufferSize);
        mFlag.reset(cFlag_ResetFrameBufferSize);
    }
}

/**
 * Makes the scan out buffer refer to the color texture of the frame buffer.
 */
void RenderDisplay::copyScanOutBuffer_() const
{
    if (mFrameBuffer == nullptr || mColorTexture == nullptr)
    {
        return;
    }

    mScanOutBuffer.setPhysicalArea(mFrameBuffer->getPhysicalArea());
    mScanOutBuffer.setVirtualSize(mFrameBuffer->getVirtualSize());
    mScanOutTarget.applyTextureData(*mColorTexture);
    mFlag.set(cFlag_ScanOutBufferCopied);
}

/**
 * Allocates or shares the frame buffer textures of the display.
 * @param multiSample multi sample type
 * @param pShare display to share the textures with, or nullptr to allocate them
 * @param colorFormat color texture format
 * @param colorNum -1 to disable the color texture
 * @param depthFormat depth texture format
 * @param depthNum -1 to disable the depth texture
 * @param useZCull whether a zcull buffer is allocated for the depth texture
 * @param b2 passed to the texture allocator
 */
void RenderDisplay::allocFrameBuffer(MultiSampleType multiSample, RenderDisplay* pShare,
                                     TextureFormat colorFormat, s32 colorNum,
                                     TextureFormat depthFormat, s32 depthNum, bool useZCull,
                                     bool b2)
{
    u32 width = mFrameBufferSize.x;
    u32 height = mFrameBufferSize.y;

    if (s32(multiSample) == 0)
    {
        mColorTextureData.initialize_(TextureType::cTextureType_2D, colorFormat, width, height, 1,
                                      1, TextureAttribute(0), MultiSampleType(0), true);
        mDepthTextureData.initialize_(TextureType::cTextureType_2D, depthFormat, width, height, 1,
                                      1, TextureAttribute(0), MultiSampleType(0), true);
    }
    else
    {
        if (s32(multiSample) >= 1)
        {
            mColorTextureData.initialize_(TextureType(5), colorFormat, width, height, 1, 1,
                                          TextureAttribute(0), multiSample, true);
            mDepthTextureData.initialize_(TextureType(5), depthFormat, width, height, 1, 1,
                                          TextureAttribute(0), multiSample, true);
        }
        else
        {
            mColorTextureData.initialize_(TextureType::cTextureType_2D, colorFormat, width, height,
                                          1, 1, TextureAttribute(0), MultiSampleType(0), true);
            mDepthTextureData.initialize_(TextureType::cTextureType_2D, depthFormat, width, height,
                                          1, 1, TextureAttribute(0), MultiSampleType(0), true);
        }

        mResolveTextureData.initialize_(TextureType::cTextureType_2D, colorFormat, width, height,
                                        1, s32(multiSample), TextureAttribute(0),
                                        MultiSampleType(0), true);
    }

    if (pShare == nullptr)
    {
        utl::DynamicTextureAllocator* pAllocator = utl::DynamicTextureAllocator::instance();

        if (colorNum != -1)
        {
            GPUMemVoidAddr addr;
            mColorTexture = pAllocator->allocMultiSampleWithoutContext(
                nullptr, "agl::RenderDisplay(color)", colorFormat, width, height,
                MultiSampleType(mColorTextureData.getSurface().getMultiSampleType()), &addr,
                utl::DynamicTextureAllocator::AllocateType(0), false, b2);
            mColorTarget.setZCullBufferDirect(addr);
        }

        if (depthNum != -1)
        {
            GPUMemVoidAddr addr;
            mDepthTexture = pAllocator->allocMultiSampleWithoutContext(
                nullptr, "agl::RenderDisplay(depth)", depthFormat, width, height,
                MultiSampleType(mDepthTextureData.getSurface().getMultiSampleType()),
                useZCull ? &addr : nullptr, utl::DynamicTextureAllocator::AllocateType(0), false,
                b2);
            mDepthTarget.setZCullBuffer(addr);
        }

        mFlag.set(cFlag_FrameBufferAllocated);
    }
    else
    {
        if (colorNum != -1)
        {
            mColorTexture = &mColorTextureData;
            mColorTextureData.setImagePtr(pShare->mColorTexture->getImagePtr(), 0);
            mColorTarget.setZCullBufferDirect(pShare->mColorTarget.getZCullBuffer());
        }

        if (pShare->mResolveTexture != nullptr)
        {
            mResolveTexture = &mResolveTextureData;
            mResolveTextureData.setImagePtr(pShare->mResolveTexture->getImagePtr(), 0);
        }
        else
        {
            mResolveTexture = nullptr;
        }

        if (depthNum != -1)
        {
            mDepthTexture = &mDepthTextureData;
            mDepthTextureData.setImagePtr(pShare->mDepthTexture->getImagePtr(), 0);
            mDepthTarget.setZCullBuffer(pShare->mDepthTarget.getZCullBuffer());
        }

        mFlag.reset(cFlag_FrameBufferAllocated);
    }

    if (mColorTexture != nullptr)
    {
        mFrameBuffer->setRenderTargetColor(&mColorTarget);
        mColorTarget.applyTextureData(*mColorTexture);
    }
    else
    {
        mFrameBuffer->setRenderTargetColor(nullptr);
    }

    if (mDepthTexture != nullptr)
    {
        mFrameBuffer->setRenderTargetDepth(&mDepthTarget);
        mDepthTarget.applyTextureData(*mDepthTexture);
    }
    else
    {
        mFrameBuffer->setRenderTargetDepth(nullptr);
    }

    if (mResolveTexture != nullptr)
    {
        mResolveTarget.applyTextureData(*mResolveTexture);
    }
}

/**
 * Clears the layer list and the recorded display lists.
 */
void RenderDisplay::clear()
{
    mLayer.clear();
    mRenderDLPtr.clear();
}

/**
 * Adds a layer to the display.
 * @param pLayer layer to add
 */
void RenderDisplay::pushBack(Layer* pLayer)
{
    mLayer.pushBack(pLayer);

    if (mFlag.isOn(1 << 1))
    {
        mLayer.heapSort_<Layer>(compareRandom);
    }
}

/**
 * Removes a layer from the display and invalidates its recorded display lists.
 * @param pLayer layer to remove
 */
void RenderDisplay::erase(Layer* pLayer)
{
    s32 index = 0;

    for (auto& rLayer : mLayer)
    {
        if (&rLayer == pLayer)
        {
            for (auto& rDL : mRenderDLPtr)
            {
                if (rDL.mLayer == pLayer)
                {
                    rDL.invalidate();
                }
            }

            mLayer.erase(index);
            return;
        }

        index++;
    }
}

/**
 * Registers a recorded render display list.
 * @param pDrawContext draw context the list was recorded with
 * @param pDL recorded render display list
 * @param pLayer layer the list was recorded for, or nullptr
 */
void RenderDisplay::pushBackDL_(DrawContext* pDrawContext, RenderDL* pDL,
                                const Layer* pLayer) const
{
    if (!pDL->isValid())
    {
        return;
    }

    if (pLayer != nullptr)
    {
        pLayer->setLastDisplayListSize(pDL->getValidSize());
        pDL->mLayer = pLayer;
    }
    else
    {
        pDL->mLayer = nullptr;
    }

    if (pLayer == nullptr || pLayer->mFlag.isOn(1 << 10))
    {
        mRenderDLPtr.pushBack(pDL);
    }

    if (mRenderStepDrawMethod != nullptr)
    {
        RenderInfo info(pDrawContext, mDisplayIndex, mFrameBuffer);
        info.setRenderStep(4);
        mRenderStepDrawMethod->invoke(info);
    }
}

/**
 * Calls the begin callback and binds and clears the frame buffer.
 * @param pDrawContext draw context to draw with
 */
void RenderDisplay::beginDraw_(DrawContext* pDrawContext) const
{
    if (mBeginDrawMethod != nullptr)
    {
        RenderInfo info(pDrawContext, mDisplayIndex, mFrameBuffer);
        info.setRenderStep(mDisplayIndex);
        mBeginDrawMethod->invoke(info);
    }

    if (mFrameBuffer != nullptr)
    {
        bindAndClearRenderBuffer(pDrawContext);
    }
}

/**
 * Binds the frame buffer and clears it if clearing is enabled.
 * @param pDrawContext draw context to draw with
 */
void RenderDisplay::bindAndClearRenderBuffer(DrawContext* pDrawContext) const
{
    if (mFlag.isOn(1 << 7))
    {
        u32 clearFlag = mClearFlag;

        if (mFrameBuffer->getRenderTargetDepth() == nullptr)
        {
            clearFlag &= ~2u;
        }

        mFrameBuffer->bind(pDrawContext);
        mFrameBuffer->fastClear(pDrawContext, 0, clearFlag, mClearColor, mClearDepth,
                                mClearStencil, mViewportOrigin, true);
    }
    else
    {
        mViewportOrigin.apply(pDrawContext, *mFrameBuffer);
    }

    if (mIsClear)
    {
        mIsClear = false;
    }
}

/**
 * Prepares a layer for drawing.
 * @param pDrawContext draw context to draw with
 * @param pLayer layer to draw
 * @param frameworkType framework type of the draw
 */
void RenderDisplay::preDrawLayer_(DrawContext* pDrawContext, const Layer* pLayer,
                                  FrameworkType frameworkType) const
{
    RenderInfo info(pDrawContext, mDisplayIndex, frameworkType, mFrameBuffer,
                    mFlag.isOn(cFlag_DrawDebugInfo), pLayer);
    if (mFrameBuffer != nullptr)
    {
        pLayer->getViewport().apply(pDrawContext, *mFrameBuffer);
    }

    pLayer->preDrawImpl(info);
    pLayer->clearColor_(info);
}

/**
 * Finishes drawing a layer.
 * @param pDrawContext draw context to draw with
 * @param pLayer layer that was drawn
 * @param frameworkType framework type of the draw
 */
void RenderDisplay::postDrawLayer_(DrawContext* pDrawContext, const Layer* pLayer,
                                   FrameworkType frameworkType) const
{
    RenderInfo info(pDrawContext, mDisplayIndex, frameworkType, mFrameBuffer,
                    mFlag.isOn(cFlag_DrawDebugInfo), pLayer);
    pLayer->drawDebugInfo_(info);
    pLayer->postDrawImpl(info);
}

/**
 * Draws a layer directly without display lists.
 * @param pDrawContext draw context to draw with
 * @param pLayer layer to draw
 * @param frameworkType framework type of the draw
 */
void RenderDisplay::drawLayerDirect_(DrawContext* pDrawContext, const Layer* pLayer,
                                     FrameworkType frameworkType) const
{
    preDrawLayer_(pDrawContext, pLayer, frameworkType);

    {
        RenderInfo info(pDrawContext, mDisplayIndex, frameworkType, mFrameBuffer,
                        mFlag.isOn(cFlag_DrawDebugInfo), pLayer);
        s32 num = pLayer->getRenderStepNum();

        for (info.setRenderStep(0); info.getRenderStep() < num;
             info.setRenderStep(info.getRenderStep() + 1))
        {
            pLayer->isRenderStepNoDependency(info.getRenderStep());
            pLayer->drawRenderStep_(info);
        }
    }

    postDrawLayer_(pDrawContext, pLayer, frameworkType);
}

/**
 * Draws the render steps of a layer selected by a flag.
 * @param pDrawContext draw context to draw with
 * @param pLayer layer to draw
 * @param flag selects which render steps are drawn
 * @param frameworkType framework type of the draw
 * @param useDL whether each render step is recorded into its own display list
 * @param priority sort priority of the first render step
 */
void RenderDisplay::drawRenderStep_(DrawContext* pDrawContext, const Layer* pLayer, u32 flag,
                                    FrameworkType frameworkType, bool useDL, s32 priority) const
{
    RenderInfo info(pDrawContext, mDisplayIndex, frameworkType, mFrameBuffer,
                    mFlag.isOn(cFlag_DrawDebugInfo), pLayer);
    s32 num = pLayer->getRenderStepNum();

    for (info.setRenderStep(0); info.getRenderStep() < num;
         info.setRenderStep(info.getRenderStep() + 1))
    {
        bool isSkipDependency = (flag & 2) == 0;
        bool isGPUCalc = (flag & 1) ? false : pLayer->isRenderStepGPUCalc(info.getRenderStep());
        bool isNoDependency = pLayer->isRenderStepNoDependency(info.getRenderStep());

        if (isGPUCalc)
        {
            continue;
        }

        if ((isSkipDependency || isNoDependency) != (isNoDependency && (flag & 4) != 0))
        {
            continue;
        }

        if (useDL)
        {
            s32 index = mDLBuffer->begin(pDrawContext, sead::SafeString(sead::SafeString::cEmptyString),
                                         priority + info.getRenderStep());
            pLayer->drawRenderStep_(info);

            if (index != -1)
            {
                pushBackDL_(pDrawContext, mDLBuffer->end(pDrawContext, index), pLayer);
            }
        }
        else
        {
            pLayer->drawRenderStep_(info);
        }
    }
}

/**
 * Records the display lists of a layer.
 * @param pDrawContext draw context to record with
 * @param pLayer layer to record
 * @param priority sort priority of the layer
 */
void RenderDisplay::calcLayerDL_(DrawContext* pDrawContext, const Layer* pLayer,
                                 s32 priority) const
{
    u32 flag = mFlag;
    s32 preIndex;
    s32 index;
    u32 stepFlag;

    if (flag & cFlag_11)
    {
        stepFlag = (flag & (1 << 6)) ? 2 : 6;
        preIndex = mDLBuffer->begin(pDrawContext, sead::SafeString(sead::SafeString::cEmptyString), priority);
        index = -1;
    }
    else
    {
        index = mDLBuffer->begin(pDrawContext, sead::SafeString(sead::SafeString::cEmptyString), priority);
        stepFlag = 6;
        preIndex = -1;
    }

    preDrawLayer_(pDrawContext, pLayer, FrameworkType(0));

    if (preIndex != -1)
    {
        pushBackDL_(pDrawContext, mDLBuffer->end(pDrawContext, preIndex), pLayer);
    }

    priority++;
    drawRenderStep_(pDrawContext, pLayer, stepFlag, FrameworkType(0), (flag & cFlag_11) != 0,
                    priority);

    s32 postIndex;

    if (flag & cFlag_11)
    {
        postIndex = mDLBuffer->begin(pDrawContext, sead::SafeString(sead::SafeString::cEmptyString),
                                     pLayer->getRenderStepNum() + priority);
    }
    else
    {
        postIndex = -1;
    }

    postDrawLayer_(pDrawContext, pLayer, FrameworkType(0));

    if (postIndex != -1)
    {
        pushBackDL_(pDrawContext, mDLBuffer->end(pDrawContext, postIndex), pLayer);
    }

    if (index != -1)
    {
        pushBackDL_(pDrawContext, mDLBuffer->end(pDrawContext, index), pLayer);
    }
}

/**
 * Records the sub display lists of a layer.
 * @param pDrawContext draw context to record with
 * @param pLayer layer to record
 * @param priority sort priority of the layer
 */
void RenderDisplay::calcSubLayerDL_(DrawContext* pDrawContext, const Layer* pLayer,
                                    s32 priority) const
{
    if ((mFlag & (cFlag_11 | (1 << 6))) == (cFlag_11 | (1 << 6)))
    {
        drawRenderStep_(pDrawContext, pLayer, 4, FrameworkType(0), true, priority + 1);
    }
}

/**
 * Calls the end callback.
 * @param pDrawContext draw context to draw with
 */
void RenderDisplay::endDraw_(DrawContext* pDrawContext) const
{
    if (mEndDrawMethod != nullptr)
    {
        RenderInfo info(pDrawContext, mDisplayIndex, mFrameBuffer);
        info.setRenderStep(mDisplayIndex + 2);
        mEndDrawMethod->invoke(info);
    }
}

/**
 * Records the GPU calculation display list of a layer.
 * @param pDrawContext draw context to record with
 * @param pLayer layer to record
 * @param priority sort priority of the display list
 */
void RenderDisplay::calcGPU_(DrawContext* pDrawContext, const Layer* pLayer, s32 priority) const
{
    s32 index = mDLBuffer->begin(pDrawContext, sead::SafeString(sead::SafeString::cEmptyString), priority);

    RenderInfo info(pDrawContext, mDisplayIndex, FrameworkType(0), mFrameBuffer,
                    mFlag.isOn(cFlag_DrawDebugInfo), pLayer);
    s32 num = pLayer->getRenderStepNum();

    for (info.setRenderStep(0); info.getRenderStep() < num;
         info.setRenderStep(info.getRenderStep() + 1))
    {
        if (pLayer->isRenderStepGPUCalc(info.getRenderStep()))
        {
            pLayer->drawRenderStep_(info);
        }
    }

    pushBackDL_(pDrawContext, mDLBuffer->end(pDrawContext, index), pLayer);
}

/**
 * Draws every layer directly.
 * @param pDrawContext draw context to draw with
 */
void RenderDisplay::draw(DrawContext* pDrawContext) const
{
    beginDraw_(pDrawContext);

    for (auto& rLayer : mLayer)
    {
        drawLayerDirect_(pDrawContext, &rLayer, FrameworkType(0));
    }

    endDraw_(pDrawContext);

    if (mResolveTexture != nullptr && mFrameBuffer != nullptr)
    {
        mColorTarget.expandAuxBuffer(pDrawContext);

        sead::GraphicsContext graphicsContext;
        graphicsContext.setDepthEnable(false, false);
        graphicsContext.setBlendEnable(false);
        graphicsContext.apply(pDrawContext);

        RenderTargetColor* pColor = mFrameBuffer->getRenderTargetColor();
        RenderTargetDepth* pDepth = mFrameBuffer->getRenderTargetDepth();
        mFrameBuffer->setRenderTargetColor(const_cast<RenderTargetColor*>(&mResolveTarget));
        mFrameBuffer->setRenderTargetDepth(nullptr);
        mFrameBuffer->bind(pDrawContext);
        mViewportOrigin.apply(pDrawContext, *mFrameBuffer);

        TextureSampler sampler;
        sampler.applyTextureData(mColorTarget);
        utl::ImageFilter2D::drawTextureMSAA(pDrawContext, sampler, mViewportOrigin,
                                            sead::Vector2f::ones, sead::Vector2f::zero);

        mFrameBuffer->setRenderTargetColor(pColor);
        mFrameBuffer->setRenderTargetDepth(pDepth);
    }
}

/**
 * Queues the display list jobs of every layer.
 * @param pJobs job array to queue into, or nullptr to run the jobs immediately
 */
void RenderDisplay::calcDL(LayerJobArray* pJobs)
{
    s32 minWeight = 0x7fffffff;

    for (auto it = mLayer.begin(), end = mLayer.end(); it != end; ++it)
    {
        it->calcJobWeight();
        s32 weight = it->_a0;
        it->mJobDraw->pushBackTo(this, weight, pJobs);
        it->mJobSubDraw->pushBackTo(this, weight - 1, pJobs);
        minWeight = sead::Mathi::min(weight, minWeight);
    }

    mJob(0).pushBackTo(this, 0x7fffffff, pJobs);
    mJob[1].pushBackTo(this, minWeight - 1, pJobs);
}

/**
 * Sorts the recorded display lists by priority.
 */
void RenderDisplay::sortDL() const
{
    mRenderDLPtr.heapSort(RenderDL::compare);
}

/**
 * Queues the GPU calculation jobs of every layer.
 * @param pJobs job array to queue into, or nullptr to run the jobs immediately
 */
void RenderDisplay::calcGPU(LayerJobArray* pJobs) const
{
    for (auto& rLayer : mLayer)
    {
        rLayer.mJobGPUCalc->pushBackTo(this, rLayer._a0, pJobs);
    }
}

/**
 * Calls the recorded display lists.
 * @param pDrawContext draw context to draw with
 * @param copyDisplayList whether the recorded display lists are copied first
 */
void RenderDisplay::callDisplayList(DrawContext* pDrawContext, bool copyDisplayList) const
{
    if (copyDisplayList)
    {
        mRenderDLSorted.clear();

        for (auto& rDL : mRenderDLPtr)
        {
            if (rDL.isValid())
            {
                RenderDL* pCopy = &mRenderDL[mRenderDLSorted.size()];
                rDL.copyToRenderDL(pCopy);
                mRenderDLSorted.pushBack(pCopy);
            }
        }
    }

    switch (_9)
    {
    case 0:
        for (auto& rDL : mRenderDLSorted)
        {
            rDL.callDirect(pDrawContext);

            if (mFlag.isOn(1 << 12))
            {
                driver::GraphicsDriverMgr::instance()->waitDrawDone(pDrawContext);
            }
        }

        break;
    case 1:
        for (auto& rDL : mRenderDLSorted)
        {
            nvnCommandBufferCallCommands(pDrawContext->getNvnCommandBuffer(), 1,
                                         rDL.getHandlePtr());
            if (mFlag.isOn(1 << 12))
            {
                driver::GraphicsDriverMgr::instance()->waitDrawDone(pDrawContext);
            }
        }

        break;
    case 2:
    {
        const DisplayList& rDisplayList = mDisplayList[mDLBuffer->mCurrentBuffer];
        DrawContext context;
        context.setCommandBuffer(const_cast<DisplayList*>(&rDisplayList));

        if (const_cast<DisplayList&>(rDisplayList).beginDisplayList())
        {
            for (auto& rDL : mRenderDLSorted)
            {
                nvnCommandBufferCallCommands(context.getNvnCommandBuffer(), 1, rDL.getHandlePtr());
            }

            const_cast<DisplayList&>(rDisplayList).endDisplayList();

            if (rDisplayList.isValid())
            {
                rDisplayList.callDirect(pDrawContext);
            }
        }

        break;
    }
    }

    if (mFlag.isOn(1 << 13))
    {
        driver::GraphicsDriverMgr::instance()->waitDrawDone(pDrawContext);
    }

    mDLTotalSize = 0;

    for (auto& rDL : mRenderDLSorted)
    {
        mDLTotalSize += rDL.getValidSize();
    }

    if (mDLMaxSize < mDLTotalSize)
    {
        mDLMaxSize = mDLTotalSize;
    }
}

/**
 * Dumps information about the recorded display lists.
 */
void RenderDisplay::dumpDisplayListInfo() const
{
    for (auto& rDL : mRenderDLSorted)
    {
        rDL.dumpInfo();
    }
}

/**
 * Dumps the recorded display lists.
 */
void RenderDisplay::dumpRawDisplayList() const
{
    for (auto& rDL : mRenderDLSorted)
    {
        rDL.dump();
    }
}

/**
 * Corrupts a random word of a random recorded display list (debug feature).
 */
void RenderDisplay::destroyDisplayList() const
{
    RenderDL* pDL =
        mRenderDLPtr.unsafeAt(sead::GlobalRandom::instance()->getF32() * mRenderDLPtr.size());
    u32 offset = sead::GlobalRandom::instance()->getF32() * (pDL->getValidSize() / 4);
    GPUMemAddr<u8> addr(pDL->getBuffer(), offset);
    u8* pBuffer = static_cast<u8*>(nvnMemoryPoolMap(addr.getMemoryPool()->getDriverPool()));
    *reinterpret_cast<u32*>(pBuffer + addr.getByteOffset()) = 0xeeeeeeee;
    addr.flushCPUCache(4);
}

/**
 * Generates the host I/O messages of the display.
 * @param pContext host I/O context
 */
void RenderDisplay::genMessage(sead::hostio::Context* pContext)
{
    const sead::LogicalFrameBuffer* pFrameBuffer = mLogicalFrameBuffer;
    {
        sead::FormatFixedSafeString<1024> header("GroupHeader = %s screen, Dir = Y",
                                                 sead::SafeString::cEmptyString.getStringTop());
    }

    {
        sead::FormatFixedSafeString<1024> info(
            "仮想キャンバス   : (%.1f, %.1f)\nフレームバッファ : (%.1f, %.1f) - ( %.1f, %.1f )",
            pFrameBuffer->getVirtualSize().x, pFrameBuffer->getVirtualSize().y,
            pFrameBuffer->getPhysicalArea().getMin().x, pFrameBuffer->getPhysicalArea().getMin().y,
            pFrameBuffer->getPhysicalArea().getMax().x,
            pFrameBuffer->getPhysicalArea().getMax().y);
    }

    {
        sead::FormatFixedSafeString<1024> info("DisplayList x %3d = %12d[byte] Max:%12d[byte]",
                                               mRenderDLPtr.size(), mDLTotalSize, mDLMaxSize);
    }

    for (auto it = mRenderDLPtr.begin(), end = mRenderDLPtr.end(); it != end; ++it)
    {
        {
            sead::FormatFixedSafeString<1024> priority("%6d", it->mPriority);
        }

        {
            sead::FormatFixedSafeString<1024> core("%2d", it->mCoreId);
        }

        {
            sead::FormatFixedSafeString<1024> size("%12d (%12d)", it->getValidSize(),
                                                   it->getControlMemoryUsed());
        }
    }
}

/**
 * Handles a host I/O property event.
 * @param pEvent property event
 */
void RenderDisplay::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    switch (reinterpret_cast<uintptr_t>(pEvent->getId()))
    {
    case 10000:
        mFlag.set(cFlag_ResetFrameBufferSize);
        break;
    case 10001:
        mDLMaxSize = 0;
        break;
    }
}

/**
 * Handles a host I/O node event (no-op in release builds).
 * @param pEvent node event
 */
void RenderDisplay::listenNodeEvent(const sead::hostio::NodeEvent* pEvent) {}

}  // namespace agl::lyr
