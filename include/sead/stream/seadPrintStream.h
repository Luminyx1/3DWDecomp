#pragma once

#include "stream/seadBufferStream.h"
#include "stream/seadStream.h"
#include "stream/seadStreamSrc.h"

namespace sead
{
class PrintStreamSrc : public StreamSrc
{
public:
    u32 read(void* pData, u32 size) override;
    u32 write(const void* pData, u32 size) override;
    u32 skip(s32 offset) override;
    void rewind() override;
    bool isEOF() override { return false; }

    static PrintStreamSrc sPrintStreamSrc;
};

class PrintWriteStream : public WriteStream
{
public:
    explicit PrintWriteStream(Stream::Modes mode);
    explicit PrintWriteStream(StreamFormat* pFormat);
    ~PrintWriteStream() override;

private:
    BufferMultiByteNullTerminatedTextWriteStreamSrc mSrc;
    char mBuffer[0x80];
};
}  // namespace sead
