#include "stream/seadRamStream.h"

#include <cstring>

namespace sead
{
/**
 * Creates a stream source over a memory buffer.
 * @param pBuffer memory buffer
 * @param bufferSize size of the buffer in bytes
 */
RamStreamSrc::RamStreamSrc(void* pBuffer, u32 bufferSize)
    : mBuffer(static_cast<u8*>(pBuffer)), mBufferSize(bufferSize)
{
}

/**
 * Destroys the stream source.
 */
RamStreamSrc::~RamStreamSrc() = default;

/**
 * Reads bytes at the current position, clamped to the end of the buffer.
 * @param pData destination buffer
 * @param size number of bytes requested
 * @return number of bytes read
 */
u32 RamStreamSrc::read(void* pData, u32 size)
{
    if (mCurrentPos + size > mBufferSize)
    {
        size = mBufferSize - mCurrentPos;
    }

    std::memcpy(pData, mBuffer + mCurrentPos, size);
    mCurrentPos += size;
    return size;
}

/**
 * Writes bytes at the current position, clamped to the end of the buffer.
 * @param pData source data
 * @param size number of bytes requested
 * @return number of bytes written
 */
u32 RamStreamSrc::write(const void* pData, u32 size)
{
    if (mCurrentPos + size > mBufferSize)
    {
        size = mBufferSize - mCurrentPos;
    }

    std::memcpy(mBuffer + mCurrentPos, pData, size);
    mCurrentPos += size;
    return size;
}

/**
 * Moves the current position, clamped to the buffer bounds.
 * @param offset relative offset in bytes
 * @return offset actually moved
 */
u32 RamStreamSrc::skip(s32 offset)
{
    if (offset > 0 && mCurrentPos + offset > mBufferSize)
    {
        offset = mBufferSize - mCurrentPos;
    }

    if (offset < 0 && mCurrentPos < static_cast<u32>(-offset))
    {
        offset = -mCurrentPos;
    }

    mCurrentPos += offset;
    return offset;
}

/**
 * Creates a write stream over a memory buffer using a basic format.
 * @param pBuffer memory buffer
 * @param bufferSize size of the buffer in bytes
 * @param mode basic stream format
 */
RamWriteStream::RamWriteStream(void* pBuffer, u32 bufferSize, Stream::Modes mode)
    : mSrc(pBuffer, bufferSize)
{
    setSrc(&mSrc);
    setMode(mode);
}

/**
 * Creates a write stream over a memory buffer using a user format.
 * @param pBuffer memory buffer
 * @param bufferSize size of the buffer in bytes
 * @param pFormat stream format
 */
RamWriteStream::RamWriteStream(void* pBuffer, u32 bufferSize, StreamFormat* pFormat)
    : mSrc(pBuffer, bufferSize)
{
    setSrc(&mSrc);
    setUserFormat(pFormat);
}

/**
 * Flushes the stream and detaches the source.
 */
RamWriteStream::~RamWriteStream()
{
    flush();
    setSrc(nullptr);
}

/**
 * Creates a read stream over a memory buffer using a basic format.
 * @param pBuffer memory buffer
 * @param bufferSize size of the buffer in bytes
 * @param mode basic stream format
 */
RamReadStream::RamReadStream(const void* pBuffer, u32 bufferSize, Stream::Modes mode)
    : mSrc(const_cast<void*>(pBuffer), bufferSize)
{
    setSrc(&mSrc);
    setMode(mode);
}

/**
 * Creates a read stream over a memory buffer using a user format.
 * @param pBuffer memory buffer
 * @param bufferSize size of the buffer in bytes
 * @param pFormat stream format
 */
RamReadStream::RamReadStream(const void* pBuffer, u32 bufferSize, StreamFormat* pFormat)
    : mSrc(const_cast<void*>(pBuffer), bufferSize)
{
    setSrc(&mSrc);
    setUserFormat(pFormat);
}

/**
 * Detaches the source.
 */
RamReadStream::~RamReadStream()
{
    setSrc(nullptr);
}
}  // namespace sead
