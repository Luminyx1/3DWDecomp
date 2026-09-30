#include "common/aglVertexBuffer.h"

#include <nvn/nvn_FuncPtrInline.h>

#include "driver/aglNVNMgr.h"

namespace agl {

/**
 * Constructs an empty vertex buffer.
 */
VertexBuffer::VertexBuffer() : mStride(0), mVertexNum(0), mBufferSize(0) {}

/**
 * Releases the NVN buffer.
 */
VertexBuffer::~VertexBuffer()
{
    cleanUp_();
}

/**
 * Releases the NVN buffer, disables all streams and forgets the vertex memory.
 */
void VertexBuffer::cleanUp_()
{
    if (!mBuffer.isValid())
    {
        return;
    }

    nvnBufferFinalize(&mNvnBuffer);
    for (s32 i = 0; i < cVertexStreamMax; i++)
    {
        mStreams[i].mEnable = false;
    }

    mBuffer.invalidate();
    mStride = 0;
    mVertexNum = 0;
    mBufferSize = 0;
}

/**
 * Sets up the buffer for vertex memory.
 * @param buffer vertex memory
 * @param stride size of one vertex in bytes
 * @param size size of the vertex memory in bytes
 */
void VertexBuffer::setUpBuffer(ConstGPUMemVoidAddr buffer, u64 stride, u64 size)
{
    cleanUp_();

    mBuffer = buffer;
    mStride = stride;
    if (mStride != 0)
    {
        mVertexNum = size / stride;
        size = mVertexNum * stride;
    }
    else
    {
        mVertexNum = 1;
    }

    mBufferSize = size;

    NVNbufferBuilder builder;
    nvnBufferBuilderSetDevice(&builder, driver::NVNMgr::instance()->getNvnDevice());
    nvnBufferBuilderSetDefaults(&builder);
    nvnBufferBuilderSetStorage(&builder, mBuffer.getMemoryPool()->getDriverPool(),
                               mBuffer.getByteOffset(), mBufferSize);
    nvnBufferInitialize(&mNvnBuffer, &builder);

    flushCPUCache(0, mBufferSize);
}

/**
 * Flushes the CPU cache for a range of the vertex memory.
 * @param offset byte offset of the range
 * @param size size of the range in bytes
 */
void VertexBuffer::flushCPUCache(u32 offset, u64 size) const
{
    ConstGPUMemVoidAddr(getBuffer(), offset).flushCPUCache(size);
}

/**
 * Maps the vertex memory (no GPU cache invalidation is needed on NVN).
 * @param pDrawContext unused
 */
void VertexBuffer::invalidateGPUCache(DrawContext* pDrawContext) const
{
    mBuffer.getMappedBase();
}

/**
 * Enables a vertex stream.
 * @param index stream index
 * @param format vertex format of the stream
 * @param offset byte offset of the stream inside a vertex
 * @param divisor whether the stream advances per instance
 */
void VertexBuffer::setUpStream(s32 index, VertexStreamFormat format, u64 offset, bool divisor)
{
    Stream& rStream = mStreams[index];
    rStream.mFormat = format;
    rStream.mOffset = offset;
    rStream.mEnable = true;
    rStream.mDivisor = divisor;
}

/**
 * Replaces the vertex memory while keeping the streams.
 * @param buffer vertex memory
 * @param size size of the vertex memory in bytes
 */
void VertexBuffer::setBufferPtr(ConstGPUMemVoidAddr buffer, u32 size)
{
    mBuffer = buffer;
    mBufferSize = size;
    nvnBufferFinalize(&mNvnBuffer);

    NVNbufferBuilder builder;
    nvnBufferBuilderSetDevice(&builder, driver::NVNMgr::instance()->getNvnDevice());
    nvnBufferBuilderSetDefaults(&builder);
    nvnBufferBuilderSetStorage(&builder, mBuffer.getMemoryPool()->getDriverPool(),
                               mBuffer.getByteOffset(), mBufferSize);
    nvnBufferInitialize(&mNvnBuffer, &builder);
}

}  // namespace agl
