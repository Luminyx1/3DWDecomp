#include "stream/seadStream.h"

#include "stream/seadStreamFormat.h"
#include "stream/seadStreamSrc.h"

namespace sead
{
BinaryStreamFormat sBinaryStreamInstance;
TextStreamFormat sTextStreamInstance;

StreamFormat* Stream::BASIC_STREAM_FORMAT[2]{
    &sBinaryStreamInstance,
    &sTextStreamInstance,
};

Stream::Stream() = default;

Stream::Stream(StreamSrc* pSrc, StreamFormat* pFormat)
{
    mSrc = pSrc;
    mFormat = pFormat;
}

void Stream::skip(u32 bytes)
{
    mFormat->skip(mSrc, bytes);
}

void Stream::skip(u32 blockSize, u32 count)
{
    for (u32 i = 0; i < count; i++)
    {
        skip(blockSize);
    }
}

void Stream::rewind()
{
    mFormat->rewind(mSrc);
}

bool Stream::isEOF()
{
    return mSrc->isEOF();
}

void Stream::setBinaryEndian(Endian::Types endian)
{
    mEndian = endian;
}

void Stream::setMode(Modes mode)
{
    mFormat = BASIC_STREAM_FORMAT[(u32)mode];
}

void Stream::setUserFormat(StreamFormat* pFormat)
{
    mFormat = pFormat;
}

u8 ReadStream::readU8()
{
    return mFormat->readU8(mSrc, mEndian);
}

void ReadStream::readU8(u8& rValue)
{
    rValue = readU8();
}

u16 ReadStream::readU16()
{
    return mFormat->readU16(mSrc, mEndian);
}

void ReadStream::readU16(u16& rValue)
{
    rValue = readU16();
}

u32 ReadStream::readU32()
{
    return mFormat->readU32(mSrc, mEndian);
}

void ReadStream::readU32(u32& rValue)
{
    rValue = readU32();
}

u64 ReadStream::readU64()
{
    return mFormat->readU64(mSrc, mEndian);
}

void ReadStream::readU64(u64& rValue)
{
    rValue = readU64();
}

s8 ReadStream::readS8()
{
    return mFormat->readS8(mSrc, mEndian);
}

void ReadStream::readS8(s8& rValue)
{
    rValue = readS8();
}

s16 ReadStream::readS16()
{
    return mFormat->readS16(mSrc, mEndian);
}

void ReadStream::readS16(s16& rValue)
{
    rValue = readS16();
}

s32 ReadStream::readS32()
{
    return mFormat->readS32(mSrc, mEndian);
}

void ReadStream::readS32(s32& rValue)
{
    rValue = readS32();
}

s64 ReadStream::readS64()
{
    return mFormat->readS64(mSrc, mEndian);
}

void ReadStream::readS64(s64& rValue)
{
    rValue = readS64();
}

f32 ReadStream::readF32()
{
    return mFormat->readF32(mSrc, mEndian);
}

void ReadStream::readF32(f32& rValue)
{
    rValue = readF32();
}

void ReadStream::readBit(void* pData, u32 bits)
{
    mFormat->readBit(mSrc, pData, bits);
}

void ReadStream::readString(BufferedSafeString* pStr, u32 size)
{
    mFormat->readString(mSrc, pStr, size);
}

u32 ReadStream::readMemBlock(void* pBuffer, u32 size)
{
    return mFormat->readMemBlock(mSrc, pBuffer, size);
}

f32 ReadStream::readF32BitImpl_(u32 integerBits, u32 fractionalBits)
{
    u32 rawValue = 0;
    readBit(&rawValue, integerBits + fractionalBits);
    rawValue = Endian::toHostU32(Endian::cLittle, rawValue);

    return static_cast<f32>(rawValue) / (1 << fractionalBits);
}

f64 ReadStream::readF64BitImpl_(u32 integerBits, u32 fractionalBits)
{
    u64 rawValue = 0;
    readBit(&rawValue, integerBits + fractionalBits);
    rawValue = Endian::toHostU64(Endian::cLittle, rawValue);

    return static_cast<f64>(rawValue) / (1 << fractionalBits);
}

void WriteStream::writeU8(u8 value)
{
    mFormat->writeU8(mSrc, mEndian, value);
}

void WriteStream::writeU16(u16 value)
{
    mFormat->writeU16(mSrc, mEndian, value);
}

void WriteStream::writeU32(u32 value)
{
    mFormat->writeU32(mSrc, mEndian, value);
}

void WriteStream::writeU64(u64 value)
{
    mFormat->writeU64(mSrc, mEndian, value);
}

void WriteStream::writeS8(s8 value)
{
    mFormat->writeS8(mSrc, mEndian, value);
}

void WriteStream::writeS16(s16 value)
{
    mFormat->writeS16(mSrc, mEndian, value);
}

void WriteStream::writeS32(s32 value)
{
    mFormat->writeS32(mSrc, mEndian, value);
}

void WriteStream::writeS64(s64 value)
{
    mFormat->writeS64(mSrc, mEndian, value);
}

void WriteStream::writeF32(f32 value)
{
    mFormat->writeF32(mSrc, mEndian, value);
}

void WriteStream::writeBit(const void* pData, u32 bits)
{
    mFormat->writeBit(mSrc, pData, bits);
}

void WriteStream::writeString(const SafeString& rStr, u32 size)
{
    mFormat->writeString(mSrc, rStr, size);
}

void WriteStream::writeMemBlock(const void* pBuffer, u32 size)
{
    mFormat->writeMemBlock(mSrc, pBuffer, size);
}

void WriteStream::writeComment(const SafeString& rComment)
{
    mFormat->writeDecorationText(mSrc, "/* ");
    mFormat->writeDecorationText(mSrc, rComment);
    mFormat->writeDecorationText(mSrc, " */");
}

void WriteStream::writeLineComment(const SafeString& rComment)
{
    mFormat->writeDecorationText(mSrc, "// ");
    mFormat->writeDecorationText(mSrc, rComment);
    mFormat->writeDecorationText(mSrc, "\n");
}

void WriteStream::writeDecorationText(const SafeString& rText)
{
    mFormat->writeDecorationText(mSrc, rText);
}

void WriteStream::writeNullChar()
{
    mFormat->writeNullChar(mSrc);
}

void WriteStream::flush()
{
    mFormat->flush(mSrc);
    mSrc->flush();
}

void WriteStream::writeF32BitImpl_(f32 value, u32 integerBits, u32 fractionalBits)
{
    u32 rawValue = static_cast<u32>(value * (1 << fractionalBits) + 0.5f);
    rawValue = Endian::fromHostU32(Endian::Types::cLittle, rawValue);

    writeBit(&rawValue, integerBits + fractionalBits);
}

void WriteStream::writeF64BitImpl_(f64 value, u32 integerBits, u32 fractionalBits)
{
    u64 rawValue = static_cast<u64>(value * (1 << fractionalBits) + 0.5f);
    rawValue = Endian::fromHostU64(Endian::Types::cLittle, rawValue);

    writeBit(&rawValue, integerBits + fractionalBits);
}
}  // namespace sead
