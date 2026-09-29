#pragma once

#include <gfx/seadColor.h>
#include <gfx/seadGraphics.h>
#include <prim/seadDelegate.h>
#include <thread/seadAtomic.h>
#include <thread/seadCriticalSection.h>
#include <time/seadTickTime.h>
#include <nn/gfx/gfx_Variation-api.nvn.h>
#include "nvn/nvn.h"

namespace nn::gfx::detail
{
template <typename TTarget>
class DeviceImpl;
}  // namespace nn::gfx::detail

namespace sead
{
class DisplayBufferNvn;

class GraphicsNvn : public Graphics
{
    friend class GameFrameworkNx;

public:
    class CreateArg
    {
    public:
        NVNdevice* mNvnDevice;
        s32 mTextureDescriptorNum;
        bool mIsApplyDeferredFinalizes;
        bool _d;
    };

    struct NvnDebugCallbackParam
    {
        NVNdebugCallbackSource mSource;
        NVNdebugCallbackType mType;
        s32 mId;
        NVNdebugCallbackSeverity mSeverity;
        const char* mMessage;
    };

    using NvnDebugCallback = IDelegate1<const NvnDebugCallbackParam&>;
    using GfxDevice = nn::gfx::detail::DeviceImpl<nn::gfx::ApiVariationNvn8>;

    GraphicsNvn(const CreateArg& rArg);

    void initializeDrawLockContext(Heap* pHeap) override;
    void initializeImpl(Heap* pHeap) override;

    s32 getNewSamplerId();
    s32 getNewTextureId();

    void setDisplayBufferWindowCrop(s32 x, s32 y, s32 w, s32 h);
    void getDisplayBufferWindowCrop(s32* pX, s32* pY, s32* pW, s32* pH) const;

    void registerQueue(NVNqueue* pQueue);
    void registerDefaultCommandBuffer(NVNcommandBuffer* pCommandBuffer);
    void registerDisplayBufferNvn(DisplayBufferNvn* pDisplayBuffer);
    void applyDeferredFinalizes();

    static void nvnDebugCallback(NVNdebugCallbackSource source, NVNdebugCallbackType type, s32 id,
                                 NVNdebugCallbackSeverity severity, const char* pMessage,
                                 void* pUserParam);

    static u64 convertGPUTimeStampToSystemTick(const NVNcounterData* pCounterData);
    static s32 convertNvnDebugLevel(u32 level);

    void setViewportImpl(f32, f32, f32, f32) override;
    void setScissorImpl(f32, f32, f32, f32) override;
    void setDepthTestEnableImpl(bool) override;
    void setDepthWriteEnableImpl(bool) override;
    void setDepthFuncImpl(Graphics::DepthFunc) override;
    bool setVBlankWaitIntervalImpl(u32 interval) override;
    void setCullingModeImpl(Graphics::CullingMode) override;
    void setBlendEnableImpl(bool) override;
    void setBlendEnableMRTImpl(u32, bool) override;
    void setBlendFactorImpl(Graphics::BlendFactor, Graphics::BlendFactor, Graphics::BlendFactor,
                            Graphics::BlendFactor) override;
    void setBlendFactorMRTImpl(u32, Graphics::BlendFactor, Graphics::BlendFactor,
                               Graphics::BlendFactor, Graphics::BlendFactor) override;
    void setBlendEquationImpl(Graphics::BlendEquation, Graphics::BlendEquation) override;
    void setBlendEquationMRTImpl(u32, Graphics::BlendEquation, Graphics::BlendEquation) override;
    void setBlendConstantColorImpl(const Color4f&) override;
    void waitForVBlankImpl() override;
    void setColorMaskImpl(bool, bool, bool, bool) override;
    void setColorMaskMRTImpl(u32, bool, bool, bool, bool) override;
    void setAlphaTestEnableImpl(bool) override;
    void setAlphaTestFuncImpl(Graphics::AlphaFunc, f32) override;
    void setStencilTestEnableImpl(bool) override;
    void setStencilTestFuncImpl(Graphics::StencilFunc, s32, u32) override;
    void setStencilTestOpImpl(Graphics::StencilOp, Graphics::StencilOp,
                              Graphics::StencilOp) override;
    void setPolygonModeImpl(Graphics::PolygonMode, Graphics::PolygonMode) override;
    void setPolygonOffsetEnableImpl(bool, bool, bool) override;

    NVNdevice* getNvnDevice() const { return mNvnDevice; }
    NVNqueue* getNvnQueue() const { return mNvnQueue; }
    NVNcommandBuffer* getDefaultCommandBuffer() const { return mDefaultCommandBuffer; }
    GfxDevice* getGfxDevice() const { return mGfxDevice; }
    DisplayBufferNvn* getDisplayBuffer() const { return mDisplayBuffer; }

    NVNtexturePool* getTexturePool() { return &mNvnTexturePool; }
    NVNsamplerPool* getSamplerPool() { return &mNvnSamplerPool; }

    s32 getTextureSamplerID() const { return mTextureSamplerID; }

    CriticalSection* getCriticalSection1() { return &mCriticalSection1; }
    CriticalSection* getCriticalSection2() { return &mCriticalSection2; }

    static GraphicsNvn* instance() { return static_cast<GraphicsNvn*>(Graphics::instance()); }

    static TickTime sBaseTime;

private:
    void defaultNvnDebugCallback_(const NvnDebugCallbackParam& rParam);

    NVNdevice* mNvnDevice;
    NVNqueue* mNvnQueue = nullptr;
    NVNcommandBuffer* mDefaultCommandBuffer = nullptr;
    GfxDevice* mGfxDevice = nullptr;
    DisplayBufferNvn* mDisplayBuffer = nullptr;
    NVNtexturePool mNvnTexturePool;
    NVNsamplerPool mNvnSamplerPool;
    NVNsampler mNvnSampler;
    s32 mTextureSamplerID = 0;
    u32 mVBlankWaitInterval = 0;
    NVNmemoryPool* mDescriptorMemoryPool;
    Atomic<s32> mSamplerIdCounter = 0;
    Atomic<s32> mTextureIdCounter = 0;
    s32 mTextureDescriptorNum;
    s32 mSamplerDescriptorNum = 0x1000;
    CriticalSection mCriticalSection1;
    CriticalSection mCriticalSection2;
    CriticalSection mCriticalSection3;
    Delegate1<GraphicsNvn, const NvnDebugCallbackParam&> mDefaultDebugCallback;
    NvnDebugCallback* mDebugCallback;
    bool _200 = false;
    bool mIsApplyDeferredFinalizes;
    bool _202;
};
static_assert(sizeof(GraphicsNvn) == 0x208);

}  // namespace sead
