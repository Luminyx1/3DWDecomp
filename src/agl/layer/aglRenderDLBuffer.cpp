#include "layer/aglRenderDLBuffer.h"

#include <cstring>
#include <math/seadMathCalcCommon.h>
#include <mc/seadCoreInfo.h>
#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglDrawContext.h"
#include "detail/aglMemoryPoolHeap.h"

namespace agl::lyr {

/**
 * Constructs an empty render display list buffer.
 */
RenderDLBuffer::RenderDLBuffer()
    : mControlMemory(nullptr), mRenderDLNum(0), mMultiBufferNum(0), mCurrentBuffer(0), _112(0),
      mMaxUsedSize(0), mMaxControlMemoryUsed(0), _128(0)
{
}

/**
 * Frees all buffers owned by the render display list buffer.
 */
RenderDLBuffer::~RenderDLBuffer()
{
    for (auto& rCore : mCoreBuffer)
    {
        rCore.mAddress.freeBuffer();
    }
    mRenderDL.freeBuffer();
    if (mControlMemory)
    {
        delete static_cast<u8*>(mControlMemory);
    }
}

/**
 * Allocates the command and control memory for every core and the render display lists.
 * @param bufferSize command memory size per core and buffer
 * @param controlMemorySize control memory size per core
 * @param renderDLNum number of render display lists
 * @param multiBufferNum number of command memory buffers per core
 * @param pHeap heap to allocate from
 * @param pDebugHeap unused
 */
void RenderDLBuffer::initialize(u64 bufferSize, u64 controlMemorySize, s32 renderDLNum,
                                s32 multiBufferNum, sead::Heap* pHeap, sead::Heap* pDebugHeap)
{
    const u64 alignedSize = (bufferSize + 3) & ~u64(3);
    mMultiBufferNum = multiBufferNum;
    const u64 totalSize = alignedSize * mMultiBufferNum * cCoreNum;

    if (controlMemorySize * cCoreNum != 0)
    {
        mControlMemory = new (pHeap, 8) u8[controlMemorySize * cCoreNum];
    }

    if (totalSize != 0)
    {
        mGPUBuffer.allocBuffer(totalSize / sizeof(u32), pHeap, 4, MemoryAttribute::_00);
        GPUMemAddr<u32> addr(mGPUBuffer, 0);
        u32* pBuffer = reinterpret_cast<u32*>(
            static_cast<u8*>(nvnMemoryPoolMap(mGPUBuffer.getMemoryPool()->getDriverPool())) +
            mGPUBuffer.getByteOffset());
        s32 num = mGPUBuffer.getSize() / sizeof(u32);
        for (s32 i = 0; i < num; i++)
        {
            pBuffer[i] = 0xbeef2929;
        }

        u64 offset[2] = {0, 0};
        s32 blockIdx = 0;
        for (s32 core = 0; core < cCoreNum; core++)
        {
            CoreBuffer& rCore = mCoreBuffer[core];
            rCore.mUsedSize = 0;
            rCore.mBufferSize = alignedSize;
            rCore.mAddress.tryAllocBuffer(mMultiBufferNum, pHeap);
            rCore.mControlMemory = static_cast<u8*>(mControlMemory) + controlMemorySize * core;
            rCore.mControlMemoryUsed = 0;
            rCore.mControlMemorySize = controlMemorySize;
            for (auto& rAddress : rCore.mAddress)
            {
                u64 blockOffset = offset[blockIdx];
                rAddress = GPUMemAddr<u8>(blockIdx == 0 ? mGPUBuffer : mGPUBufferSub,
                                          sizeof(u32) * s32(blockOffset / sizeof(u32)));
                offset[blockIdx] = blockOffset + alignedSize;
                if (blockIdx == 1)
                {
                    blockIdx = 0;
                }
                else
                {
                    blockIdx = s32(mGPUBufferSub.getSize() / sizeof(u32)) > 0;
                }
            }
        }
    }

    mRenderDL.tryAllocBuffer(renderDLNum, pHeap);
}

/**
 * Resets the used sizes of every core and optionally swaps to the next buffer.
 * @param swapBuffer whether to advance to the next command memory buffer
 */
void RenderDLBuffer::clear(bool swapBuffer)
{
    for (auto& rCore : mCoreBuffer)
    {
        if (rCore.mUsedSize > mMaxUsedSize)
        {
            mMaxUsedSize = rCore.mUsedSize;
        }
        rCore.mUsedSize = 0;
        if (rCore.mControlMemoryUsed > mMaxControlMemoryUsed)
        {
            mMaxControlMemoryUsed = rCore.mControlMemoryUsed;
        }
        rCore.mControlMemoryUsed = 0;
    }
    mRenderDLNum = 0;

    if (swapBuffer)
    {
        mCurrentBuffer = (mCurrentBuffer + 1) % mMultiBufferNum;
    }
}

/**
 * Clears the current command memory buffer of every core.
 */
void RenderDLBuffer::invalidateCurrentBuffer() const
{
    for (const auto& rCore : mCoreBuffer)
    {
        GPUMemAddr<u8> addr = rCore.mAddress[mCurrentBuffer];
        std::memset(addr.getPtr(), 0, rCore.mBufferSize);
        addr.flushCPUCache(rCore.mBufferSize);
    }
}

/**
 * Begins recording a render display list on the current core.
 * @param pDrawContext draw context to record into
 * @param rName name of the display list
 * @param priority sort priority of the display list
 * @return index of the render display list, or -1 if recording could not begin
 */
s32 RenderDLBuffer::begin(DrawContext* pDrawContext, const sead::SafeString& rName, s32 priority)
{
    s32 core = sead::CoreInfo::getCurrentCoreId();
    s32 index = mRenderDLNum.increment();
    CoreBuffer& rCore = mCoreBuffer[core];
    RenderDL& rDL = mRenderDL[index];
    GPUMemAddr<u8> addr(rCore.mAddress[mCurrentBuffer], rCore.mUsedSize);
    u32 size = sead::Mathu::min(rCore.mBufferSize - rCore.mUsedSize, 0x400000u);

    rDL.setControlMemory(rCore.mControlMemory + rCore.mControlMemoryUsed,
                         s32(rCore.mControlMemorySize - rCore.mControlMemoryUsed));
    rDL.setBuffer(addr, size);
    rDL.setName(rName.cstr());
    rDL.mCoreId = core;
    rDL.mPriority = priority;
    rDL.mLayer = nullptr;
    rDL.mBeginTime.setNow();

    pDrawContext->setCommandBuffer(&rDL);
    if (!rDL.beginDisplayList())
    {
        return -1;
    }
    return index;
}

/**
 * Ends recording a render display list on the current core.
 * @param pDrawContext draw context that was recorded into
 * @param index index returned by begin
 * @return the recorded render display list
 */
RenderDL* RenderDLBuffer::end(DrawContext* pDrawContext, s32 index)
{
    s32 core = sead::CoreInfo::getCurrentCoreId();
    RenderDL& rDL = mRenderDL[index];
    u32 size = rDL.endDisplayList();
    CoreBuffer& rCore = mCoreBuffer[core];
    rCore.mUsedSize = (rCore.mUsedSize + size + 3) & ~3u;
    rCore.mControlMemoryUsed =
        (rCore.mControlMemoryUsed + rDL.getControlMemoryUsed() + 7) & ~7u;
    rDL.mEndTime.setNow();
    return &rDL;
}

}  // namespace agl::lyr
