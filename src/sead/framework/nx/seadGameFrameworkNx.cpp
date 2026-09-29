#include <framework/nx/seadGameFrameworkNx.h>

#include <cstdlib>

#include <nn/mem.h>
#include <nn/vi.h>
#include <nv.h>
#include <nvn/nvn_FuncPtrInline.h>

#include <framework/nx/seadPerformanceMgrNx.h>
#include <framework/seadSingleScreenMethodTreeMgr.h>
#include <framework/seadTaskMgr.h>
#include <gfx/nin/seadGraphicsNvn.h>
#include <gfx/nvn/seadDebugFontMgrNvn.h>
#include <gfx/nvn/seadFrameBufferNvn.h>
#include <gfx/nvn/seadPrimitiveDrawMgrNvn.h>
#include <gfx/seadDrawContext.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>
#include <mc/seadCoreInfo.h>
#include <prim/seadDelegate.h>
#include <thread/seadDelegateThread.h>
#include <thread/seadThreadUtil.h>

namespace nn::os
{
void WaitSystemEvent(SystemEventType* pEvent);
}  // namespace nn::os

namespace sead
{
/**
 * Initializes the framework and the performance manager.
 * @param rArg the initialization settings
 */
void GameFrameworkNx::initialize(const Framework::InitializeArg& rArg)
{
    GameFramework::initialize(rArg);
    PerformanceMgrNx::initialize();
}

/**
 * Constructs the framework from its creation settings.
 * @param rArg the creation settings
 */
GameFrameworkNx::GameFrameworkNx(const CreateArg& rArg)
    : mCreateArg(rArg), mFrameTicks(0), mPrevFrameTick(nn::os::GetSystemTick().GetInt64Value()),
      mMethodFrameBuffer(nullptr),
      mMethodLogicalFrameBuffer(Vector2f(1.0f, 1.0f), 0.0f, 0.0f, 1.0f, 1.0f), mVBlankWaitTicks(0),
      mDisplayBuffer(nullptr), mGpuWaitCallback(nullptr), _140(nullptr),
      mCommandMemoryPool(nullptr), mControlMemory(nullptr), mCommandBuffer(nullptr),
      mCounterBuffer(nullptr), mCounterData(nullptr), mShaderScratchMemoryPool(nullptr),
      mShaderScratchMemorySize(0), mGraphicsDevToolsAllocator(nullptr), mCommandMemoryUsedMax(0),
      mControlMemoryUsedMax(0), mQueue(nullptr), mOverrideFrameBuffer(nullptr), mDisplay(nullptr),
      mLayer(nullptr), mPresentationThread(nullptr), mGpuSync(nullptr), mCaption(""),
      mCommandHandle(0), mIsPresentAsync(true), mIsPresentDone(false), mFrameStepFlags(0),
      mIsUseGpu(true), mRequestChangeUseGpu(0)
{
    mUnk6 = nullptr;
}

/**
 * Destroys the framework.
 */
GameFrameworkNx::~GameFrameworkNx()
{
    SEAD_ASSERT(mPresentationThread != nullptr);
}

/**
 * Allocates graphics driver memory.
 * @param size the size to allocate
 * @param alignment the alignment of the allocation
 * @return the allocated memory
 */
static void* allocateGraphicsMemory_(size_t size, size_t alignment, void*)
{
    return aligned_alloc(alignment, size);
}

/**
 * Frees graphics driver memory.
 * @param pMemory the memory to free
 */
static void freeGraphicsMemory_(void* pMemory, void*)
{
    free(pMemory);
}

/**
 * Reallocates graphics driver memory.
 * @param pMemory the memory to reallocate
 * @param newSize the new size
 * @return the reallocated memory
 */
static void* reallocateGraphicsMemory_(void* pMemory, size_t newSize, void*)
{
    return realloc(pMemory, newSize);
}

/**
 * Allocates graphics development tools memory from a standard allocator.
 * @param size the size to allocate
 * @param alignment the alignment of the allocation
 * @param pUserData the standard allocator
 * @return the allocated memory
 */
static void* allocateGraphicsDevToolsMemory_(size_t size, size_t alignment, void* pUserData)
{
    return static_cast<nn::mem::StandardAllocator*>(pUserData)->Allocate(size, alignment);
}

/**
 * Frees graphics development tools memory to a standard allocator.
 * @param pMemory the memory to free
 * @param pUserData the standard allocator
 */
static void freeGraphicsDevToolsMemory_(void* pMemory, void* pUserData)
{
    static_cast<nn::mem::StandardAllocator*>(pUserData)->Free(pMemory);
}

/**
 * Reallocates graphics development tools memory from a standard allocator.
 * @param pMemory the memory to reallocate
 * @param newSize the new size
 * @param pUserData the standard allocator
 * @return the reallocated memory
 */
static void* reallocateGraphicsDevToolsMemory_(void* pMemory, size_t newSize, void* pUserData)
{
    return static_cast<nn::mem::StandardAllocator*>(pUserData)->Reallocate(pMemory, newSize);
}

/**
 * Initializes the graphics driver, the display, the NVN device, queue and command buffer, and the
 * presentation thread.
 * @param pHeap the heap to allocate the graphics objects from
 * @param rVirtualSize the virtual size of the method frame buffer
 */
void GameFrameworkNx::initializeGraphicsSystem(Heap* pHeap, const Vector2f& rVirtualSize)
{
    nv::SetGraphicsAllocator(allocateGraphicsMemory_, freeGraphicsMemory_,
                             reallocateGraphicsMemory_, nullptr);
    nv::InitializeGraphics(pHeap->alloc(mCreateArg.graphics_memory_size, 0x1000),
                           mCreateArg.graphics_memory_size);

    if (mCreateArg.graphics_devtools_memory_size != 0)
    {
        void* memory = pHeap->alloc(mCreateArg.graphics_devtools_memory_size, 8);
        mGraphicsDevToolsAllocator = new (pHeap, 8)
            nn::mem::StandardAllocator(memory, mCreateArg.graphics_devtools_memory_size);
        nv::SetGraphicsDevtoolsAllocator(
            allocateGraphicsDevToolsMemory_, freeGraphicsDevToolsMemory_,
            reallocateGraphicsDevToolsMemory_, mGraphicsDevToolsAllocator);
    }

    nn::vi::Initialize();
    nn::vi::OpenDefaultDisplay(&mDisplay);
    nn::vi::CreateLayer(&mLayer, mDisplay);
    nn::vi::GetDisplayVsyncEvent(&mVsyncEvent, mDisplay);

    mMethodLogicalFrameBuffer.setVirtualSize(rVirtualSize);
    mMethodLogicalFrameBuffer.setPhysicalArea(0.0f, 0.0f, f32(mCreateArg.display_width),
                                              f32(mCreateArg.display_height));

    auto deviceInitialize =
        reinterpret_cast<PFNNVNDEVICEINITIALIZEPROC>(nvnBootstrapLoader("nvnDeviceInitialize"));
    auto deviceGetProcAddress = reinterpret_cast<PFNNVNDEVICEGETPROCADDRESSPROC>(
        nvnBootstrapLoader("nvnDeviceGetProcAddress"));
    if (!deviceInitialize || !deviceGetProcAddress)
    {
        return;
    }

    nvnLoadCProcs(nullptr, deviceGetProcAddress);

    NVNdevice* device;
    {
        NVNdeviceBuilder deviceBuilder;
        nvnDeviceBuilderSetDefaults(&deviceBuilder);
        nvnDeviceBuilderSetFlags(&deviceBuilder,
                                 mCreateArg.is_debug ?
                                     GraphicsNvn::convertNvnDebugLevel(mCreateArg.nvn_debug_level) :
                                     0);

        device = new (pHeap, 8) NVNdevice;
        if (!deviceInitialize(device, &deviceBuilder))
        {
            return;
        }
    }

    nvnLoadCProcs(device, deviceGetProcAddress);

    {
        int majorVersion;
        int minorVersion;
        nvnDeviceGetInteger(device, NVN_DEVICE_INFO_API_MAJOR_VERSION, &majorVersion);
        nvnDeviceGetInteger(device, NVN_DEVICE_INFO_API_MINOR_VERSION, &minorVersion);
        if (majorVersion != 53 || minorVersion < 313)
        {
            return;
        }
    }

    if (mCreateArg.is_debug || mCreateArg.is_apply_deferred_finalizes)
    {
        nvnDeviceInstallDebugCallback(
            device, reinterpret_cast<PFNNVNDEBUGCALLBACKPROC>(GraphicsNvn::nvnDebugCallback),
            nullptr, true);
    }

    {
        GraphicsNvn::CreateArg graphicsArg;
        graphicsArg.mNvnDevice = device;
        graphicsArg.mTextureDescriptorNum = mCreateArg.texture_descriptor_num;
        graphicsArg.mIsApplyDeferredFinalizes = mCreateArg.is_apply_deferred_finalizes;
        graphicsArg._d = mCreateArg.is_debug || mCreateArg.is_apply_deferred_finalizes;
        Graphics::sInstance = new (pHeap, 8) GraphicsNvn(graphicsArg);
        Graphics::sInstance->initialize(pHeap);
    }

    {
        NVNqueueBuilder queueBuilder;
        nvnQueueBuilderSetDefaults(&queueBuilder);
        nvnQueueBuilderSetFlags(&queueBuilder, NVN_QUEUE_BUILDER_FLAGS_NO_FRAGMENT_INTERLOCK);
        nvnQueueBuilderSetDevice(&queueBuilder, GraphicsNvn::instance()->getNvnDevice());

        int memorySize = mCreateArg.queue_compute_memory_size;
        if (memorySize == -1)
        {
            nvnDeviceGetInteger(device, NVN_DEVICE_INFO_QUEUE_COMPUTE_MEMORY_DEFAULT_SIZE,
                                &memorySize);
        }
        nvnQueueBuilderSetComputeMemorySize(&queueBuilder, memorySize);

        memorySize = mCreateArg.queue_command_memory_size;
        if (memorySize == -1)
        {
            nvnDeviceGetInteger(device, NVN_DEVICE_INFO_QUEUE_COMMAND_MEMORY_DEFAULT_SIZE,
                                &memorySize);
        }
        nvnQueueBuilderSetCommandMemorySize(&queueBuilder, memorySize);
        nvnQueueBuilderSetCommandFlushThreshold(&queueBuilder, memorySize);

        memorySize = mCreateArg.queue_control_memory_size;
        if (memorySize == -1)
        {
            nvnDeviceGetInteger(device, NVN_DEVICE_INFO_QUEUE_CONTROL_MEMORY_DEFAULT_SIZE,
                                &memorySize);
        }
        nvnQueueBuilderSetControlMemorySize(&queueBuilder, memorySize);

        size_t queueMemorySize = nvnQueueBuilderGetQueueMemorySize(&queueBuilder);
        nvnQueueBuilderSetQueueMemory(&queueBuilder, pHeap->alloc(queueMemorySize, 0x1000),
                                      queueMemorySize);

        mQueue = new (pHeap, 8) NVNqueue;
        nvnQueueInitialize(mQueue, &queueBuilder);
        GraphicsNvn::instance()->registerQueue(mQueue);
    }

    mDisplayBuffer = new (pHeap, 8) DisplayBufferNvn();
    {
        void* nativeWindow;
        nn::vi::GetNativeWindow(&nativeWindow, mLayer);
        mDisplayBuffer->setNativeWindow(nativeWindow);
    }
    mDisplayBuffer->setPresentInterval(mCreateArg.vblank_wait_interval);
    mDisplayBuffer->setTripleBuffer(mCreateArg.is_triple_buffer);
    mDisplayBuffer->initialize(
        Vector2f(f32(mCreateArg.display_width), f32(mCreateArg.display_height)), pHeap);

    if (mCreateArg.vblank_wait_interval != 0)
    {
        mVBlankWaitTicks = (f32(mCreateArg.vblank_wait_interval) + 0.02f) / 60.0f *
                           f32(TickSpan::makeFromSeconds(1).toS64());
    }

    GraphicsNvn::instance()->registerDisplayBufferNvn(mDisplayBuffer);

    static const size_t cCommandMemoryPoolSize =
        MathSizeT::roundUp(mCreateArg.command_memory_size + 0x20, 0x1000);

    NVNdevice* graphicsDevice = GraphicsNvn::instance()->getNvnDevice();
    mCommandBuffer = new (pHeap, 8) NVNcommandBuffer;
    nvnCommandBufferInitialize(mCommandBuffer, graphicsDevice);

    mCommandMemoryPool = new (pHeap, 8) NVNmemoryPool;
    {
        NVNmemoryPoolBuilder poolBuilder;
        nvnMemoryPoolBuilderSetDefaults(&poolBuilder);
        nvnMemoryPoolBuilderSetDevice(&poolBuilder, graphicsDevice);
        nvnMemoryPoolBuilderSetFlags(&poolBuilder, NVN_MEMORY_POOL_FLAGS_CPU_UNCACHED |
                                                       NVN_MEMORY_POOL_FLAGS_GPU_UNCACHED);
        nvnMemoryPoolBuilderSetStorage(&poolBuilder, new (pHeap, 0x1000) u8[cCommandMemoryPoolSize],
                                       cCommandMemoryPoolSize);
        nvnMemoryPoolInitialize(mCommandMemoryPool, &poolBuilder);

        {
            int controlAlignment = 0;
            nvnDeviceGetInteger(graphicsDevice, NVN_DEVICE_INFO_COMMAND_BUFFER_CONTROL_ALIGNMENT,
                                &controlAlignment);
            mControlMemory = pHeap->alloc(mCreateArg.control_memory_size, controlAlignment);
        }
        nvnCommandBufferSetMemoryCallback(mCommandBuffer, outOfMemoryCallback_);
        nvnCommandBufferSetMemoryCallbackData(mCommandBuffer, mCommandBuffer);
        GraphicsNvn::instance()->registerDefaultCommandBuffer(mCommandBuffer);

        {
            int scratchAlignment;
            int scratchGranularity;
            int scratchScale;
            nvnDeviceGetInteger(device, NVN_DEVICE_INFO_SHADER_SCRATCH_MEMORY_ALIGNMENT,
                                &scratchAlignment);
            nvnDeviceGetInteger(device, NVN_DEVICE_INFO_SHADER_SCRATCH_MEMORY_GRANULARITY,
                                &scratchGranularity);
            nvnDeviceGetInteger(device,
                                NVN_DEVICE_INFO_SHADER_SCRATCH_MEMORY_SCALE_FACTOR_RECOMMENDED,
                                &scratchScale);
            u32 scratchSize = mCreateArg.shader_scratch_memory_scale * scratchScale;
            scratchSize =
                (scratchSize + scratchAlignment - 1) / scratchAlignment * scratchAlignment;
            scratchSize =
                (scratchSize + scratchGranularity - 1) / scratchGranularity * scratchGranularity;
            mShaderScratchMemorySize = (scratchSize + 0xfff) & ~0xfff;
        }

        mShaderScratchMemoryPool = new (pHeap, 8) NVNmemoryPool;
        nvnMemoryPoolBuilderSetDefaults(&poolBuilder);
        nvnMemoryPoolBuilderSetDevice(&poolBuilder, GraphicsNvn::instance()->getNvnDevice());
        nvnMemoryPoolBuilderSetFlags(&poolBuilder, NVN_MEMORY_POOL_FLAGS_CPU_NO_ACCESS |
                                                       NVN_MEMORY_POOL_FLAGS_GPU_CACHED);
        nvnMemoryPoolBuilderSetStorage(&poolBuilder,
                                       new (pHeap, 0x1000) u8[mShaderScratchMemorySize],
                                       mShaderScratchMemorySize);
        nvnMemoryPoolInitialize(mShaderScratchMemoryPool, &poolBuilder);
    }

    {
        NVNbufferBuilder bufferBuilder;
        nvnBufferBuilderSetDevice(&bufferBuilder, GraphicsNvn::instance()->getNvnDevice());
        nvnBufferBuilderSetDefaults(&bufferBuilder);
        nvnBufferBuilderSetStorage(&bufferBuilder, mCommandMemoryPool,
                                   mCreateArg.command_memory_size, 0x20);
        mCounterBuffer = new (pHeap, 8) NVNbuffer;
        nvnBufferInitialize(mCounterBuffer, &bufferBuilder);
        mCounterData = static_cast<NVNcounterData*>(nvnBufferMap(mCounterBuffer));
    }

    mGpuSync = new (pHeap, 8) NVNsync;
    nvnSyncInitialize(mGpuSync, device);

    if (mCreateArg.create_method_frame_buffer)
    {
        mMethodFrameBuffer = FrameBufferNvn::create(pHeap, rVirtualSize, mCreateArg.display_width,
                                                    mCreateArg.display_height);
    }

    mPresentationThread = new (pHeap, 8) DelegateThread(
        "Presentation Thread",
        new (pHeap, 8)
            Delegate2<GameFrameworkNx, Thread*, s64>(this, &GameFrameworkNx::presentAsync_),
        pHeap, ThreadUtil::ConvertPrioritySeadToPlatform(mCreateArg.present_thread_priority),
        MessageQueue::BlockType::Blocking, 0x7fffffff, 0x4000, 0x20);
    mPresentationThread->setAffinity(CoreIdMask(CoreId::cSub1));
    mPresentationThread->start();
}

/**
 * Handles the command buffer running out of memory; nothing is done.
 * @param pCommandBuffer the command buffer
 * @param event the kind of memory that ran out
 * @param minSize the minimum size needed
 * @param pCallbackData the callback data
 */
void GameFrameworkNx::outOfMemoryCallback_(NVNcommandBuffer* pCommandBuffer,
                                           NVNcommandBufferMemoryEvent event, size_t minSize,
                                           void* pCallbackData)
{
}

/**
 * Presents the frame from the presentation thread.
 * @param pThread the presentation thread
 * @param msg the received message
 */
void GameFrameworkNx::presentAsync_(Thread* pThread, s64 msg)
{
    present_();
}

/**
 * Gets the display buffer texture currently acquired for drawing.
 * @return the acquired texture
 */
NVNtexture* GameFrameworkNx::getAcquiredDisplayBufferTexture() const
{
    return mDisplayBuffer->getAcquiredTexture();
}

/**
 * Sets how many vertical blanks each frame waits for.
 * @param interval the number of vertical blanks
 */
void GameFrameworkNx::setVBlankWaitInterval(u32 interval)
{
    mDisplayBuffer->setPresentInterval(interval);
    mCreateArg.vblank_wait_interval = interval;
}

/**
 * Requests enabling or disabling GPU submission at the start of the next frame.
 * @param useGpu whether the GPU should be used
 */
void GameFrameworkNx::requestChangeUseGPU(bool useGpu)
{
    mRequestChangeUseGpu = useGpu ? 1 : 2;
}

/**
 * Gets the free size of the graphics development tools allocator.
 * @return the free size, or 0 without an allocator
 */
size_t GameFrameworkNx::getGraphicsDevToolsAllocatorTotalFreeSize() const
{
    if (mGraphicsDevToolsAllocator)
    {
        return mGraphicsDevToolsAllocator->GetTotalFreeSize();
    }
    return 0;
}

/**
 * Prints the performance settings before running.
 * @param pHeap the heap passed to run
 */
void GameFrameworkNx::initRun_(Heap* pHeap)
{
    PerformanceMgrNx::printPerformance();
}

/**
 * Waits for the display to start, then runs the main loop.
 */
void GameFrameworkNx::runImpl_()
{
    waitStartDisplayLoop_();
    mPrevFrameTick = nn::os::GetSystemTick().GetInt64Value();
    mainLoop_();
}

/**
 * Creates the single screen method tree manager.
 * @param pHeap the heap to allocate from
 * @return the method tree manager
 */
MethodTreeMgr* GameFrameworkNx::createMethodTreeMgr_(Heap* pHeap)
{
    return new (pHeap, 8) SingleScreenMethodTreeMgr();
}

/**
 * Processes frames forever.
 */
void GameFrameworkNx::mainLoop_()
{
    while (true)
    {
        procFrame_();
    }
}

/**
 * Processes one frame: draw, calc, reset, GPU synchronization and frame timing.
 */
void GameFrameworkNx::procFrame_()
{
    if ((mFrameStepFlags & 3) != cFrameStep_Enabled)
    {
        switch (mRequestChangeUseGpu)
        {
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

        if (mDisplayStarted == 1)
        {
            mDisplayStarted = 2;
        }
    }

    mTaskMgr->afterCalc();
    procDraw_();
    procCalc_();
    procReset_();
    waitForGpuDone_();
    nn::os::GetSystemTick();
    setGpuTimeStamp_();

    if ((mFrameStepFlags & 3) != (cFrameStep_Enabled | cFrameStep_Advance))
    {
        mFrameTicks = nn::os::GetSystemTick().GetInt64Value() - mPrevFrameTick;
        mPrevFrameTick = nn::os::GetSystemTick().GetInt64Value();
    }

    if (mFrameStepFlags & cFrameStep_Enabled)
    {
        mFrameStepFlags ^= cFrameStep_Advance;
    }
}

/**
 * Records the frame's draw commands and hands them to presentation.
 */
void GameFrameworkNx::procDraw_()
{
    if ((mFrameStepFlags & 3) == cFrameStep_Enabled)
    {
        return;
    }

    mIsPresentDone = false;
    if (mUnk6)
    {
        mUnk6(true);
    }

    {
        DrawContext context;
        context.getCommandBuffer()->ToData()->pNvnCommandBuffer = mCommandBuffer;

        nvnCommandBufferAddCommandMemory(mCommandBuffer, mCommandMemoryPool, 0,
                                         mCreateArg.command_memory_size);
        nvnCommandBufferAddControlMemory(mCommandBuffer, mControlMemory,
                                         mCreateArg.control_memory_size);
        nvnCommandBufferBeginRecording(mCommandBuffer);
        GraphicsNvn::instance()->_200 = true;

        nvnCommandBufferReportCounter(mCommandBuffer, NVN_COUNTER_TYPE_TIMESTAMP_TOP,
                                      nvnBufferGetAddress(mCounterBuffer));
        nvnCommandBufferSetSamplerPool(mCommandBuffer, GraphicsNvn::instance()->getSamplerPool());
        nvnCommandBufferSetTexturePool(mCommandBuffer, GraphicsNvn::instance()->getTexturePool());
        nvnCommandBufferSetShaderScratchMemory(mCommandBuffer, mShaderScratchMemoryPool, 0,
                                               mShaderScratchMemorySize);

        if (mMethodFrameBuffer)
        {
            mMethodFrameBuffer->bind(&context);
            clearFrameBuffers_(3);
        }

        DynamicCast<SingleScreenMethodTreeMgr>(mMethodTreeMgr)->draw();

        if (mOverrideFrameBuffer)
        {
            mOverrideFrameBuffer->copyToDisplayBuffer(&context, mDisplayBuffer);
        }
        else if (mMethodFrameBuffer)
        {
            mMethodFrameBuffer->copyToDisplayBuffer(&context, mDisplayBuffer);
        }

        nvnCommandBufferReportCounter(mCommandBuffer, NVN_COUNTER_TYPE_TIMESTAMP,
                                      nvnBufferGetAddress(mCounterBuffer) + 0x10);
        GraphicsNvn::instance()->_200 = false;
        mCommandHandle = nvnCommandBufferEndRecording(mCommandBuffer);

        if (!mIsPresentAsync)
        {
            present_();
        }
    }

    if (mIsPresentAsync)
    {
        mPresentationThread->sendMessage(1, MessageQueue::BlockType::Blocking);
    }

    if (PrimitiveDrawMgrNvn::instance())
    {
        PrimitiveDrawMgrNvn::instance()->swapUniformBlockBuffer();
    }

    if (DebugFontMgrNvn::instance())
    {
        DebugFontMgrNvn::instance()->swapUniformBlockBuffer();
    }

    if (DebugFontMgrJis1Nvn::instance())
    {
        DebugFontMgrJis1Nvn::instance()->swapUniformBlockBuffer();
    }

    if (mUnk6)
    {
        mUnk6(false);
    }
}

/**
 * Runs the frame's calc methods.
 */
void GameFrameworkNx::procCalc_()
{
    mTaskMgr->beforeCalc();
    DynamicCast<SingleScreenMethodTreeMgr>(mMethodTreeMgr)->calc();
}

/**
 * Submits the recorded commands to the GPU and presents the display buffer.
 */
void GameFrameworkNx::present_()
{
    if (!mIsUseGpu)
    {
        return;
    }

    CriticalSection* cs = GraphicsNvn::instance()->getCriticalSection1();
    cs->lock();
    mIsPresentDone = true;
    nvnQueueSubmitCommands(mQueue, 1, &mCommandHandle);
    nvnQueueFenceSync(mQueue, mGpuSync, NVN_SYNC_CONDITION_ALL_GPU_COMMANDS_COMPLETE, 0);
    nvnQueueFlush(mQueue);

    mCommandMemoryUsedMax =
        MathSizeT::max(mCommandMemoryUsedMax, nvnCommandBufferGetCommandMemoryUsed(mCommandBuffer));
    mControlMemoryUsedMax =
        MathSizeT::max(mControlMemoryUsedMax, nvnCommandBufferGetControlMemoryUsed(mCommandBuffer));

    swapBuffer_();
    cs->unlock();
}

/**
 * Waits for the vertical blanks of one frame.
 */
void GameFrameworkNx::waitVsyncEvent_()
{
    nn::os::WaitSystemEvent(&mVsyncEvent);

    if (mCreateArg.vblank_wait_interval >= 2)
    {
        const s64 waitTicks = 0.99f / (60.0f / f32(mCreateArg.vblank_wait_interval)) *
                              f32(TickSpan::makeFromSeconds(1).toS64());
        while (nn::os::GetSystemTick().GetInt64Value() - mPrevFrameTick < waitTicks)
        {
            nn::os::WaitSystemEvent(&mVsyncEvent);
        }
    }
}

/**
 * Presents the current display buffer texture once the display has started.
 */
void GameFrameworkNx::swapBuffer_()
{
    if (mDisplayStarted == 2)
    {
        mDisplayBuffer->presentTextureAndAcquireNext();
    }
}

/**
 * Clears the method frame buffer with the clear color.
 * @param flags the buffers to clear
 */
void GameFrameworkNx::clearFrameBuffers_(s32 flags)
{
    if (!mMethodFrameBuffer || !mCommandBuffer)
    {
        return;
    }

    DrawContext context;
    context.getCommandBuffer()->ToData()->pNvnCommandBuffer = mCommandBuffer;
    FrameBuffer* frameBuffer = mMethodFrameBuffer;
    Viewport viewport(*frameBuffer);
    viewport.applyScissor(&context, *frameBuffer);
    frameBuffer->clear(&context, 7, mCreateArg.clear_color, 1.0f, 0);
}

/**
 * Waits for the GPU to finish the previous frame and for the next vertical blank.
 */
void GameFrameworkNx::waitForGpuDone_()
{
    if ((mFrameStepFlags & 3) == (cFrameStep_Enabled | cFrameStep_Advance))
    {
        const s64 frameTicks = f32(mCreateArg.vblank_wait_interval) * 0.5f / 60.0f *
                               f32(TickSpan::makeFromSeconds(1).toS64());
        const s64 prevTick = mPrevFrameTick;
        const s64 sleepTicks = prevTick - nn::os::GetSystemTick().GetInt64Value() + frameTicks;
        if (sleepTicks > 0)
        {
            Thread::sleep(TickSpan(sleepTicks));
        }
        return;
    }

    if (mIsUseGpu)
    {
        while (!mIsPresentDone)
        {
            Thread::sleep(TickSpan::makeFromMilliSeconds(1));
        }

        CriticalSection* cs = GraphicsNvn::instance()->getCriticalSection1();
        cs->lock();

        if (mGpuWaitCallback)
        {
            mGpuWaitCallback(0);
        }

        if (mDisplayStarted == 2)
        {
            mDisplayBuffer->waitAcquireDone();
        }
        else
        {
            waitVsyncEvent_();
        }

        nvnSyncWait(mGpuSync, u64(-1));

        if (mGpuWaitCallback)
        {
            mGpuWaitCallback(1);
        }

        cs->unlock();
    }
    else if (mCreateArg.vblank_wait_interval != 0)
    {
        waitVsyncEvent_();
    }

    mDisplayBuffer->applyChangeWindowCrop();

    if (mCreateArg.is_apply_deferred_finalizes)
    {
        GraphicsNvn::instance()->applyDeferredFinalizes();
    }
}

/**
 * Converts the GPU time stamps of the last frame.
 */
void GameFrameworkNx::setGpuTimeStamp_()
{
    if ((mFrameStepFlags & 3) != (cFrameStep_Enabled | cFrameStep_Advance))
    {
        GraphicsNvn::convertGPUTimeStampToSystemTick(mCounterData);
        GraphicsNvn::convertGPUTimeStampToSystemTick(mCounterData + 1);
    }
}

/**
 * Gets the frame buffer of a method.
 * @param methodType the method type
 * @return the method frame buffer for draw methods, otherwise nullptr
 */
FrameBuffer* GameFrameworkNx::getMethodFrameBuffer(s32 methodType) const
{
    switch (methodType)
    {
    case 2:
    case 3:
    case 4:
        return mMethodFrameBuffer;
    default:
        return nullptr;
    }
}

/**
 * Gets the logical frame buffer of a method.
 * @param methodType the method type
 * @return the method logical frame buffer for draw methods, otherwise nullptr
 */
LogicalFrameBuffer* GameFrameworkNx::getMethodLogicalFrameBuffer(s32 methodType) const
{
    switch (methodType)
    {
    case 2:
    case 3:
    case 4:
        return const_cast<LogicalFrameBuffer*>(&mMethodLogicalFrameBuffer);
    default:
        return nullptr;
    }
}

}  // namespace sead
