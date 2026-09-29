#include "utility/aglDynamicTextureAllocator.h"
#include <hostio/seadHostIOPropertyEvent.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadScopedLock.h>
#include "common/aglGPUMemBlock.h"
#include "common/aglTextureFormatInfo.h"
#include "detail/aglRootNode.h"
#include "driver/aglGraphicsDriverMgr.h"

namespace agl::utl {

namespace {

const char* const cAllocatorName[] = {"Main", "Debug", "DebugHeap"};
const char* const cAllocatorShortName[] = {"Mem", "Dbg", "Dh"};

template <typename T, typename Compare>
void shakerSort(sead::PtrArray<T>* pArray, Compare cmp)
{
    T** ptrs = pArray->data();
    s32 hi = pArray->size() - 1u;
    if (hi <= 0)
    {
        return;
    }
    s32 lo = 0;
    while (lo < hi)
    {
        s32 last = lo;
        for (s32 i = lo; i < hi; i++)
        {
            if (cmp(ptrs[i], ptrs[i + 1]) > 0)
            {
                T* tmp = ptrs[i + 1];
                ptrs[i + 1] = ptrs[i];
                ptrs[i] = tmp;
                last = i;
            }
        }
        hi = last;
        if (hi <= lo)
        {
            break;
        }

        last = hi;
        for (s32 i = hi; i > lo; i--)
        {
            if (cmp(ptrs[i], ptrs[i - 1]) <= 0)
            {
                T* tmp = ptrs[i - 1];
                ptrs[i - 1] = ptrs[i];
                ptrs[i] = tmp;
                last = i;
            }
        }
        if (last == hi)
        {
            break;
        }
        lo = last;
    }
}

}  // namespace

SEAD_SINGLETON_DISPOSER_IMPL(DynamicTextureAllocator)

/**
 * Constructs the allocator without any memory.
 */
DynamicTextureAllocator::DynamicTextureAllocator() : mFlags(0x40)
{
    mFlags.set(0x200);
    detail::RootNode::setNodeMeta(this, "Icon=TEXTURE");
}

DynamicTextureAllocator::~DynamicTextureAllocator()
{
    for (s32 i = 0; i < cContextNum; i++)
    {
        Context& rContext = mContexts[i];
        rContext.mAllocators.freeBuffer();
        rContext.mDisplayList.getBuffer().deleteGPUMemBlock();
        rContext.mDisplayList.clear();
        rContext.mSuspendMemory = nullptr;
        rContext.mSuspendSize = 0;
        rContext.mIndex = i;
    }
    mTextures.freeBuffer();
    mFreeAddrs.freeBuffer();
}

void DynamicTextureAllocator::initialize(s32 textureNum, u64 size, u64 debugSize,
                                         sead::Heap* pHeap, sead::Heap* pDebugHeap)
{
    mTextures.tryAllocBuffer(textureNum, pHeap);
    mFreeAddrs.tryAllocBuffer(textureNum, pHeap);
    mStorages.tryAllocBuffer(cStorageNum, pHeap);
    mSize = size;

    const u32 attribute = mFlags.isOn(0x200) ? 0x84 : 4;

    auto* pBlock = new (pHeap) GPUMemBlock<u8>;
    pBlock->allocBuffer(size, pHeap, 8, MemoryAttribute(attribute));
    GPUMemVoidAddr addr(*pBlock, 0);

    GPUMemVoidAddr debugAddr;
    u64 debugHeapSize = 0;
    if (debugSize != 0 && pDebugHeap)
    {
        auto* pDebugBlock = new (pDebugHeap) GPUMemBlock<u8>;
        pDebugBlock->allocBuffer(debugSize, pDebugHeap, 8, MemoryAttribute(attribute));
        debugAddr = GPUMemVoidAddr(*pDebugBlock, 0);
        debugHeapSize = 0x40000000;
    }

    s32 storageIndex = 0;
    for (auto& rStorage : mStorages)
    {
        rStorage.mIsUseStorage = (attribute & 0x80) != 0;
        switch (storageIndex)
        {
        case 0:
            rStorage.mAddr = addr;
            rStorage.mSize = size;
            break;
        case 1:
            rStorage.mAddr = debugAddr;
            rStorage.mSize = debugSize;
            break;
        case 2:
            rStorage.mAddr = debugAddr;
            rStorage.mSize = debugHeapSize;
            rStorage.mIsUseStorage = false;
            break;
        }
        if (rStorage.mIsUseStorage && rStorage.mAddr.isValid())
        {
            TextureMemoryAllocator::setupStorage(&rStorage.mStorage, rStorage.mAddr,
                                                 rStorage.mSize, pHeap);
        }
        storageIndex++;
    }

    for (u32 i = 0; i < cContextNum; i++)
    {
        Context& rContext = mContexts[i];
        rContext.mAllocators.tryAllocBuffer(cStorageNum, pHeap);
        s32 allocatorIndex = 0;
        for (auto& rAllocator : rContext.mAllocators)
        {
            Storage& rStorage = mStorages[allocatorIndex];
            if (rStorage.mAddr.isValid())
            {
                rAllocator.initialize(rStorage.mAddr,
                                      rStorage.mIsUseStorage ? &rStorage.mStorage : nullptr,
                                      rStorage.mSize, mTextures.size(), pHeap);
            }
            allocatorIndex++;
        }
        if (i != cContextNum - 1)
        {
            auto* pDisplayListBlock = new (pHeap) GPUMemBlock<u8>;
            pDisplayListBlock->allocBuffer(0x1000, pHeap, 4, MemoryAttribute(0));
            rContext.mDisplayList.setBuffer(GPUMemAddr<u8>(*pDisplayListBlock, 0), 0x1000);
        }
    }

    for (s32 i = 0; i < mTextures.size(); i++)
    {
        mTextures(i).mLabel.format("agl::DTA[%d]", i);
    }
}

/**
 * Sets the heap used by the debug heap allocator of every context.
 * @param pHeap debug heap
 */
void DynamicTextureAllocator::setDebugHeap(sead::Heap* pHeap)
{
    mDebugHeap = pHeap;
    for (s32 i = 0; i < cContextNum; i++)
    {
        mContexts[i].mAllocators[2].setHeap(pHeap);
    }
}

/**
 * Frees the GPU memory blocks whose release was deferred.
 */
void DynamicTextureAllocator::calc()
{
    mFlags.set(0x4000);
    if (mFreeAddrNum == 0)
    {
        return;
    }

    driver::GraphicsDriverMgr::instance()->waitDrawDone();
    const u32 num = sead::Mathu::clampMax(mFreeAddrNum, mFreeAddrs.size());
    FreeAddr* pAddr = mFreeAddrs.getBufferPtr();
    for (u32 i = 0; i != num; i++)
    {
        pAddr[i].deleteGPUMemBlock();
    }
    mFreeAddrNum = 0;
}

/**
 * Allocates a 2D texture for the current core.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width width
 * @param height height
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::alloc(DrawContext* pDrawContext,
                                            const sead::SafeString& rName, TextureFormat format,
                                            u32 width, u32 height, u32 mipLevelNum,
                                            GPUMemVoidAddr* pAddr, AllocateType type, bool b1,
                                            bool b2)
{
    return alloc_(pDrawContext, &getCurrentContext(), rName, format, TextureType(1), width,
                  height, mipLevelNum, MultiSampleType(0), 1, pAddr, true, type, false, b2);
}

TextureDataEx* DynamicTextureAllocator::alloc_(DrawContext* pDrawContext, Context* pContext,
                                               const sead::SafeString& rName,
                                               TextureFormat format, TextureType textureType,
                                               u32 width, u32 height, u32 mipLevelNum,
                                               MultiSampleType multiSample, u32 slice,
                                               GPUMemVoidAddr* pAddr, bool withContext,
                                               AllocateType type, bool b1, bool b2)
{
    const u32 attributeFlags = u32(b2 | mFlags.isOn(0x10)) | u32(pAddr != nullptr) << 1 |
                               u32(mFlags.isOn(0x200)) << 2 | u32(mFlags.isOn(0x400)) << 3;
    const s32 allocatorIndex = u32(type) < cAllocateType_2 ? 0 : type - 1;
    const u32 sliceNum = textureType == TextureType(8) ? slice * 6 : slice;

    TextureDataEx* pTexture = nullptr;
    bool isNeedInitialize = true;

    mTextureCS.lock();
    mFrame++;
    if (mFlags.isOn(0x40))
    {
        for (auto& rTexture : mTextures)
        {
            if ((rTexture.mState.getDirect() & 3) == 2 &&
                rTexture.isSame(rName, format, textureType, width, height, multiSample, sliceNum,
                                mipLevelNum, attributeFlags))
            {
                pTexture = &rTexture;
                break;
            }
        }

        if (pTexture)
        {
            pTexture->mState.set(1);
            isNeedInitialize = pTexture->mOption.isOn(1);
        }
        else
        {
            s32 maxAge = 0;
            for (auto& rTexture : mTextures)
            {
                if (rTexture.mState.isOn(1))
                {
                    continue;
                }
                const s32 age = sead::Mathi::abs(mFrame - rTexture.mFrame);
                if (rTexture.mState.isOn(2) && mTextures.size() < age)
                {
                    rTexture.mFrame = 0;
                    rTexture.mState.reset(2);
                    rTexture.release();
                }
                if (maxAge < age)
                {
                    maxAge = age;
                    pTexture = &rTexture;
                }
            }
            pTexture->mState.set(3);
        }
    }
    else
    {
        for (auto it = mTextures.begin(mSearchIndex); it != mTextures.end(); ++it)
        {
            if (!it->mState.isOn(1))
            {
                mSearchIndex = it.getIndex();
                pTexture = &*it;
                break;
            }
        }
        if (!pTexture)
        {
            u32 i = 0;
            while (mTextures.getBufferPtr()[i].mState.isOn(1))
            {
                i++;
            }
            mSearchIndex = i;
            pTexture = &mTextures.getBufferPtr()[i];
        }
        pTexture->mState.reset(2);
        pTexture->mState.set(1);
    }
    mTextureCS.unlock();

    if (isNeedInitialize)
    {
        pTexture->initialize(format, textureType, width, height, mipLevelNum, multiSample, slice,
                             pAddr, attributeFlags);
    }
    pTexture->mContext = pContext;
    pTexture->mAllocateType = allocatorIndex;

    mAllocatorCS.lock();
    TextureMemoryAllocator& rAllocator = pContext->mAllocators[allocatorIndex];
    pTexture->mAllocatorIndex = allocatorIndex;
    pTexture->mMemoryBlock = rAllocator.alloc(pTexture->mAllocateArg, pAddr, withContext);
    if (pTexture->mMemoryBlock && !isValid_(withContext ? pContext : nullptr))
    {
        rAllocator.free(pTexture->mMemoryBlock, true);
        pTexture->mMemoryBlock = nullptr;
    }
    if (!pTexture->mMemoryBlock)
    {
        dumpAll();
    }
    mAllocatorCS.unlock();

    pTexture->reset(pDrawContext, rName, true, mFrame);
    return pTexture;
}

/**
 * Allocates a 2D array texture for the current core.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width width
 * @param height height
 * @param arrayNum number of array layers
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::allocArray(DrawContext* pDrawContext,
                                                 const sead::SafeString& rName,
                                                 TextureFormat format, u32 width, u32 height,
                                                 u32 arrayNum, u32 mipLevelNum,
                                                 GPUMemVoidAddr* pAddr, AllocateType type, bool b1,
                                                 bool b2)
{
    return alloc_(pDrawContext, &getCurrentContext(), rName, format, TextureType(4), width,
                  height, mipLevelNum, MultiSampleType(0), arrayNum, pAddr, true, type, false, b2);
}

/**
 * Allocates a 3D texture for the current core.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width width
 * @param height height
 * @param depth depth
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::alloc3D(DrawContext* pDrawContext,
                                              const sead::SafeString& rName, TextureFormat format,
                                              u32 width, u32 height, u32 depth, u32 mipLevelNum,
                                              GPUMemVoidAddr* pAddr, AllocateType type, bool b1,
                                              bool b2)
{
    return alloc_(pDrawContext, &getCurrentContext(), rName, format, TextureType(2), width,
                  height, mipLevelNum, MultiSampleType(0), depth, pAddr, true, type, false, b2);
}

/**
 * Allocates a cube map texture for the current core.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width edge length
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::allocCube(DrawContext* pDrawContext,
                                                const sead::SafeString& rName,
                                                TextureFormat format, u32 width, u32 mipLevelNum,
                                                GPUMemVoidAddr* pAddr, AllocateType type, bool b1,
                                                bool b2)
{
    return alloc_(pDrawContext, &getCurrentContext(), rName, format, TextureType(8), width, width,
                  mipLevelNum, MultiSampleType(0), 1, pAddr, true, type, false, b2);
}

/**
 * Allocates a cube map array texture for the current core.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width edge length
 * @param arrayNum number of cube maps
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::allocCubeArray(DrawContext* pDrawContext,
                                                     const sead::SafeString& rName,
                                                     TextureFormat format, u32 width, u32 arrayNum,
                                                     u32 mipLevelNum, GPUMemVoidAddr* pAddr,
                                                     AllocateType type, bool b1, bool b2)
{
    return alloc_(pDrawContext, &getCurrentContext(), rName, format, TextureType(8), width, width,
                  mipLevelNum, MultiSampleType(0), arrayNum, pAddr, true, type, false, b2);
}

/**
 * Allocates a multisampled 2D texture for the current core.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width width
 * @param height height
 * @param multiSample sample count type
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::allocMultiSample(DrawContext* pDrawContext,
                                                       const sead::SafeString& rName,
                                                       TextureFormat format, u32 width, u32 height,
                                                       MultiSampleType multiSample,
                                                       GPUMemVoidAddr* pAddr, AllocateType type,
                                                       bool b1, bool b2)
{
    return alloc_(pDrawContext, &getCurrentContext(), rName, format,
                  multiSample != MultiSampleType(0) ? TextureType(5) : TextureType(1), width,
                  height, 1, multiSample, 1, pAddr, true, type, false, b2);
}

/**
 * Allocates a 2D texture from the shared context.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width width
 * @param height height
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::allocWithoutContext(DrawContext* pDrawContext,
                                                          const sead::SafeString& rName,
                                                          TextureFormat format, u32 width,
                                                          u32 height, u32 mipLevelNum,
                                                          GPUMemVoidAddr* pAddr, AllocateType type,
                                                          bool b1, bool b2)
{
    return alloc_(pDrawContext, &mContexts[cContextNum - 1], rName, format, TextureType(1), width,
                  height, mipLevelNum, MultiSampleType(0), 1, pAddr, false, type, false, b2);
}

/**
 * Allocates a texture with the layout of another texture from the shared context.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param rTextureData texture to copy the layout from
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::allocWithoutContext(DrawContext* pDrawContext,
                                                          const sead::SafeString& rName,
                                                          const TextureData& rTextureData,
                                                          GPUMemVoidAddr* pAddr, AllocateType type,
                                                          bool b1, bool b2)
{
    return alloc_(pDrawContext, &mContexts[cContextNum - 1], rName,
                  TextureFormat(rTextureData.getTextureFormat()),
                  TextureType(rTextureData.getTextureType()), rTextureData.getWidth(0),
                  rTextureData.getHeight(0), rTextureData.getMipLevelNum(),
                  MultiSampleType(rTextureData.getMultiSampleType()),
                  rTextureData.getMipSlice(0), pAddr, false, type, false, b2);
}

/**
 * Allocates a 2D array texture from the shared context.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width width
 * @param height height
 * @param arrayNum number of array layers
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::allocArrayWithoutContext(
    DrawContext* pDrawContext, const sead::SafeString& rName, TextureFormat format, u32 width,
    u32 height, u32 arrayNum, u32 mipLevelNum, GPUMemVoidAddr* pAddr, AllocateType type, bool b1,
    bool b2)
{
    return alloc_(pDrawContext, &mContexts[cContextNum - 1], rName, format, TextureType(4), width,
                  height, mipLevelNum, MultiSampleType(0), arrayNum, pAddr, false, type, false,
                  b2);
}

/**
 * Allocates a 3D texture from the shared context.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width width
 * @param height height
 * @param depth depth
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::alloc3DWithoutContext(
    DrawContext* pDrawContext, const sead::SafeString& rName, TextureFormat format, u32 width,
    u32 height, u32 depth, u32 mipLevelNum, GPUMemVoidAddr* pAddr, AllocateType type, bool b1,
    bool b2)
{
    return alloc_(pDrawContext, &mContexts[cContextNum - 1], rName, format, TextureType(2), width,
                  height, mipLevelNum, MultiSampleType(0), depth, pAddr, false, type, false, b2);
}

/**
 * Allocates a cube map texture from the shared context.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width edge length
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::allocCubeWithoutContext(DrawContext* pDrawContext,
                                                              const sead::SafeString& rName,
                                                              TextureFormat format, u32 width,
                                                              u32 mipLevelNum,
                                                              GPUMemVoidAddr* pAddr,
                                                              AllocateType type, bool b1, bool b2)
{
    return alloc_(pDrawContext, &mContexts[cContextNum - 1], rName, format, TextureType(8), width,
                  width, mipLevelNum, MultiSampleType(0), 1, pAddr, false, type, false, b2);
}

/**
 * Allocates a cube map array texture from the shared context.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width edge length
 * @param arrayNum number of cube maps
 * @param mipLevelNum number of mip levels
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::allocCubeArrayWithoutContext(
    DrawContext* pDrawContext, const sead::SafeString& rName, TextureFormat format, u32 width,
    u32 arrayNum, u32 mipLevelNum, GPUMemVoidAddr* pAddr, AllocateType type, bool b1, bool b2)
{
    return alloc_(pDrawContext, &mContexts[cContextNum - 1], rName, format, TextureType(8), width,
                  width, mipLevelNum, MultiSampleType(0), arrayNum, pAddr, false, type, false,
                  b2);
}

/**
 * Allocates a multisampled 2D texture from the shared context.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param format texture format
 * @param width width
 * @param height height
 * @param multiSample sample count type
 * @param pAddr user memory address
 * @param type allocation type
 * @param b1 unused
 * @param b2 whether to force the linear attribute
 * @return allocated texture
 */
TextureData* DynamicTextureAllocator::allocMultiSampleWithoutContext(
    DrawContext* pDrawContext, const sead::SafeString& rName, TextureFormat format, u32 width,
    u32 height, MultiSampleType multiSample, GPUMemVoidAddr* pAddr, AllocateType type, bool b1,
    bool b2)
{
    return alloc_(pDrawContext, &mContexts[cContextNum - 1], rName, format,
                  multiSample != MultiSampleType(0) ? TextureType(5) : TextureType(1), width,
                  height, 1, multiSample, 1, pAddr, false, type, false, b2);
}

bool DynamicTextureAllocator::free(const TextureData* pTexture)
{
    return free_(pTexture);
}

/**
 * Returns the memory of a texture to its allocator.
 * @param pTexture texture to free
 * @return always true
 */
bool DynamicTextureAllocator::free_(const TextureData* pTexture)
{
    auto* pTextureEx = const_cast<TextureDataEx*>(static_cast<const TextureDataEx*>(pTexture));

    sead::ScopedLock<sead::CriticalSection> allocatorLock(&mAllocatorCS);
    if (mFlags.isOn(0x4000))
    {
        if (pTextureEx->mMemoryBlock->mMemBlockAddr.isValid())
        {
            mFreeAddrs[mFreeAddrNum++] = pTextureEx->mMemoryBlock->mMemBlockAddr;
        }
        pTextureEx->mContext->mAllocators[pTextureEx->mAllocatorIndex].free(
            pTextureEx->mMemoryBlock, false);
    }
    else
    {
        pTextureEx->mContext->mAllocators[pTextureEx->mAllocatorIndex].free(
            pTextureEx->mMemoryBlock, true);
    }
    pTextureEx->mMemoryBlock = nullptr;

    {
        sead::ScopedLock<sead::CriticalSection> textureLock(&mTextureCS);
        pTextureEx->mState.reset(1);
    }
    return true;
}

/**
 * Starts recording the texture cache display list of the current core.
 */
void DynamicTextureAllocator::beginCache()
{
    Context& rContext = getCurrentContext();
    rContext.mSuspendSize = DisplayList::suspend(&rContext.mSuspendMemory);
    if (rContext.mDisplayList.beginDisplayList())
    {
        rContext.mFlags.set(cContextFlag_DisplayList);
    }
    rContext.mFlags.set(cContextFlag_Cache);
}

/**
 * Stops recording the texture cache display list of the current core.
 */
void DynamicTextureAllocator::endCache()
{
    Context& rContext = getCurrentContext();
    if (rContext.mFlags.isOn(cContextFlag_DisplayList))
    {
        rContext.mDisplayList.endDisplayList();
    }
    DisplayList::resume(rContext.mSuspendMemory, rContext.mSuspendSize);
    rContext.mFlags.reset(cContextFlag_Cache);
}

/**
 * Looks up the texture owning a memory block.
 * @param pBlock memory block
 * @param ppName receives the texture name
 * @param pContextIndex receives the allocation type index
 * @return whether a texture owns the block
 */
bool DynamicTextureAllocator::queryTextureMemoryInfo(
    const TextureMemoryAllocator::MemoryBlock* pBlock, const char** ppName,
    s32* pContextIndex) const
{
    for (s32 i = 0; i < mTextures.size(); i++)
    {
        const TextureDataEx& rTexture = mTextures[i];
        if (rTexture.mMemoryBlock == pBlock)
        {
            if (ppName)
            {
                *ppName = rTexture.mName.cstr();
            }
            if (pContextIndex)
            {
                *pContextIndex = rTexture.mAllocateType;
            }
            return true;
        }
    }
    return false;
}

/**
 * Checks whether the texture can be reused for an allocation request.
 * @param rName texture name
 * @param format texture format
 * @param type texture type
 * @param width width
 * @param height height
 * @param multiSample sample count type
 * @param slice number of slices
 * @param mipLevelNum number of mip levels
 * @param flags attribute flags
 * @return whether the layout is identical
 */
bool TextureDataEx::isSame(const sead::SafeString& rName, TextureFormat format, TextureType type,
                           u32 width, u32 height, MultiSampleType multiSample, u32 slice,
                           s32 mipLevelNum, u32 flags) const
{
    return getTextureFormat() == s32(format) && getTextureType() == s32(type) &&
           getWidth(0) == width && getHeight(0) == height &&
           getMultiSampleType() == s32(multiSample) &&
           u32(getMipSlice(0)) == slice && mMipLevelNum == mipLevelNum &&
           mAttributeFlags == flags;
}

/**
 * Releases the NVN texture.
 */
void TextureDataEx::release()
{
    releaseNVNtexture();
}

void TextureDataEx::initialize(TextureFormat format, TextureType type, u32 width, u32 height,
                               u32 mipLevelNum, MultiSampleType multiSample, u32 slice,
                               GPUMemVoidAddr* pAddr, u32 flags)
{
    clear();
    mMipLevelNum = mipLevelNum;
    mAttributeFlags = flags;
    mInfo.set(1);

    u32 attribute = flags & 1;
    if ((flags & 4) && TextureFormatInfo::isRenderTargetCompressAvailable(format) &&
        !mOption.isOn(0x20))
    {
        attribute |= 2;
    }

    const auto textureAttribute = TextureAttribute(attribute | ((mAttributeFlags & 8) >> 1));
    switch (s32(type))
    {
    case 0:
        initialize_(TextureType(0), format, width, 1, 1, mipLevelNum, textureAttribute,
                    MultiSampleType(0), true);
        break;
    case 1:
        initialize_(TextureType(1), format, width, height, 1, mipLevelNum, textureAttribute,
                    MultiSampleType(0), true);
        break;
    case 2:
        initialize_(TextureType(2), format, width, height, slice, mipLevelNum, textureAttribute,
                    MultiSampleType(0), true);
        break;
    case 3:
        initialize_(TextureType(3), format, width, slice, 1, mipLevelNum, textureAttribute,
                    MultiSampleType(0), true);
        break;
    case 4:
        initialize_(TextureType(4), format, width, height, slice, mipLevelNum, textureAttribute,
                    MultiSampleType(0), true);
        break;
    case 5:
        if (s32(multiSample) >= 1)
        {
            initialize_(TextureType(5), format, width, height, 1, 1, textureAttribute,
                        multiSample, true);
        }
        else
        {
            initialize_(TextureType(1), format, width, height, 1, 1, textureAttribute,
                        MultiSampleType(0), true);
        }
        break;
    case 8:
        if (slice == 1)
        {
            initialize_(TextureType(8), format, width, height, 6, mipLevelNum, textureAttribute,
                        MultiSampleType(0), true);
        }
        else
        {
            initializeCubeMapArray(format, width, height, slice, mipLevelNum, textureAttribute);
        }
        break;
    default:
        break;
    }

    const detail::Surface& rSurface = getSurface();
    mAllocateArg.mAlignment = rSurface.mAlignment;
    mAllocateArg.mImageOffset = rSurface.mStorageSize;
    if (getMipLevelNum() >= 2)
    {
        mAllocateArg.mMipSize = rSurface._14;
    }
    else
    {
        mAllocateArg.mMipSize = 0;
    }
    mAllocateArg.mExtraSize = 0;
    mAllocateArg.mExtraAlignment = 4;
    mAllocateArg.mStorageClass = rSurface.mStorageClass;

    u32 size = mAllocateArg.mImageOffset;
    if (mAllocateArg.mMipSize != 0)
    {
        size = sead::Mathu::roundUpPow2(size, mAllocateArg.mAlignment) + mAllocateArg.mMipSize;
    }
    mAllocateArg.mSize = size + mAllocateArg.mAlignment;
}

/**
 * Checks that no context allocator overlaps the shared context allocators.
 * @param pContext context to check, or null to check every per-core context
 * @return whether the allocators are disjoint
 */
bool DynamicTextureAllocator::isValid_(const Context* pContext) const
{
    if (pContext)
    {
        return isContextValid_(pContext);
    }

    for (s32 i = 0; i < cContextNum - 1; i++)
    {
        if (!isContextValid_(&mContexts[i]))
        {
            return false;
        }
    }
    return true;
}

/**
 * Dumps the allocations of every context.
 */
void DynamicTextureAllocator::dumpAll() const
{
    for (s32 i = 0; i < cContextNum; i++)
    {
        dump_(i);
    }
}

/**
 * Binds the allocated memory to the texture.
 * @param pDrawContext draw context
 * @param rName texture name
 * @param updateImage whether to update the image and mip pointers
 * @param frame current allocation frame
 */
void TextureDataEx::reset(DrawContext* pDrawContext, const sead::SafeString& rName,
                          bool updateImage, u32 frame)
{
    if (updateImage)
    {
        const GPUMemVoidAddr imageAddr = mMemoryBlock->mImageAddr;
        const s32 textureID = getTextureID();
        setImagePtr(imageAddr, 0);
        if (mMemoryBlock->mMipAddr.isValid())
        {
            setMipPtr(mMemoryBlock->mMipAddr);
        }
        mInfo.change(2, textureID != getTextureID());
    }
    mUseSize = mContext->mAllocators[mAllocatorIndex].getUsedSize();
    mName = rName;
    setCompSelDefault();
    mFrame = frame;
}

/**
 * Checks that the allocators of a context do not overlap the shared context allocators.
 * @param pContext context to check
 * @return whether the allocators are disjoint
 */
bool DynamicTextureAllocator::isContextValid_(const Context* pContext) const
{
    bool isOverlapped = false;
    const u32 num = sead::Mathu::min(pContext->mAllocators.size(), 2);
    const TextureMemoryAllocator* pAllocators = pContext->mAllocators.getBufferPtr();
    for (u32 i = 0; i != num; i++)
    {
        isOverlapped |=
            pAllocators[i].isOverwrapperd(mContexts[cContextNum - 1].mAllocators[i]);
    }
    return !isOverlapped;
}

void DynamicTextureAllocator::dump_(s32 index) const
{
    const Context& rContext = mContexts[index];
    sead::FixedSafeString<1024> str;
    for (const auto& rAllocator : rContext.mAllocators)
    {
        rAllocator.dumpDetail();
    }
    for (const auto& rTexture : mTextures)
    {
        if (rTexture.mMemoryBlock && rTexture.mContext == &rContext)
        {
            const detail::Surface& rSurface = rTexture.getSurface();
            str.format("[%s] (%4dx%4d) size:%d [%s]", rTexture.mName.cstr(),
                       rTexture.getWidth(0), rTexture.getHeight(0),
                       rSurface.mStorageSize + rSurface._14,
                       cAllocatorName[rTexture.mAllocatorIndex]);
        }
    }
}

u64 DynamicTextureAllocator::getUseSize() const
{
    const Context& rContext = mContexts[sead::CoreInfo::getCurrentCoreId()];
    if (rContext.mAllocators.size() == 0)
    {
        return 0;
    }
    return rContext.mAllocators(0).getUsedSize();
}

/**
 * Gets the used size of the main allocator of the shared context.
 * @return used size in bytes
 */
u64 DynamicTextureAllocator::getUseSizeWithoutContext() const
{
    const Context& rContext = mContexts[cContextNum - 1];
    if (rContext.mAllocators.size() == 0)
    {
        return 0;
    }
    return rContext.mAllocators(0).getUsedSize();
}

/**
 * Dumps the allocations of the current core's context.
 */
void DynamicTextureAllocator::dump() const
{
    dump_(sead::CoreInfo::getCurrentCoreId());
}

void DynamicTextureAllocator::genMessage(sead::hostio::Context* pContext)
{
    sead::FixedSafeString<1024> str;
    const Context& rContext = mContexts[mCurrentContext];
    {
        sead::FormatFixedSafeString<1024> header("GroupHeader= context: %d, Dir=Y",
                                                 mCurrentContext);
    }

    s32 allocatorIndex = 0;
    for (const auto& rAllocator : rContext.mAllocators)
    {
        rAllocator.genMessageInfo(cAllocatorName[allocatorIndex], pContext);
        allocatorIndex++;
    }

    sead::FixedPtrArray<TextureDataEx, 512> textures;
    for (auto& rTexture : mTextures)
    {
        if (rTexture.mState.isOn(2) && rTexture.mContext == &rContext)
        {
            textures.pushBack(&rTexture);
        }
    }
    shakerSort(&textures, [](const TextureDataEx* pA, const TextureDataEx* pB) {
        return pA->mName.compare(pB->mName);
    });

    for (auto& rTexture : textures)
    {
        rTexture.genMessage(pContext);
    }
}

void TextureDataEx::genMessage(sead::hostio::Context* pContext)
{
    sead::FixedSafeString<1024> str;
    const detail::Surface& rSurface = getSurface();
    str.format("[%s] (%4dx%4d) size:%d align:%d [%s]->[%s](use:%d)", mName.cstr(), getWidth(0),
               getHeight(0), rSurface.mStorageSize + rSurface._14, rSurface.mAlignment,
               cAllocatorShortName[mAllocateType], cAllocatorShortName[mAllocatorIndex],
               mUseSize);
}

/**
 * Handles the host IO buttons that flush the cache and dump the allocations.
 * @param pEvent property event
 */
void DynamicTextureAllocator::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    switch (reinterpret_cast<uintptr_t>(pEvent->getId()))
    {
    case 1000:
    {
        sead::ScopedLock<sead::CriticalSection> lock(&mTextureCS);
        for (auto& rTexture : mTextures)
        {
            rTexture.mState.reset(2);
        }
        break;
    }
    case 1001:
        dump_(mCurrentContext);
        break;
    default:
        break;
    }
}

/**
 * Constructs an unused texture entry.
 */
TextureDataEx::TextureDataEx() : mMagic(0xee), mOption(0), _18d(0), mAttributeFlags(0)
{
    setDebugLabel(mLabel);
    clear();
}

/**
 * Resets the allocation state of the texture entry.
 */
void TextureDataEx::clear()
{
    mFrame = 0;
    mMipLevelNum = 0;
    mMemoryBlock = nullptr;
    mAllocateType = 0;
    mAllocatorIndex = 0;
    mUseSize = 0;
    mContext = nullptr;
    mAttributeFlags = 0;
    mInfo.reset(3);
}

}  // namespace agl::utl
