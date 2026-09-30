#include "common/aglGPUMemAddr.h"

#include <nvn/nvn_FuncPtrInline.h>

namespace agl {

/**
 * Constructs an address pointing at a byte offset inside a GPU memory block.
 * @param rMemBlock memory block the address points into
 * @param offset byte offset from the start of the block
 */
GPUMemAddrBase::GPUMemAddrBase(const GPUMemBlockBase& rMemBlock, u64 offset)
    : mMemoryPool(nullptr), mAlignmentAddr(offset),
      mMemoryBlock(const_cast<GPUMemBlockBase*>(&rMemBlock))
{
    mMemoryPool = rMemBlock.mpMemoryPool;
    mAlignmentAddr = rMemBlock.getByteOffset() + offset;
    verify_();
}

/**
 * Checks the address against its memory block.
 * @return 0 if valid, 1 if the offset is out of range, 2 if the memory pool does not match
 */
u32 GPUMemAddrBase::verify_() const
{
    const GPUMemBlockBase* pBlock = mMemoryBlock;
    if (pBlock)
    {
        if (pBlock->mMemoryBufferSize != 0)
        {
            if (pBlock->getByteOffset() + pBlock->mMemoryBufferSize <
                static_cast<u32>(mAlignmentAddr))
            {
                return 1;
            }

            if (mMemoryBlock->mpMemoryPool != mMemoryPool)
            {
                return 2;
            }
        }
        else if (mMemoryPool)
        {
            return 2;
        }
    }

    return 0;
}

/**
 * Deletes the memory block this address points into.
 */
void GPUMemAddrBase::deleteGPUMemBlock() const
{
    if (mMemoryBlock)
    {
        delete mMemoryBlock;
    }
}

/**
 * Resets the address to an invalid state.
 */
void GPUMemAddrBase::invalidate()
{
    mMemoryPool = nullptr;
    mAlignmentAddr = 0;
    mMemoryBlock = nullptr;
}

/**
 * Gets the byte offset of the address inside its memory pool.
 * @return byte offset inside the memory pool
 */
u32 GPUMemAddrBase::getAlignmentAddress() const
{
    return mAlignmentAddr;
}

/**
 * Sets the byte offset from a CPU pointer into the mapped memory pool.
 * @param pPtr CPU pointer inside the mapped memory pool
 */
void GPUMemAddrBase::setByteOffsetByPtr(void* pPtr)
{
    mAlignmentAddr = reinterpret_cast<uintptr_t>(pPtr) -
                     reinterpret_cast<uintptr_t>(
                         mMemoryPool ? nvnMemoryPoolMap(mMemoryPool->getDriverPool()) : nullptr);
}

/**
 * Rounds the byte offset up to a power of two alignment.
 * @param alignment alignment to round up to
 */
void GPUMemAddrBase::roundUp(int alignment)
{
    mAlignmentAddr = (mAlignmentAddr + alignment - 1) & -alignment;
}

/**
 * Flushes the CPU cache for a range starting at the address, if the pool is CPU cached.
 * @param size size of the range in bytes
 */
void GPUMemAddrBase::flushCPUCache(u64 size) const
{
    if (mMemoryPool &&
        (nvnMemoryPoolGetFlags(mMemoryPool->getDriverPool()) & NVN_MEMORY_POOL_FLAGS_CPU_CACHED))
    {
        nvnMemoryPoolFlushMappedRange(mMemoryPool->getDriverPool(),
                                      static_cast<u32>(mAlignmentAddr), size);
    }
}

/**
 * Invalidates the CPU cache for a range starting at the address, if the pool is CPU cached.
 * @param size size of the range in bytes
 */
void GPUMemAddrBase::invalidateCPUCache(u64 size) const
{
    if (mMemoryPool &&
        (nvnMemoryPoolGetFlags(mMemoryPool->getDriverPool()) & NVN_MEMORY_POOL_FLAGS_CPU_CACHED))
    {
        nvnMemoryPoolInvalidateMappedRange(mMemoryPool->getDriverPool(),
                                           static_cast<u32>(mAlignmentAddr), size);
    }
}

}  // namespace agl
