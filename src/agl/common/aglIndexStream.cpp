#include "common/aglIndexStream.h"

#include <nvn/nvn_FuncPtrInline.h>

#include "driver/aglNVNMgr.h"

namespace agl {

/**
 * Constructs an empty 16-bit triangle index stream.
 */
IndexStream::IndexStream()
    : mFormat(cIndexStreamFormat_u16), mPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLES), mCount(0),
      mStride(0)
{
}

/**
 * Releases the NVN buffer.
 */
IndexStream::~IndexStream()
{
    cleanUp_();
}

/**
 * Releases the NVN buffer and forgets the index memory.
 */
void IndexStream::cleanUp_()
{
    if (!mBuffer.isValid())
    {
        return;
    }

    nvnBufferFinalize(&mNvnBuffer);
    mBuffer.invalidate();
    mCount = 0;
    mStride = 0;
}

/**
 * Sets up the stream for index memory.
 * @param buffer index memory
 * @param format index format
 * @param count number of indices
 */
void IndexStream::setUpStream_(ConstGPUMemVoidAddr buffer, IndexStreamFormat format, u32 count)
{
    cleanUp_();

    mBuffer = buffer;
    mFormat = format;
    mCount = count;
    mStride = format == cIndexStreamFormat_u16 ? sizeof(u16) : sizeof(u32);
    NVNbufferBuilder builder;
    nvnBufferBuilderSetDevice(&builder, driver::NVNMgr::instance()->getNvnDevice());
    nvnBufferBuilderSetDefaults(&builder);
    nvnBufferBuilderSetStorage(&builder, mBuffer.getMemoryPool()->getDriverPool(),
                               mBuffer.getByteOffset(), mStride * mCount);
    nvnBufferInitialize(&mNvnBuffer, &builder);
    flushCPUCache(0, count);
}

/**
 * Flushes the CPU cache for a range of indices.
 * @param start index of the first index to flush
 * @param count number of indices to flush
 */
void IndexStream::flushCPUCache(u32 start, u32 count) const
{
    u64 size = mStride * count;
    ConstGPUMemVoidAddr(getBuffer(), mStride * start).flushCPUCache(size);
}

/**
 * Maps the index memory (no GPU cache invalidation is needed on NVN).
 * @param pDrawContext unused
 */
void IndexStream::invalidateGPUCache(DrawContext* pDrawContext) const
{
    mBuffer.getMappedBase();
}

/**
 * Replaces the index memory while keeping the format.
 * @param buffer index memory
 * @param count number of indices
 */
void IndexStream::setBufferPtr(ConstGPUMemVoidAddr buffer, u32 count)
{
    mBuffer = buffer;
    mCount = count;
    nvnBufferFinalize(&mNvnBuffer);
    NVNbufferBuilder builder;
    nvnBufferBuilderSetDevice(&builder, driver::NVNMgr::instance()->getNvnDevice());
    nvnBufferBuilderSetDefaults(&builder);
    nvnBufferBuilderSetStorage(&builder, mBuffer.getMemoryPool()->getDriverPool(),
                               mBuffer.getByteOffset(), mStride * mCount);
    nvnBufferInitialize(&mNvnBuffer, &builder);
}

}  // namespace agl
