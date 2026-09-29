#include "detail/aglDynamicUniformBlock.h"

#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglGPUMemAddr.h"
#include "driver/aglNVNMgr.h"

namespace agl::detail {

SEAD_SINGLETON_DISPOSER_IMPL(DynamicUniformBlock)

/**
 * Constructs the dynamic uniform block without GPU memory.
 */
DynamicUniformBlock::DynamicUniformBlock() = default;

/**
 * Finalizes the NVN buffer.
 */
DynamicUniformBlock::~DynamicUniformBlock()
{
    nvnBufferFinalize(&mBuffer);
}

/**
 * Allocates the GPU memory and creates the NVN buffer over it.
 * @param size requested size (unused, a fixed 64 KiB block is allocated)
 * @param pHeap heap to allocate the GPU memory from
 */
void DynamicUniformBlock::initialize(u64 size, sead::Heap* pHeap)
{
    mMemBlock.allocBuffer_(0x10000, pHeap, 0x100, MemoryAttribute(2));
    NVNmemoryPool* pPool;
    u32 offset;
    {
        GPUMemAddrBase addr(mMemBlock, 0);
        pPool = addr.getMemoryPool()->getDriverPool();
        offset = addr.getByteOffset();
    }

    NVNbufferBuilder builder;
    nvnBufferBuilderSetDevice(&builder, driver::NVNMgr::instance()->getNvnDevice());
    nvnBufferBuilderSetDefaults(&builder);
    nvnBufferBuilderSetStorage(&builder, pPool, offset, mMemBlock.getSize());
    nvnBufferInitialize(&mBuffer, &builder);
    mAddress = nvnBufferGetAddress(&mBuffer);
}

/**
 * Generates the host IO message (empty in release builds).
 * @param pContext host IO context
 */
void DynamicUniformBlock::genMessage(sead::hostio::Context* pContext) {}

/**
 * Handles a host IO property event (empty in release builds).
 * @param pEvent property event
 */
void DynamicUniformBlock::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

}  // namespace agl::detail
