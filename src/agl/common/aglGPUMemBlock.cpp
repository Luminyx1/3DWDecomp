#include "common/aglGPUMemBlock.h"

#include <heap/seadHeapMgr.h>
#include <nn/gfx/gfx_Interoperation-api.nvn.8.h>
#include <nn/gfx/gfx_MemoryPool.h>

#include "common/aglGPUMemAddr.h"
#include "detail/aglGPUMemBlockMgr.h"
#include "detail/aglMemoryPoolHeap.h"

namespace agl {

/**
 * Constructs an empty memory block.
 */
GPUMemBlockBase::GPUMemBlockBase() : mFlags(0)
{
    clear();
}

/**
 * Resets the block to an empty state without freeing anything.
 */
void GPUMemBlockBase::clear()
{
    mMemoryBuffer = nullptr;
    mMemoryBufferSize = 0;
    mpMemoryPool = nullptr;
    mMemoryPoolHeap = nullptr;
    mpTail = nullptr;
    mFlags &= ~1;
    mAttribute = 0;
}

/**
 * Frees the buffer and destroys a virtual memory pool.
 */
GPUMemBlockBase::~GPUMemBlockBase()
{
    freeBuffer();

    if (mFlags & 2)
    {
        mpMemoryPool->finalize();
        delete mpMemoryPool;
        mpMemoryPool = nullptr;
    }
}

/**
 * Frees the buffer and clears the block.
 */
void GPUMemBlockBase::freeBuffer()
{
    if (mMemoryBuffer == nullptr)
    {
        return;
    }

    if (mMemoryPoolHeap != nullptr)
    {
        mMemoryPoolHeap->freeToHeap(this);
    }
    else
    {
        mpMemoryPool->finalize();

        if (!(mFlags & 3))
        {
            delete static_cast<u8*>(mMemoryBuffer);
        }
    }

    clear();
}

/**
 * Frees the buffer and clears the block.
 */
void GPUMemBlockBase::free()
{
    freeBuffer();
}

/**
 * Allocates the buffer from the GPU memory block manager.
 * @param size size in bytes
 * @param pHeap heap to allocate from, or nullptr for the current heap
 * @param alignment alignment in bytes
 * @param attribute memory attribute
 */
void GPUMemBlockBase::allocBuffer_(u64 size, sead::Heap* pHeap, s32 alignment,
                                   MemoryAttribute attribute)
{
    if (pHeap == nullptr)
    {
        pHeap = sead::HeapMgr::instance()->getCurrentHeap();
    }

    tryAllocBuffer_(size, pHeap, alignment, attribute);
    mAttribute = static_cast<u16>(attribute);
}

/**
 * Tries to allocate the buffer from the GPU memory block manager.
 * @param size size in bytes
 * @param pHeap heap to allocate from
 * @param alignment alignment in bytes
 * @param attribute memory attribute
 * @return whether the allocation succeeded (true for a zero size)
 */
bool GPUMemBlockBase::tryAllocBuffer_(u64 size, sead::Heap* pHeap, s32 alignment,
                                      MemoryAttribute attribute)
{
    clear();

    if (size == 0)
    {
        return true;
    }

    if (!detail::GPUMemBlockMgr::instance()->tryAllocMemory(this, pHeap, size, alignment,
                                                            attribute))
    {
        return false;
    }

    mAttribute = static_cast<u16>(attribute);
    return true;
}

/**
 * Uses caller-provided memory as the buffer and creates a memory pool for it.
 * @param size size in bytes
 * @param pBuffer buffer memory
 * @param pPoolStorage storage for the memory pool object
 * @param attribute memory attribute
 */
void GPUMemBlockBase::setBuffer_(u64 size, void* pBuffer, void* pPoolStorage,
                                 MemoryAttribute attribute)
{
    clear();

    if (size == 0)
    {
        return;
    }

    mMemoryBuffer = pBuffer;
    mMemoryBufferSize = size;
    void* pStorage =
        reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(pPoolStorage) + 7) & ~uintptr_t(7));
    mpMemoryPool = new (pStorage) detail::MemoryPool();
    mpMemoryPool->initialize(mMemoryBuffer, size, detail::MemoryPoolType::convert(attribute));
    mFlags |= 1;
}

/**
 * Creates a virtual memory pool mapped onto the physical pool of another address.
 * @param size size in bytes
 * @param pHeap heap to allocate the memory pool object from
 * @param attribute memory attribute
 * @param physicalAddr address in the physical memory pool
 * @param storageClass storage class of the virtual pool
 */
void GPUMemBlockBase::setVirtual_(u64 size, sead::Heap* pHeap, MemoryAttribute attribute,
                                  GPUMemVoidAddr physicalAddr, s32 storageClass)
{
    if (size == 0)
    {
        return;
    }

    mMemoryBuffer = physicalAddr.getMemoryBlock()->mMemoryBuffer;
    mMemoryBufferSize = size;
    mpMemoryPool = new (pHeap, 8) detail::MemoryPool();
    mpMemoryPool->initialize(mMemoryBuffer, size, detail::MemoryPoolType::convert(attribute),
                             *physicalAddr.getMemoryBlock()->mpMemoryPool, storageClass);
    mFlags |= 2;
}

/**
 * Initializes an nn::gfx memory pool that wraps this block's memory pool.
 * @param pMemoryPool nn::gfx memory pool to initialize
 */
void GPUMemBlockBase::initializeGfxMemoryPool(nn::gfx::MemoryPool* pMemoryPool) const
{
    if (pMemoryPool->ToData()->state)
    {
        pMemoryPool->ToData()->state = nn::gfx::MemoryPool::DataType::State_NotInitialized;
    }

    nn::gfx::TInteroperation<nn::gfx::ApiVariationNvn8>::ConvertToGfxMemoryPool(
        pMemoryPool, mpMemoryPool->getDriverPool(),
        (mMemoryPoolHeap != nullptr) ? mMemoryPoolHeap->getBuffer() : mMemoryBuffer);
}

/**
 * Appends a block to the end of the list starting at this block.
 * @param pBlock block to append
 * @return number of blocks in the list including the appended one
 */
s32 GPUMemBlockBase::addList(GPUMemBlockBase* pBlock)
{
    s32 count = 1;
    GPUMemBlockBase* pLast = this;

    while (pLast->mpTail != nullptr)
    {
        pLast = pLast->mpTail;
        count++;
    }

    pLast->mpTail = pBlock;
    return count + 1;
}

/**
 * Sets the buffer and the memory pool it belongs to.
 * @param pBuffer buffer memory
 * @param size size in bytes
 * @param pMemoryPool memory pool the buffer belongs to
 */
void GPUMemBlockBase::setMemoryPool(void* pBuffer, u64 size, detail::MemoryPool* pMemoryPool)
{
    mMemoryBuffer = pBuffer;
    mMemoryBufferSize = size;
    mpMemoryPool = pMemoryPool;
}

/**
 * Sets the buffer and the memory pool heap it was allocated from.
 * @param pBuffer buffer memory
 * @param size size in bytes
 * @param pMemoryPoolHeap memory pool heap the buffer was allocated from
 */
void GPUMemBlockBase::setMemoryPoolHeap(void* pBuffer, u64 size,
                                        detail::MemoryPoolHeap* pMemoryPoolHeap)
{
    mMemoryBuffer = pBuffer;
    mMemoryBufferSize = size;
    mpMemoryPool = pMemoryPoolHeap->getMemoryPool();
    mMemoryPoolHeap = pMemoryPoolHeap;
    pMemoryPoolHeap->pushBack(this);
}

/**
 * Gets the offset of the buffer inside its memory pool heap.
 * @return byte offset, or 0 if the block was not allocated from a memory pool heap
 */
u64 GPUMemBlockBase::getByteOffset() const
{
    if (mMemoryPoolHeap == nullptr)
    {
        return 0;
    }

    return reinterpret_cast<uintptr_t>(mMemoryBuffer) -
           reinterpret_cast<uintptr_t>(mMemoryPoolHeap->getBuffer());
}

/**
 * Gets the memory pool type without the GPU access bits.
 * @return memory pool type
 */
u32 GPUMemBlockBase::getMemoryPoolType() const
{
    const detail::MemoryPoolType& rType =
        (mpMemoryPool != nullptr) ? mpMemoryPool->getMemoryType() : detail::MemoryPoolType::cInvalidPoolType;
    return rType.getDirect() & ~detail::cGPUAccessMask;
}

}  // namespace agl
