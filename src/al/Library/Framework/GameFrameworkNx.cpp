#include "Library/Framework/GameFrameworkNx.hpp"

#include <nvn/nvn_FuncPtrInline.h>

#include <common/aglDisplayList.h>
#include <common/aglGPUMemAddr.h>
#include <common/aglGPUMemBlock.h>
#include <common/aglInitArg.h>
#include <common/aglRenderBuffer.h>
#include <common/aglRenderTarget.h>
#include <common/aglTextureData.h>
#include <controller/nin/seadNinDebugController.h>
#include <controller/nin/seadNinDebugPadDevice.h>
#include <controller/nin/seadNinJoyNpadDevice.h>
#include <controller/seadControllerMgr.h>
#include <framework/seadSingleScreenMethodTreeMgr.h>
#include <framework/seadTaskMgr.h>
#include <gfx/nin/seadGraphicsNvn.h>
#include <gfx/nvn/seadDebugFontMgrNvn.h>
#include <gfx/nvn/seadFrameBufferNvn.h>
#include <gfx/nvn/seadPrimitiveDrawMgrNvn.h>
#include <gfx/seadViewport.h>
#include <heap/seadExpHeap.h>
#include <heap/seadHeapMgr.h>
#include <system/seadExceptionHandler.h>
#include <thread/seadEvent.h>
#include <thread/seadThread.h>

#include "Library/Application/ApplicationMessageReceiver.hpp"
#include "Library/Controller/NpadController.hpp"
#include "Library/Controller/PadGyroAddon.hpp"
#include "Library/Memory/HeapUtil.hpp"
#include "Library/System/DrawSystemInfo.hpp"
#include "Library/System/GameSystemInfo.hpp"
#include "Project/Controller/JoyPadAccelerometerAddon.hpp"
#include "Project/Controller/PadTouchDevice.hpp"
#include "Project/Controller/PadUiKeyInputAddon.hpp"
#include "Project/Draw/GpuPerf.hpp"

namespace nn::os {
bool TryWaitSystemEvent(SystemEventType* pEvent);
void WaitSystemEvent(SystemEventType* pEvent);
}  // namespace nn::os

extern "C" {
/**
 * Ignores GPU errors.
 */
void gpuErrorCallback() {}
}

namespace al {
namespace {
using AddonList = sead::OffsetList<sead::ControllerAddon>;

/**
 * Makes a style set with every Npad style supported by the game.
 * @return the style set
 */
nn::hid::NpadStyleSet makeAllStyleSet() {
    nn::hid::NpadStyleSet styleSet;
    styleSet.Reset();
    styleSet.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleFullKey), true);
    styleSet.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleHandheld), true);
    styleSet.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyDual), true);
    styleSet.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyLeft), true);
    styleSet.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyRight), true);
    return styleSet;
}

/**
 * Creates a GPU memory block and allocates its buffer.
 * @param size buffer size in bytes
 * @param pHeap heap to allocate from
 * @param alignment buffer alignment
 * @param attribute memory attribute
 * @return the created block
 */
agl::GPUMemBlockU8* createGpuMemBlock(u64 size, sead::Heap* pHeap, s32 alignment,
                                      agl::MemoryAttribute attribute) {
    auto* block = new (pHeap) agl::GPUMemBlockU8;
    block->allocBuffer_(size, pHeap, alignment, attribute);
    return block;
}

/**
 * Sets whether the draw system info refers to the docked render buffer.
 * @param pInfo draw system info
 * @param isDocked whether the console is docked
 */
void setDocked(DrawSystemInfo* pInfo, bool isDocked) {
    pInfo->mIsDocked = isDocked;
}

/**
 * Sets the image memory of a texture.
 * @param pTexture texture data
 * @param addr image memory
 */
void setImage(agl::TextureData* pTexture, agl::GPUMemVoidAddr addr) {
    pTexture->setImagePtr(addr, 0);
}

/**
 * Allocates the control memory of both display lists.
 * @param pMemory receives the two control memory buffers
 * @param rArg agl initialization settings
 */
void allocControlMemory(void** pMemory, const GameFrameworkNx::AglInitArg& rArg) {
    s32 alignment = 0;
    nvnDeviceGetInteger(sead::GraphicsNvn::instance()->getNvnDevice(),
                        NVN_DEVICE_INFO_COMMAND_BUFFER_CONTROL_ALIGNMENT, &alignment);
    pMemory[0] = rArg.heap->alloc(0x180000, alignment);
    pMemory[1] = rArg.heap->alloc(0x180000, alignment);
}
}  // namespace

GameFrameworkNx* GameFrameworkNx::sInstance = nullptr;

/**
 * Creates the framework and the application message receiver.
 * @param rArg framework creation settings
 */
GameFrameworkNx::GameFrameworkNx(const CreateArg& rArg) : sead::GameFrameworkNx(rArg) {
    sInstance = this;
    sead::ExceptionHandler::initialize();
    mMessageReceiver = new ApplicationMessageReceiver;
    mMessageReceiver->init();
    _140 = reinterpret_cast<void*>(&gpuErrorCallback);
    mDrawReadyEvent = new sead::Event(true);
    mDrawReadyEvent->setSignal();
}

/**
 * Creates the controller manager with its devices and controllers.
 * @param pBase the parent task
 */
void GameFrameworkNx::createControllerMgr(sead::TaskBase* pBase) {
    sead::TaskBase::SystemMgrTaskArg arg(&sead::TTaskFactory<sead::ControllerMgr>);
    arg.parent = pBase;
    auto* param = new sead::ControllerMgr::Parameter;
    param->controllerMax = 16;
    param->proc = nullptr;
    arg.parameter = param;
    mTaskMgr->createSingletonTaskSync<sead::ControllerMgr>(arg);

    sead::ControllerMgr* mgr = sead::ControllerMgr::instance();
    sead::Heap* heap = pBase->mHeapArray.getPrimaryHeap();
    sead::ScopedCurrentHeapSetter setter(heap);

    mgr->getControlDeviceList().pushBack(new (heap) sead::NinDebugPadDevice(mgr));

    auto* npadDevice = new (heap) sead::NinJoyNpadDevice(mgr, heap);
    npadDevice->setNpadIdUpdateNum(4);
    npadDevice->setSupportedNpadStyleSet(makeAllStyleSet());
    npadDevice->setNpadJoyHoldType(nn::hid::NpadJoyHoldType::Horizontal);
    mgr->getControlDeviceList().pushBack(npadDevice);

    mgr->getControlDeviceList().pushBack(new (heap) PadTouchDevice(mgr));
    mgr->getControllerList().pushBack(new (heap) sead::NinDebugController(mgr));

    for (s32 i = 0; i < 4; i++) {
        auto* controller = new (heap) NpadController(mgr);

        if (i == 0) {
            controller->setAnyControllerMode(true);
        } else {
            controller->setIndexControllerMode(i, true);
        }

        AddonList& addons = controller->getAddonList();
        addons.pushBack(new (heap) JoyPadAccelerometerAddon(controller, 0));
        addons.pushBack(new (heap) JoyPadAccelerometerAddon(controller, 1));
        addons.pushBack(new (heap) PadGyroAddon(controller, 0));
        addons.pushBack(new (heap) PadGyroAddon(controller, 1));
        addons.pushBack(new (heap) PadUiKeyInputAddon(controller));
        npadDevice->setNpadJoyAssignmentModeDual(i);
        mgr->getControllerList().pushBack(controller);
    }

    mgr->getControllerList().pushBack(new (heap) PadTouchController(mgr));
}

/**
 * Initializes agl, the render buffers and the display lists.
 * @param rArg agl initialization settings
 */
void GameFrameworkNx::initAgl(const AglInitArg& rArg) {
    agl::InitArg initArg;
    sead::ExpHeap* aglHeap = sead::ExpHeap::create(
        rArg.aglHeapSize, "AglHeap", sead::HeapMgr::instance()->getCurrentHeap(), 8,
        sead::Heap::cHeapDirection_Forward, false);
    addNamedHeap(aglHeap, "AglHeap");
    initArg.mHeap = aglHeap;
    initArg.mWorkHeapSize = rArg.workHeapSize;
    initArg.mMinGPUMemBlockSize = rArg.minGPUMemBlockSize;
    initArg.mDynamicTextureSize = rArg.dynamicTextureSize;
    agl::Initialize(initArg);

    mDrawContext = new (rArg.heap) agl::DrawContext;
    mGpuPerf = new GpuPerf;

    mDockedRenderBuffer = new (rArg.heap)
        agl::RenderBuffer(sead::Vector2f(rArg.virtualWidth, rArg.virtualHeight), 0.0f, 0.0f,
                          rArg.dockedWidth, rArg.dockedHeight);
    mDockedRenderTargetColor = new (rArg.heap) agl::RenderTargetColor;
    mDockedRenderBuffer->setRenderTargetColor(mDockedRenderTargetColor);

    auto* dockedTexture = new (rArg.heap) agl::TextureData;
    dockedTexture->initialize_(agl::TextureType::cTextureType_2D,
                               agl::TextureFormat::cTextureFormat_R10_G10_B10_A2_uNorm,
                               rArg.dockedWidth, rArg.dockedHeight, 1, 1, agl::TextureAttribute(),
                               agl::MultiSampleType(), true);
    agl::GPUMemBlockU8* imageBlock =
        createGpuMemBlock(dockedTexture->getImageByteSize(), rArg.heap,
                          dockedTexture->getAlignment(), agl::MemoryAttribute::Default);
    agl::GPUMemVoidAddr imageAddr;
    imageAddr = agl::GPUMemVoidAddr(*imageBlock, 0);
    setImage(dockedTexture, imageAddr);
    mDockedRenderTargetColor->applyTextureData(*dockedTexture);

    mHandheldRenderBuffer = new (rArg.heap)
        agl::RenderBuffer(sead::Vector2f(rArg.virtualWidth, rArg.virtualHeight), 0.0f, 0.0f,
                          rArg.handheldWidth, rArg.handheldHeight);
    mHandheldRenderTargetColor = new (rArg.heap) agl::RenderTargetColor;
    mHandheldRenderBuffer->setRenderTargetColor(mHandheldRenderTargetColor);

    auto* handheldTexture = new (rArg.heap) agl::TextureData;
    handheldTexture->initialize_(agl::TextureType::cTextureType_2D,
                                 agl::TextureFormat::cTextureFormat_R10_G10_B10_A2_uNorm,
                                 rArg.handheldWidth, rArg.handheldHeight, 1, 1,
                                 agl::TextureAttribute(), agl::MultiSampleType(), true);
    setImage(handheldTexture, imageAddr);
    mHandheldRenderTargetColor->applyTextureData(*handheldTexture);

    agl::RenderBuffer::sIsSRGBWrite = true;
    mOverrideFrameBuffer = mHandheldRenderBuffer;

    allocControlMemory(mControlMemory, rArg);

    mDisplayLists[0] = new (rArg.heap) agl::DisplayList;
    mDisplayLists[0]->setName("コマンド０");
    mDisplayLists[0]->setControlMemory(mControlMemory[0], 0x180000);
    mDisplayLists[0]->setBuffer(
        agl::GPUMemAddr<u8>(*createGpuMemBlock(0x400000, rArg.heap, 4, agl::MemoryAttribute::_00),
                            0),
        0x400000);

    mDisplayLists[1] = new (rArg.heap) agl::DisplayList;
    mDisplayLists[1]->setName("コマンド１");
    mDisplayLists[1]->setControlMemory(mControlMemory[1], 0x180000);
    mDisplayLists[1]->setBuffer(
        agl::GPUMemAddr<u8>(*createGpuMemBlock(0x400000, rArg.heap, 4, agl::MemoryAttribute::_00),
                            0),
        0x400000);

    mCurrentDisplayList = nullptr;
    mDrawSystemInfo = new DrawSystemInfo(mDockedRenderBuffer, mHandheldRenderBuffer, mDrawContext);
}

/**
 * Creates the infinite loop checker unless it is disabled.
 * @param pBase the parent task
 * @param rSpan unused, the check span is always four seconds
 * @param num passed through to the checker
 */
void GameFrameworkNx::createInfLoopChecker(sead::TaskBase* pBase,
                                           [[maybe_unused]] const sead::TickSpan& rSpan, s32 num) {
    if (mIsDisableInfLoopChecker) {
        return;
    }

    sead::GameFramework::createInfLoopChecker(pBase, sead::TickSpan::makeFromSeconds(4), num);
}

/**
 * Binds the current render buffer and clears it if enabled.
 */
void GameFrameworkNx::clearFrameBuffer() {
    agl::RenderBuffer* renderBuffer = getCurrentRenderBuffer();
    renderBuffer->bind(mDrawContext);

    if (mIsClearRenderBuffer) {
        renderBuffer->fastClear(mDrawContext, 0, 7, mCreateArg.clear_color, 1.0f, 0,
                                sead::Viewport(*renderBuffer), true);
    }
}

/**
 * Gets the frame buffer methods draw to.
 * @param methodType unused
 * @return the docked or handheld render buffer
 */
sead::FrameBuffer* GameFrameworkNx::getMethodFrameBuffer([[maybe_unused]] s32 methodType) const {
    return getCurrentRenderBuffer();
}

/**
 * Starts recording the frame's display list.
 */
void GameFrameworkNx::startCommandList() {
    if (isSkipDrawFrame()) {
        mCurrentDisplayList = nullptr;
        return;
    }

    mGpuPerf->update();
    mCurrentDisplayList = mDisplayLists[mDisplayListIndex];
    mDrawContext->setCommandBuffer(mCurrentDisplayList);
    mCurrentDisplayList->beginDisplayList();
    mGpuPerf->beginPerf(mDrawContext);

    nvnCommandBufferReportCounter(mCurrentDisplayList->getNvnCommandBuffer(),
                                  NVN_COUNTER_TYPE_TIMESTAMP_TOP,
                                  nvnBufferGetAddress(mCounterBuffer));
    nvnCommandBufferSetSamplerPool(mCurrentDisplayList->getNvnCommandBuffer(),
                                   sead::GraphicsNvn::instance()->getSamplerPool());
    nvnCommandBufferSetTexturePool(mCurrentDisplayList->getNvnCommandBuffer(),
                                   sead::GraphicsNvn::instance()->getTexturePool());
    nvnCommandBufferSetShaderScratchMemory(mCurrentDisplayList->getNvnCommandBuffer(),
                                           mShaderScratchMemoryPool, 0, mShaderScratchMemorySize);

    setDocked(mDrawSystemInfo, mIsDocked);
    mOverrideFrameBuffer = !mIsDocked ? mHandheldRenderBuffer : mDockedRenderBuffer;
    mOverrideFrameBuffer->bind(mDrawContext);
}

/**
 * Ends recording the frame's display list and optionally presents it.
 * @param isPresent whether to present and wait for the GPU
 */
void GameFrameworkNx::endCommandList(bool isPresent) {
    if (isSkipDrawFrame()) {
        return;
    }

    mCurrentDisplayList->endDisplayList();
    mCommandHandle = mCurrentDisplayList->getHandle();
    mPrevDisplayListIndex = mDisplayListIndex;
    mDisplayListIndex = 1 - mDisplayListIndex;

    if (!isPresent) {
        return;
    }

    mDrawReadyEvent->resetSignal();

    if (!mIsPresentAsync) {
        present_();
    }

    waitForGpuDone_();
    finishGpuProfile();
}

/**
 * Stores the GPU time stamps and measures the frame time.
 */
void GameFrameworkNx::finishGpuProfile() {
    nn::os::GetSystemTick();
    setGpuTimeStamp_();
    mFrameTicks = nn::os::GetSystemTick().GetInt64Value() - mPrevFrameTick;
    mPrevFrameTick = nn::os::GetSystemTick().GetInt64Value();
}

/**
 * Processes one frame.
 */
void GameFrameworkNx::procFrame_() {
    switch (mRequestChangeUseGpu) {
    case 0:
        break;
    case 1:
        mIsUseGpu = true;
        mRequestChangeUseGpu = 0;
        break;
    case 2:
        mIsUseGpu = false;
        mRequestChangeUseGpu = 0;
        break;
    default:
        mRequestChangeUseGpu = 0;
        break;
    }

    if (mDisplayStarted == 1) {
        if (mIsEnableDisplayStart) {
            mDisplayStarted = 2;
        }
    } else if (mDisplayStarted == 0) {
        mDisplayStarted = 1;
    }

    updateCpuBoost();
    mMessageReceiver->update();

    if (mMessageReceiver->isResumed() || mMessageReceiver->isUpdatedOperationMode() ||
        mMessageReceiver->isBackground()) {
        mDisplayStarted = 0;
    }

    mTaskMgr->afterCalc();

    if (mIsPresentAsync && mCurrentDisplayList != nullptr) {
        present_();
    }

    if (_27b && !nn::os::TryWaitSystemEvent(&mVsyncEvent)) {
        nn::os::WaitSystemEvent(&mVsyncEvent);
    }

    procCalc_();
    startCommandList();
    procDraw_();
    procReset_();
    endCommandList(true);
    finishGpuProfile();

    if (_27b) {
        _27c = !_27c;
    }
}

/**
 * Draws the frame into the current display list.
 */
void GameFrameworkNx::procDraw_() {
    if (isSkipDrawFrame()) {
        return;
    }

    if (mUnk6 != nullptr) {
        mUnk6(true);
    }

    sead::DynamicCast<sead::SingleScreenMethodTreeMgr>(mMethodTreeMgr)->draw();

    sead::CriticalSection* cs = sead::GraphicsNvn::instance()->getCriticalSection1();
    cs->lock();
    mOverrideFrameBuffer->copyToDisplayBuffer(mDrawContext, mDisplayBuffer);
    cs->unlock();

    nvnCommandBufferReportCounter(mCurrentDisplayList->getNvnCommandBuffer(),
                                  NVN_COUNTER_TYPE_TIMESTAMP,
                                  nvnBufferGetAddress(mCounterBuffer) + 0x10);
    mGpuPerf->endPerf(mDrawContext);

    if (sead::PrimitiveDrawMgrNvn::instance() != nullptr) {
        sead::PrimitiveDrawMgrNvn::instance()->swapUniformBlockBuffer();
    }

    if (sead::DebugFontMgrNvn::instance() != nullptr) {
        sead::DebugFontMgrNvn::instance()->swapUniformBlockBuffer();
    }

    if (sead::DebugFontMgrJis1Nvn::instance() != nullptr) {
        sead::DebugFontMgrJis1Nvn::instance()->swapUniformBlockBuffer();
    }

    if (mUnk6 != nullptr) {
        mUnk6(false);
    }
}

/**
 * Waits for the GPU and the vertical blank, then for the rest of the frame time when frame
 * stepping.
 */
void GameFrameworkNx::waitForGpuDone_() {
    if (mIsUseGpu) {
        sead::CriticalSection* cs = sead::GraphicsNvn::instance()->getCriticalSection1();
        cs->lock();

        if (mGpuWaitCallback != nullptr) {
            mGpuWaitCallback(0);
        }

        if (mDisplayStarted == 2) {
            mDisplayBuffer->waitAcquireDone();
        } else {
            waitVsyncEvent_();
        }

        nvnSyncWait(mGpuSync, u64(-1));

        if (mGpuWaitCallback != nullptr) {
            mGpuWaitCallback(1);
        }

        cs->unlock();
    } else if (mCreateArg.vblank_wait_interval != 0) {
        waitVsyncEvent_();
    }

    mDisplayBuffer->applyChangeWindowCrop();

    if (mCreateArg.is_apply_deferred_finalizes) {
        sead::GraphicsNvn::instance()->applyDeferredFinalizes();
    }

    if (mFrameStepFlags & cFrameStep_Enabled) {
        const s64 frameTicks = f32(mCreateArg.vblank_wait_interval) / 30.0f *
                               f32(sead::TickSpan::makeFromSeconds(1).toS64());
        const s64 prevTick = mPrevFrameTick;
        const s64 sleepTicks = prevTick - nn::os::GetSystemTick().GetInt64Value() + frameTicks;

        if (sleepTicks > 0) {
            sead::Thread::sleep(sead::TickSpan(sleepTicks));
        }
    }
}

/**
 * Presents the frame and signals that drawing may continue.
 */
void GameFrameworkNx::present_() {
    sead::GameFrameworkNx::present_();
    mDrawReadyEvent->setSignal();
}
}  // namespace al
