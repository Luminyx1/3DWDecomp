#include "utility/aglDynamicTextureCache.h"
#include "utility/aglDynamicTextureAllocator.h"

namespace agl::utl {

/**
 * Constructs an empty texture cache.
 */
DynamicTextureCache::DynamicTextureCache() = default;

/**
 * Releases the cache buffers.
 */
DynamicTextureCache::~DynamicTextureCache()
{
    mCache.freeBuffer();
    mBuffer.freeBuffer();
}

/**
 * Allocates room for the cached textures.
 * @param num maximum number of cached textures
 * @param pHeap heap used for the buffers
 */
void DynamicTextureCache::initialize(s32 num, sead::Heap* pHeap)
{
    mCache.allocBuffer(num, pHeap);
    mBuffer.tryAllocBuffer(num, pHeap);
}

/**
 * Starts recording a new cache if the allocator is caching.
 * @return always true
 */
bool DynamicTextureCache::begin()
{
    if (DynamicTextureAllocator::instance()->isCacheEnabled())
    {
        mFlags.reset(1);
        mCache.clear();
    }

    return true;
}

/**
 * Switches to replaying the cached textures if the allocator is caching.
 */
void DynamicTextureCache::end()
{
    mFlags.change(1, DynamicTextureAllocator::instance()->getCurrentContext().mFlags.isOn(
                         DynamicTextureAllocator::cContextFlag_Cache));
}

/**
 * Records a copy of an allocated texture if the allocator is caching.
 * @param pTexture allocated texture
 */
void DynamicTextureCache::pushBack_(TextureData* pTexture)
{
    if (!DynamicTextureAllocator::instance()->isCacheEnabled())
    {
        return;
    }

    mBuffer[mCache.size()] = *pTexture;
    mCache.pushBack(&mBuffer[mCache.size()]);
}

/**
 * Takes the oldest cached texture.
 * @param pDrawContext draw context
 * @return cached texture, or null if the cache is empty
 */
TextureData* DynamicTextureCache::popFront_(DrawContext* pDrawContext)
{
    return mCache.popFront();
}

/**
 * Allocates a 2D texture or replays the next cached one.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width width
 * @param height height
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b passed to the allocator
 * @return allocated or cached texture
 */
TextureData* DynamicTextureCache::alloc(DrawContext* pDrawContext, const sead::SafeString& rName,
                                        TextureFormat format, u32 width, u32 height,
                                        u32 mipLevelNum, GPUMemVoidAddr* pAddr, AllocateType type,
                                        bool b)
{
    if (mFlags.isOn(1))
    {
        return popFront_(pDrawContext);
    }

    TextureData* pTexture = DynamicTextureAllocator::instance()->alloc(
        pDrawContext, rName, format, width, height, mipLevelNum, pAddr,
        DynamicTextureAllocator::AllocateType(type), b, false);
    pushBack_(pTexture);
    return pTexture;
}

/**
 * Allocates a 2D array texture or replays the next cached one.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width width
 * @param height height
 * @param arrayNum number of array layers
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b passed to the allocator
 * @return allocated or cached texture
 */
TextureData* DynamicTextureCache::allocArray(DrawContext* pDrawContext,
                                             const sead::SafeString& rName, TextureFormat format,
                                             u32 width, u32 height, u32 arrayNum, u32 mipLevelNum,
                                             GPUMemVoidAddr* pAddr, AllocateType type, bool b)
{
    if (mFlags.isOn(1))
    {
        return popFront_(pDrawContext);
    }

    TextureData* pTexture = DynamicTextureAllocator::instance()->allocArray(
        pDrawContext, rName, format, width, height, arrayNum, mipLevelNum, pAddr,
        DynamicTextureAllocator::AllocateType(type), b, false);
    pushBack_(pTexture);
    return pTexture;
}

/**
 * Allocates a 3D texture or replays the next cached one.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width width
 * @param height height
 * @param depth depth
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b passed to the allocator
 * @return allocated or cached texture
 */
TextureData* DynamicTextureCache::alloc3D(DrawContext* pDrawContext, const sead::SafeString& rName,
                                          TextureFormat format, u32 width, u32 height, u32 depth,
                                          u32 mipLevelNum, GPUMemVoidAddr* pAddr,
                                          AllocateType type, bool b)
{
    if (mFlags.isOn(1))
    {
        return popFront_(pDrawContext);
    }

    TextureData* pTexture = DynamicTextureAllocator::instance()->alloc3D(
        pDrawContext, rName, format, width, height, depth, mipLevelNum, pAddr,
        DynamicTextureAllocator::AllocateType(type), b, false);
    pushBack_(pTexture);
    return pTexture;
}

/**
 * Allocates a cube map texture or replays the next cached one.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width edge length
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b passed to the allocator
 * @return allocated or cached texture
 */
TextureData* DynamicTextureCache::allocCube(DrawContext* pDrawContext,
                                            const sead::SafeString& rName, TextureFormat format,
                                            u32 width, u32 mipLevelNum, GPUMemVoidAddr* pAddr,
                                            AllocateType type, bool b)
{
    if (mFlags.isOn(1))
    {
        return popFront_(pDrawContext);
    }

    TextureData* pTexture = DynamicTextureAllocator::instance()->allocCube(
        pDrawContext, rName, format, width, mipLevelNum, pAddr,
        DynamicTextureAllocator::AllocateType(type), b, false);
    pushBack_(pTexture);
    return pTexture;
}

/**
 * Allocates a cube map array texture or replays the next cached one.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width edge length
 * @param arrayNum number of cube maps
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b passed to the allocator
 * @return allocated or cached texture
 */
TextureData* DynamicTextureCache::allocCubeArray(DrawContext* pDrawContext,
                                                 const sead::SafeString& rName,
                                                 TextureFormat format, u32 width, u32 arrayNum,
                                                 u32 mipLevelNum, GPUMemVoidAddr* pAddr,
                                                 AllocateType type, bool b)
{
    if (mFlags.isOn(1))
    {
        return popFront_(pDrawContext);
    }

    TextureData* pTexture = DynamicTextureAllocator::instance()->allocCubeArray(
        pDrawContext, rName, format, width, arrayNum, mipLevelNum, pAddr,
        DynamicTextureAllocator::AllocateType(type), b, false);
    pushBack_(pTexture);
    return pTexture;
}

/**
 * Allocates a multisampled 2D texture or replays the next cached one.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width width
 * @param height height
 * @param multiSample sample count type
 * @param pAddr user memory address
 * @param type allocation type
 * @param b passed to the allocator
 * @return allocated or cached texture
 */
TextureData* DynamicTextureCache::allocMultiSample(DrawContext* pDrawContext,
                                                   const sead::SafeString& rName,
                                                   TextureFormat format, u32 width, u32 height,
                                                   MultiSampleType multiSample,
                                                   GPUMemVoidAddr* pAddr, AllocateType type,
                                                   bool b)
{
    if (mFlags.isOn(1))
    {
        return popFront_(pDrawContext);
    }

    TextureData* pTexture = DynamicTextureAllocator::instance()->allocMultiSample(
        pDrawContext, rName, format, width, height, multiSample, pAddr,
        DynamicTextureAllocator::AllocateType(type), b, false);
    pushBack_(pTexture);
    return pTexture;
}

/**
 * Frees a texture unless the cache is replaying.
 * @param pTexture texture to free
 */
void DynamicTextureCache::free(TextureData* pTexture)
{
    if (mFlags.isOn(1))
    {
        return;
    }

    DynamicTextureAllocator::instance()->free(pTexture);
}

}  // namespace agl::utl
