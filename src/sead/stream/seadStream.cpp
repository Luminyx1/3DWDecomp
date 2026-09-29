#include "stream/seadStream.h"

#include "stream/seadStreamFormat.h"
#include "stream/seadStreamSrc.h"

namespace sead
{
static BinaryStreamFormat sBinaryStreamInstance;
static TextStreamFormat sTextStreamInstance;

StreamFormat* const Stream::BASIC_STREAM_FORMAT[2]{
    &sBinaryStreamInstance,
    &sTextStreamInstance,
};

/**
 * Creates a stream without a source or format.
 */
Stream::Stream() = default;

/**
 * Creates a stream over a source using a basic format.
 * @param pSrc stream source
 * @param mode basic stream format
 */
Stream::Stream(StreamSrc* pSrc, Modes mode)
{
    mSrc = pSrc;
    setMode(mode);
}

/**
 * Selects one of the basic stream formats.
 * @param mode basic stream format
 */
void Stream::setMode(Modes mode)
{
    mFormat = BASIC_STREAM_FORMAT[static_cast<u32>(mode)];
}

/**
 * Creates a stream over a source using a user format.
 * @param pSrc stream source
 * @param pFormat stream format
 */
Stream::Stream(StreamSrc* pSrc, StreamFormat* pFormat)
{
    mSrc = pSrc;
    mFormat = pFormat;
}

/**
 * Selects a user stream format.
 * @param pFormat stream format
 */
void Stream::setUserFormat(StreamFormat* pFormat)
{
    mFormat = pFormat;
}

/**
 * Sets the endianness used by binary formats.
 * @param endian endianness of the stream data
 */
void Stream::setBinaryEndian(Endian::Types endian)
{
    mEndian = endian;
}

/**
 * Skips bytes through the stream format.
 * @param bytes number of bytes to skip
 */
void Stream::skip(u32 bytes)
{
    mFormat->skip(mSrc, bytes);
}

/**
 * Skips a number of fixed-size blocks.
 * @param blockSize size of one block in bytes
 * @param count number of blocks
 */
void Stream::skip(u32 blockSize, u32 count)
{
    for (u32 i = 0; i < count; i++)
    {
        skip(blockSize);
    }
}

/**
 * Rewinds the stream to its start.
 */
void Stream::rewind()
{
    mFormat->rewind(mSrc);
}

/**
 * Checks whether the source has reached its end.
 * @return true if at the end of the source
 */
bool Stream::isEOF()
{
    return mSrc->isEOF();
}

/**
 * Reads a u8 value.
 * @return value read
 */
u8 ReadStream::readU8()
{
    return mFormat->readU8(mSrc, mEndian);
}

/**
 * Reads a u16 value.
 * @return value read
 */
u16 ReadStream::readU16()
{
    return mFormat->readU16(mSrc, mEndian);
}

/**
 * Reads a u32 value.
 * @return value read
 */
u32 ReadStream::readU32()
{
    return mFormat->readU32(mSrc, mEndian);
}

/**
 * Reads a u64 value.
 * @return value read
 */
u64 ReadStream::readU64()
{
    return mFormat->readU64(mSrc, mEndian);
}

/**
 * Reads a s8 value.
 * @return value read
 */
s8 ReadStream::readS8()
{
    return mFormat->readS8(mSrc, mEndian);
}

/**
 * Reads a s16 value.
 * @return value read
 */
s16 ReadStream::readS16()
{
    return mFormat->readS16(mSrc, mEndian);
}

/**
 * Reads a s32 value.
 * @return value read
 */
s32 ReadStream::readS32()
{
    return mFormat->readS32(mSrc, mEndian);
}

/**
 * Reads a s64 value.
 * @return value read
 */
s64 ReadStream::readS64()
{
    return mFormat->readS64(mSrc, mEndian);
}

/**
 * Reads a f32 value.
 * @return value read
 */
f32 ReadStream::readF32()
{
    return mFormat->readF32(mSrc, mEndian);
}

/**
 * Reads a u8 value into a reference.
 * @param rValue destination
 */
void ReadStream::readU8(u8& rValue)
{
    rValue = readU8();
}

/**
 * Reads a u16 value into a reference.
 * @param rValue destination
 */
void ReadStream::readU16(u16& rValue)
{
    rValue = readU16();
}

/**
 * Reads a u32 value into a reference.
 * @param rValue destination
 */
void ReadStream::readU32(u32& rValue)
{
    rValue = readU32();
}

/**
 * Reads a u64 value into a reference.
 * @param rValue destination
 */
void ReadStream::readU64(u64& rValue)
{
    rValue = readU64();
}

/**
 * Reads a s8 value into a reference.
 * @param rValue destination
 */
void ReadStream::readS8(s8& rValue)
{
    rValue = readS8();
}

/**
 * Reads a s16 value into a reference.
 * @param rValue destination
 */
void ReadStream::readS16(s16& rValue)
{
    rValue = readS16();
}

/**
 * Reads a s32 value into a reference.
 * @param rValue destination
 */
void ReadStream::readS32(s32& rValue)
{
    rValue = readS32();
}

/**
 * Reads a s64 value into a reference.
 * @param rValue destination
 */
void ReadStream::readS64(s64& rValue)
{
    rValue = readS64();
}

/**
 * Reads a f32 value into a reference.
 * @param rValue destination
 */
void ReadStream::readF32(f32& rValue)
{
    rValue = readF32();
}

/**
 * Reads a bit field.
 * @param pData destination buffer
 * @param bits number of bits to read
 */
void ReadStream::readBit(void* pData, u32 bits)
{
    mFormat->readBit(mSrc, pData, bits);
}

/**
 * Reads a string.
 * @param pStr destination string
 * @param size maximum size in bytes
 */
void ReadStream::readString(BufferedSafeString* pStr, u32 size)
{
    mFormat->readString(mSrc, pStr, size);
}

/**
 * Reads a raw memory block.
 * @param pBuffer destination buffer
 * @param size size in bytes
 * @return number of bytes read
 */
u32 ReadStream::readMemBlock(void* pBuffer, u32 size)
{
    return mFormat->readMemBlock(mSrc, pBuffer, size);
}

/**
 * Writes a u8 value.
 * @param value value to write
 */
void WriteStream::writeU8(u8 value)
{
    mFormat->writeU8(mSrc, mEndian, value);
}

/**
 * Writes a u16 value.
 * @param value value to write
 */
void WriteStream::writeU16(u16 value)
{
    mFormat->writeU16(mSrc, mEndian, value);
}

/**
 * Writes a u32 value.
 * @param value value to write
 */
void WriteStream::writeU32(u32 value)
{
    mFormat->writeU32(mSrc, mEndian, value);
}

/**
 * Writes a u64 value.
 * @param value value to write
 */
void WriteStream::writeU64(u64 value)
{
    mFormat->writeU64(mSrc, mEndian, value);
}

/**
 * Writes a s8 value.
 * @param value value to write
 */
void WriteStream::writeS8(s8 value)
{
    mFormat->writeS8(mSrc, mEndian, value);
}

/**
 * Writes a s16 value.
 * @param value value to write
 */
void WriteStream::writeS16(s16 value)
{
    mFormat->writeS16(mSrc, mEndian, value);
}

/**
 * Writes a s32 value.
 * @param value value to write
 */
void WriteStream::writeS32(s32 value)
{
    mFormat->writeS32(mSrc, mEndian, value);
}

/**
 * Writes a s64 value.
 * @param value value to write
 */
void WriteStream::writeS64(s64 value)
{
    mFormat->writeS64(mSrc, mEndian, value);
}

/**
 * Writes a f32 value.
 * @param value value to write
 */
void WriteStream::writeF32(f32 value)
{
    mFormat->writeF32(mSrc, mEndian, value);
}

/**
 * Writes a bit field.
 * @param pData source data
 * @param bits number of bits to write
 */
void WriteStream::writeBit(const void* pData, u32 bits)
{
    mFormat->writeBit(mSrc, pData, bits);
}

/**
 * Writes a string.
 * @param rStr string to write
 * @param size size in bytes
 */
void WriteStream::writeString(const SafeString& rStr, u32 size)
{
    mFormat->writeString(mSrc, rStr, size);
}

/**
 * Writes a raw memory block.
 * @param pBuffer source data
 * @param size size in bytes
 */
void WriteStream::writeMemBlock(const void* pBuffer, u32 size)
{
    mFormat->writeMemBlock(mSrc, pBuffer, size);
}

/**
 * Writes a block comment as decoration text.
 * @param rComment comment text
 */
void WriteStream::writeComment(const SafeString& rComment)
{
    mFormat->writeDecorationText(mSrc, "/* ");
    mFormat->writeDecorationText(mSrc, rComment);
    mFormat->writeDecorationText(mSrc, " */");
}

/**
 * Writes a line comment as decoration text.
 * @param rComment comment text
 */
void WriteStream::writeLineComment(const SafeString& rComment)
{
    mFormat->writeDecorationText(mSrc, "// ");
    mFormat->writeDecorationText(mSrc, rComment);
    mFormat->writeDecorationText(mSrc, "\n");
}

/**
 * Writes decoration text (ignored by binary formats).
 * @param rText text to write
 */
void WriteStream::writeDecorationText(const SafeString& rText)
{
    mFormat->writeDecorationText(mSrc, rText);
}

/**
 * Writes a null terminator.
 */
void WriteStream::writeNullChar()
{
    mFormat->writeNullChar(mSrc);
}

/**
 * Flushes the format and the source.
 */
void WriteStream::flush()
{
    mFormat->flush(mSrc);
    mSrc->flush();
}

/**
 * Writes a float as a little-endian fixed-point bit field.
 * @param value value to write
 * @param integerBits number of integer bits
 * @param fractionalBits number of fractional bits
 */
void WriteStream::writeF32BitImpl_(f32 value, u32 integerBits, u32 fractionalBits)
{
    u32 rawValue = static_cast<u32>(value * (1 << fractionalBits) + 0.5f);
    rawValue = Endian::fromHostU32(Endian::cLittle, rawValue);

    writeBit(&rawValue, integerBits + fractionalBits);
}

/**
 * Reads a little-endian fixed-point bit field as a float.
 * @param integerBits number of integer bits
 * @param fractionalBits number of fractional bits
 * @return value read
 */
f32 ReadStream::readF32BitImpl_(u32 integerBits, u32 fractionalBits)
{
    u32 rawValue = 0;
    readBit(&rawValue, integerBits + fractionalBits);
    rawValue = Endian::toHostU32(Endian::cLittle, rawValue);

    return static_cast<f32>(rawValue) / (1 << fractionalBits);
}

/**
 * Writes a double as a little-endian fixed-point bit field.
 * @param value value to write
 * @param integerBits number of integer bits
 * @param fractionalBits number of fractional bits
 */
void WriteStream::writeF64BitImpl_(f64 value, u32 integerBits, u32 fractionalBits)
{
    u64 rawValue = static_cast<u64>(value * (1 << fractionalBits) + 0.5f);
    rawValue = Endian::fromHostU64(Endian::cLittle, rawValue);

    writeBit(&rawValue, integerBits + fractionalBits);
}

/**
 * Reads a little-endian fixed-point bit field as a double.
 * @param integerBits number of integer bits
 * @param fractionalBits number of fractional bits
 * @return value read
 */
f64 ReadStream::readF64BitImpl_(u32 integerBits, u32 fractionalBits)
{
    u64 rawValue = 0;
    readBit(&rawValue, integerBits + fractionalBits);
    rawValue = Endian::toHostU64(Endian::cLittle, rawValue);

    return static_cast<f64>(rawValue) / (1 << fractionalBits);
}
}  // namespace sead
