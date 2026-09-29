#include "gfx/nvn/seadFrameBufferNvn.h"

#include "gfx/nin/seadGraphicsNvn.h"
#include "gfx/seadDrawContext.h"
#include "nvn/nvn_FuncPtrInline.h"

namespace sead
{
FrameBufferNvn::~FrameBufferNvn() = default;

/**
 * Clears the color and/or depth-stencil targets of this frame buffer.
 * @param pDrawContext draw context
 * @param clearFlag combination of ClearFlag bits
 * @param rColor clear color
 * @param depth depth clear value
 * @param stencil stencil clear value
 */
void FrameBufferNvn::clear(DrawContext* pDrawContext, u32 clearFlag, const Color4f& rColor,
                           f32 depth, u32 stencil) const
{
    if (clearFlag & cColor)
    {
        nvnCommandBufferClearColor(pDrawContext->getNvnCommandBuffer(), 0, &rColor.r,
                                   NVN_CLEAR_COLOR_MASK_RGBA);
    }

    if (clearFlag & (cDepth | cStencil))
    {
        nvnCommandBufferClearDepthStencil(pDrawContext->getNvnCommandBuffer(), depth,
                                          (clearFlag >> 1) & 1, stencil, (clearFlag >> 2) & 1);
    }
}

/**
 * Copies the color texture to the currently acquired display buffer texture.
 * @param pDrawContext draw context
 * @param pDisplayBuffer destination display buffer
 */
void FrameBufferNvn::copyToDisplayBuffer(DrawContext* pDrawContext,
                                         const DisplayBuffer* pDisplayBuffer) const
{
    NVNcopyRegion srcRegion;
    srcRegion.xoffset = 0;
    srcRegion.yoffset = 0;
    srcRegion.zoffset = 0;
    srcRegion.width = getPhysicalArea().getSizeX();
    srcRegion.height = getPhysicalArea().getSizeY();
    srcRegion.depth = 1;

    NVNcopyRegion dstRegion;
    dstRegion.xoffset = 0;
    dstRegion.yoffset = 0;
    dstRegion.zoffset = 0;
    dstRegion.width = pDisplayBuffer->getSize().x;
    dstRegion.height = pDisplayBuffer->getSize().y;
    dstRegion.depth = 1;

    const auto* displayBuffer = static_cast<const DisplayBufferNvn*>(pDisplayBuffer);
    nvnCommandBufferCopyTextureToTexture(pDrawContext->getNvnCommandBuffer(), mColorTexture,
                                         nullptr, &srcRegion, displayBuffer->getAcquiredTexture(),
                                         nullptr, &dstRegion, NVN_COPY_FLAGS_LINEAR_FILTER_BIT);
}

/**
 * Creates a frame buffer with RGBA8 color and D24S8 depth textures.
 * @param pHeap heap to allocate from
 * @param rVirtualSize virtual size of the frame buffer
 * @param width physical width
 * @param height physical height
 * @return the created frame buffer
 */
FrameBufferNvn* FrameBufferNvn::create(Heap* pHeap, const Vector2f& rVirtualSize, u32 width,
                                       u32 height)
{
    NVNdevice* device = GraphicsNvn::instance()->getNvnDevice();

    NVNtextureBuilder colorBuilder;
    nvnTextureBuilderSetDevice(&colorBuilder, device);
    nvnTextureBuilderSetDefaults(&colorBuilder);
    nvnTextureBuilderSetSize2D(&colorBuilder, width, height);
    nvnTextureBuilderSetTarget(&colorBuilder, NVN_TEXTURE_TARGET_2D);
    nvnTextureBuilderSetFormat(&colorBuilder, NVN_FORMAT_RGBA8);
    nvnTextureBuilderSetFlags(&colorBuilder, NVN_TEXTURE_FLAGS_COMPRESSIBLE);

    NVNtextureBuilder depthBuilder = colorBuilder;
    nvnTextureBuilderSetFormat(&depthBuilder, NVN_FORMAT_DEPTH24_STENCIL8);

    auto* pool = new (pHeap, 8) NVNmemoryPool;
    auto* colorTexture = new (pHeap, 8) NVNtexture;
    auto* depthTexture = new (pHeap, 8) NVNtexture;

    size_t colorSize = nvnTextureBuilderGetStorageSize(&colorBuilder);
    s32 depthAlignment = nvnTextureBuilderGetStorageAlignment(&depthBuilder);
    size_t depthOffset = (colorSize + depthAlignment - 1) / depthAlignment * depthAlignment;
    size_t poolSize =
        (depthOffset + nvnTextureBuilderGetStorageSize(&depthBuilder) + 0xfff) & ~size_t(0xfff);

    NVNmemoryPoolBuilder poolBuilder;
    nvnMemoryPoolBuilderSetDefaults(&poolBuilder);
    nvnMemoryPoolBuilderSetDevice(&poolBuilder, device);
    nvnMemoryPoolBuilderSetFlags(&poolBuilder, NVN_MEMORY_POOL_FLAGS_CPU_NO_ACCESS |
                                                   NVN_MEMORY_POOL_FLAGS_GPU_CACHED |
                                                   NVN_MEMORY_POOL_FLAGS_COMPRESSIBLE);
    nvnMemoryPoolBuilderSetStorage(&poolBuilder, new (pHeap, 0x1000) u8[poolSize], poolSize);
    nvnMemoryPoolInitialize(pool, &poolBuilder);

    nvnTextureBuilderSetStorage(&colorBuilder, pool, 0);
    colorSize = nvnTextureBuilderGetStorageSize(&colorBuilder);
    depthAlignment = nvnTextureBuilderGetStorageAlignment(&depthBuilder);
    depthOffset = (colorSize + depthAlignment - 1) / depthAlignment * depthAlignment;
    nvnTextureInitialize(colorTexture, &colorBuilder);
    {
        CriticalSection* cs = GraphicsNvn::instance()->getCriticalSection2();
        cs->lock();
        nvnTexturePoolRegisterTexture(GraphicsNvn::instance()->getTexturePool(),
                                      GraphicsNvn::instance()->getNewTextureId(), colorTexture,
                                      nullptr);
        cs->unlock();
    }

    nvnTextureBuilderSetStorage(&depthBuilder, pool, depthOffset);
    nvnTextureInitialize(depthTexture, &depthBuilder);
    {
        CriticalSection* cs = GraphicsNvn::instance()->getCriticalSection2();
        cs->lock();
        nvnTexturePoolRegisterTexture(GraphicsNvn::instance()->getTexturePool(),
                                      GraphicsNvn::instance()->getNewTextureId(), depthTexture,
                                      nullptr);
        cs->unlock();
    }

    return new (pHeap, 8) FrameBufferNvn(rVirtualSize, width, height, colorTexture, depthTexture);
}

/**
 * Binds the color and depth textures as render targets.
 * @param pDrawContext draw context
 */
void FrameBufferNvn::bindImpl_(DrawContext* pDrawContext) const
{
    nvnCommandBufferSetRenderTargets(pDrawContext->getNvnCommandBuffer(), 1, &mColorTexture,
                                     nullptr, mDepthTexture, nullptr);
}

/**
 * Constructs a double-buffered display buffer with a present interval of one.
 */
DisplayBufferNvn::DisplayBufferNvn()
{
    mNativeWindow = nullptr;
    mPresentInterval = 1;
    mTextureNum = 2;
    mWindow = nullptr;
    mSync = nullptr;
    mTextureIndex = 0;
    mIsWindowCropChanged = false;
    mWindowCropX = 0;
    mWindowCropY = 0;
    mWindowCropW = 0;
    mWindowCropH = 0;
    for (s32 i = 0; i < cTextureNumMax; i++)
    {
        mTextures[i] = nullptr;
    }
}

/**
 * Presents the current texture and acquires the next one from the window.
 */
void DisplayBufferNvn::presentTextureAndAcquireNext()
{
    nvnQueuePresentTexture(GraphicsNvn::instance()->getNvnQueue(), mWindow, mTextureIndex);
    nvnWindowAcquireTexture(mWindow, mSync, &mTextureIndex);
}

/**
 * Waits until the texture acquire has completed.
 */
void DisplayBufferNvn::waitAcquireDone()
{
    nvnSyncWait(mSync, UINT64_MAX);
}

/**
 * Sets the present interval and applies it to the window if it exists.
 * @param interval present interval
 */
void DisplayBufferNvn::setPresentInterval(u8 interval)
{
    mPresentInterval = interval;
    if (mWindow)
    {
        nvnWindowSetPresentInterval(mWindow, interval);
    }
}

/**
 * Selects between double and triple buffering.
 * @param isTripleBuffer whether to use three textures
 */
void DisplayBufferNvn::setTripleBuffer(bool isTripleBuffer)
{
    mTextureNum = isTripleBuffer ? 3 : 2;
}

/**
 * Sets the window crop to apply on the next applyChangeWindowCrop call.
 * @param x crop x
 * @param y crop y
 * @param w crop width
 * @param h crop height
 */
void DisplayBufferNvn::setWindowCrop(s32 x, s32 y, s32 w, s32 h)
{
    mIsWindowCropChanged = true;
    mWindowCropX = x;
    mWindowCropY = y;
    mWindowCropW = w;
    mWindowCropH = h;
}

/**
 * Gets the window crop.
 * @param pX output crop x
 * @param pY output crop y
 * @param pW output crop width
 * @param pH output crop height
 */
void DisplayBufferNvn::getWindowCrop(s32* pX, s32* pY, s32* pW, s32* pH) const
{
    *pX = mWindowCropX;
    *pY = mWindowCropY;
    *pW = mWindowCropW;
    *pH = mWindowCropH;
}

/**
 * Applies a pending window crop change to the window.
 */
void DisplayBufferNvn::applyChangeWindowCrop()
{
    if (mIsWindowCropChanged)
    {
        nvnWindowSetCrop(mWindow, mWindowCropX, mWindowCropY, mWindowCropW, mWindowCropH);
        mIsWindowCropChanged = false;
    }
}

/**
 * Creates the display textures, window and sync object.
 * @param pHeap heap to allocate from
 */
void DisplayBufferNvn::initializeImpl_(Heap* pHeap)
{
    u32 width = mSize.x;
    u32 height = mSize.y;
    NVNdevice* device = GraphicsNvn::instance()->getNvnDevice();
    auto* pool = new (pHeap, 8) NVNmemoryPool;

    SafeArray<NVNtextureBuilder, cTextureNumMax> builders;
    size_t poolSize = 0;
    const u8 textureNum = getTextureNum_();
    for (s32 i = 0; i < textureNum; i++)
    {
        NVNtextureBuilder* builder = &builders[i];
        nvnTextureBuilderSetDevice(builder, device);
        nvnTextureBuilderSetDefaults(builder);
        nvnTextureBuilderSetSize2D(builder, width, height);
        nvnTextureBuilderSetTarget(builder, NVN_TEXTURE_TARGET_2D);
        nvnTextureBuilderSetFormat(builder, NVN_FORMAT_RGBA8);
        nvnTextureBuilderSetFlags(builder, NVN_TEXTURE_FLAGS_DISPLAY);
        size_t size = nvnTextureBuilderGetStorageSize(builder);
        s32 alignment = nvnTextureBuilderGetStorageAlignment(builder);
        poolSize += (size + alignment - 1) / alignment * alignment;
    }
    poolSize = (poolSize + 0xfff) & ~size_t(0xfff);

    {
    NVNmemoryPoolBuilder poolBuilder;
    nvnMemoryPoolBuilderSetDefaults(&poolBuilder);
    nvnMemoryPoolBuilderSetDevice(&poolBuilder, GraphicsNvn::instance()->getNvnDevice());
    nvnMemoryPoolBuilderSetFlags(&poolBuilder, NVN_MEMORY_POOL_FLAGS_CPU_CACHED |
                                                   NVN_MEMORY_POOL_FLAGS_GPU_CACHED);
    void* storage = new (pHeap, 0x1000) u8[poolSize];
    memset(storage, 0, poolSize);
    nvnMemoryPoolBuilderSetStorage(&poolBuilder, storage, poolSize);
    nvnMemoryPoolInitialize(pool, &poolBuilder);
    }

    size_t offset = 0;
    const u8 textureNum2 = getTextureNum_();
    for (s32 i = 0; i < textureNum2; i++)
    {
        mTextures[i] = new (pHeap, 8) NVNtexture;
        NVNtextureBuilder* builder = &builders[i];
        nvnTextureBuilderSetStorage(builder, pool, offset);
        size_t size = nvnTextureBuilderGetStorageSize(builder);
        s32 alignment = nvnTextureBuilderGetStorageAlignment(builder);
        offset += (size + alignment - 1) / alignment * alignment;
        nvnTextureInitialize(mTextures[i], builder);
    }

    mWindow = new (pHeap, 8) NVNwindow;
    {
    NVNwindowBuilder windowBuilder;
    nvnWindowBuilderSetDefaults(&windowBuilder);
    nvnWindowBuilderSetDevice(&windowBuilder, GraphicsNvn::instance()->getNvnDevice());
    nvnWindowBuilderSetNativeWindow(&windowBuilder, mNativeWindow);
    nvnWindowBuilderSetTextures(&windowBuilder, mTextureNum, mTextures.mBuffer);
    nvnWindowBuilderSetPresentInterval(&windowBuilder, mPresentInterval);
    nvnWindowInitialize(mWindow, &windowBuilder);
    }
    nvnQueueAcquireTexture(GraphicsNvn::instance()->getNvnQueue(), mWindow, &mTextureIndex);

    mWindowCropX = 0;
    mWindowCropY = 0;
    mWindowCropW = width;
    mWindowCropH = height;

    mSync = new (pHeap, 8) NVNsync;
    nvnSyncInitialize(mSync, device);
}

}  // namespace sead
