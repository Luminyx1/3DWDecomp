#include <codec/seadBase64.h>

namespace sead
{
namespace
{
inline u32 decodeChar(char c)
{
    if (u32(c - 'A') < 26)
    {
        return c - 'A';
    }

    if (c >= 'a' && c <= 'z')
    {
        return c - 'a' + 26;
    }

    if (c >= '0' && c <= '9')
    {
        return c - '0' + 52;
    }

    switch (c)
    {
        case '+':
        case '-':
            return 62;
        case '/':
        case '_':
            return 63;
        default:
            return 64;
    }
}
}  // namespace

/**
 * Encodes binary data as Base64 text without a null terminator.
 * @param pDst Output buffer, which must hold at least (length + 2) / 3 * 4 characters.
 * @param pSrc Data to encode.
 * @param length Size of the data in bytes.
 * @param urlSafe Whether to use the URL-safe alphabet ('-' and '_').
 */
void Base64::encode(char* pDst, const void* pSrc, size_t length, bool urlSafe)
{
    const char* table = urlSafe
                            ? "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
                            : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const u8* src = static_cast<const u8*>(pSrc);

    const size_t rest = length % 3;
    const size_t mainLength = length - rest;
    size_t i = 0;
    for (; i < mainLength; i += 3)
    {
        *pDst++ = table[src[i] >> 2];
        *pDst++ = table[((src[i] & 3) << 4) | (src[i + 1] >> 4)];
        *pDst++ = table[((src[i + 1] & 0xf) << 2) | (src[i + 2] >> 6)];
        *pDst++ = table[src[i + 2] & 0x3f];
    }
    src += i;

    switch (rest)
    {
        case 2:
            pDst[0] = table[src[0] >> 2];
            pDst[1] = table[((src[0] & 3) << 4) | (src[1] >> 4)];
            pDst[2] = table[(src[1] & 0xf) << 2];
            pDst[3] = '=';
            break;
        case 1:
            pDst[0] = table[src[0] >> 2];
            pDst[1] = table[(src[0] & 3) << 4];
            pDst[2] = '=';
            pDst[3] = '=';
            break;
        default:
            break;
    }
}

// NON_MATCHING: block placement around the line-break skip and a pre-indexed load
/**
 * Decodes Base64 text, skipping line breaks between groups of four characters.
 * @param pDst Output buffer.
 * @param dstSize Size of the output buffer.
 * @param pSrc Base64 text.
 * @param srcSize Length of the text.
 * @param pDecodedSize Receives the number of decoded bytes (may be null).
 * @return Whether the text was decoded successfully.
 */
bool Base64::decode(void* pDst, size_t dstSize, const char* pSrc, size_t srcSize,
                    size_t* pDecodedSize)
{
    u8* dst = static_cast<u8*>(pDst);
    size_t decoded = 0;
    size_t i = 0;
    while (i < srcSize)
    {
        if (u8(*pSrc) < 0x20)
        {
            i++;
            if (i >= srcSize)
            {
                break;
            }
            pSrc++;

            if (u8(*pSrc) < 0x20)
            {
                i++;
                if (i >= srcSize)
                {
                    break;
                }
                pSrc++;
            }
        }

        i += 4;
        if (i > srcSize)
        {
            return false;
        }

        const u32 v0 = decodeChar(pSrc[0]) & 0x3f;
        const u32 v1 = decodeChar(pSrc[1]) & 0x3f;
        const u32 v2 = decodeChar(pSrc[2]) & 0x7f;
        const u32 v3 = decodeChar(pSrc[3]) & 0x7f;

        if (decoded >= dstSize)
        {
            return false;
        }
        dst[decoded++] = (v0 << 2) | (v1 >> 4);

        if (v2 < 64)
        {
            if (decoded >= dstSize)
            {
                return false;
            }
            dst[decoded++] = (v1 << 4) | (v2 >> 2);

            if (v3 < 64)
            {
                if (decoded >= dstSize)
                {
                    return false;
                }
                dst[decoded++] = (v2 << 6) | v3;
            }
        }

        pSrc += 4;
    }

    if (pDecodedSize != nullptr)
    {
        *pDecodedSize = decoded;
    }

    return true;
}
}  // namespace sead
