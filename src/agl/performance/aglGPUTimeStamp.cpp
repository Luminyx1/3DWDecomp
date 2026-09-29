#include "performance/aglGPUTimeStamp.h"

#include <gfx/nin/seadGraphicsNvn.h>
#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglDrawContext.h"
#include "driver/aglNVNMgr.h"

namespace agl::perf {

/**
 * Writes a top-of-pipe timestamp into this stamp's counter memory.
 * @param pDrawContext draw context whose command buffer records the report
 */
void GPUTimeStamp::startTop(DrawContext* pDrawContext)
{
    nvnCommandBufferReportCounter(driver::getNvnCommandBuffer(pDrawContext),
                                  NVN_COUNTER_TYPE_TIMESTAMP_TOP, mAddress);
}

/**
 * Writes a bottom-of-pipe timestamp into this stamp's counter memory.
 * @param pDrawContext draw context whose command buffer records the report
 */
void GPUTimeStamp::startBottom(DrawContext* pDrawContext)
{
    nvnCommandBufferReportCounter(driver::getNvnCommandBuffer(pDrawContext),
                                  NVN_COUNTER_TYPE_TIMESTAMP, mAddress);
}

/**
 * Reads the recorded timestamp in nanoseconds.
 * @return the recorded GPU time in nanoseconds
 */
u64 GPUTimeStamp::get() const
{
    const auto* pData = static_cast<const NVNcounterData*>(mMemAddr.getPtr());
    u64 time = nvnDeviceGetTimestampInNanoseconds(
        sead::GraphicsNvn::instance()->getNvnDevice(), pData);
    mMemAddr.invalidateCPUCache(sizeof(NVNcounterData));
    return time;
}

/**
 * Constructs an empty timestamp array.
 */
GPUTimeStampArray::GPUTimeStampArray() = default;

/**
 * Frees the timestamp buffers and the NVN counter buffer.
 */
GPUTimeStampArray::~GPUTimeStampArray()
{
    for (auto& rEntry : mEntries)
    {
        rEntry.mBuffer.freeBuffer();
    }
    nvnBufferFinalize(&mNvnBuffer);
}

/**
 * Allocates counter memory and the timestamp buffers for every type.
 * @param num number of timestamps per type
 * @param pHeap heap to allocate from
 */
void GPUTimeStampArray::allocBuffer(s32 num, sead::Heap* pHeap)
{
    mMemBlock.allocBuffer(num * cType_Num, pHeap, sizeof(NVNcounterData), MemoryAttribute::_04);
    GPUMemAddr<NVNcounterData> addr(mMemBlock, 0);

    NVNbufferBuilder builder;
    nvnBufferBuilderSetDevice(&builder, driver::NVNMgr::instance()->getNvnDevice());
    nvnBufferBuilderSetDefaults(&builder);
    nvnBufferBuilderSetStorage(&builder, mMemBlock.getMemoryPool()->getDriverPool(),
                               mMemBlock.getByteOffset(), mMemBlock.getSize());
    nvnBufferInitialize(&mNvnBuffer, &builder);

    s32 index = 0;
    for (auto& rEntry : mEntries)
    {
        rEntry.mBuffer.tryAllocBuffer(num, pHeap);
        for (auto& rStamp : rEntry.mBuffer)
        {
            u64 offset = index * sizeof(NVNcounterData);
            GPUMemAddr<NVNcounterData> memAddr(mMemBlock, offset);
            GPUTimeStamp stamp(memAddr);
            stamp.setAddress(nvnBufferGetAddress(&mNvnBuffer) + offset);
            rStamp = stamp;
            index++;
        }
    }
}

/**
 * Returns the next timestamp of a type, cycling through its buffer.
 * @param type timestamp type
 * @return the next timestamp
 */
GPUTimeStamp* GPUTimeStampArray::getNext(Type type)
{
    s32 t = type;
    u32 index = mEntries[t].mIndex.increment();
    return &mEntries[t].mBuffer[index % mEntries[t].mBuffer.size()];
}

}  // namespace agl::perf
