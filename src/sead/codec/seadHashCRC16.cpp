#include <codec/seadHashCRC16.h>

namespace sead {
u16 HashCRC16::sTable[256];
bool HashCRC16::sInitialized = false;

/** Builds the lookup table for the reflected CRC polynomial. */
void HashCRC16::initialize()
{
    for (u32 i = 0; i < 256; ++i) {
        u32 value = i;

        for (u32 bit = 0; bit < 8; ++bit)
            value = (value & 1) ? (value >> 1) ^ 0xa001 : value >> 1;
        sTable[i] = value;
    }

    sInitialized = true;
}

/**
 * Hashes a byte buffer with a fresh context.
 * @param pData Bytes to hash.
 * @param size Number of bytes to read.
 * @return The CRC of the buffer.
 */
u32 HashCRC16::calcHash(const void* pData, u32 size)
{
    Context context;
    return calcHashWithContext(&context, pData, size);
}

/**
 * Extends a running CRC with a byte buffer.
 * @param pContext Running hash state, updated in place.
 * @param pData Bytes to append to the hash.
 * @param size Number of bytes to read.
 * @return The CRC after processing the buffer.
 */
u32 HashCRC16::calcHashWithContext(Context* pContext, const void* pData, u32 size)
{
    if (!sInitialized)
        initialize();
    u32 hash = pContext->hash;
    const u8* data = static_cast<const u8*>(pData);

    while (size--) {
        const u8 byte = *data++;
        hash = sTable[(hash & 0xff) ^ byte] ^ (hash >> 8);
    }

    pContext->hash = hash;
    return hash;
}

/**
 * Hashes a null-terminated string with a fresh context.
 * @param pString String to hash, excluding its terminator.
 * @return The CRC of the string.
 */
u32 HashCRC16::calcStringHash(const char* pString)
{
    Context context;
    return calcStringHashWithContext(&context, pString);
}

/**
 * Extends a running CRC with a null-terminated string.
 * @param pContext Running hash state, updated in place.
 * @param pString String to append, excluding its terminator.
 * @return The CRC after processing the string.
 */
u32 HashCRC16::calcStringHashWithContext(Context* pContext, const char* pString)
{
    if (!sInitialized)
        initialize();
    u32 hash = pContext->hash;

    while (*pString) {
        hash = sTable[(hash ^ *pString++) & 0xff] ^ (hash >> 8);
    }

    pContext->hash = hash;
    return hash;
}
}  // namespace sead
