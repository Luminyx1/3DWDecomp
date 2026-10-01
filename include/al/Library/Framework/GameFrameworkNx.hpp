#pragma once

#include <common/aglDrawContext.h>
#include <framework/nx/seadGameFrameworkNx.h>
#include <gfx/seadDrawContext.h>
#include <prim/seadRuntimeTypeInfo.h>

namespace agl {
class DisplayList;
class RenderBuffer;
class RenderTargetColor;
}  // namespace agl

namespace sead {
class Event;
}

namespace al {
class ApplicationMessageReceiver;
struct DrawSystemInfo;
class GpuPerf;

class GameFrameworkNx : public sead::GameFrameworkNx {
    SEAD_RTTI_OVERRIDE(GameFrameworkNx, sead::GameFrameworkNx)

public:
    struct AglInitArg {
        sead::Heap* heap;
        s32 virtualWidth;
        s32 virtualHeight;
        s32 dockedWidth;
        s32 dockedHeight;
        s32 handheldWidth;
        s32 handheldHeight;
        s32 aglHeapSize;
        s32 workHeapSize;
        s32 minGPUMemBlockSize;
        s32 dynamicTextureSize;
    };

    explicit GameFrameworkNx(const CreateArg& rArg);

    void createControllerMgr(sead::TaskBase* pBase) override;

    void createHostIOMgr(sead::TaskBase* pBase, sead::HostIOMgr::Parameter* pParam,
                         sead::Heap* pHeap) override {}

    void createInfLoopChecker(sead::TaskBase* pBase, const sead::TickSpan& rSpan,
                              s32 num) override;
    sead::FrameBuffer* getMethodFrameBuffer(s32 methodType) const override;

    void initAgl(const AglInitArg& rArg);
    void clearFrameBuffer();
    void startCommandList();
    void endCommandList(bool isPresent);
    void finishGpuProfile();

    agl::RenderBuffer* getCurrentRenderBuffer() const {
        return mIsDocked ? mDockedRenderBuffer : mHandheldRenderBuffer;
    }

    bool isSkipDrawFrame() const { return _27b && !_27c; }

    static sead::DrawContext* getDrawContext() { return sInstance->mDrawContext; }

    static agl::DrawContext* getAglDrawContext() { return sInstance->mDrawContext; }

    static GameFrameworkNx* sInstance;

protected:
    void procFrame_() override;
    void procDraw_() override;
    void present_() override;
    void waitForGpuDone_() override;

public:
    agl::DrawContext* mDrawContext = nullptr;
    agl::RenderBuffer* mDockedRenderBuffer = nullptr;
    agl::RenderTargetColor* mDockedRenderTargetColor = nullptr;
    agl::RenderBuffer* mHandheldRenderBuffer = nullptr;
    agl::RenderTargetColor* mHandheldRenderTargetColor = nullptr;
    agl::DisplayList* mDisplayLists[2];
    agl::DisplayList* mCurrentDisplayList;
    GpuPerf* mGpuPerf = nullptr;
    s16 mDisplayListIndex = 0;
    s16 mPrevDisplayListIndex = 1;
    void* mControlMemory[2];
    bool mIsClearRenderBuffer = true;
    bool mIsDocked = false;
    bool mIsEnableDisplayStart = true;
    bool _27b = false;
    bool _27c;
    sead::Event* mDrawReadyEvent = nullptr;
    DrawSystemInfo* mDrawSystemInfo;
    ApplicationMessageReceiver* mMessageReceiver;
    bool mIsDisableInfLoopChecker = false;
};

static_assert(sizeof(GameFrameworkNx) == 0x2a0);
}  // namespace al
