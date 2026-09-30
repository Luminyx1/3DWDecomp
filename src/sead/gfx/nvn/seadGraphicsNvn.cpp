#include "gfx/nin/seadGraphicsNvn.h"

#include <nn/gfx/gfx_Device.h>
#include <nn/os.h>
#include <nn/time.h>
#include "gfx/nvn/seadFrameBufferNvn.h"
#include "nvn/nvn_FuncPtrInline.h"

namespace nn
{
namespace os
{
Tick ConvertToTick(TimeSpan timeSpan);
}  // namespace os

namespace gfx::detail::Nvn
{
::nn::util::BitPack32 GetDeviceFeature(const NVNdevice* pDevice);
}  // namespace gfx::detail::Nvn
}  // namespace nn

namespace sead
{
TickTime GraphicsNvn::sBaseTime;

/**
 * Constructs the NVN graphics backend from the given creation arguments.
 * @param rArg creation arguments (NVN device, texture descriptor count, flags)
 */
GraphicsNvn::GraphicsNvn(const CreateArg& rArg)
    : mNvnDevice(rArg.mNvnDevice), mTextureDescriptorNum(rArg.mTextureDescriptorNum),
      mDefaultDebugCallback(this, &GraphicsNvn::defaultNvnDebugCallback_),
      mDebugCallback(&mDefaultDebugCallback), mIsApplyDeferredFinalizes(rArg.mIsApplyDeferredFinalizes),
      _202(rArg._d)
{
    nvnDeviceSetWindowOriginMode(mNvnDevice, NVN_WINDOW_ORIGIN_MODE_UPPER_LEFT);
}

/**
 * Default NVN debug callback; does nothing.
 * @param rParam debug callback parameters
 */
void GraphicsNvn::defaultNvnDebugCallback_(const NvnDebugCallbackParam& rParam) {}

/**
 * Initializes the draw lock context using the base implementation.
 * @param pHeap heap to allocate from
 */
void GraphicsNvn::initializeDrawLockContext(Heap* pHeap)
{
    Graphics::initializeDrawLockContext(pHeap);
}

/**
 * Creates the gfx device wrapper, descriptor pools and the default sampler.
 * @param pHeap heap to allocate from
 */
void GraphicsNvn::initializeImpl(Heap* pHeap)
{
    mGfxDevice = new (pHeap, 8) GfxDevice();
    mGfxDevice->ToData()->pNvnDevice = mNvnDevice;
    mGfxDevice->ToData()->state = nn::gfx::DeviceImplData<nn::gfx::ApiVariationNvn8>::State_Initialized;
    mGfxDevice->ToData()->supportedFeatures = nn::gfx::detail::Nvn::GetDeviceFeature(mNvnDevice);

    s32 textureDescriptorSize;
    s32 samplerDescriptorSize;
    nvnDeviceGetInteger(mNvnDevice, NVN_DEVICE_INFO_TEXTURE_DESCRIPTOR_SIZE, &textureDescriptorSize);
    nvnDeviceGetInteger(mNvnDevice, NVN_DEVICE_INFO_SAMPLER_DESCRIPTOR_SIZE, &samplerDescriptorSize);
    s32 reservedTextureNum;
    s32 reservedSamplerNum;
    nvnDeviceGetInteger(mNvnDevice, NVN_DEVICE_INFO_RESERVED_TEXTURE_DESCRIPTORS,
                        &reservedTextureNum);
    nvnDeviceGetInteger(mNvnDevice, NVN_DEVICE_INFO_RESERVED_SAMPLER_DESCRIPTORS,
                        &reservedSamplerNum);

    mDescriptorMemoryPool = new (pHeap, 8) NVNmemoryPool;
    u32 poolSize = (mSamplerDescriptorNum * samplerDescriptorSize +
                    mTextureDescriptorNum * textureDescriptorSize + 0xfff) &
                   ~0xfff;

    {
    NVNmemoryPoolBuilder poolBuilder;
    nvnMemoryPoolBuilderSetDefaults(&poolBuilder);
    nvnMemoryPoolBuilderSetDevice(&poolBuilder, mNvnDevice);
    nvnMemoryPoolBuilderSetFlags(&poolBuilder, NVN_MEMORY_POOL_FLAGS_CPU_UNCACHED |
                                                   NVN_MEMORY_POOL_FLAGS_GPU_CACHED);
    nvnMemoryPoolBuilderSetStorage(&poolBuilder, new (pHeap, 0x1000) u8[poolSize], poolSize);
    nvnMemoryPoolInitialize(mDescriptorMemoryPool, &poolBuilder);
    }

    nvnSamplerPoolInitialize(&mNvnSamplerPool, mDescriptorMemoryPool, 0, mSamplerDescriptorNum);
    nvnTexturePoolInitialize(&mNvnTexturePool, mDescriptorMemoryPool,
                             ptrdiff_t(mSamplerDescriptorNum) * samplerDescriptorSize,
                             mTextureDescriptorNum);

    mSamplerIdCounter.storeNonAtomic(reservedSamplerNum);
    mTextureIdCounter.storeNonAtomic(reservedTextureNum);
    sBaseTime.setNow();

    {
    NVNsamplerBuilder samplerBuilder;
    nvnSamplerBuilderSetDefaults(&samplerBuilder);
    nvnSamplerBuilderSetMinMagFilter(&samplerBuilder, NVN_MIN_FILTER_LINEAR_MIPMAP_NEAREST,
                                     NVN_MAG_FILTER_LINEAR);
    nvnSamplerBuilderSetDevice(&samplerBuilder, mNvnDevice);
    nvnSamplerInitialize(&mNvnSampler, &samplerBuilder);
    }

    mCriticalSection3.lock();
    mTextureSamplerID = getNewSamplerId();
    nvnSamplerPoolRegisterSampler(&mNvnSamplerPool, mTextureSamplerID, &mNvnSampler);
    mCriticalSection3.unlock();
}

/**
 * Allocates a new sampler descriptor id.
 * @return the allocated sampler id
 */
s32 GraphicsNvn::getNewSamplerId()
{
    return mSamplerIdCounter.increment();
}

/**
 * Allocates a new texture descriptor id.
 * @return the allocated texture id
 */
s32 GraphicsNvn::getNewTextureId()
{
    return mTextureIdCounter.increment();
}

/**
 * Sets the window crop of the registered display buffer.
 * @param x crop x
 * @param y crop y
 * @param w crop width
 * @param h crop height
 */
void GraphicsNvn::setDisplayBufferWindowCrop(s32 x, s32 y, s32 w, s32 h)
{
    mDisplayBuffer->setWindowCrop(x, y, w, h);
}

/**
 * Gets the window crop of the registered display buffer.
 * @param pX output crop x
 * @param pY output crop y
 * @param pW output crop width
 * @param pH output crop height
 */
void GraphicsNvn::getDisplayBufferWindowCrop(s32* pX, s32* pY, s32* pW, s32* pH) const
{
    mDisplayBuffer->getWindowCrop(pX, pY, pW, pH);
}

/**
 * Registers the NVN queue.
 * @param pQueue NVN queue
 */
void GraphicsNvn::registerQueue(NVNqueue* pQueue)
{
    mNvnQueue = pQueue;
}

/**
 * Registers the default NVN command buffer.
 * @param pCommandBuffer NVN command buffer
 */
void GraphicsNvn::registerDefaultCommandBuffer(NVNcommandBuffer* pCommandBuffer)
{
    mDefaultCommandBuffer = pCommandBuffer;
}

/**
 * Registers the display buffer.
 * @param pDisplayBuffer display buffer
 */
void GraphicsNvn::registerDisplayBufferNvn(DisplayBufferNvn* pDisplayBuffer)
{
    mDisplayBuffer = pDisplayBuffer;
}

/**
 * Applies deferred finalizes on the NVN device if enabled.
 */
void GraphicsNvn::applyDeferredFinalizes()
{
    if (mIsApplyDeferredFinalizes)
    {
        nvnDeviceApplyDeferredFinalizes(mNvnDevice, 5);
    }
}

/**
 * NVN debug callback that forwards the message to the registered delegate.
 * @param source message source
 * @param type message type
 * @param id message id
 * @param severity message severity
 * @param pMessage message text
 * @param pUserParam unused user parameter
 */
void GraphicsNvn::nvnDebugCallback(NVNdebugCallbackSource source, NVNdebugCallbackType type,
                                   s32 id, NVNdebugCallbackSeverity severity, const char* pMessage,
                                   void* pUserParam)
{
    GraphicsNvn* graphics = instance();

    if (!graphics)
    {
        return;
    }

    NvnDebugCallbackParam param;
    param.mSource = source;
    param.mType = type;
    param.mId = id;
    param.mSeverity = severity;
    param.mMessage = pMessage;

    if (graphics->mDebugCallback)
    {
        graphics->mDebugCallback->invoke(param);
    }
}

/**
 * Converts a GPU counter timestamp to a system tick.
 * @param pCounterData GPU counter data
 * @return the corresponding system tick
 */
u64 GraphicsNvn::convertGPUTimeStampToSystemTick(const NVNcounterData* pCounterData)
{
    u64 ns = nvnDeviceGetTimestampInNanoseconds(instance()->mNvnDevice, pCounterData);
    return nn::os::ConvertToTick(nn::TimeSpan::FromNanoSeconds(ns));
}

/**
 * Converts a sead debug level to NVN debug layer flags.
 * @param level debug level
 * @return NVN debug flags
 */
s32 GraphicsNvn::convertNvnDebugLevel(u32 level)
{
    switch (level)
    {
    case 1:
        return 0x40;
    case 2:
        return 0x1;
    case 3:
        return 0x4;
    case 4:
        return 0x10;
    default:
        return 0x20;
    }
}

/**
 * Sets the viewport; not supported on this backend.
 */
void GraphicsNvn::setViewportImpl(f32, f32, f32, f32) {}

/**
 * Sets the scissor; not supported on this backend.
 */
void GraphicsNvn::setScissorImpl(f32, f32, f32, f32) {}

/**
 * Sets depth test enable; not supported on this backend.
 */
void GraphicsNvn::setDepthTestEnableImpl(bool) {}

/**
 * Sets depth write enable; not supported on this backend.
 */
void GraphicsNvn::setDepthWriteEnableImpl(bool) {}

/**
 * Sets the depth function; not supported on this backend.
 */
void GraphicsNvn::setDepthFuncImpl(Graphics::DepthFunc) {}

/**
 * Sets the VBlank wait interval.
 * @param interval VBlank wait interval
 * @return always true
 */
bool GraphicsNvn::setVBlankWaitIntervalImpl(u32 interval)
{
    mVBlankWaitInterval = interval;
    return true;
}

/**
 * Sets the culling mode; not supported on this backend.
 */
void GraphicsNvn::setCullingModeImpl(Graphics::CullingMode) {}

/**
 * Sets blend enable; not supported on this backend.
 */
void GraphicsNvn::setBlendEnableImpl(bool) {}

/**
 * Sets blend enable for a render target; not supported on this backend.
 */
void GraphicsNvn::setBlendEnableMRTImpl(u32, bool) {}

/**
 * Sets blend factors; not supported on this backend.
 */
void GraphicsNvn::setBlendFactorImpl(Graphics::BlendFactor, Graphics::BlendFactor,
                                     Graphics::BlendFactor, Graphics::BlendFactor)
{
}

/**
 * Sets blend factors for a render target; not supported on this backend.
 */
void GraphicsNvn::setBlendFactorMRTImpl(u32, Graphics::BlendFactor, Graphics::BlendFactor,
                                        Graphics::BlendFactor, Graphics::BlendFactor)
{
}

/**
 * Sets blend equations; not supported on this backend.
 */
void GraphicsNvn::setBlendEquationImpl(Graphics::BlendEquation, Graphics::BlendEquation) {}

/**
 * Sets blend equations for a render target; not supported on this backend.
 */
void GraphicsNvn::setBlendEquationMRTImpl(u32, Graphics::BlendEquation, Graphics::BlendEquation)
{
}

/**
 * Sets the blend constant color; not supported on this backend.
 */
void GraphicsNvn::setBlendConstantColorImpl(const Color4f&) {}

/**
 * Waits for VBlank; not supported on this backend.
 */
void GraphicsNvn::waitForVBlankImpl() {}

/**
 * Sets the color mask; not supported on this backend.
 */
void GraphicsNvn::setColorMaskImpl(bool, bool, bool, bool) {}

/**
 * Sets the color mask for a render target; not supported on this backend.
 */
void GraphicsNvn::setColorMaskMRTImpl(u32, bool, bool, bool, bool) {}

/**
 * Sets alpha test enable; not supported on this backend.
 */
void GraphicsNvn::setAlphaTestEnableImpl(bool) {}

/**
 * Sets the alpha test function; not supported on this backend.
 */
void GraphicsNvn::setAlphaTestFuncImpl(Graphics::AlphaFunc, f32) {}

/**
 * Sets stencil test enable; not supported on this backend.
 */
void GraphicsNvn::setStencilTestEnableImpl(bool) {}

/**
 * Sets the stencil test function; not supported on this backend.
 */
void GraphicsNvn::setStencilTestFuncImpl(Graphics::StencilFunc, s32, u32) {}

/**
 * Sets the stencil operations; not supported on this backend.
 */
void GraphicsNvn::setStencilTestOpImpl(Graphics::StencilOp, Graphics::StencilOp,
                                       Graphics::StencilOp)
{
}

/**
 * Sets the polygon mode; not supported on this backend.
 */
void GraphicsNvn::setPolygonModeImpl(Graphics::PolygonMode, Graphics::PolygonMode) {}

/**
 * Sets polygon offset enable; not supported on this backend.
 */
void GraphicsNvn::setPolygonOffsetEnableImpl(bool, bool, bool) {}

}  // namespace sead
