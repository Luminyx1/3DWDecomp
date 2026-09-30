#include "stream/seadBufferStream.h"

#include "math/seadMathCalcCommon.h"

namespace sead
{
/**
 * Constructs a read buffer that fills itself from another source.
 * @param pSrc the source to read from
 * @param pBuffer the buffer
 * @param bufferSize the buffer's size in bytes
 */
BufferReadStreamSrc::BufferReadStreamSrc(StreamSrc* pSrc, void* pBuffer, u32 bufferSize)
    : mSrc(pSrc), mBuffer(pBuffer), mBufferSize(bufferSize)
{
}

/**
 * Destroys the buffered read source.
 */
BufferReadStreamSrc::~BufferReadStreamSrc() = default;

// NOTE: cannot take negative `offset`, but expects `mSrc->skip(X)` to work with negatives
/**
 * Reads data through the buffer, refilling it from the source as needed.
 * @param data the destination for the read bytes
 * @param size the number of bytes to read
 * @return the number of bytes read
 */
u32 BufferReadStreamSrc::read(void* data, u32 size)
{
    u32 totalBytesRead = 0;
    while (true)
    {
        if (mCurrentPos < mCurrentSize)
        {
            u32 readSize = sead::Mathu::clampMax(size - totalBytesRead, mCurrentSize - mCurrentPos);

            memcpy((u8*)data + totalBytesRead, (u8*)mBuffer + mCurrentPos, readSize);
            totalBytesRead += readSize;
            mCurrentPos += readSize;
        }

        if (size <= totalBytesRead)
        {
            break;
        }

        mCurrentSize = mSrc->read(mBuffer, mBufferSize);
        mCurrentPos = 0;

        if (mCurrentSize == 0)
        {
            break;
        }
    }

    return totalBytesRead;
}

/**
 * Does nothing, since this source is read-only.
 * @param pData unused
 * @param size unused
 * @return always 0
 */
u32 BufferReadStreamSrc::write([[maybe_unused]] const void* pData, [[maybe_unused]] u32 size)
{
    return 0;
}

/**
 * Skips bytes, first within the buffer and then in the underlying source.
 * @param offset the number of bytes to skip
 * @return the number of bytes skipped
 */
u32 BufferReadStreamSrc::skip(s32 offset)
{
    s32 remainingBytes = mCurrentSize - mCurrentPos;

    if (remainingBytes >= offset)
    {
        mCurrentPos += offset;
        return offset;
    }

    mCurrentSize = 0;
    mCurrentPos = 0;
    return mSrc->skip(offset - remainingBytes) + remainingBytes;
}

/**
 * Rewinds the underlying source and discards the buffered data.
 */
void BufferReadStreamSrc::rewind()
{
    mSrc->rewind();
    mCurrentSize = 0;
    mCurrentPos = 0;
}

/**
 * Checks whether both the source and the buffer are exhausted.
 * @return whether the end of the stream has been reached
 */
bool BufferReadStreamSrc::isEOF()
{
    return mSrc->isEOF() && mCurrentPos >= mCurrentSize;
}

/**
 * Wraps a read stream in a read buffer.
 * @param pStream the stream to read through
 * @param pBuffer the buffer
 * @param bufferSize the buffer's size in bytes
 */
BufferReadStream::BufferReadStream(ReadStream* pStream, const void* pBuffer, u32 bufferSize)
    : mSrc(pStream->getSrc(), const_cast<void*>(pBuffer), bufferSize)
{
    setSrc(&mSrc);
    setUserFormat(pStream->getUserFormat());
    setBinaryEndian(pStream->getBinaryEndian());
}

/**
 * Detaches the buffer from the stream.
 */
BufferReadStream::~BufferReadStream()
{
    setSrc(nullptr);
}

/**
 * Constructs a write buffer that flushes to another source.
 * @param pSrc the source the buffer is flushed to
 * @param pBuffer the buffer
 * @param bufferSize the buffer's size in bytes
 */
BufferWriteStreamSrc::BufferWriteStreamSrc(StreamSrc* pSrc, void* pBuffer, u32 bufferSize)
    : mSrc(pSrc), mBuffer(pBuffer), mBufferSize(bufferSize)
{
}

/**
 * Destroys the buffered write source.
 */
BufferWriteStreamSrc::~BufferWriteStreamSrc() = default;

/**
 * Does nothing, since this source is write-only.
 * @param pData unused
 * @param size unused
 * @return always 0
 */
u32 BufferWriteStreamSrc::read([[maybe_unused]] void* pData, [[maybe_unused]] u32 size)
{
    return 0;
}

/**
 * Writes data through the buffer, flushing whenever it fills up.
 * @param pData the data to write
 * @param size the number of bytes to write
 * @return the number of bytes written
 */
u32 BufferWriteStreamSrc::write(const void* pData, u32 size)
{
    u32 totalBytesWritten = 0;
    do
    {
        if (mCurrentPos >= mBufferSize)
        {
            continue;
        }

        u32 writeSize = sead::Mathu::min(mBufferSize - mCurrentPos, size - totalBytesWritten);

        memcpy((u8*)mBuffer + mCurrentPos, (u8*)pData + totalBytesWritten, writeSize);
        totalBytesWritten += writeSize;
        mCurrentPos += writeSize;
    } while (totalBytesWritten < size && flush());

    return totalBytesWritten;
}

/**
 * Does nothing, since skipping is not supported.
 * @param offset unused
 * @return always 0
 */
u32 BufferWriteStreamSrc::skip([[maybe_unused]] s32 offset)
{
    return 0;
}

/**
 * Flushes the buffer, then rewinds the underlying source.
 */
void BufferWriteStreamSrc::rewind()
{
    flush();
    mSrc->rewind();
}

/**
 * Writes the buffered data to the underlying source and empties the buffer.
 * @return whether the whole buffer was written
 */
bool BufferWriteStreamSrc::flush()
{
    if (mCurrentPos == 0)
    {
        return true;
    }

    bool success = mSrc->write(mBuffer, mCurrentPos) >= mCurrentPos;
    mCurrentPos = 0;
    return success;
}

/**
 * Wraps a write stream in a write buffer.
 * @param pStream the stream to write through
 * @param pBuffer the buffer
 * @param bufferSize the buffer's size in bytes
 */
BufferWriteStream::BufferWriteStream(WriteStream* pStream, void* pBuffer, u32 bufferSize)
    : mSrc(pStream->getSrc(), pBuffer, bufferSize)
{
    setSrc(&mSrc);
    setUserFormat(pStream->getUserFormat());
    setBinaryEndian(pStream->getBinaryEndian());
}

/**
 * Flushes the buffer and detaches it from the stream.
 */
BufferWriteStream::~BufferWriteStream()
{
    flush();
    setSrc(nullptr);
}

/**
 * Constructs a buffered text source that never splits a UTF-8 character between two flushes.
 * @param pSrc the source the buffer is flushed to
 * @param pBuffer the buffer
 * @param bufferSize the buffer's size in bytes
 */
BufferMultiByteTextWriteStreamSrc::BufferMultiByteTextWriteStreamSrc(StreamSrc* pSrc, void* pBuffer,
                                                                     u32 bufferSize)
    : BufferWriteStreamSrc(pSrc, pBuffer, bufferSize)
{
}

/**
 * Writes text through the buffer, flushing whenever it fills up. When a flush is due, a UTF-8
 * character that would be cut in two is held back for the next buffer instead.
 * @param pData the text to write
 * @param size the number of bytes to write
 * @return the number of bytes written
 */
u32 BufferMultiByteTextWriteStreamSrc::write(const void* pData, u32 size)
{
    const u8* pBytes = static_cast<const u8*>(pData);
    u32 totalBytesWritten = 0;
    do
    {
        if (mCurrentPos >= mBufferSize)
        {
            continue;
        }

        u32 writeSize = mBufferSize - mCurrentPos;
        if (writeSize >= size - totalBytesWritten)
        {
            writeSize = size - totalBytesWritten;
        }
        else
        {
            const u32 end = writeSize + totalBytesWritten;
            u8 c = pBytes[end - 1];
            if (c & 0x80)
            {
                u32 cut = 0;
                if ((c & 0xc0) != 0x80)
                {
                    cut = 1;
                }
                else
                {
                    const s32 maxLength = Mathi::min(s32(writeSize), 4);
                    for (s32 i = 2; i <= maxLength; i++)
                    {
                        c = pBytes[end - i];
                        if ((c & 0xc0) != 0x80)
                        {
                            if (getUtf8CharLength_(c) > u32(i))
                            {
                                cut = i;
                            }

                            break;
                        }
                    }
                }

                if (cut != 0)
                {
                    static_cast<u8*>(mBuffer)[mBufferSize - cut] = 0;
                    writeSize -= cut;
                }
            }
        }

        memcpy(static_cast<u8*>(mBuffer) + mCurrentPos, pBytes + totalBytesWritten, writeSize);
        totalBytesWritten += writeSize;
        mCurrentPos += writeSize;
    } while (totalBytesWritten < size && flush());

    return totalBytesWritten;
}

/**
 * Terminates the buffered text with a null character, then flushes it.
 * @return whether the whole buffer was written
 */
bool BufferMultiByteNullTerminatedTextWriteStreamSrc::flush()
{
    static_cast<u8*>(mBuffer)[mCurrentPos] = 0;
    return BufferWriteStreamSrc::flush();
}

/**
 * Wraps a write stream in a UTF-8 aware text buffer.
 * @param pStream the stream to write through
 * @param pBuffer the buffer
 * @param bufferSize the buffer's size in bytes
 */
BufferMultiByteTextWriteStream::BufferMultiByteTextWriteStream(WriteStream* pStream, void* pBuffer,
                                                               u32 bufferSize)
    : mSrc(pStream->getSrc(), pBuffer, bufferSize)
{
    setSrc(&mSrc);
    setUserFormat(pStream->getUserFormat());
    setBinaryEndian(pStream->getBinaryEndian());
}

/**
 * Flushes the buffer and detaches it from the stream.
 */
BufferMultiByteTextWriteStream::~BufferMultiByteTextWriteStream()
{
    flush();
    setSrc(nullptr);
}

}  // namespace sead
