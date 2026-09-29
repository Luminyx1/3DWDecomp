#ifndef SEAD_STRING_UTIL_H_
#define SEAD_STRING_UTIL_H_

#include <stdarg.h>
#include <stdio.h>

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <prim/seadSafeString.h>

namespace sead
{
namespace StringUtil
{
struct Char16Pair
{
    char16 before;
    char16 after;
};

enum class CardinalNumber
{
    BaseAuto = -1,
    Base2 = 2,
    Base8 = 8,
    Base10 = 10,
    Base16 = 16,
};

template <typename T>
bool tryParseNumber(T* pOut, const SafeString& rStr, CardinalNumber base);

template <typename T>
T parseNumber(const SafeString& rStr, CardinalNumber base);

template <>
bool tryParseNumber<u8>(u8* pOut, const SafeString& rStr, CardinalNumber base);
template <>
bool tryParseNumber<s8>(s8* pOut, const SafeString& rStr, CardinalNumber base);
template <>
bool tryParseNumber<u16>(u16* pOut, const SafeString& rStr, CardinalNumber base);
template <>
bool tryParseNumber<s16>(s16* pOut, const SafeString& rStr, CardinalNumber base);
template <>
bool tryParseNumber<u32>(u32* pOut, const SafeString& rStr, CardinalNumber base);
template <>
bool tryParseNumber<s32>(s32* pOut, const SafeString& rStr, CardinalNumber base);
template <>
bool tryParseNumber<u64>(u64* pOut, const SafeString& rStr, CardinalNumber base);
template <>
bool tryParseNumber<s64>(s64* pOut, const SafeString& rStr, CardinalNumber base);
template <>
bool tryParseNumber<f32>(f32* pOut, const SafeString& rStr, CardinalNumber base);
template <>
bool tryParseNumber<f64>(f64* pOut, const SafeString& rStr, CardinalNumber base);
template <>
u8 parseNumber<u8>(const SafeString& rStr, CardinalNumber base);
template <>
s8 parseNumber<s8>(const SafeString& rStr, CardinalNumber base);
template <>
u16 parseNumber<u16>(const SafeString& rStr, CardinalNumber base);
template <>
s16 parseNumber<s16>(const SafeString& rStr, CardinalNumber base);
template <>
u32 parseNumber<u32>(const SafeString& rStr, CardinalNumber base);
template <>
s32 parseNumber<s32>(const SafeString& rStr, CardinalNumber base);
template <>
u64 parseNumber<u64>(const SafeString& rStr, CardinalNumber base);
template <>
s64 parseNumber<s64>(const SafeString& rStr, CardinalNumber base);
template <>
f32 parseNumber<f32>(const SafeString& rStr, CardinalNumber base);
template <>
f64 parseNumber<f64>(const SafeString& rStr, CardinalNumber base);

bool tryParseU64(u64* pOut, const SafeString& rStr, CardinalNumber base);
bool tryParseS64(s64* pOut, const SafeString& rStr, CardinalNumber base);
bool tryParseU32(u32* pOut, const SafeString& rStr, CardinalNumber base);
bool tryParseS32(s32* pOut, const SafeString& rStr, CardinalNumber base);
bool tryParseU16(u16* pOut, const SafeString& rStr, CardinalNumber base);
bool tryParseS16(s16* pOut, const SafeString& rStr, CardinalNumber base);
bool tryParseU8(u8* pOut, const SafeString& rStr, CardinalNumber base);
bool tryParseS8(s8* pOut, const SafeString& rStr, CardinalNumber base);
bool tryParseF32(f32* pOut, const SafeString& rStr);
bool tryParseF64(f64* pOut, const SafeString& rStr);

inline u64 parseU64(const SafeString& rStr, CardinalNumber base)
{
    return parseNumber<u64>(rStr, base);
}

inline s64 parseS64(const SafeString& rStr, CardinalNumber base)
{
    return parseNumber<s64>(rStr, base);
}

inline u32 parseU32(const SafeString& rStr, CardinalNumber base)
{
    return parseNumber<u32>(rStr, base);
}

inline s32 parseS32(const SafeString& rStr, CardinalNumber base)
{
    return parseNumber<s32>(rStr, base);
}

inline u16 parseU16(const SafeString& rStr, CardinalNumber base)
{
    return parseNumber<u16>(rStr, base);
}

inline s16 parseS16(const SafeString& rStr, CardinalNumber base)
{
    return parseNumber<s16>(rStr, base);
}

inline u8 parseU8(const SafeString& rStr, CardinalNumber base)
{
    return parseNumber<u8>(rStr, base);
}

inline s8 parseS8(const SafeString& rStr, CardinalNumber base)
{
    return parseNumber<s8>(rStr, base);
}

char16* wcs16cpy(char16*, size_t n, const char16*);

s32 snprintf(char* s, size_t n, const char* format, ...);
s32 sw16printf(char16* s, size_t n, const char16* format, ...);
s32 vsnprintf(char* s, size_t n, const char* format, va_list args);
s32 vsw16printf(char16* s, size_t n, const char16* format, std::va_list args);
// TODO
s32 vsnw16printf(char16* s, size_t n, const char16* format, std::va_list args);

bool tryConvertSjisToUtf16(s32* pOutLength, char16* pDst, u32 dstLength, const char* pSrc,
                           s32 srcLength);
bool tryConvertUtf16ToSjis(s32* pOutLength, char* pDst, u32 dstLength, const char16* pSrc,
                           s32 srcLength);
bool tryConvertUtf8ToUtf16(s32* pOutLength, char16* pDst, u32 dstLength, const char* pSrc,
                           s32 srcLength);
bool tryConvertUtf16ToUtf8(s32* pOutLength, char* pDst, u32 dstLength, const char16* pSrc,
                           s32 srcLength);
bool tryConvertSjisToUtf8(s32* pOutLength, char* pDst, u32 dstLength, const char* pSrc,
                          s32 srcLength);
bool tryConvertUtf8ToSjis(s32* pOutLength, char* pDst, u32 dstLength, const char* pSrc,
                          s32 srcLength);

s32 convertSjisToUtf16(char16* dst, u32 dst_len, const char* src, s32 src_len);
s32 convertUtf16ToSjis(char* dst, u32 dst_len, const char16* src, s32 src_len);
s32 convertUtf8ToUtf16(char16* dst, u32 dst_len, const char* src, s32 src_len);
s32 convertUtf16ToUtf8(char* dst, u32 dst_len, const char16* src, s32 src_len);
s32 convertSjisToUtf8(char* dst, u32 dst_len, const char* src, s32 src_len);
s32 convertUtf8ToSjis(char* dst, u32 dst_len, const char* src, s32 src_len);

s32 compareChar16Pair(const Char16Pair& rPair, const char16& rKey);
char16 replace(char16 c, const Buffer<const Char16Pair>& rSortedTable);

char16 toUpperCapital(char16 c);
void toUpperCapitalFirstCharactor(WBufferedSafeString* str);
void toUpperCapitalFirstCharactor(BufferedSafeString* pStr);

char16 toLowerCapital(char16 c);

inline char toUpperCapital(char c)
{
    return (c >= 'a' && c <= 'z') ? static_cast<char>(c - ('a' - 'A')) : c;
}

inline char toLowerCapital(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
}
void toLowerCapitalFirstCharactor(WBufferedSafeString* str);
void toLowerCapitalFirstCharactor(BufferedSafeString* pStr);

}  // namespace StringUtil
}  // namespace sead

#endif  // SEAD_STRING_UTIL_H_
