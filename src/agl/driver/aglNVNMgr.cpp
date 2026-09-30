#include "driver/aglNVNMgr.h"

#include <codec/seadHashCRC16.h>
#include <gfx/seadColor.h>
#include <nvn/nvn_FuncPtrInline.h>
#include <prim/seadScopedLock.h>

#include "common/aglDrawContext.h"
#include "common/aglGPUMemAddr.h"
#include "common/aglGPUMemBlock.h"
#include "common/aglShaderLocation.h"

namespace agl
{
class TextureFormatInfo
{
public:
    static NVNformat convFormatAGLToDriver(TextureFormat format);
};
}  // namespace agl

namespace agl::driver
{
namespace
{

template <typename T>
T& graphicsMember(sead::GraphicsNvn* pGraphics, uintptr_t offset)
{
    return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(pGraphics) + offset);
}

template <typename T, typename Compare>
void sortPtrArray(sead::PtrArray<T>& rArray, Compare compare)
{
    const s32 num = rArray.size();
    if (num < 2)
    {
        return;
    }

    T** pArray = rArray.data();
    s32 lo = 0;
    s32 hi = num - 1;
    do
    {
        s32 last = lo;
        for (s32 i = lo; i < hi; ++i)
        {
            if (compare(pArray[i], pArray[i + 1]) > 0)
            {
                T* tmp = pArray[i + 1];
                pArray[i + 1] = pArray[i];
                pArray[i] = tmp;
                last = i;
            }
        }

        hi = last;
        if (hi <= lo)
        {
            break;
        }

        last = hi;
        for (s32 i = hi; i > lo; --i)
        {
            if (compare(pArray[i], pArray[i - 1]) < 0)
            {
                T* tmp = pArray[i - 1];
                pArray[i - 1] = pArray[i];
                pArray[i] = tmp;
                last = i;
            }
        }

        lo = last;
    } while (lo != hi);
}

constexpr NVNboolean cNvnTrue = 1;

const char* const cNvnDeviceInfoNames[] = {
    "NVN_DEVICE_INFO_API_MAJOR_VERSION",
    "NVN_DEVICE_INFO_API_MINOR_VERSION",
    "NVN_DEVICE_INFO_UNIFORM_BUFFER_BINDINGS_PER_STAGE",
    "NVN_DEVICE_INFO_MAX_UNIFORM_BUFFER_SIZE",
    "NVN_DEVICE_INFO_UNIFORM_BUFFER_ALIGNMENT",
    "NVN_DEVICE_INFO_COLOR_BUFFER_BINDINGS",
    "NVN_DEVICE_INFO_VERTEX_BUFFER_BINDINGS",
    "NVN_DEVICE_INFO_TRANSFORM_FEEDBACK_BUFFER_BINDINGS",
    "NVN_DEVICE_INFO_SHADER_STORAGE_BUFFER_BINDINGS_PER_STAGE",
    "NVN_DEVICE_INFO_TEXTURE_BINDINGS_PER_STAGE",
    "NVN_DEVICE_INFO_COUNTER_ALIGNMENT",
    "NVN_DEVICE_INFO_TRANSFORM_FEEDBACK_BUFFER_ALIGNMENT",
    "NVN_DEVICE_INFO_TRANSFORM_FEEDBACK_CONTROL_ALIGNMENT",
    "NVN_DEVICE_INFO_INDIRECT_DRAW_ALIGNMENT",
    "NVN_DEVICE_INFO_VERTEX_ATTRIBUTES",
    "NVN_DEVICE_INFO_TEXTURE_DESCRIPTOR_SIZE",
    "NVN_DEVICE_INFO_SAMPLER_DESCRIPTOR_SIZE",
    "NVN_DEVICE_INFO_RESERVED_TEXTURE_DESCRIPTORS",
    "NVN_DEVICE_INFO_RESERVED_SAMPLER_DESCRIPTORS",
    "NVN_DEVICE_INFO_COMMAND_BUFFER_COMMAND_ALIGNMENT",
    "NVN_DEVICE_INFO_COMMAND_BUFFER_CONTROL_ALIGNMENT",
    "NVN_DEVICE_INFO_COMMAND_BUFFER_MIN_COMMAND_SIZE",
    "NVN_DEVICE_INFO_COMMAND_BUFFER_MIN_CONTROL_SIZE",
    "NVN_DEVICE_INFO_SHADER_SCRATCH_MEMORY_SCALE_FACTOR_MINIMUM",
    "NVN_DEVICE_INFO_SHADER_SCRATCH_MEMORY_SCALE_FACTOR_RECOMMENDED",
    "NVN_DEVICE_INFO_SHADER_SCRATCH_MEMORY_ALIGNMENT",
    "NVN_DEVICE_INFO_SHADER_SCRATCH_MEMORY_GRANULARITY",
    "NVN_DEVICE_INFO_MAX_TEXTURE_ANISOTROPY",
    "NVN_DEVICE_INFO_MAX_COMPUTE_WORK_GROUP_SIZE_X",
    "NVN_DEVICE_INFO_MAX_COMPUTE_WORK_GROUP_SIZE_Y",
    "NVN_DEVICE_INFO_MAX_COMPUTE_WORK_GROUP_SIZE_Z",
    "NVN_DEVICE_INFO_MAX_COMPUTE_WORK_GROUP_SIZE_THREADS",
    "NVN_DEVICE_INFO_MAX_COMPUTE_DISPATCH_WORK_GROUPS_X",
    "NVN_DEVICE_INFO_MAX_COMPUTE_DISPATCH_WORK_GROUPS_Y",
    "NVN_DEVICE_INFO_MAX_COMPUTE_DISPATCH_WORK_GROUPS_Z",
    "NVN_DEVICE_INFO_IMAGE_BINDINGS_PER_STAGE",
    "NVN_DEVICE_INFO_MAX_TEXTURE_POOL_SIZE",
    "NVN_DEVICE_INFO_MAX_SAMPLER_POOL_SIZE",
    "NVN_DEVICE_INFO_MAX_VIEWPORTS",
    "NVN_DEVICE_INFO_MEMPOOL_TEXTURE_OBJECT_PAGE_ALIGNMENT",
    "NVN_DEVICE_INFO_SUPPORTS_MIN_MAX_FILTERING",
    "NVN_DEVICE_INFO_SUPPORTS_STENCIL8_FORMAT",
    "NVN_DEVICE_INFO_SUPPORTS_ASTC_FORMATS",
    "NVN_DEVICE_INFO_L2_SIZE",
    "NVN_DEVICE_INFO_MAX_TEXTURE_LEVELS",
    "NVN_DEVICE_INFO_MAX_TEXTURE_LAYERS",
    "NVN_DEVICE_INFO_GLSLC_MAX_SUPPORTED_GPU_CODE_MAJOR_VERSION",
    "NVN_DEVICE_INFO_GLSLC_MIN_SUPPORTED_GPU_CODE_MAJOR_VERSION",
    "NVN_DEVICE_INFO_GLSLC_MAX_SUPPORTED_GPU_CODE_MINOR_VERSION",
    "NVN_DEVICE_INFO_GLSLC_MIN_SUPPORTED_GPU_CODE_MINOR_VERSION",
    "NVN_DEVICE_INFO_SUPPORTS_CONSERVATIVE_RASTER",
    "NVN_DEVICE_INFO_SUBPIXEL_BITS",
    "NVN_DEVICE_INFO_MAX_SUBPIXEL_BIAS_BITS",
    "NVN_DEVICE_INFO_INDIRECT_DISPATCH_ALIGNMENT",
    "NVN_DEVICE_INFO_ZCULL_SAVE_RESTORE_ALIGNMENT",
    "NVN_DEVICE_INFO_SHADER_SCRATCH_MEMORY_COMPUTE_SCALE_FACTOR_MINIMUM",
    "NVN_DEVICE_INFO_LINEAR_TEXTURE_STRIDE_ALIGNMENT",
    "NVN_DEVICE_INFO_LINEAR_RENDER_TARGET_STRIDE_ALIGNMENT",
    "NVN_DEVICE_INFO_MEMORY_POOL_PAGE_SIZE",
    "NVN_DEVICE_INFO_SUPPORTS_ZERO_FROM_UNMAPPED_VIRTUAL_POOL_PAGES",
    "NVN_DEVICE_INFO_UNIFORM_BUFFER_UPDATE_ALIGNMENT",
    "NVN_DEVICE_INFO_MAX_TEXTURE_SIZE",
    "NVN_DEVICE_INFO_MAX_BUFFER_TEXTURE_SIZE",
    "NVN_DEVICE_INFO_MAX_3D_TEXTURE_SIZE",
    "NVN_DEVICE_INFO_MAX_CUBE_MAP_TEXTURE_SIZE",
    "NVN_DEVICE_INFO_MAX_RECTANGLE_TEXTURE_SIZE",
    "NVN_DEVICE_INFO_SUPPORTS_PASSTHROUGH_GEOMETRY_SHADERS",
    "NVN_DEVICE_INFO_SUPPORTS_VIEWPORT_SWIZZLE",
    "NVN_DEVICE_INFO_SUPPORTS_SPARSE_TILED_PACKAGED_TEXTURES",
    "NVN_DEVICE_INFO_SUPPORTS_ADVANCED_BLEND_MODES",
    "NVN_DEVICE_INFO_MAX_PRESENT_INTERVAL",
    "NVN_DEVICE_INFO_SUPPORTS_DRAW_TEXTURE",
    "NVN_DEVICE_INFO_SUPPORTS_TARGET_INDEPENDENT_RASTERIZATION",
    "NVN_DEVICE_INFO_SUPPORTS_FRAGMENT_COVERAGE_TO_COLOR",
    "NVN_DEVICE_INFO_SUPPORTS_POST_DEPTH_COVERAGE",
    "NVN_DEVICE_INFO_SUPPORTS_IMAGES_USING_TEXTURE_HANDLES",
    "NVN_DEVICE_INFO_SUPPORTS_SAMPLE_LOCATIONS",
    "NVN_DEVICE_INFO_MAX_SAMPLE_LOCATION_TABLE_ENTRIES",
    "NVN_DEVICE_INFO_SHADER_CODE_MEMORY_POOL_PADDING_SIZE",
    "NVN_DEVICE_INFO_MAX_PATCH_SIZE",
    "NVN_DEVICE_INFO_QUEUE_COMMAND_MEMORY_GRANULARITY",
    "NVN_DEVICE_INFO_QUEUE_COMMAND_MEMORY_MIN_SIZE",
    "NVN_DEVICE_INFO_QUEUE_COMMAND_MEMORY_DEFAULT_SIZE",
    "NVN_DEVICE_INFO_QUEUE_COMPUTE_MEMORY_GRANULARITY",
    "NVN_DEVICE_INFO_QUEUE_COMPUTE_MEMORY_MIN_SIZE",
    "NVN_DEVICE_INFO_QUEUE_COMPUTE_MEMORY_DEFAULT_SIZE",
};

template <typename T, typename Key>
s32 binarySearch(const sead::PtrArray<T>& rArray, const Key* pKey,
                 s32 (*compare)(const T*, const Key*))
{
    if (rArray.size() == 0)
    {
        return -1;
    }

    s32 a = 0;
    s32 b = rArray.size() - 1;
    while (a < b)
    {
        const s32 m = (a + b) / 2;
        const s32 c = compare(rArray.unsafeAt(m), pKey);
        if (c == 0)
        {
            return m;
        }

        if (c < 0)
        {
            a = m + 1;
        }
        else
        {
            b = m;
        }
    }

    if (compare(rArray.unsafeAt(a), pKey) == 0)
    {
        return a;
    }

    return -1;
}


}  // namespace

/**
 * Creates the driver manager singleton, using the GraphicsDriverMgr singleton disposer.
 * @param pHeap heap to allocate the instance from
 * @return the driver manager instance
 */
NVNMgr* NVNMgr::createInstance(sead::Heap* pHeap)
{
    if (!sInstance)
    {
        auto* buffer = new (pHeap, alignof(NVNMgr)) u8[sizeof(NVNMgr)];
        auto* disposerBuffer = buffer + offsetof(NVNMgr, mSingletonDisposerBuf_);
        SingletonDisposer_::sStaticDisposer = new (disposerBuffer) SingletonDisposer_(pHeap);
        sInstance = new (buffer) NVNMgr();
    }

    return static_cast<NVNMgr*>(sInstance);
}

/**
 * Constructs the manager, taking the NVN device and queue from the sead graphics instance.
 */
NVNMgr::NVNMgr()
    : mDevice(sead::GraphicsNvn::instance()->getNvnDevice()),
      mQueue(graphicsMember<NVNqueue*>(sead::GraphicsNvn::instance(), 0x38)),
      _48(graphicsMember<void*>(sead::GraphicsNvn::instance(), 0x40)), mFlags(0x18c881),
      mSamplerCursor(0), mTextureCursor(0), mRegisteredSamplerNum(0), mRegisteredTextureNum(0),
      mDebugCallback(this, &NVNMgr::debugCallback_), mCopyNum(0),
      mCopySize(0), mCopyTime(0), mTileWidth(16), mTileHeight(16)
{
    _30 = graphicsMember<void*>(sead::GraphicsNvn::instance(), 0x48);
    graphicsMember<void*>(sead::GraphicsNvn::instance(), 0x1f8) = &mDebugCallback;
    for (auto& counter : mCounters)
    {
        counter.storeNonAtomic(0);
    }

    mName.append("Default");
}

/**
 * Handles an NVN debug layer message (empty in release builds).
 * @param rParam callback parameters
 */
void NVNMgr::debugCallback_(const sead::GraphicsNvn::NvnDebugCallbackParam& rParam) {}

/**
 * Frees the registration tables and the command buffer memory.
 */
NVNMgr::~NVNMgr()
{
    mSamplers.freeBuffer();
    mSamplerList.freeBuffer();
    mTextures.freeBuffer();
    GPUMemAddr<u8> buffer = mDisplayList.getBuffer();
    buffer.deleteGPUMemBlock();
}

/**
 * Allocates the command buffer memory and the sampler and texture registration tables.
 * @param pHeap heap to allocate from
 * @param pDebugHeap debug heap (unused)
 */
void NVNMgr::initialize(sead::Heap* pHeap, sead::Heap* pDebugHeap)
{
    initialize_(pHeap);

    auto* block = new (pHeap, 8) GPUMemBlock<u8>;
    block->allocBuffer_(0x400000, pHeap, 4, MemoryAttribute::_00);
    GPUMemAddrBase addr(*block, 0);
    mDisplayList.setBuffer(addr, 0x400000);

    auto* graphics = sead::GraphicsNvn::instance();
    mSamplerIdBase = graphics->getNewSamplerId();
    mSamplers.tryAllocBuffer(graphicsMember<s32>(graphics, 0x114) - mSamplerIdBase, pHeap);
    mSamplerList.allocBuffer(mSamplers.size(), pHeap);
    for (auto it = mSamplers.begin(), end = mSamplers.end(); it != end; ++it)
    {
        memset(it->mSampler, 0, sizeof(it->mSampler));
        it->mRefCount = 0;
        it->mKey = 0;
    }

    if (graphicsMember<bool>(graphics, 0x202))
    {
        mFlags.set(8);
    }

    mTextureIdBase = graphics->getNewTextureId();
    u32 textureNum = graphicsMember<s32>(graphics, 0x110) - mTextureIdBase;
    textureNum |= textureNum >> 1;
    textureNum |= textureNum >> 2;
    textureNum |= textureNum >> 4;
    textureNum |= textureNum >> 8;
    textureNum |= textureNum >> 16;
    textureNum = (textureNum + 1) >> 1;
    mTextures.tryAllocBuffer(textureNum, pHeap);
    for (auto& texture : mTextures)
    {
        texture.mRefCount.storeNonAtomic(-1);
        texture.mName = nullptr;
        texture.mFlags.storeNonAtomic(0);
    }

    registerFastClearColor(TextureFormat(0x2b), sead::Color4f::cBlack);
}

/**
 * Registers a floating point fast clear color for a texture format.
 * @param format texture format
 * @param rColor clear color
 * @return whether the registration succeeded
 */
bool NVNMgr::registerFastClearColor(TextureFormat format, const sead::Color4f& rColor)
{
    NVNformat nvnFormat = TextureFormatInfo::convFormatAGLToDriver(format);
    return nvnDeviceRegisterFastClearColor(mDevice, &rColor.r, nvnFormat) == cNvnTrue;
}

/**
 * Registers an unsigned integer fast clear color for a texture format.
 * @param format texture format
 * @param rColor clear color
 * @return whether the registration succeeded
 */
bool NVNMgr::registerFastClearColor(TextureFormat format, const sead::Vector4<u32>& rColor)
{
    NVNformat nvnFormat = TextureFormatInfo::convFormatAGLToDriver(format);
    u32 color[4] = {rColor.x, rColor.y, rColor.z, rColor.w};
    return nvnDeviceRegisterFastClearColorui(mDevice, color, nvnFormat) == cNvnTrue;
}

/**
 * Registers a signed integer fast clear color for a texture format.
 * @param format texture format
 * @param rColor clear color
 * @return whether the registration succeeded
 */
bool NVNMgr::registerFastClearColor(TextureFormat format, const sead::Vector4<s32>& rColor)
{
    NVNformat nvnFormat = TextureFormatInfo::convFormatAGLToDriver(format);
    s32 color[4] = {rColor.x, rColor.y, rColor.z, rColor.w};
    return nvnDeviceRegisterFastClearColori(mDevice, color, nvnFormat) == cNvnTrue;
}

/**
 * Registers a fast clear depth value.
 * @param depth clear depth
 * @return whether the registration succeeded
 */
bool NVNMgr::registerFastClearDepth(f32 depth)
{
    return nvnDeviceRegisterFastClearDepth(mDevice, depth) == cNvnTrue;
}

u32 NVNMgr::registerSampler(const NVNsampler* pSampler, const char* pName)
{
    const u32 key = sead::HashCRC16::calcHash(reinterpret_cast<const u8*>(pSampler) + 8, 0x58);
    graphicsMember<sead::Atomic<s32>>(sead::GraphicsNvn::instance(), 0x108).load();

    sead::ScopedLock<sead::CriticalSection> lock(&mSamplerCS);

    if (mFlags.isOnBit(0))
    {
        if (mFlags.isOnBit(3))
        {
            for (auto it = mSamplerList.begin(), end = mSamplerList.end(); it != end; ++it)
            {
                if (isEqual(*reinterpret_cast<const NVNsampler*>((*it).mSampler), *pSampler))
                {
                    u32 id = (&(*it) - mSamplers.getBufferPtr()) + mSamplerIdBase;
                    if ((*it).mRefCount++ == 0)
                    {
                        mRegisteredSamplerNum++;
                    }

                    return id;
                }
            }
        }
        else
        {
            const u16 key16 = key;
            s32 index = binarySearch(mSamplerList, &key16, compareSamplerKeyU16_);
            if (index >= 0)
            {
                const s32 num = mSamplerList.size();
                for (s32 i = index;; i++)
                {
                    if (isEqual(*reinterpret_cast<const NVNsampler*>(mSamplerList.at(i)),
                                *pSampler))
                    {
                        u32 id = (mSamplerList.at(i) - mSamplers.getBufferPtr()) + mSamplerIdBase;

                        if (mSamplerList.unsafeAt(i)->mRefCount++ == 0)
                        {
                            mRegisteredSamplerNum++;
                        }

                        return id;
                    }

                    if (i + 1 >= num || mSamplerList.unsafeAt(i + 1)->mKey != key16)
                    {
                        break;
                    }
                }
            }
        }
    }

    const u16 start = mSamplerCursor;
    for (mSamplerCursor++; mSamplerCursor < mSamplers.size(); mSamplerCursor++)
    {
        s32 id = registerSampler_(mSamplerCursor, pSampler, key & 0xffff);
        if (id != -1)
        {
            return id;
        }
    }

    for (mSamplerCursor = 0; mSamplerCursor < start; mSamplerCursor++)
    {
        s32 id = registerSampler_(mSamplerCursor, pSampler, key & 0xffff);
        if (id != -1)
        {
            return id;
        }
    }

    return 0;
}

/**
 * Compares the key of a registered sampler with a 16-bit key.
 * @param pA registered sampler
 * @param pKey key to compare with
 * @return the difference between the keys
 */
s32 NVNMgr::compareSamplerKeyU16_(const SamplerKey* pA, const u16* pKey)
{
    return pA->mKey - *pKey;
}

/**
 * Checks whether two samplers are identical.
 * @param rA first sampler
 * @param rB second sampler
 * @return whether the samplers are identical
 */
bool NVNMgr::isEqual(const NVNsampler& rA, const NVNsampler& rB)
{
    return nvnSamplerCompare(&rA, &rB);
}

s32 NVNMgr::registerSampler_(s32 index, const NVNsampler* pSampler, u32 key)
{
    SamplerKey& sampler = mSamplers[index];
    if (sampler.mRefCount != 0)
    {
        return -1;
    }

    struct SamplerData
    {
        u8 mData[0x60];
    };

    *reinterpret_cast<SamplerData*>(sampler.mSampler) =
        *reinterpret_cast<const SamplerData*>(pSampler);
    sampler.mKey = key;

    const s32 id = mSamplerIdBase + index;
    auto* graphics = sead::GraphicsNvn::instance();
    sead::CriticalSection* cs = &graphicsMember<sead::CriticalSection>(graphics, 0x198);
    cs->lock();
    nvnSamplerPoolRegisterSampler(
        &graphicsMember<NVNsamplerPool>(sead::GraphicsNvn::instance(), 0x78), id,
        reinterpret_cast<const NVNsampler*>(sampler.mSampler));
    mRegisteredSamplerNum++;
    sampler.mRefCount++;
    if (!(sampler.mFlags & 1))
    {
        mSamplerList.pushBack(&sampler);
        sampler.mFlags |= 1;
        sortPtrArray(mSamplerList, compareSamplerKey_);
    }

    cs->unlock();
    return id;
}

/**
 * Compares the keys of two registered samplers.
 * @param pA first sampler
 * @param pB second sampler
 * @return the difference between the keys
 */
s32 NVNMgr::compareSamplerKey_(const SamplerKey* pA, const SamplerKey* pB)
{
    return pA->mKey - pB->mKey;
}

/**
 * Increments the reference count of a registered sampler.
 * @param id sampler ID
 * @return true
 */
bool NVNMgr::countupSampler(u32 id)
{
    sead::ScopedLock<sead::CriticalSection> lock(&mSamplerCS);
    mSamplers[id - mSamplerIdBase].mRefCount++;
    return true;
}

/**
 * Decrements the reference count of a registered sampler.
 * @param id sampler ID
 * @return whether the sampler is no longer referenced
 */
bool NVNMgr::releaseSampler(u32 id)
{
    sead::ScopedLock<sead::CriticalSection> lock(&mSamplerCS);
    if (--mSamplers[id - mSamplerIdBase].mRefCount == 0)
    {
        mRegisteredSamplerNum--;
        return true;
    }

    return false;
}

/**
 * Checks whether a registered sampler is identical to a sampler.
 * @param id sampler ID
 * @param rSampler sampler to compare with
 * @return whether the samplers are identical
 */
bool NVNMgr::isEqual(u32 id, const NVNsampler& rSampler) const
{
    return nvnSamplerCompare(
        reinterpret_cast<const NVNsampler*>(mSamplers[id - mSamplerIdBase].mSampler), &rSampler);
}

/**
 * Checks whether two textures are identical.
 * @param rA first texture
 * @param rB second texture
 * @return whether the textures are identical
 */
bool NVNMgr::isEqual(const NVNtexture& rA, const NVNtexture& rB)
{
    return nvnTextureCompare(&rA, &rB);
}

/**
 * Registers a texture in a free slot of the texture pool.
 * @param pTexture texture to register
 * @param pView texture view, or nullptr
 * @param pName debug name
 * @return the texture ID, or 0 if the pool is full
 */
s32 NVNMgr::registerTexture(const NVNtexture* pTexture, const NVNtextureView* pView,
                            const char* pName)
{
    graphicsMember<sead::Atomic<s32>>(sead::GraphicsNvn::instance(), 0x10c).load();

    const s32 start = mTextureCursor++ & (mTextures.size() - 1);
    s32 index = start;
    do
    {
        mTextures[(index + mTextures.size() / 2) & (mTextures.size() - 1)]
            .mRefCount.compareExchange(0, -1);
        s32 id = registerTexture_(index, pTexture, pView, pName);
        if (id != -1)
        {
            return id;
        }

        index = mTextureCursor++ & (mTextures.size() - 1);
    } while (index != start);

    return 0;
}

/**
 * Registers a texture in a slot of the texture pool if the slot is free.
 * @param index slot index
 * @param pTexture texture to register
 * @param pView texture view, or nullptr
 * @param pName debug name
 * @return the texture ID, or -1 if the slot is in use
 */
s32 NVNMgr::registerTexture_(s32 index, const NVNtexture* pTexture, const NVNtextureView* pView,
                             const char* pName)
{
    TextureInfo& texture = mTextures[index];
    s32 id = -1;
    if (texture.mRefCount.compareExchange(-1, 1))
    {
        id = mTextureIdBase + index;
        {
            auto* graphics = sead::GraphicsNvn::instance();
            sead::ScopedLock<sead::CriticalSection> lock(graphics->getCriticalSection2());
            nvnTexturePoolRegisterTexture(sead::GraphicsNvn::instance()->getTexturePool(), id,
                                          pTexture, pView);
        }

        if (!pView)
        {
            if (nvnTextureGetFlags(pTexture) & NVN_TEXTURE_FLAGS_COMPRESSIBLE)
            {
                texture.mFlags.fetchOr(1);
            }
            else
            {
                texture.mFlags.fetchAnd(~1u);
            }
        }

        mRegisteredTextureNum++;
    }

    return id;
}

/**
 * Increments the reference count of a registered texture.
 * @param id texture ID
 * @return whether the texture was already referenced
 */
bool NVNMgr::countupTexture(u32 id)
{
    return mTextures[id - mTextureIdBase].mRefCount++ != 0;
}

/**
 * Decrements the reference count of a registered texture.
 * @param id texture ID
 * @return whether the texture is no longer referenced
 */
bool NVNMgr::releaseTexture(u32 id)
{
    if (mTextures[id - mTextureIdBase].mRefCount-- == 1)
    {
        mRegisteredTextureNum--;
        return true;
    }

    return false;
}

/**
 * Clears the dirty flag of a compressible texture.
 * @param id texture ID
 * @return whether the flag was set
 */
bool NVNMgr::offDirtyTextureCompressTexture_(u32 id)
{
    return mTextures[id - mTextureIdBase].mFlags.fetchAnd(~1u) & 1;
}

/**
 * Submits the draw context's commands and waits for the GPU to finish.
 * @param pDrawContext draw context
 */
void NVNMgr::waitDrawDone(DrawContext* pDrawContext) const
{
    NVNcommandBuffer* commandBuffer = getNvnCommandBuffer(pDrawContext);
    auto* graphics = sead::GraphicsNvn::instance();
    sead::CriticalSection* cs = &graphicsMember<sead::CriticalSection>(graphics, 0x118);
    cs->lock();
    NVNcommandHandle handle = nvnCommandBufferEndRecording(commandBuffer);
    nvnQueueSubmitCommands(mQueue, 1, &handle);
    nvnQueueFinish(mQueue);
    sead::GraphicsNvn::instance()->applyDeferredFinalizes();
    nvnCommandBufferBeginRecording(commandBuffer);
    cs->unlock();
}

/**
 * Converts a shader type to an NVN shader stage.
 * @param type shader type
 * @return the NVN shader stage
 */
NVNshaderStage NVNMgr::getNVNshaderStage(ShaderType type)
{
    static const NVNshaderStage cStages[] = {
        NVN_SHADER_STAGE_VERTEX, NVN_SHADER_STAGE_FRAGMENT, NVN_SHADER_STAGE_GEOMETRY,
        NVN_SHADER_STAGE_COMPUTE, NVN_SHADER_STAGE_VERTEX};
    return cStages[type];

}

/**
 * Computes the NVN texture flags for a texture.
 * @param compressible whether the texture should be compressible
 * @param minimalLayout whether the texture should use the minimal layout
 * @param format NVN texture format
 * @return the NVN texture flags
 */
u32 NVNMgr::getTextureFlags(bool compressible, bool minimalLayout, NVNformat format) const
{
    const bool isDepth = u32(format - NVN_FORMAT_DEPTH16) < 5;
    u32 flags = minimalLayout ? NVN_TEXTURE_FLAGS_MINIMAL_LAYOUT : 0;
    if (compressible || isDepth)
    {
        flags |= NVN_TEXTURE_FLAGS_COMPRESSIBLE;
        if (isDepth && mFlags.isOnBit(10))
        {
            flags |= NVN_TEXTURE_FLAGS_ADAPTIVE_ZCULL;
        }
    }

    return flags;
}

/**
 * Sets the CPU access flags of a texture memory pool.
 * @param pFlags memory pool flags to modify
 */
void NVNMgr::setMemoryPoolSettingTexture(sead::BitFlag32* pFlags) const
{
    const bool cached = mFlags.isOnBit(1);
    pFlags->reset(7);
    pFlags->set(cached ? 4u : 1u);
}

/**
 * Enables tiled caching for the draw context.
 * @param pDrawContext draw context
 * @param tileWidth tile width
 * @param tileHeight tile height
 */
void NVNMgr::enableTiledCaching(DrawContext* pDrawContext, u32 tileWidth, u32 tileHeight) const
{
    if (mFlags.getDirect() & 0x30000)
    {
        return;
    }

    if (mFlags.isOnBit(18))
    {
        tileWidth = mTileWidth;
        tileHeight = mTileHeight;
    }

    nvnCommandBufferSetTiledCacheAction(getNvnCommandBuffer(pDrawContext),
                                        NVN_TILED_CACHE_ACTION_ENABLE);
    nvnCommandBufferSetTiledCacheTileSize(getNvnCommandBuffer(pDrawContext), tileWidth,
                                          tileHeight);
}

/**
 * Disables tiled caching for the draw context.
 * @param pDrawContext draw context
 */
void NVNMgr::disableTiledCaching(DrawContext* pDrawContext) const
{
    if (mFlags.getDirect() & 0x30000)
    {
        return;
    }

    nvnCommandBufferSetTiledCacheAction(getNvnCommandBuffer(pDrawContext),
                                        NVN_TILED_CACHE_ACTION_DISABLE);
}

/**
 * Enables tiled caching when tiled caching debugging is active.
 * @param pDrawContext draw context
 */
void NVNMgr::beginTiledCachingDebug(DrawContext* pDrawContext) const
{
    if (!mFlags.isOnBit(16))
    {
        return;
    }

    nvnCommandBufferSetTiledCacheAction(getNvnCommandBuffer(pDrawContext),
                                        NVN_TILED_CACHE_ACTION_ENABLE);
    nvnCommandBufferSetTiledCacheTileSize(getNvnCommandBuffer(pDrawContext), mTileWidth,
                                          mTileHeight);
}

/**
 * Disables tiled caching when tiled caching debugging is active.
 * @param pDrawContext draw context
 */
void NVNMgr::endTiledCachingDebug(DrawContext* pDrawContext) const
{
    if (!mFlags.isOnBit(16))
    {
        return;
    }

    nvnCommandBufferSetTiledCacheAction(getNvnCommandBuffer(pDrawContext),
                                        NVN_TILED_CACHE_ACTION_DISABLE);
}

/**
 * Decrements a debug counter.
 * @param index counter index
 */
void NVNMgr::countDown(s32 index)
{
    mCounters[index]--;
}

/**
 * Prints the names of all registered textures (output removed in release builds).
 */
void NVNMgr::dampRegisteredTextureList() const
{
    for (const auto& texture : mTextures)
    {
        if (texture.mRefCount > 0)
        {
            texture.mRefCount.load();
            const char* name = texture.mName ? texture.mName : "untitled";
            sead::FormatFixedSafeString<65> str("%.64s", name);
        }
    }
}

/**
 * Initializes a memory pool.
 * @param pPool memory pool to initialize
 * @param pBuilder memory pool builder
 */
void NVNMgr::nvnMemoryPoolInitialize(NVNmemoryPool* pPool, const NVNmemoryPoolBuilder* pBuilder)
{
    ::nvnMemoryPoolInitialize(pPool, pBuilder);
}

/**
 * Binds a texture to every shader stage that uses it.
 * @param pDrawContext draw context
 * @param handle texture handle
 * @param rLocation sampler location
 * @param textureId texture ID used for dirty tracking
 */
void NVNMgr::nvnCommandBufferBindTexture(DrawContext* pDrawContext, u64 handle,
                                         const ShaderLocation& rLocation, s32 textureId)
{
    NVNcommandBuffer* commandBuffer = getNvnCommandBuffer(pDrawContext);
    u32 mask = 0;
    for (s32 i = 0; i < cShaderType_Num; i++)
    {
        const s32 location = rLocation.getLocation(ShaderType(i));
        if (location != -1)
        {
            ::nvnCommandBufferBindTexture(commandBuffer, getNVNshaderStage(ShaderType(i)),
                                          location, handle);
            mask |= 1 << i;
        }
    }

    if (mask != 0 && !mFlags.isOnBit(25) && pDrawContext->isTextureDirty(mask, textureId))
    {
        pDrawContext->barrierTexture(2);
    }
}

/**
 * Binds an image to every shader stage that uses it.
 * @param pDrawContext draw context
 * @param handle image handle
 * @param rLocation image location
 * @param textureId texture ID used for dirty tracking
 */
void NVNMgr::nvnCommandBufferBindImage(DrawContext* pDrawContext, u64 handle,
                                       const ShaderLocation& rLocation, s32 textureId)
{
    NVNcommandBuffer* commandBuffer = getNvnCommandBuffer(pDrawContext);
    u32 mask = 0;
    for (s32 i = 0; i < cShaderType_Num; i++)
    {
        const s32 location = rLocation.getLocation(ShaderType(i));
        if (location != -1)
        {
            ::nvnCommandBufferBindImage(commandBuffer, getNVNshaderStage(ShaderType(i)), location,
                                        handle);
            mask |= 1 << i;
        }
    }

    if (mask != 0 && !mFlags.isOnBit(25) && pDrawContext->isTextureDirty(mask, textureId))
    {
        pDrawContext->barrierTexture(6);
    }
}

/**
 * Invalidates the GPU texture cache after rendering to a color target.
 * @param pDrawContext draw context
 * @param textureId texture ID of the render target
 */
void NVNMgr::invalidateGPUCacheColor(DrawContext* pDrawContext, s32 textureId) const
{
    if (textureId == -1)
    {
        return;
    }

    if (mFlags.isOnBit(21) && !(mTextures[textureId - mTextureIdBase].mFlags.load() & 2))
    {
        pDrawContext->setTextureDirty(textureId);
        return;
    }

    if (mFlags.isOnBit(24))
    {
        pDrawContext->barrierTexture(1);
    }
    else
    {
        pDrawContext->barrierTexture(2);
    }
}

/**
 * Invalidates the GPU texture cache after rendering to a depth target.
 * @param pDrawContext draw context
 * @param textureId texture ID of the render target
 */
void NVNMgr::invalidateGPUCacheDepth(DrawContext* pDrawContext, s32 textureId) const
{
    if (textureId == -1)
    {
        return;
    }

    if (mFlags.isOnBit(21) && !(mTextures[textureId - mTextureIdBase].mFlags.load() & 2))
    {
        pDrawContext->setTextureDirty(textureId);
        return;
    }

    if (mFlags.isOnBit(24))
    {
        pDrawContext->barrierTexture(1);
    }
    else
    {
        pDrawContext->barrierTexture(2);
    }
}

/**
 * Inserts a texture or shader barrier.
 * @param pDrawContext draw context
 * @param barrier barrier type
 */
void NVNMgr::nvnCommandBufferBarrier(DrawContext* pDrawContext, Barrier barrier)
{
    switch (barrier)
    {
    case cBarrier_Texture:
        pDrawContext->barrierTexture(4);
        break;
    case cBarrier_Shader:
        pDrawContext->barrierShader(8);
        break;
    }
}

/**
 * Inserts a shader order and texture invalidation barrier.
 * @param pDrawContext draw context
 * @param enable whether to insert the barrier
 */
void NVNMgr::nvnCommandBufferBarrier_Shader(DrawContext* pDrawContext, bool enable)
{
    if (enable)
    {
        ::nvnCommandBufferBarrier(getNvnCommandBuffer(pDrawContext),
                                NVN_BARRIER_ORDER_INDIRECT_DATA_BIT |
                                    NVN_BARRIER_INVALIDATE_SHADER_BIT);
    }
}

void NVNMgr::genMessage(sead::hostio::Context* pContext)
{
    {
        sead::FormatFixedSafeString<1024> msg(
            "Texture: %d/%d", mRegisteredTextureNum.load(),
            graphicsMember<s32>(sead::GraphicsNvn::instance(), 0x110));
    }

    {
        sead::FormatFixedSafeString<1024> msg("Num:%d Total:%d[byte] %d[ns]", mCopyNum.load(),
                                              mCopySize.load(), mCopyTime.load());
    }

    {
        sead::FormatFixedSafeString<1024> msg(
            "Time:%f[MB/s]",
            static_cast<f32>(mCopySize.load() >> 20) / static_cast<f32>(mCopyTime.load() / 1000000));
    }

    {
        sead::FormatFixedSafeString<1024> msg("%s:%d", "nvnCommandBufferInitialize/Finalize",
                                              mCounters[0].load());
    }

    {
        sead::FormatFixedSafeString<1024> msg("%s:%d", "nvnTextureInitialize/Finalize",
                                              mCounters[1].load());
    }

    {
        sead::FormatFixedSafeString<1024> msg("%s:%d", "nvnSamplerInitialize/Finalize",
                                              mCounters[2].load());
    }

    {
        sead::FormatFixedSafeString<1024> msg("%s:%d", "nvnBufferInitialize/Finalize(Vertex)",
                                              mCounters[3].load());
    }

    {
        sead::FormatFixedSafeString<1024> msg(
            "%s:%d", "nvnBufferInitialize/Finalize(UniformBlock)", mCounters[4].load());
    }

    for (s32 i = 0; i < 0x56; i++)
    {
        if (mFilter.isEmpty() || sead::SafeString(cNvnDeviceInfoNames[i]).include(mFilter))
        {
            int value;
            nvnDeviceGetInteger(mDevice, static_cast<NVNdeviceInfo>(i), &value);
            sead::FormatFixedSafeString<1024> msg("%s:%d(0x%4x)", cNvnDeviceInfoNames[i], value,
                                                  value);
        }
    }

    {
        sead::FormatFixedSafeString<1024> msg("??:%d/%d", mRegisteredSamplerNum,
                                              mSamplers.size());
    }

    {
        sead::ScopedLock<sead::CriticalSection> lock(&mSamplerCS);
        for (auto it = mSamplers.begin(), end = mSamplers.end(); it != end; ++it)
        {
            if (it->mRefCount != 0)
            {
                sead::FormatFixedSafeString<1024> msg("%4d / ref:%6d",
                                                      it.getIndex() + mSamplerIdBase,
                                                      it->mRefCount);
            }
        }
    }

    {
        sead::FormatFixedSafeString<1024> msg("??:%d/%d", mRegisteredTextureNum.load(),
                                              mTextures.size());
    }

    for (auto it = mTextures.begin(), end = mTextures.end(); it != end; ++it)
    {
        const TextureInfo& texture = *it;
        if (texture.mRefCount > 0)
        {
            sead::FormatFixedSafeString<1024> msg(
                "%4d / ref:%4d [%-48s] %s %s", it.getIndex() + mTextureIdBase,
                texture.mRefCount.load(), texture.mName, (texture.mFlags & 1) ? "cmp" : "---",
                (texture.mFlags & 2) ? "ref" : "---");
        }
    }
}

/**
 * Handles a host IO property event.
 * @param pEvent property event
 */
void NVNMgr::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    switch (*reinterpret_cast<const u64*>(reinterpret_cast<uintptr_t>(pEvent) + 8))
    {
    case 1000:
        dampRegisteredTextureList();
        break;
    case 1001:
    {
        const f32 color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        nvnDeviceRegisterFastClearColor(mDevice, color, NVN_FORMAT_RGBA16F);
        break;
    }
    }
}

}  // namespace agl::driver
