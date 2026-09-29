#include "stream/seadPrintStream.h"

namespace sead
{
PrintStreamSrc PrintStreamSrc::sPrintStreamSrc;

/**
 * Discards the data (printing is compiled out in release builds).
 * @param pData data to print
 * @param size size of the data in bytes
 * @return size
 */
u32 PrintStreamSrc::write(const void* pData, u32 size)
{
    return size;
}

/**
 * Skipping is not supported.
 * @param offset offset in bytes
 * @return 0
 */
u32 PrintStreamSrc::skip(s32 offset)
{
    return 0;
}

/**
 * Reading is not supported.
 * @param pData destination buffer
 * @param size size in bytes
 * @return 0
 */
u32 PrintStreamSrc::read(void* pData, u32 size)
{
    return 0;
}

/**
 * Rewinding is not supported.
 */
void PrintStreamSrc::rewind() {}

/**
 * Creates a buffered text stream that prints to the debug output using a basic format.
 * @param mode basic stream format
 */
PrintWriteStream::PrintWriteStream(Stream::Modes mode)
    : mSrc(&PrintStreamSrc::sPrintStreamSrc, mBuffer, sizeof(mBuffer) - 1)
{
    setSrc(&mSrc);
    setMode(mode);
}

/**
 * Creates a buffered text stream that prints to the debug output using a user format.
 * @param pFormat stream format
 */
PrintWriteStream::PrintWriteStream(StreamFormat* pFormat)
    : mSrc(&PrintStreamSrc::sPrintStreamSrc, mBuffer, sizeof(mBuffer) - 1)
{
    setSrc(&mSrc);
    setUserFormat(pFormat);
}

/**
 * Flushes the stream and detaches the source.
 */
PrintWriteStream::~PrintWriteStream()
{
    flush();
    setSrc(nullptr);
}
}  // namespace sead
