#include "stream/seadStreamFormat.h"

#include <cstdio>

#include "codec/seadBase64.h"
#include "prim/seadScopedLock.h"
#include "prim/seadStringUtil.h"
#include "stream/seadStreamSrc.h"
#include "thread/seadMutex.h"

namespace sead
{
static FixedSafeString<1024> sBuffer;
static Mutex sMutex;

/**
 * Creates a text format that separates tokens with whitespace.
 */
TextStreamFormat::TextStreamFormat() : mDelimiter(" \t\r\n") {}

/**
 * Reads the next token and parses it as a u8 value.
 * @param pSrc stream source
 * @param endian unused
 * @return parsed value (0 if the token is not a number)
 */
u8 TextStreamFormat::readU8(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian)
{
    ScopedLock<Mutex> lock(&sMutex);
    u8 value = 0;
    getNextData_(pSrc);
    StringUtil::tryParseNumber(&value, sBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

/**
 * Reads the next token into the shared buffer, skipping delimiters and comments and handling
 * quoted strings.
 * @param pSrc stream source
 */
void TextStreamFormat::getNextData_(StreamSrc* pSrc)
{
    sBuffer.clear();

    char c;

    if (pSrc->read(&c, 1) == 0)
    {
        return;
    }

    bool isInString = false;
    char commentEnd = '\0';
    s32 length = 0;
    do
    {
        if (commentEnd != '\0')
        {
            if (c != commentEnd)
            {
                if (commentEnd == '/')
                {
                    commentEnd = '*';
                }

                continue;
            }

            if (commentEnd == '*')
            {
                commentEnd = '/';
                continue;
            }

            c = mDelimiter[0];
        }

        if (isInString)
        {
            if (c == '"')
            {
                if (length < 2 || sBuffer[length - 1] != '\\')
                {
                    return;
                }

                sBuffer.copyAt(length - 1, "\"", 1);
            }
            else
            {
                sBuffer.append(c);
                length++;
            }

            commentEnd = '\0';
            isInString = true;
        }
        else if (length == 0 && c == '"')
        {
            length = 0;
            commentEnd = '\0';
            isInString = true;
        }
        else if (mDelimiter.include(c) || c == '\0')
        {
            if (!sBuffer.isEmpty())
            {
                return;
            }

            commentEnd = '\0';
            isInString = false;
        }
        else
        {
            sBuffer.append(c);
            s32 newLength = length + 1;
            commentEnd = '\0';

            if (length >= 0 && sBuffer[length] == '#')
            {
                sBuffer.trim(length);
                newLength = length;
                commentEnd = '\n';
            }

            if (newLength >= 2 && sBuffer[newLength - 2] == '/' && sBuffer[newLength - 1] == '/')
            {
                newLength -= 2;
                sBuffer.trim(newLength);
                commentEnd = '\n';
            }
            else if (newLength >= 2 && sBuffer[newLength - 2] == '/' &&
                     sBuffer[newLength - 1] == '*')
            {
                newLength -= 2;
                sBuffer.trim(newLength);
                commentEnd = '*';
            }

            length = newLength;
            isInString = false;
        }
    } while (pSrc->read(&c, 1) != 0);
}

/**
 * Reads the next token and parses it as a u16 value.
 * @param pSrc stream source
 * @param endian unused
 * @return parsed value (0 if the token is not a number)
 */
u16 TextStreamFormat::readU16(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian)
{
    ScopedLock<Mutex> lock(&sMutex);
    u16 value = 0;
    getNextData_(pSrc);
    StringUtil::tryParseNumber(&value, sBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

/**
 * Reads the next token and parses it as a u32 value.
 * @param pSrc stream source
 * @param endian unused
 * @return parsed value (0 if the token is not a number)
 */
u32 TextStreamFormat::readU32(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian)
{
    ScopedLock<Mutex> lock(&sMutex);
    u32 value = 0;
    getNextData_(pSrc);
    StringUtil::tryParseNumber(&value, sBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

/**
 * Reads the next token and parses it as a u64 value.
 * @param pSrc stream source
 * @param endian unused
 * @return parsed value (0 if the token is not a number)
 */
u64 TextStreamFormat::readU64(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian)
{
    ScopedLock<Mutex> lock(&sMutex);
    u64 value = 0;
    getNextData_(pSrc);
    StringUtil::tryParseNumber(&value, sBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

/**
 * Reads the next token and parses it as a s8 value.
 * @param pSrc stream source
 * @param endian unused
 * @return parsed value (0 if the token is not a number)
 */
s8 TextStreamFormat::readS8(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian)
{
    ScopedLock<Mutex> lock(&sMutex);
    s8 value = 0;
    getNextData_(pSrc);
    StringUtil::tryParseNumber(&value, sBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

/**
 * Reads the next token and parses it as a s16 value.
 * @param pSrc stream source
 * @param endian unused
 * @return parsed value (0 if the token is not a number)
 */
s16 TextStreamFormat::readS16(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian)
{
    ScopedLock<Mutex> lock(&sMutex);
    s16 value = 0;
    getNextData_(pSrc);
    StringUtil::tryParseNumber(&value, sBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

/**
 * Reads the next token and parses it as a s32 value.
 * @param pSrc stream source
 * @param endian unused
 * @return parsed value (0 if the token is not a number)
 */
s32 TextStreamFormat::readS32(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian)
{
    ScopedLock<Mutex> lock(&sMutex);
    s32 value = 0;
    getNextData_(pSrc);
    StringUtil::tryParseNumber(&value, sBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

/**
 * Reads the next token and parses it as a s64 value.
 * @param pSrc stream source
 * @param endian unused
 * @return parsed value (0 if the token is not a number)
 */
s64 TextStreamFormat::readS64(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian)
{
    ScopedLock<Mutex> lock(&sMutex);
    s64 value = 0;
    getNextData_(pSrc);
    StringUtil::tryParseNumber(&value, sBuffer.cstr(), StringUtil::CardinalNumber::BaseAuto);
    return value;
}

/**
 * Reads the next token and parses it as an f32 value.
 * @param pSrc stream source
 * @param endian unused
 * @return parsed value (0 if the token is empty)
 */
f32 TextStreamFormat::readF32(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian)
{
    ScopedLock<Mutex> lock(&sMutex);
    f32 value = 0.0f;
    getNextData_(pSrc);

    if (sBuffer.calcLength() != 0)
    {
        std::sscanf(sBuffer.cstr(), "%f", &value);
    }

    return value;
}

/**
 * Reads the next token as a string.
 * @param pSrc stream source
 * @param pStr destination string
 * @param size unused
 */
void TextStreamFormat::readString(StreamSrc* pSrc, BufferedSafeString* pStr,
                                  [[maybe_unused]] u32 size)
{
    ScopedLock<Mutex> lock(&sMutex);
    getNextData_(pSrc);
    pStr->copy(sBuffer);
}

/**
 * Reads the next token as a binary literal (optionally prefixed with 0b), most significant bit
 * first.
 * @param pSrc stream source
 * @param pData destination buffer
 * @param bits number of bits to read
 */
void TextStreamFormat::readBit(StreamSrc* pSrc, void* pData, u32 bits)
{
    ScopedLock<Mutex> lock(&sMutex);
    getNextData_(pSrc);

    u8* data = static_cast<u8*>(pData);
    SafeString str = sBuffer;

    if (str.comparen("0b", 2) == 0)
    {
        str = str.getPart(2);
    }

    const s32 length = str.calcLength();
    u32 bitIndex = 0;
    u8 value = 0;

    for (s32 i = 0; bitIndex < bits && i <= length; i++)
    {
        value = (value << 1) | (str.at(i) == '1');
        bitIndex++;

        if (bitIndex % 8 == 0)
        {
            data[bitIndex / 8 - 1] = value;
            value = 0;
        }
    }

    if (bitIndex % 8 != 0)
    {
        const u8 mask = 0xFF << (bitIndex % 8);
        data[bitIndex / 8] = (data[bitIndex / 8] & mask) | value;
    }

}

/**
 * Reads the next token as Base64 data.
 * @param pSrc stream source
 * @param pBuffer destination buffer
 * @param size size of the destination buffer
 * @return number of bytes decoded
 */
u32 TextStreamFormat::readMemBlock(StreamSrc* pSrc, void* pBuffer, u32 size)
{
    ScopedLock<Mutex> lock(&sMutex);
    getNextData_(pSrc);
    const u32 length = sBuffer.calcLength();
    size_t decodedSize = 0;
    Base64::decode(pBuffer, size, sBuffer.cstr(), length, &decodedSize);
    return decodedSize;
}

/**
 * Writes a u8 value as text followed by a delimiter.
 * @param pSrc stream source
 * @param endian unused
 * @param value value to write
 */
void TextStreamFormat::writeU8(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian, u8 value)
{
    FixedSafeString<32> str;
    str.format("%u", value);
    const s32 length = str.calcLength();
    pSrc->write(str.cstr(), length);
    pSrc->write(mDelimiter.cstr(), 1);
}

/**
 * Writes a u16 value as text followed by a delimiter.
 * @param pSrc stream source
 * @param endian unused
 * @param value value to write
 */
void TextStreamFormat::writeU16(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian, u16 value)
{
    FixedSafeString<32> str;
    str.format("%u", value);
    const s32 length = str.calcLength();
    pSrc->write(str.cstr(), length);
    pSrc->write(mDelimiter.cstr(), 1);
}

/**
 * Writes a u32 value as text followed by a delimiter.
 * @param pSrc stream source
 * @param endian unused
 * @param value value to write
 */
void TextStreamFormat::writeU32(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian, u32 value)
{
    FixedSafeString<32> str;
    str.format("%u", value);
    const s32 length = str.calcLength();
    pSrc->write(str.cstr(), length);
    pSrc->write(mDelimiter.cstr(), 1);
}

/**
 * Writes a u64 value as text followed by a delimiter.
 * @param pSrc stream source
 * @param endian unused
 * @param value value to write
 */
void TextStreamFormat::writeU64(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian, u64 value)
{
    FixedSafeString<32> str;
    str.format("%llu", value);
    const s32 length = str.calcLength();
    pSrc->write(str.cstr(), length);
    pSrc->write(mDelimiter.cstr(), 1);
}

/**
 * Writes a s8 value as text followed by a delimiter.
 * @param pSrc stream source
 * @param endian unused
 * @param value value to write
 */
void TextStreamFormat::writeS8(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian, s8 value)
{
    FixedSafeString<32> str;
    str.format("%d", value);
    const s32 length = str.calcLength();
    pSrc->write(str.cstr(), length);
    pSrc->write(mDelimiter.cstr(), 1);
}

/**
 * Writes a s16 value as text followed by a delimiter.
 * @param pSrc stream source
 * @param endian unused
 * @param value value to write
 */
void TextStreamFormat::writeS16(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian, s16 value)
{
    FixedSafeString<32> str;
    str.format("%d", value);
    const s32 length = str.calcLength();
    pSrc->write(str.cstr(), length);
    pSrc->write(mDelimiter.cstr(), 1);
}

/**
 * Writes a s32 value as text followed by a delimiter.
 * @param pSrc stream source
 * @param endian unused
 * @param value value to write
 */
void TextStreamFormat::writeS32(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian, s32 value)
{
    FixedSafeString<32> str;
    str.format("%d", value);
    const s32 length = str.calcLength();
    pSrc->write(str.cstr(), length);
    pSrc->write(mDelimiter.cstr(), 1);
}

/**
 * Writes a s64 value as text followed by a delimiter.
 * @param pSrc stream source
 * @param endian unused
 * @param value value to write
 */
void TextStreamFormat::writeS64(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian, s64 value)
{
    FixedSafeString<32> str;
    str.format("%lld", value);
    const s32 length = str.calcLength();
    pSrc->write(str.cstr(), length);
    pSrc->write(mDelimiter.cstr(), 1);
}

/**
 * Writes a f32 value as text followed by a delimiter.
 * @param pSrc stream source
 * @param endian unused
 * @param value value to write
 */
void TextStreamFormat::writeF32(StreamSrc* pSrc, [[maybe_unused]] Endian::Types endian, f32 value)
{
    FixedSafeString<32> str;
    str.format("%.8f", value);
    const s32 length = str.calcLength();
    pSrc->write(str.cstr(), length);
    pSrc->write(mDelimiter.cstr(), 1);
}

/**
 * Writes a bit field as a binary literal prefixed with 0b, most significant bit first.
 * @param pSrc stream source
 * @param pData source data
 * @param bits number of bits to write
 */
void TextStreamFormat::writeBit(StreamSrc* pSrc, const void* pData, u32 bits)
{
    ScopedLock<Mutex> lock(&sMutex);
    const u8* data = static_cast<const u8*>(pData);
    sBuffer.copy("0b");

    const u32 byteCount = (bits + 7) / 8;

    for (u32 i = 0; i < byteCount; i++)
    {
        const s32 bitCount = bits - i * 8 < 8 ? bits - i * 8 : 8;

        for (s32 j = bitCount - 1; j >= 0; j--)
        {
            if (data[i] & (1 << j))
            {
                sBuffer.append('1');
            }
            else
            {
                sBuffer.append('0');
            }
        }
    }

    pSrc->write(sBuffer.cstr(), bits + 2);
    pSrc->write(mDelimiter.cstr(), 1);
}

/**
 * Writes a string in double quotes, escaping double quotes with backslashes.
 * @param pSrc stream source
 * @param rStr string to write
 * @param size maximum number of characters to write
 */
void TextStreamFormat::writeString(StreamSrc* pSrc, const SafeString& rStr, u32 size)
{
    u32 length = rStr.calcLength();

    if (length > size)
    {
        length = size;
    }

    char quote = '"';
    char backslash = '\\';
    pSrc->write(&quote, 1);

    for (u32 i = 0; i < length; i++)
    {
        if (rStr.at(i) == '"')
        {
            pSrc->write(&backslash, 1);
        }

        pSrc->write(rStr.cstr() + i, 1);
    }

    pSrc->write(&quote, 1);
}

/**
 * Writes a memory block as Base64 data in double quotes.
 * @param pSrc stream source
 * @param pBuffer source data
 * @param size size in bytes
 */
void TextStreamFormat::writeMemBlock(StreamSrc* pSrc, const void* pBuffer, u32 size)
{
    ScopedLock<Mutex> lock(&sMutex);
    sBuffer.clear();

    u32 blockCount = size / 3;

    if (size % 3 != 0)
    {
        blockCount++;
    }

    const u32 encodedLength = blockCount * 4;

    if (encodedLength + 1 < static_cast<u32>(sBuffer.getBufferSize()))
    {
        char* dst = const_cast<char*>(sBuffer.cstr());
        dst[encodedLength] = '\0';
        Base64::encode(dst, pBuffer, size, false);

        const s32 length = sBuffer.calcLength();
        pSrc->write("\"", 1);
        pSrc->write(sBuffer.cstr(), length);
        pSrc->write("\"", 1);
        pSrc->write(mDelimiter.cstr(), 1);
    }

}

/**
 * Writes text as is.
 * @param pSrc stream source
 * @param rText text to write
 */
void TextStreamFormat::writeDecorationText(StreamSrc* pSrc, const SafeString& rText)
{
    const s32 length = rText.calcLength();
    pSrc->write(rText.cstr(), length);
}

/**
 * Writes a null character.
 * @param pSrc stream source
 */
void TextStreamFormat::writeNullChar(StreamSrc* pSrc)
{
    char c = '\0';
    pSrc->write(&c, 1);
}

/**
 * Skips the next token.
 * @param pSrc stream source
 * @param offset unused
 */
void TextStreamFormat::skip(StreamSrc* pSrc, [[maybe_unused]] u32 offset)
{
    ScopedLock<Mutex> lock(&sMutex);
    getNextData_(pSrc);
}

/**
 * Rewinds the source.
 * @param pSrc stream source
 */
void TextStreamFormat::rewind(StreamSrc* pSrc)
{
    pSrc->rewind();
}
}  // namespace sead
