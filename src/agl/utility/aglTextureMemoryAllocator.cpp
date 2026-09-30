#include "utility/aglTextureMemoryAllocator.h"
#include <container/seadSafeArray.h>
#include <prim/seadSafeString.h>
#include "common/aglTextureFormatInfo.h"
#include "detail/aglSurface.h"

namespace agl::utl
{

/**
 * Constructs an empty allocator.
 */
TextureMemoryAllocator::TextureMemoryAllocator()
{
    mUsedList.initOffset(offsetof(MemoryBlock, mListNode));
    mFreeList.initOffset(offsetof(MemoryBlock, mListNode));
    mUnusedList.initOffset(offsetof(MemoryBlock, mListNode));
    mUsedList.clear();
    mFreeList.clear();
    mUnusedList.clear();
}

/**
 * Frees the memory block buffer.
 */
TextureMemoryAllocator::~TextureMemoryAllocator()
{
    mBlocks.freeBuffer();
}

/**
 * Sets up the allocator for a memory range.
 * @param addr start of the managed memory
 * @param pStorage physical storages (may be null)
 * @param size size of the managed memory in bytes
 * @param blockNum maximum number of memory blocks
 * @param pHeap heap used for the memory block buffer
 */
void TextureMemoryAllocator::initialize(GPUMemVoidAddr addr, sead::Buffer<Storage>* pStorage,
                                        u64 size, s32 blockNum, sead::Heap* pHeap)
{
    if (size == 0)
    {
        return;
    }

    mStorage = pStorage;
    mBlocks.tryAllocBuffer(blockNum, pHeap);

    for (auto& block : mBlocks)
    {
        block.mSize = 0;
        block.mStorageClass = 0;
        mUnusedList.pushBack(&block);
    }

    MemoryBlock* block = mUnusedList.popBack();
    block->mAddr = addr;
    block->mSize = static_cast<u32>(size);
    mFreeList.pushBack(block);

    mAddr = block->mAddr;
    mEndOffset = block->mAddr.getByteOffset() + block->mSize;
}

/**
 * Allocates a memory block from the first free block that is large enough.
 * @param rArg allocation parameters
 * @param pAddr receives the address of the extra area (may be null)
 * @param fromFront whether to search from the front and allocate at the start of free blocks
 * @return the allocated block, or null if no free block is large enough
 */
TextureMemoryAllocator::MemoryBlock*
TextureMemoryAllocator::alloc(const AllocateArg& rArg, GPUMemVoidAddr* pAddr, bool fromFront)
{
    if (!mAddr.isValid())
    {
        return nullptr;
    }

    if (fromFront)
    {
        for (MemoryBlock* block = mFreeList.front(); block; block = mFreeList.next(block))
        {
            if (alloc_(block, rArg, pAddr, true))
            {
                return block;
            }
        }
    }
    else
    {
        for (MemoryBlock* block = mFreeList.back(); block; block = mFreeList.prev(block))
        {
            if (alloc_(block, rArg, pAddr, false))
            {
                return block;
            }
        }
    }

    return nullptr;
}

/**
 * Tries to allocate from a free block, splitting off the remainder.
 * @param pBlock free block to allocate from
 * @param rArg allocation parameters
 * @param pAddr receives the address of the extra area (may be null)
 * @param fromFront whether to allocate at the start of the block
 * @return whether the allocation succeeded
 */
bool TextureMemoryAllocator::alloc_(MemoryBlock* pBlock, const AllocateArg& rArg,
                                    GPUMemVoidAddr* pAddr, bool fromFront)
{
    u64 size = rArg.mSize;

    if (mFlags & 1)
    {
        size = (size + 0xffff) & ~u64(0xffff);
    }

    if (pBlock->mSize < size)
    {
        return false;
    }

    if (size < pBlock->mSize)
    {
        MemoryBlock* rest = mUnusedList.popBack();
        rest->mSize = pBlock->mSize - size;

        if (fromFront)
        {
            rest->mAddr = GPUMemVoidAddr(pBlock->mAddr, size);
            mFreeList.insertAfter(pBlock, rest);
        }
        else
        {
            rest->mAddr = pBlock->mAddr;
            pBlock->mAddr = GPUMemVoidAddr(pBlock->mAddr, rest->mSize);
            mFreeList.insertBefore(pBlock, rest);
        }
    }

    pBlock->mSize = size;

    GPUMemVoidAddr baseAddr;
    sead::Heap* heap = mHeap;

    if (!heap)
    {
        pBlock->mMemBlockAddr.invalidate();
        baseAddr = pBlock->mAddr;

        if (sead::Buffer<Storage>* storages = mStorage)
        {
            s32 storageClass = -1;

            for (auto& storage : *storages)
            {
                if (storage.mStorageClass == rArg.mStorageClass)
                {
                    storageClass = rArg.mStorageClass;

                    if (storage.mBlock.getSize() != 0)
                    {
                        baseAddr = GPUMemVoidAddr(GPUMemVoidAddr(storage.mBlock, 0),
                                                  baseAddr.getByteOffset());
                    }

                    break;
                }
            }

            pBlock->mStorageClass = storageClass;
        }
    }
    else
    {
        const s32 alignment = fromFront ? rArg.mAlignment : -rArg.mAlignment;
        auto* memBlock = new (heap, 8) GPUMemBlock<u8>;
        memBlock->allocBuffer_(size, heap, alignment, MemoryAttribute::CompressibleMemory);
        baseAddr = GPUMemVoidAddr(*memBlock, 0);
        pBlock->mMemBlockAddr = baseAddr;
    }

    GPUMemVoidAddr addr = baseAddr;
    addr.roundUp(rArg.mAlignment);
    pBlock->mImageAddr = addr;
    addr = GPUMemVoidAddr(addr, rArg.mImageOffset);

    if (rArg.mMipSize != 0)
    {
        addr.roundUp(rArg.mAlignment);
        pBlock->mMipAddr = addr;
    }
    else
    {
        pBlock->mMipAddr.invalidate();
    }

    if (pAddr && rArg.mExtraSize != 0)
    {
        addr.roundUp(rArg.mExtraAlignment);
        *pAddr = addr;
        addr = GPUMemVoidAddr(addr, rArg.mExtraSize);
    }

    mFreeList.erase(pBlock);
    mUsedList.pushBack(pBlock);
    mUsedSize += pBlock->mSize;
    return true;
}

/**
 * Computes the size of the largest free block.
 * @return size of the largest free block in bytes
 */
u64 TextureMemoryAllocator::getMaxAllocatableSize() const
{
    u64 maxSize = 0;

    for (const MemoryBlock* block = mFreeList.front(); block; block = mFreeList.next(block))
    {
        if (maxSize < block->mSize)
        {
            maxSize = block->mSize;
        }
    }

    return maxSize;
}

/**
 * Returns a block to the free list, merging it with adjacent free blocks.
 * @param pBlock block to free
 * @param deleteMemBlock whether to delete the GPU memory block created for it
 */
void TextureMemoryAllocator::free(MemoryBlock* pBlock, bool deleteMemBlock)
{
    mUsedSize -= pBlock->mSize;

    if (pBlock->mMemBlockAddr.isValid())
    {
        if (deleteMemBlock)
        {
            pBlock->mMemBlockAddr.deleteGPUMemBlock();
        }

        pBlock->mMemBlockAddr.invalidate();
    }

    mUsedList.erase(pBlock);

    if (!mFreeList.front())
    {
        mFreeList.pushFront(pBlock);
        return;
    }

    for (auto& block : mFreeList)
    {
        if (pBlock->getEndOffset() <= block.mAddr.getByteOffset())
        {
            mFreeList.insertBefore(&block, pBlock);

            if (pBlock->getEndOffset() == block.mAddr.getByteOffset())
            {
                pBlock->mSize += block.mSize;
                mFreeList.erase(&block);
                mUnusedList.pushBack(&block);
            }

            MemoryBlock* prev = mFreeList.prev(pBlock);

            if (prev && prev->getEndOffset() == pBlock->mAddr.getByteOffset())
            {
                pBlock->mAddr = prev->mAddr;
                pBlock->mSize += prev->mSize;
                mFreeList.erase(prev);
                mUnusedList.pushBack(prev);
            }

            break;
        }
    }

    if (!pBlock->mListNode.isLinked())
    {
        mFreeList.pushBack(pBlock);
        MemoryBlock* prev = mFreeList.prev(pBlock);

        if (prev && prev->getEndOffset() == pBlock->mAddr.getByteOffset())
        {
            pBlock->mAddr = prev->mAddr;
            pBlock->mSize += prev->mSize;
            mFreeList.erase(prev);
            mUnusedList.pushBack(prev);
        }
    }

    for (auto& block : mFreeList)
    {
        static_cast<void>(block);
    }
}

/**
 * Prints a memory block (empty in release builds).
 * @param rTitle title of the dump
 * @param rIndent indentation
 * @param rBlock block to print
 */
void TextureMemoryAllocator::dump(const sead::SafeString& rTitle, const sead::SafeString& rIndent,
                                  const MemoryBlock& rBlock) const
{
}

/**
 * Prints every free and used block (empty in release builds).
 */
void TextureMemoryAllocator::dumpDetail() const
{
    if (mFreeList.front())
    {
        for (const auto& block : mFreeList)
        {
            dump("", "", block);
        }
    }

    if (mUsedList.front())
    {
        for (const auto& block : mUsedList)
        {
            dump("", "", block);
        }
    }
}

/**
 * Formats the memory range of this allocator for host IO.
 * @param rName name of the allocator
 * @param pContext host IO context
 */
void TextureMemoryAllocator::genMessageInfo(const sead::SafeString& rName,
                                            sead::hostio::Context* pContext) const
{
    if (!mAddr.isValid())
    {
        return;
    }

    sead::FixedSafeString<1024> info;
    info.format("メモリ[%s] サイズ：[%d]byte ( 0x%zx - 0x%zx )", rName.cstr(),
                mEndOffset - mAddr.getByteOffset(), u64(mAddr.getByteOffset()), mEndOffset);
}

/**
 * Checks whether the free range of another allocator ends before this one's starts.
 * @param rOther other allocator
 * @return whether the other allocator's first free block ends before this one's last
 */
bool TextureMemoryAllocator::isOverwrapperd(const TextureMemoryAllocator& rOther) const
{
    u64 start = 0;

    if (const MemoryBlock* last = mFreeList.back())
    {
        start = last->mAddr.getByteOffset();
    }

    u64 end = 0;

    if (const MemoryBlock* first = rOther.mFreeList.front())
    {
        end = first->getEndOffset();
    }

    return end < start;
}

/**
 * Creates one virtual storage per distinct storage class used by the texture formats.
 * @param pStorage receives the storages
 * @param addr start of the physical memory
 * @param size size of each storage
 * @param pHeap heap used for the storages
 */
void TextureMemoryAllocator::setupStorage(sead::Buffer<Storage>* pStorage, GPUMemVoidAddr addr,
                                          u64 size, sead::Heap* pHeap)
{
    struct StorageInfo
    {
        s32 mStorageClass;
        bool mIsNotCompressible;
    };

    sead::SafeArray<StorageInfo, 63> infos = {};
    s32 num = 0;

    const auto add = [&infos, &num](s32 storageClass) {
        for (s32 i = 0; i < num; ++i)
        {
            if (infos[i].mStorageClass == storageClass)
            {
                return;
            }
        }

        infos[num++].mStorageClass = storageClass;
    };

    for (s32 format = 1; format < 63; ++format)
    {
        {
            detail::Surface surface;
            surface.initialize(TextureType(1), TextureFormat(format), 1, TextureAttribute(0),
                               MultiSampleType(0));
            surface.initializeSize(0x80, 0x80, 1);
            surface.calcSizeAndAlignment();
            add(surface.mStorageClass);
        }

        {
            detail::Surface surface;
            surface.initialize(TextureType(1), TextureFormat(format), 1, TextureAttribute(2),
                               MultiSampleType(0));
            surface.initializeSize(0x80, 0x80, 1);
            surface.calcSizeAndAlignment();
            add(surface.mStorageClass);
        }

        if (!TextureFormatInfo::isCompressed(TextureFormat(format)))
        {
            detail::Surface surface;
            surface.initialize(TextureType(1), TextureFormat(format), 1, TextureAttribute(1),
                               MultiSampleType(0));
            surface.initializeSize(0x80, 0x80, 1);
            surface.calcSizeAndAlignment();
            add(surface.mStorageClass);
        }
    }

    if (num == 0)
    {
        return;
    }

    pStorage->tryAllocBuffer(num, pHeap);
    s32 i = 0;

    for (auto& storage : *pStorage)
    {
        const StorageInfo& info = infos[i];
        storage.mStorageClass = info.mStorageClass;
        storage.mBlock.setVirtual_(static_cast<s32>(size), pHeap,
                                   info.mIsNotCompressible ?
                                       MemoryAttribute::CompressibleMemory :
                                       MemoryAttribute(0x104),
                                   addr, info.mStorageClass);
        const GPUMemVoidAddr storageAddr(storage.mBlock, 0);
        ++i;
    }
}

}  // namespace agl::utl
