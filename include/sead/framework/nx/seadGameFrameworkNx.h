#pragma once

#include <framework/seadGameFramework.h>
#include <gfx/seadColor.h>
#include <gfx/seadFrameBuffer.h>
#include <math/seadVector.h>
#include <nn/os.h>
#include <nvn/nvn.h>
#include <thread/seadThread.h>
#include <time/seadTickSpan.h>

namespace nn
{
namespace mem
{
class StandardAllocator;
}
namespace vi
{
class Layer;
class Display;
}  // namespace vi
}  // namespace nn

namespace sead
{
class DelegateThread;
class DisplayBufferNvn;

class GameFrameworkNx : public GameFramework
{
    SEAD_RTTI_OVERRIDE(GameFrameworkNx, GameFramework)

public:
    struct CreateArg
    {
        u32 vblank_wait_interval;
        Color4f clear_color;
        u32 display_width;
        u32 display_height;
        bool create_method_frame_buffer;
        bool is_triple_buffer;
        bool is_debug;
        bool is_apply_deferred_finalizes;
        u32 command_memory_size;
        u32 control_memory_size;
        u32 shader_scratch_memory_scale;
        u32 graphics_memory_size;
        u32 graphics_devtools_memory_size;
        s32 queue_compute_memory_size;
        s32 queue_command_memory_size;
        s32 queue_control_memory_size;
        u32 nvn_debug_level;
        s32 texture_descriptor_num;
        s32 present_thread_priority;
    };

    static_assert(sizeof(CreateArg) == 0x4c);

    static void initialize(const Framework::InitializeArg& rArg);

    explicit GameFrameworkNx(const CreateArg& rArg);
    ~GameFrameworkNx() override;

    FrameBuffer* getMethodFrameBuffer(s32 methodType) const override;
    LogicalFrameBuffer* getMethodLogicalFrameBuffer(s32 methodType) const override;
    void initRun_(Heap* pHeap) override;
    void runImpl_() override;
    MethodTreeMgr* createMethodTreeMgr_(Heap* pHeap) override;
    f32 calcFps() override { return f32(TickSpan::makeFromSeconds(1).toS64()) / f32(mFrameTicks); }
    virtual void setCaption(const SafeString&) {}
    virtual void mainLoop_();
    virtual void procFrame_();
    virtual void procDraw_();
    virtual void procCalc_();
    virtual void present_();
    virtual void swapBuffer_();
    virtual void clearFrameBuffers_(s32 flags);
    virtual void waitForGpuDone_();
    virtual void setGpuTimeStamp_();

    void initializeGraphicsSystem(Heap* pHeap, const Vector2f& rVirtualSize);
    NVNtexture* getAcquiredDisplayBufferTexture() const;
    void setVBlankWaitInterval(u32 interval);
    void requestChangeUseGPU(bool useGpu);
    size_t getGraphicsDevToolsAllocatorTotalFreeSize() const;

    NVNcommandBuffer* get158() const { return mCommandBuffer; }
    NVNcommandBuffer* getCommandBuffer() const { return mCommandBuffer; }

protected:
    enum FrameStepFlag
    {
        cFrameStep_Enabled = 1 << 0,
        cFrameStep_Advance = 1 << 1
    };

    static void outOfMemoryCallback_(NVNcommandBuffer* pCommandBuffer,
                                     NVNcommandBufferMemoryEvent event, size_t minSize,
                                     void* pCallbackData);

    void presentAsync_(Thread* pThread, s64 msg);
    void waitVsyncEvent_();

    CreateArg mCreateArg;
    s64 mFrameTicks;
    s64 mPrevFrameTick;
    FrameBuffer* mMethodFrameBuffer;
    LogicalFrameBuffer mMethodLogicalFrameBuffer;
    s64 mVBlankWaitTicks;
    DisplayBufferNvn* mDisplayBuffer;
    void (*mGpuWaitCallback)(int);
    void* _140;
    NVNmemoryPool* mCommandMemoryPool;
    void* mControlMemory;
    NVNcommandBuffer* mCommandBuffer;
    NVNbuffer* mCounterBuffer;
    NVNcounterData* mCounterData;
    NVNmemoryPool* mShaderScratchMemoryPool;
    u32 mShaderScratchMemorySize;
    nn::mem::StandardAllocator* mGraphicsDevToolsAllocator;
    u64 mCommandMemoryUsedMax;
    u64 mControlMemoryUsedMax;
    NVNqueue* mQueue;
    FrameBuffer* mOverrideFrameBuffer;
    nn::vi::Display* mDisplay;
    nn::vi::Layer* mLayer;
    DelegateThread* mPresentationThread;
    NVNsync* mGpuSync;
    SafeString mCaption;
    NVNcommandHandle mCommandHandle;
    nn::os::SystemEventType mVsyncEvent;
    bool mIsPresentAsync;
    bool mIsPresentDone;
    u8 mFrameStepFlags;
    bool mIsUseGpu;
    u8 mRequestChangeUseGpu;
};

}  // namespace sead
