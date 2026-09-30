#include "layer/aglRenderer.h"

#include <controller/seadController.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <mc/seadCoreInfo.h>
#include <prim/seadSafeString.h>

#include "common/aglDrawContext.h"
#include "detail/aglRootNode.h"
#include "layer/aglLayer.h"
#include "layer/aglRenderDisplay.h"

namespace agl::lyr {

SEAD_SINGLETON_DISPOSER_IMPL(Renderer)

namespace {

const char* const cDisplayName[] = {"TV", "DRC"};

template <typename T>
bool isEventTarget(const sead::hostio::PropertyEvent* pEvent, const T& rMember)
{
    const void* id = pEvent->getId();
    return (pEvent->getType() & 2) == 0 && id < &rMember + 1 && id >= &rMember;
}

}  // namespace

/**
 * Constructs the default creation arguments.
 */
Renderer::CreateArg::CreateArg()
{
    for (s32 i = 0; i < cDisplayMax; i++)
    {
        mVirtualSize[i].set(1280, 720);
        mPhysicalSize[i].set(1280, 720);
        mLogicalFrameBuffer[i] = nullptr;
    }
}

/**
 * Sets the virtual and physical size of a display.
 * @param index display index
 * @param rVirtualSize virtual size of the display
 * @param rPhysicalSize physical size of the display
 */
void Renderer::CreateArg::setDisplayInfo(s32 index, const sead::Vector2i& rVirtualSize,
                                         const sead::Vector2i& rPhysicalSize)
{
    mVirtualSize[index] = rVirtualSize;
    mPhysicalSize[index] = rPhysicalSize;
}

/**
 * Constructs the renderer.
 */
Renderer::Renderer()
{
    _508 = 0;
    _50c = 0;
    _510 = 0;
}

/**
 * Allocates the displays, layer slots, jobs and job queues.
 * @param rArg creation arguments
 * @param pHeap heap to allocate from
 * @param pDebugHeap heap for debug allocations
 */
void Renderer::initialize(const CreateArg& rArg, sead::Heap* pHeap, sead::Heap* pDebugHeap)
{
    mRenderDLNum = rArg.mRenderDLNum;
    mLayer.tryAllocBuffer(rArg.mLayerNum, pHeap);
    mJobDraw.tryAllocBuffer(mLayer.size(), pHeap);
    mJobSubDraw.tryAllocBuffer(mLayer.size(), pHeap);
    mJobGPUCalc.tryAllocBuffer(mLayer.size(), pHeap);

    if (rArg.mDLBufferSize != 0 && rArg.mDLMultiBufferNum != 0)
    {
        mDLBuffer.initialize(rArg.mDLBufferSize, rArg.mDLControlMemorySize, 0x1000,
                             rArg.mDLMultiBufferNum, pHeap, pDebugHeap);
    }

    mMultiSampleType = rArg.mMultiSampleType;
    mDisplay.tryAllocBuffer(rArg.mDisplayNum, pHeap);
    mFlag.change(1 << 15, (rArg.mFlag & 2) == 0);
    mFlag.change(1 << 4, (rArg.mFlag & 1) == 0);

    s32 i = 0;
    for (auto& rpDisplay : mDisplay)
    {
        RenderDisplay* pDisplay = new (pHeap) RenderDisplay();
        pDisplay->initialize(i, getDisplayName(i), rArg.mVirtualSize[i], rArg.mPhysicalSize[i],
                             rArg.mLogicalFrameBuffer[i], mLayer.size(), mRenderDLNum,
                             (rArg.mFlag & 1) == 0, pHeap);
        if (rArg.mDLBufferSize != 0)
        {
            pDisplay->mDLBuffer = &mDLBuffer;
        }

        rpDisplay = pDisplay;
        i++;
    }

    s32 layerNum = mLayer.size();
    for (s32 j = 0; j < layerNum; j++)
    {
        mLayer(j) = nullptr;
    }

    for (auto& rPtr : _1c0)
    {
        rPtr = nullptr;
    }

    for (i = 0; i < cJobQueueNum; i++)
    {
        switch (i)
        {
        case 0:
        case 1:
        case 2:
            mJobQueue[i].initialize(mLayer.size() + 5, pHeap);
            break;
        case 3:
            mJobQueue[i].initialize(mLayer.size(), pHeap);
            break;
        }

        mJobQueue[i].clear();
    }

    detail::RootNode::setNodeMeta(this, "Icon = DRAW");
}

/**
 * Gets the name of a display.
 * @param index display index
 * @return display name
 */
sead::SafeString Renderer::getDisplayName(s32 index)
{
    return cDisplayName[index];
}

/**
 * Destroys the layers, displays and jobs.
 */
Renderer::~Renderer()
{
    for (auto& pLayer : mLayer)
    {
        if (pLayer)
        {
            delete pLayer;
            pLayer = nullptr;
        }
    }

    for (auto& rJobQueue : mJobQueue)
    {
        rJobQueue.clear();
    }

    mLayer.freeBuffer();
    mJobDraw.freeBuffer();
    mJobSubDraw.freeBuffer();
    mJobGPUCalc.freeBuffer();

    for (auto* pDisplay : mDisplay)
    {
        if (pDisplay)
        {
            delete pDisplay;
        }
    }

    mDisplay.freeBuffer();
}

/**
 * Updates the layers and displays for the current frame.
 * @param swapBuffer whether the display list buffers may be swapped
 */
void Renderer::calc(bool swapBuffer)
{
    mLayerListCS.lock();

    bool isChanged = false;
    for (auto* pLayer : mLayer)
    {
        if (pLayer && pLayer->mFlag.isOn(Layer::cFlag_ListDirty))
        {
            mFlag.set(1);
            if (pLayer->mFlag.isOn(1 << 11))
            {
                pLayer->mFlag.reset(1 << 4);
                isChanged = true;
            }

            pLayer->mFlag.reset(Layer::cFlag_ListDirty);
        }
    }

    if (mFlag.isOn(1))
    {
        if (isChanged)
        {
            for (auto* pLayer : mLayer)
            {
                if (pLayer && !pLayer->mFlag.isOn(1 << 11))
                {
                    pLayer->mFlag.set(1 << 4);
                }
            }
        }

        mFlag.reset(1);
    }

    for (auto* pDisplay : mDisplay)
    {
        pDisplay->mFlag.change(1 << 6, mFlag.isOn(1 << 2));
        pDisplay->calc();
        if (pDisplay->mFlag.isOn(RenderDisplay::cFlag_FrameBufferSizeChanged))
        {
            pDisplay->mFlag.reset(RenderDisplay::cFlag_FrameBufferSizeChanged);
            mFlag.set(1 << 4);
        }
    }

    for (auto* pDisplay : mDisplay)
    {
        if (mFlag.isOn(1 << 15) && mFlag.isOn(1 << 4))
        {
            pDisplay->freeFrameBuffer();
        }
    }

    if (mFlag.isOn(1 << 4) && mFlag.isOn(1 << 15))
    {
        for (s32 i = 0; i < mDisplay.size(); i++)
        {
            RenderDisplay* pShare = nullptr;
            if (i != 0 && mFlag.isOn(1 << 3))
            {
                pShare = mDisplay[0];
            }

            mDisplay(i)->allocFrameBuffer(
                MultiSampleType(mMultiSampleType), pShare, TextureFormat(mColorFormat),
                mColorTextureNum, TextureFormat(mDepthFormat),
                mFlag.isOn(1 << 5) ? mDepthTextureNum : -1, mFlag.isOn(1 << 11),
                mFlag.isOn(1 << 13));
        }
    }

    const sead::Controller* pController = nullptr;
    if (mDebugCameraState == cDebugCameraState_1)
    {
        pController = mDebugCameraController;
        if (pController)
        {
            if (mDebugFlag & 1)
            {
                if (pController->isTrig(1 << 5))
                {
                    mDebugCameraControllerIndex = mDebugCameraControllerIndex > 0 ?
                                                      0 :
                                                      mDebugCameraControllerIndex + 1;
                }
            }
            else
            {
                mDebugCameraControllerIndex = 0;
            }
        }
    }

    for (auto* pLayer : mLayer)
    {
        if (pLayer)
        {
            pLayer->_9a = 0;
            if (pLayer->isEnable())
            {
                pLayer->calc_(pController, mDebugCameraControllerIndex, (mDebugFlag >> 1) & 1);
            }
        }
    }

    if (mDebugCameraState == cDebugCameraState_None && mDebugCameraMessageTimer != 0)
    {
        mDebugCameraMessageTimer = 0;
    }

    for (s32 i = 0; i < mDisplay.size(); i++)
    {
        RenderDisplay* pDisplay = mDisplay(i);
        pDisplay->mFlag.change(1 << 7, mFlag.isOn(1 << 10));
        pDisplay->clear();
        pDisplay->mViewportOrigin = pDisplay->mViewport;

        u8 index = pDisplay->mDisplayIndex;
        s32 displayType = mDisplayControl == cDisplayControl_1 ? index == 0 : index;
        if (!pDisplay->mFlag.isOn(1))
        {
            for (auto* pLayer : mLayer)
            {
                if (pLayer && pLayer->isRenderingEnabled())
                {
                    s8 layerDisplayType = pLayer->mDisplayTypeOverride;
                    if (layerDisplayType == -1)
                    {
                        layerDisplayType = pLayer->mDisplayType;
                    }

                    if (displayType == layerDisplayType)
                    {
                        pDisplay->pushBack(pLayer);
                        pLayer->_9a |= 1 << i;
                    }
                }
            }
        }

        pDisplay->mFlag.change(RenderDisplay::cFlag_DrawDebugInfo, !mFlag.isOn(1 << 1));
    }

    bool isSwap = (mFlag & 0x300) == 0x100;
    mFlag.change(1 << 14, isSwap && swapBuffer);
    mDLBuffer.clear(isSwap && swapBuffer);
    mFlag.reset(1 << 4);
    mFlag.set(1 << 8);
    mFrameCounter++;

    for (auto& rJobQueue : mJobQueue)
    {
        rJobQueue.clear();
        for (u32 core = 0; core < sead::CoreInfo::getNumCores(); core++)
        {
            rJobQueue.setGranularity(core, mJobGranularity);
        }
    }

    mLayerListCS.unlock();
}

/**
 * Records the display lists of the displays selected by a mask.
 * @param useJobQueue whether the layer jobs are queued instead of run immediately
 * @param displayMask mask of the displays to record
 */
void Renderer::calcCommand(bool useJobQueue, u32 displayMask)
{
    if (mFrameCounter % mCalcInterval != 0)
    {
        return;
    }

    if (!isDisplayList_())
    {
        return;
    }

    sead::SafeArray<sead::FixedPtrArray<LayerJob, 256>, cJobQueueNum> jobs;
    for (auto& rJobs : jobs)
    {
        rJobs.clear();
    }

    if (useJobQueue)
    {
        for (s32 i = 0; i < mDisplay.size(); i++)
        {
            if (((1 << i) & displayMask) == 0)
            {
                continue;
            }

            sead::FixedPtrArray<LayerJob, 256> layerJobs;
            mDisplay(i)->calcDL(&layerJobs);
            for (auto& rJob : layerJobs)
            {
                s32 queue;
                if (rJob.mPriority <= _510)
                {
                    queue = 2;
                }
                else
                {
                    queue = rJob.mPriority <= _50c ? 1 : 0;
                }

                jobs(queue).pushBack(&rJob);
            }
        }
    }
    else
    {
        for (s32 i = 0; i < mDisplay.size(); i++)
        {
            if ((1 << i) & displayMask)
            {
                mDisplay(i)->calcDL(nullptr);
            }
        }
    }

    if (useJobQueue)
    {
        for (auto* pDisplay : mDisplay)
        {
            pDisplay->calcGPU(&jobs(3));
        }
    }

    for (s32 i = 0; i < cJobQueueNum; i++)
    {
        auto& rJobs = jobs[i];
        rJobs.heapSort_<LayerJob>(LayerJob::compare);
        for (auto& rJob : rJobs)
        {
            mJobQueue[i].enque(&rJob);
        }
    }
}

/**
 * Checks whether the renderer records display lists.
 * @return true if display lists are used
 */
bool Renderer::isDisplayList_() const
{
    return mDisplayListMode == 1;
}

/**
 * Sorts the recorded display lists and notifies the layers.
 */
void Renderer::postCalcCommand()
{
    if (isDisplayList_())
    {
        for (auto* pDisplay : mDisplay)
        {
            pDisplay->sortDL();
        }
    }

    for (auto* pLayer : mLayer)
    {
        if (pLayer)
        {
            pLayer->postCalcCommand_();
        }
    }
}

/**
 * Runs the GPU calculation of every display.
 * @param useJobQueue whether the GPU calculation was queued as jobs
 */
void Renderer::calcGPU(bool useJobQueue) const
{
    if (mFrameCounter % mCalcInterval != 0)
    {
        return;
    }

    if (!isDisplayList_())
    {
        return;
    }

    if (useJobQueue)
    {
        return;
    }

    for (auto* pDisplay : mDisplay)
    {
        pDisplay->calcGPU(nullptr);
    }
}

/**
 * Draws a display.
 * @param pDrawContext draw context to draw with
 * @param displayIndex index of the display to draw
 * @return false if nothing was drawn this frame
 */
bool Renderer::draw(DrawContext* pDrawContext, s32 displayIndex) const
{
    if (mFrameCounter % mCalcInterval != 0)
    {
        return false;
    }

    if (isDisplayList_())
    {
        if (mFlag.isOn(1 << 6))
        {
            mFlag.reset(1 << 6);
            mDisplay[displayIndex]->destroyDisplayList();
        }

        mDisplay[displayIndex]->callDisplayList(pDrawContext, mFlag.isOn(1 << 8));
        pDrawContext->invalidateShaderMode();
    }
    else
    {
        mDisplay[displayIndex]->draw(pDrawContext);
    }

    mDisplay[displayIndex]->copyScanOutBuffer_();
    return true;
}

/**
 * Sets a draw callback of the displays.
 * @param type callback type
 * @param pMethod draw method to call
 */
void Renderer::setCallback(CallbackType type, DrawMethod* pMethod)
{
    switch (type)
    {
    case cCallbackType_0:
        mDisplay[0]->mBeginDrawMethod = pMethod;
        break;
    case cCallbackType_1:
        mDisplay[1]->mBeginDrawMethod = pMethod;
        break;
    case cCallbackType_2:
        mDisplay[0]->mEndDrawMethod = pMethod;
        break;
    case cCallbackType_3:
        mDisplay[1]->mEndDrawMethod = pMethod;
        break;
    case cCallbackType_4:
        mDisplay[0]->mRenderStepDrawMethod = pMethod;
        mDisplay[1]->mRenderStepDrawMethod = pMethod;
        break;
    }
}

/**
 * Removes every draw method bound to an object from all layers.
 * @param pObject object the draw methods are bound to
 */
void Renderer::removeDrawMethodByObject(const void* pObject)
{
    mLayerListCS.lock();
    for (auto* pLayer : mLayer)
    {
        if (pLayer)
        {
            pLayer->removeDrawMethodByObject(pObject);
        }
    }

    mLayerListCS.unlock();
}

/**
 * Removes a draw method from all layers.
 * @param pMethod draw method to remove
 */
void Renderer::removeDrawMethod(const DrawMethod* pMethod)
{
    mLayerListCS.lock();
    for (auto* pLayer : mLayer)
    {
        if (pLayer)
        {
            pLayer->removeDrawMethod(pMethod);
        }
    }

    mLayerListCS.unlock();
}

/**
 * Removes a layer from the renderer and its displays.
 * @param pLayer layer to remove
 * @return true if the layer was registered
 */
bool Renderer::removeLayer(Layer* pLayer)
{
    mLayerListCS.lock();
    bool result = false;
    s32 index = searchLayerIndex(pLayer);
    if (index != -1)
    {
        for (auto* pDisplay : mDisplay)
        {
            pDisplay->erase(pLayer);
        }

        mLayer[index] = nullptr;
        pLayer->mJobDraw->finalize();
        pLayer->mJobSubDraw->finalize();
        pLayer->mJobGPUCalc->finalize();
        mFlag.set(1);
        result = true;
    }

    mLayerListCS.unlock();
    return result;
}

/**
 * Searches the slot index of a layer.
 * @param pLayer layer to search
 * @return slot index, or -1 if not found
 */
s32 Renderer::searchLayerIndex(const Layer* pLayer) const
{
    if (!pLayer)
    {
        return -1;
    }

    mLayerListCS.lock();
    s32 result = -1;
    for (auto it = mLayer.begin(); it != mLayer.end(); ++it)
    {
        if (*it == pLayer)
        {
            result = it.getIndex();
            break;
        }
    }

    mLayerListCS.unlock();
    return result;
}

/**
 * Searches the first empty layer slot.
 * @return slot index, or -1 if every slot is used
 */
s32 Renderer::searchEmptyLayerIndexFromFront() const
{
    mLayerListCS.lock();
    s32 result = -1;
    for (auto it = mLayer.begin(); it != mLayer.end(); ++it)
    {
        if (!*it)
        {
            result = it.getIndex();
            break;
        }
    }

    mLayerListCS.unlock();
    return result;
}

/**
 * Searches the last empty layer slot.
 * @return slot index, or -1 if every slot is used
 */
s32 Renderer::searchEmptyLayerIndexFromBack() const
{
    mLayerListCS.lock();
    s32 result = -1;
    for (s32 i = mLayer.size() - 1; i >= 0; i--)
    {
        if (!mLayer[i])
        {
            result = i;
            break;
        }
    }

    mLayerListCS.unlock();
    return result;
}

/**
 * Sets how a display clears its frame buffer.
 * @param clearFlag clear flags
 * @param rColor clear color
 * @param depth clear depth
 * @param stencil clear stencil
 * @param displayIndex index of the display
 */
void Renderer::setFrameBufferClear(u32 clearFlag, const sead::Color4f& rColor, f32 depth,
                                   u32 stencil, s32 displayIndex)
{
    RenderDisplay* pDisplay = mDisplay[displayIndex];
    pDisplay->mClearFlag = clearFlag;
    pDisplay->mClearColor = rColor;
    pDisplay->mClearDepth = depth;
    pDisplay->mClearStencil = stencil;
}

/**
 * Changes the debug camera state.
 * @param state new debug camera state
 */
void Renderer::changeDebugCameraState(DebugCameraState state)
{
    if (mDebugCameraState == state)
    {
        return;
    }

    mLayerListCS.lock();
    for (auto* pLayer : mLayer)
    {
        if (pLayer)
        {
            if (mDebugCameraState == cDebugCameraState_None)
            {
                pLayer->copyCurrentCameraToDebugCamera_();
                pLayer->copyCurrentProjectionToDebugProjection_();
            }

            pLayer->mDebugFlag.change(1, state != cDebugCameraState_None);
        }
    }

    mDebugCameraState = state;
    mDebugCameraMessageTimer = 60;
    if (state == cDebugCameraState_None)
    {
        mDebugCameraControllerIndex = 0;
    }

    mLayerListCS.unlock();
}

/**
 * Advances the debug camera state.
 */
void Renderer::changeDebugCameraStateNext()
{
    if (mDebugCameraState == cDebugCameraState_2)
    {
        changeDebugCameraState(cDebugCameraState_None);
    }
    else
    {
        changeDebugCameraState(DebugCameraState(mDebugCameraState + 1));
    }
}

/**
 * Resets the debug camera state.
 */
void Renderer::resetDebugCamera()
{
    mDebugCameraState = cDebugCameraState_None;
    mDebugCameraControllerIndex = 0;
    mDebugCameraMessageTimer = 0;
}

/**
 * Sets how layers are assigned to displays.
 * @param control display control mode
 */
void Renderer::setDisplayControl(DisplayControl control)
{
    mDisplayControl = control;
}

/**
 * Registers a layer in a slot and initializes it.
 * @param pLayer layer to register
 * @param layerIndex slot index
 * @param rName layer name
 * @param displayType display type of the layer
 * @param pHeap heap to allocate from
 */
void Renderer::initLayer_(Layer* pLayer, s32 layerIndex, const sead::SafeString& rName,
                          s32 displayType, sead::Heap* pHeap)
{
    pLayer->mRenderer = this;
    pLayer->mName.copy(rName);
    pLayer->mLayerIndex = layerIndex;
    pLayer->mFlag.set(Layer::cFlag_Initialized);
    pLayer->setDisplayType(displayType);
    pLayer->initialize_(pHeap);
    pLayer->mDisplayViewport = mDisplay[displayType]->mViewport;

    mLayerListCS.lock();
    mLayer[layerIndex] = pLayer;
    mJobDraw[layerIndex].initialize(LayerJob::cType_Draw, pLayer, pHeap);
    mJobSubDraw[layerIndex].initialize(LayerJob::cType_SubDraw, pLayer, pHeap);
    mJobGPUCalc[layerIndex].initialize(LayerJob::cType_GPUCalc, pLayer, pHeap);
    pLayer->mJobDraw = &mJobDraw[layerIndex];
    pLayer->mJobSubDraw = &mJobSubDraw[layerIndex];
    pLayer->mJobGPUCalc = &mJobGPUCalc[layerIndex];
    mFlag.set(1);
    mLayerListCS.unlock();
}

/**
 * Locks the layer list.
 */
void Renderer::lockLayerList_()
{
    mLayerListCS.lock();
}

/**
 * Unlocks the layer list.
 */
void Renderer::unlockLayerList_()
{
    mLayerListCS.unlock();
}

/**
 * Draws the renderer status (no-op in release builds).
 * @param rInfo render information
 */
void Renderer::drawStatus(const RenderInfo& rInfo) const {}

/**
 * Dumps information about the display lists of every display.
 */
void Renderer::dumpDisplayListInfo() const
{
    for (auto* pDisplay : mDisplay)
    {
        pDisplay->dumpDisplayListInfo();
    }
}

/**
 * Dumps the raw display lists of every display.
 */
void Renderer::dumpRawDisplayList() const
{
    for (auto* pDisplay : mDisplay)
    {
        pDisplay->dumpRawDisplayList();
    }
}

/**
 * Generates the host I/O messages of the renderer.
 * @param pContext host I/O context
 */
void Renderer::genMessage(sead::hostio::Context* pContext)
{
    for (s32 i = 1; i <= 2; i++)
    {
        sead::FormatFixedSafeString<1024> name("cJobQueueType_CalcDL%d", i);
    }

    {
        sead::FormatFixedSafeString<1024> usage("1DL max usage:%d[byte]", mDLBuffer.mMaxUsedSize);
    }

    {
        sead::FormatFixedSafeString<1024> usage("1DL max usage (ControlMemory):%d[byte]",
                                                mDLBuffer.mMaxControlMemoryUsed);
    }
}

/**
 * Handles a host I/O property event.
 * @param pEvent property event
 */
void Renderer::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if (isEventTarget(pEvent, mFlag) || isEventTarget(pEvent, mMultiSampleType) ||
        isEventTarget(pEvent, mColorFormat) || isEventTarget(pEvent, mDepthFormat) ||
        isEventTarget(pEvent, mColorTextureNum) || isEventTarget(pEvent, mDepthTextureNum) ||
        reinterpret_cast<uintptr_t>(pEvent->getId()) == 10002)
    {
        mFlag.set(1 << 4);
    }

    switch (reinterpret_cast<uintptr_t>(pEvent->getId()))
    {
    case 10001:
        changeDebugCameraStateNext();
        break;
    case 10003:
        mFlag.set(1 << 6);
        break;
    case 10004:
        dumpRawDisplayList();
        break;
    case 10005:
        dumpDisplayListInfo();
        break;
    }
}

}  // namespace agl::lyr
