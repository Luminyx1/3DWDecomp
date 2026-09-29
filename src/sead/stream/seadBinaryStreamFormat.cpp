#include "stream/seadStreamFormat.h"

#include "stream/seadStreamSrc.h"

namespace sead
{
/**
 * Reads a u8 value and converts it to host endianness.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @return value read
 */
u8 BinaryStreamFormat::readU8(StreamSrc* pSrc, Endian::Types endian)
{
    u8 rawValue = 0;
    pSrc->read(&rawValue, sizeof(u8));
    return Endian::toHostU8(endian, rawValue);
}

/**
 * Reads a u16 value and converts it to host endianness.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @return value read
 */
u16 BinaryStreamFormat::readU16(StreamSrc* pSrc, Endian::Types endian)
{
    u16 rawValue = 0;
    pSrc->read(&rawValue, sizeof(u16));
    return Endian::toHostU16(endian, rawValue);
}

/**
 * Reads a u32 value and converts it to host endianness.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @return value read
 */
u32 BinaryStreamFormat::readU32(StreamSrc* pSrc, Endian::Types endian)
{
    u32 rawValue = 0;
    pSrc->read(&rawValue, sizeof(u32));
    return Endian::toHostU32(endian, rawValue);
}

/**
 * Reads a u64 value and converts it to host endianness.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @return value read
 */
u64 BinaryStreamFormat::readU64(StreamSrc* pSrc, Endian::Types endian)
{
    u64 rawValue = 0;
    pSrc->read(&rawValue, sizeof(u64));
    return Endian::toHostU64(endian, rawValue);
}

/**
 * Reads a s8 value and converts it to host endianness.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @return value read
 */
s8 BinaryStreamFormat::readS8(StreamSrc* pSrc, Endian::Types endian)
{
    s8 rawValue = 0;
    pSrc->read(&rawValue, sizeof(s8));
    return Endian::toHostS8(endian, rawValue);
}

/**
 * Reads a s16 value and converts it to host endianness.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @return value read
 */
s16 BinaryStreamFormat::readS16(StreamSrc* pSrc, Endian::Types endian)
{
    s16 rawValue = 0;
    pSrc->read(&rawValue, sizeof(s16));
    return Endian::toHostS16(endian, rawValue);
}

/**
 * Reads a s32 value and converts it to host endianness.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @return value read
 */
s32 BinaryStreamFormat::readS32(StreamSrc* pSrc, Endian::Types endian)
{
    s32 rawValue = 0;
    pSrc->read(&rawValue, sizeof(s32));
    return Endian::toHostS32(endian, rawValue);
}

/**
 * Reads a s64 value and converts it to host endianness.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @return value read
 */
s64 BinaryStreamFormat::readS64(StreamSrc* pSrc, Endian::Types endian)
{
    s64 rawValue = 0;
    pSrc->read(&rawValue, sizeof(s64));
    return Endian::toHostS64(endian, rawValue);
}

/**
 * Reads an f32 value and converts it to host endianness.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @return value read
 */
f32 BinaryStreamFormat::readF32(StreamSrc* pSrc, Endian::Types endian)
{
    u32 rawValue = 0;
    pSrc->read(&rawValue, sizeof(f32));
    return Endian::toHostF32(endian, &rawValue);
}

/**
 * Reads a bit field, keeping the unused high bits of the last byte.
 * @param pSrc stream source
 * @param pData destination buffer
 * @param bits number of bits to read
 */
void BinaryStreamFormat::readBit(StreamSrc* pSrc, void* pData, u32 bits)
{
    u8* data = static_cast<u8*>(pData);

    u32 size = bits / 8;
    pSrc->read(data, size);
    bits -= size * 8;

    if (bits <= 0)
    {
        return;
    }

    u8 lastByte;
    pSrc->read(&lastByte, 1);

    u8 mask = 0xFF << bits;
    data[size] &= mask;
    data[size] |= lastByte & ~mask;
}

/**
 * Reads a fixed-size string.
 * @param pSrc stream source
 * @param pStr destination string
 * @param size size of the string data in bytes
 */
void BinaryStreamFormat::readString(StreamSrc* pSrc, BufferedSafeString* pStr, u32 size)
{
    u32 remainingSize = 0;
    if (size > static_cast<u32>(pStr->getBufferSize()))
    {
        remainingSize = size - pStr->getBufferSize();
        size = pStr->getBufferSize();
    }

    pSrc->read(pStr->getBuffer(), size);

    if (size + 1 < static_cast<u32>(pStr->getBufferSize()))
    {
        pStr->trim(size);
    }
    else
    {
        pStr->trim(pStr->getBufferSize() - 1);
    }

    if (remainingSize != 0)
    {
        pSrc->read(pStr->getBuffer(), remainingSize);
    }
}

/**
 * Reads a raw memory block.
 * @param pSrc stream source
 * @param pBuffer destination buffer
 * @param size size in bytes
 * @return number of bytes read
 */
u32 BinaryStreamFormat::readMemBlock(StreamSrc* pSrc, void* pBuffer, u32 size)
{
    return pSrc->read(pBuffer, size);
}

/**
 * Converts a u8 value from host endianness and writes it.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @param value value to write
 */
void BinaryStreamFormat::writeU8(StreamSrc* pSrc, Endian::Types endian, u8 value)
{
    u8 rawValue = Endian::fromHostU8(endian, value);
    pSrc->write(&rawValue, sizeof(u8));
}

/**
 * Converts a u16 value from host endianness and writes it.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @param value value to write
 */
void BinaryStreamFormat::writeU16(StreamSrc* pSrc, Endian::Types endian, u16 value)
{
    u16 rawValue = Endian::fromHostU16(endian, value);
    pSrc->write(&rawValue, sizeof(u16));
}

/**
 * Converts a u32 value from host endianness and writes it.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @param value value to write
 */
void BinaryStreamFormat::writeU32(StreamSrc* pSrc, Endian::Types endian, u32 value)
{
    u32 rawValue = Endian::fromHostU32(endian, value);
    pSrc->write(&rawValue, sizeof(u32));
}

/**
 * Converts a u64 value from host endianness and writes it.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @param value value to write
 */
void BinaryStreamFormat::writeU64(StreamSrc* pSrc, Endian::Types endian, u64 value)
{
    u64 rawValue = Endian::fromHostU64(endian, value);
    pSrc->write(&rawValue, sizeof(u64));
}

/**
 * Converts a s8 value from host endianness and writes it.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @param value value to write
 */
void BinaryStreamFormat::writeS8(StreamSrc* pSrc, Endian::Types endian, s8 value)
{
    s8 rawValue = Endian::fromHostS8(endian, value);
    pSrc->write(&rawValue, sizeof(s8));
}

/**
 * Converts a s16 value from host endianness and writes it.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @param value value to write
 */
void BinaryStreamFormat::writeS16(StreamSrc* pSrc, Endian::Types endian, s16 value)
{
    s16 rawValue = Endian::fromHostS16(endian, value);
    pSrc->write(&rawValue, sizeof(s16));
}

/**
 * Converts a s32 value from host endianness and writes it.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @param value value to write
 */
void BinaryStreamFormat::writeS32(StreamSrc* pSrc, Endian::Types endian, s32 value)
{
    s32 rawValue = Endian::fromHostS32(endian, value);
    pSrc->write(&rawValue, sizeof(s32));
}

/**
 * Converts a s64 value from host endianness and writes it.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @param value value to write
 */
void BinaryStreamFormat::writeS64(StreamSrc* pSrc, Endian::Types endian, s64 value)
{
    s64 rawValue = Endian::fromHostS64(endian, value);
    pSrc->write(&rawValue, sizeof(s64));
}

/**
 * Converts an f32 value from host endianness and writes it.
 * @param pSrc stream source
 * @param endian endianness of the stream data
 * @param value value to write
 */
void BinaryStreamFormat::writeF32(StreamSrc* pSrc, Endian::Types endian, f32 value)
{
    u32 rawValue = Endian::fromHostF32(endian, &value);
    pSrc->write(&rawValue, sizeof(f32));
}

/**
 * Writes a bit field, including the unused bits of the last byte.
 * @param pSrc stream source
 * @param pData source data
 * @param bits number of bits to write
 */
void BinaryStreamFormat::writeBit(StreamSrc* pSrc, const void* pData, u32 bits)
{
    const u8* data = static_cast<const u8*>(pData);

    u8 size = bits / 8;
    pSrc->write(data, size);

    if (size * 8 == bits)
    {
        return;
    }

    pSrc->write(&data[size], 1);
}

/**
 * Writes a string padded with null characters to a fixed size.
 * @param pSrc stream source
 * @param rStr string to write
 * @param size size of the string data in bytes
 */
void BinaryStreamFormat::writeString(StreamSrc* pSrc, const SafeString& rStr, u32 size)
{
    u32 length = rStr.calcLength();
    if (length > size)
    {
        length = size;
    }

    pSrc->write(rStr.cstr(), length);

    char nullChar = '\0';
    for (; length < size; length++)
    {
        pSrc->write(&nullChar, 1);
    }
}

/**
 * Writes a raw memory block.
 * @param pSrc stream source
 * @param pBuffer source data
 * @param size size in bytes
 */
void BinaryStreamFormat::writeMemBlock(StreamSrc* pSrc, const void* pBuffer, u32 size)
{
    pSrc->write(pBuffer, size);
}

/**
 * Rewinds the source.
 * @param pSrc stream source
 */
void BinaryStreamFormat::rewind(StreamSrc* pSrc)
{
    pSrc->rewind();
}

/**
 * Skips bytes in the source.
 * @param pSrc stream source
 * @param offset number of bytes to skip
 */
void BinaryStreamFormat::skip(StreamSrc* pSrc, u32 offset)
{
    pSrc->skip(offset);
}
}  // namespace sead
