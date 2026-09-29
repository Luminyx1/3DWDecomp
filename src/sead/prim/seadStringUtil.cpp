#include <prim/seadStringUtil.h>

#include <nn/util.h>

#include <cctype>
#include <limits>
#include <type_traits>

namespace sead::StringUtil
{
static const Char16Pair cToUpperTable[] = {
    {0x0061, 0x0041}, {0x0062, 0x0042}, {0x0063, 0x0043}, {0x0064, 0x0044}, {0x0065, 0x0045},
    {0x0066, 0x0046}, {0x0067, 0x0047}, {0x0068, 0x0048}, {0x0069, 0x0049}, {0x006A, 0x004A},
    {0x006B, 0x004B}, {0x006C, 0x004C}, {0x006D, 0x004D}, {0x006E, 0x004E}, {0x006F, 0x004F},
    {0x0070, 0x0050}, {0x0071, 0x0051}, {0x0072, 0x0052}, {0x0073, 0x0053}, {0x0074, 0x0054},
    {0x0075, 0x0055}, {0x0076, 0x0056}, {0x0077, 0x0057}, {0x0078, 0x0058}, {0x0079, 0x0059},
    {0x007A, 0x005A}, {0x00E0, 0x00C0}, {0x00E1, 0x00C1}, {0x00E2, 0x00C2}, {0x00E3, 0x00C3},
    {0x00E4, 0x00C4}, {0x00E5, 0x00C5}, {0x00E6, 0x00C6}, {0x00E7, 0x00C7}, {0x00E8, 0x00C8},
    {0x00E9, 0x00C9}, {0x00EA, 0x00CA}, {0x00EB, 0x00CB}, {0x00EC, 0x00CC}, {0x00ED, 0x00CD},
    {0x00EE, 0x00CE}, {0x00EF, 0x00CF}, {0x00F0, 0x00D0}, {0x00F1, 0x00D1}, {0x00F2, 0x00D2},
    {0x00F3, 0x00D3}, {0x00F4, 0x00D4}, {0x00F5, 0x00D5}, {0x00F6, 0x00D6}, {0x00F8, 0x00D8},
    {0x00F9, 0x00D9}, {0x00FA, 0x00DA}, {0x00FB, 0x00DB}, {0x00FC, 0x00DC}, {0x00FD, 0x00DD},
    {0x00FE, 0x00DE}, {0x00FF, 0x0178}, {0x0101, 0x0100}, {0x0103, 0x0102}, {0x0105, 0x0104},
    {0x0107, 0x0106}, {0x0109, 0x0108}, {0x010B, 0x010A}, {0x010D, 0x010C}, {0x010F, 0x010E},
    {0x0111, 0x0110}, {0x0113, 0x0112}, {0x0115, 0x0114}, {0x0117, 0x0116}, {0x0119, 0x0118},
    {0x011B, 0x011A}, {0x011D, 0x011C}, {0x011F, 0x011E}, {0x0121, 0x0120}, {0x0123, 0x0122},
    {0x0125, 0x0124}, {0x0127, 0x0126}, {0x0129, 0x0128}, {0x012B, 0x012A}, {0x012D, 0x012C},
    {0x012F, 0x012E}, {0x0131, 0x0130}, {0x0133, 0x0132}, {0x0135, 0x0134}, {0x0137, 0x0136},
    {0x013A, 0x0139}, {0x013C, 0x013B}, {0x013E, 0x013D}, {0x0140, 0x013F}, {0x0142, 0x0141},
    {0x0144, 0x0143}, {0x0146, 0x0145}, {0x0148, 0x0147}, {0x014B, 0x014A}, {0x014D, 0x014C},
    {0x014F, 0x014E}, {0x0151, 0x0150}, {0x0153, 0x0152}, {0x0155, 0x0154}, {0x0157, 0x0156},
    {0x0159, 0x0158}, {0x015B, 0x015A}, {0x015D, 0x015C}, {0x015F, 0x015E}, {0x0161, 0x0160},
    {0x0163, 0x0162}, {0x0165, 0x0164}, {0x0167, 0x0166}, {0x0169, 0x0168}, {0x016B, 0x016A},
    {0x016D, 0x016C}, {0x016F, 0x016E}, {0x0171, 0x0170}, {0x0173, 0x0172}, {0x0175, 0x0174},
    {0x0177, 0x0176}, {0x017A, 0x0179}, {0x017C, 0x017B}, {0x017E, 0x017D}, {0x01C6, 0x01C5},
    {0x01F3, 0x01F2}, {0x021B, 0x021A}, {0x03AC, 0x0386}, {0x03AD, 0x0388}, {0x03AE, 0x0389},
    {0x03AF, 0x038A}, {0x03B1, 0x0391}, {0x03B2, 0x0392}, {0x03B3, 0x0393}, {0x03B4, 0x0394},
    {0x03B5, 0x0395}, {0x03B6, 0x0396}, {0x03B7, 0x0397}, {0x03B8, 0x0398}, {0x03B9, 0x0399},
    {0x03BA, 0x039A}, {0x03BB, 0x039B}, {0x03BC, 0x039C}, {0x03BD, 0x039D}, {0x03BE, 0x039E},
    {0x03BF, 0x039F}, {0x03C0, 0x03A0}, {0x03C1, 0x03A1}, {0x03C3, 0x03A3}, {0x03C4, 0x03A4},
    {0x03C5, 0x03A5}, {0x03C6, 0x03A6}, {0x03C7, 0x03A7}, {0x03C8, 0x03A8}, {0x03C9, 0x03A9},
    {0x03CA, 0x03AA}, {0x03CB, 0x03AB}, {0x03CC, 0x038C}, {0x03CD, 0x038E}, {0x03CE, 0x038F},
    {0x0430, 0x0410}, {0x0431, 0x0411}, {0x0432, 0x0412}, {0x0433, 0x0413}, {0x0434, 0x0414},
    {0x0435, 0x0415}, {0x0436, 0x0416}, {0x0437, 0x0417}, {0x0438, 0x0418}, {0x0439, 0x0419},
    {0x043A, 0x041A}, {0x043B, 0x041B}, {0x043C, 0x041C}, {0x043D, 0x041D}, {0x043E, 0x041E},
    {0x043F, 0x041F}, {0x0440, 0x0420}, {0x0441, 0x0421}, {0x0442, 0x0422}, {0x0443, 0x0423},
    {0x0444, 0x0424}, {0x0445, 0x0425}, {0x0446, 0x0426}, {0x0447, 0x0427}, {0x0448, 0x0428},
    {0x0449, 0x0429}, {0x044A, 0x042A}, {0x044B, 0x042B}, {0x044C, 0x042C}, {0x044D, 0x042D},
    {0x044E, 0x042E}, {0x044F, 0x042F}, {0x0451, 0x0401},
};

static const Char16Pair cToLowerTable[] = {
    {0x0041, 0x0061}, {0x0042, 0x0062}, {0x0043, 0x0063}, {0x0044, 0x0064}, {0x0045, 0x0065},
    {0x0046, 0x0066}, {0x0047, 0x0067}, {0x0048, 0x0068}, {0x0049, 0x0069}, {0x004A, 0x006A},
    {0x004B, 0x006B}, {0x004C, 0x006C}, {0x004D, 0x006D}, {0x004E, 0x006E}, {0x004F, 0x006F},
    {0x0050, 0x0070}, {0x0051, 0x0071}, {0x0052, 0x0072}, {0x0053, 0x0073}, {0x0054, 0x0074},
    {0x0055, 0x0075}, {0x0056, 0x0076}, {0x0057, 0x0077}, {0x0058, 0x0078}, {0x0059, 0x0079},
    {0x005A, 0x007A}, {0x00C0, 0x00E0}, {0x00C1, 0x00E1}, {0x00C2, 0x00E2}, {0x00C3, 0x00E3},
    {0x00C4, 0x00E4}, {0x00C5, 0x00E5}, {0x00C6, 0x00E6}, {0x00C7, 0x00E7}, {0x00C8, 0x00E8},
    {0x00C9, 0x00E9}, {0x00CA, 0x00EA}, {0x00CB, 0x00EB}, {0x00CC, 0x00EC}, {0x00CD, 0x00ED},
    {0x00CE, 0x00EE}, {0x00CF, 0x00EF}, {0x00D0, 0x00F0}, {0x00D1, 0x00F1}, {0x00D2, 0x00F2},
    {0x00D3, 0x00F3}, {0x00D4, 0x00F4}, {0x00D5, 0x00F5}, {0x00D6, 0x00F6}, {0x00D8, 0x00F8},
    {0x00D9, 0x00F9}, {0x00DA, 0x00FA}, {0x00DB, 0x00FB}, {0x00DC, 0x00FC}, {0x00DD, 0x00FD},
    {0x00DE, 0x00FE}, {0x0100, 0x0101}, {0x0102, 0x0103}, {0x0104, 0x0105}, {0x0106, 0x0107},
    {0x0108, 0x0109}, {0x010A, 0x010B}, {0x010C, 0x010D}, {0x010E, 0x010F}, {0x0110, 0x0111},
    {0x0112, 0x0113}, {0x0114, 0x0115}, {0x0116, 0x0117}, {0x0118, 0x0119}, {0x011A, 0x011B},
    {0x011C, 0x011D}, {0x011E, 0x011F}, {0x0120, 0x0121}, {0x0122, 0x0123}, {0x0124, 0x0125},
    {0x0126, 0x0127}, {0x0128, 0x0129}, {0x012A, 0x012B}, {0x012C, 0x012D}, {0x012E, 0x012F},
    {0x0130, 0x0131}, {0x0132, 0x0133}, {0x0134, 0x0135}, {0x0136, 0x0137}, {0x0139, 0x013A},
    {0x013B, 0x013C}, {0x013D, 0x013E}, {0x013F, 0x0140}, {0x0141, 0x0142}, {0x0143, 0x0144},
    {0x0145, 0x0146}, {0x0147, 0x0148}, {0x014A, 0x014B}, {0x014C, 0x014D}, {0x014E, 0x014F},
    {0x0150, 0x0151}, {0x0152, 0x0153}, {0x0154, 0x0155}, {0x0156, 0x0157}, {0x0158, 0x0159},
    {0x015A, 0x015B}, {0x015C, 0x015D}, {0x015E, 0x015F}, {0x0160, 0x0161}, {0x0162, 0x0163},
    {0x0164, 0x0165}, {0x0166, 0x0167}, {0x0168, 0x0169}, {0x016A, 0x016B}, {0x016C, 0x016D},
    {0x016E, 0x016F}, {0x0170, 0x0171}, {0x0172, 0x0173}, {0x0174, 0x0175}, {0x0176, 0x0177},
    {0x0178, 0x00FF}, {0x0179, 0x017A}, {0x017B, 0x017C}, {0x017D, 0x017E}, {0x01C5, 0x01C6},
    {0x01F2, 0x01F3}, {0x021A, 0x021B}, {0x0386, 0x03AC}, {0x0388, 0x03AD}, {0x0389, 0x03AE},
    {0x038A, 0x03AF}, {0x038C, 0x03CC}, {0x038E, 0x03CD}, {0x038F, 0x03CE}, {0x0391, 0x03B1},
    {0x0392, 0x03B2}, {0x0393, 0x03B3}, {0x0394, 0x03B4}, {0x0395, 0x03B5}, {0x0396, 0x03B6},
    {0x0397, 0x03B7}, {0x0398, 0x03B8}, {0x0399, 0x03B9}, {0x039A, 0x03BA}, {0x039B, 0x03BB},
    {0x039C, 0x03BC}, {0x039D, 0x03BD}, {0x039E, 0x03BE}, {0x039F, 0x03BF}, {0x03A0, 0x03C0},
    {0x03A1, 0x03C1}, {0x03A3, 0x03C3}, {0x03A4, 0x03C4}, {0x03A5, 0x03C5}, {0x03A6, 0x03C6},
    {0x03A7, 0x03C7}, {0x03A8, 0x03C8}, {0x03A9, 0x03C9}, {0x03AA, 0x03CA}, {0x03AB, 0x03CB},
    {0x0401, 0x0451}, {0x0410, 0x0430}, {0x0411, 0x0431}, {0x0412, 0x0432}, {0x0413, 0x0433},
    {0x0414, 0x0434}, {0x0415, 0x0435}, {0x0416, 0x0436}, {0x0417, 0x0437}, {0x0418, 0x0438},
    {0x0419, 0x0439}, {0x041A, 0x043A}, {0x041B, 0x043B}, {0x041C, 0x043C}, {0x041D, 0x043D},
    {0x041E, 0x043E}, {0x041F, 0x043F}, {0x0420, 0x0440}, {0x0421, 0x0441}, {0x0422, 0x0442},
    {0x0423, 0x0443}, {0x0424, 0x0444}, {0x0425, 0x0445}, {0x0426, 0x0446}, {0x0427, 0x0447},
    {0x0428, 0x0448}, {0x0429, 0x0449}, {0x042A, 0x044A}, {0x042B, 0x044B}, {0x042C, 0x044C},
    {0x042D, 0x044D}, {0x042E, 0x044E}, {0x042F, 0x044F},
};

static s32 toDecimalDigit_(char c)
{
    return c <= '9' ? c - '0' : -1;
}

static s32 toHexDigit_(s32 c)
{
    if (c <= '9')
    {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    return -1;
}

template <typename T>
static bool tryParseDecimal_(T* pOut, const SafeString& rStr, u32& rIdx, s32 sign)
{
    T value = 0;
    bool isParsed = false;
    for (;;)
    {
        const s32 digit = toDecimalDigit_(rStr[rIdx]);
        if (digit < 0)
        {
            break;
        }

        if (value < std::numeric_limits<T>::lowest() / 10 ||
            value > std::numeric_limits<T>::max() / 10)
        {
            return false;
        }

        const T next = value * 10;
        if (sign < 0)
        {
            if (next < std::numeric_limits<T>::lowest() + digit)
            {
                return false;
            }
        }
        else
        {
            if (next > std::numeric_limits<T>::max() - digit)
            {
                return false;
            }
        }

        value = next + digit * sign;
        ++rIdx;
        isParsed = true;
    }

    if (!isParsed)
    {
        return false;
    }

    if (pOut)
    {
        *pOut = value;
    }
    return true;
}

// NON_MATCHING: ~86-92%; sign/base prefix blocks are laid out differently and the 8/16-bit
// instantiations keep the non-decimal accumulator truncated
template <typename T>
static bool tryParseInteger_(T* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    using U = typename std::make_unsigned<T>::type;

    s32 sign;
    u32 idx;
    const char first = rStr[0];
    if (first == '-')
    {
        sign = -1;
        idx = 1;
    }
    else if (first == '+')
    {
        sign = 1;
        idx = 1;
    }
    else
    {
        idx = 0;
        sign = 1;
    }

    s32 base = static_cast<s32>(cardinalNumber);
    if (base <= 0)
    {
        const char c = rStr[idx];
        if (c < '0' || c > '9')
        {
            return false;
        }

        if (c == '0')
        {
            const char prefix = rStr[idx + 1];
            if (prefix == 'x')
            {
                idx += 2;
                base = 16;
            }
            else if (prefix == 'b')
            {
                idx += 2;
                base = 2;
            }
            else
            {
                base = 8;
            }
        }
        else
        {
            base = 10;
        }
    }

    if (base == 10)
    {
        return tryParseDecimal_(pOut, rStr, idx, sign);
    }

    const U max = std::numeric_limits<U>::max();
    const U limit = max / base;
    U value = 0;
    bool isParsed = false;
    for (;;)
    {
        const s32 digit = toHexDigit_(static_cast<char>(std::tolower(rStr[idx])));
        if (digit < 0 || digit >= base)
        {
            break;
        }

        if (value > limit)
        {
            return false;
        }

        const U next = value * base;
        if (next > max - digit)
        {
            return false;
        }

        value = next + digit;
        ++idx;
        isParsed = true;
    }

    if (!isParsed)
    {
        return false;
    }

    *pOut = value * sign;
    return true;
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether the whole leading number was parsed without overflow
 */
template <>
bool tryParseNumber<u8>(u8* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseU8(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success (not null-checked for bases other than 10)
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether at least one digit was parsed without overflow
 */
bool tryParseU8(u8* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseInteger_(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether the whole leading number was parsed without overflow
 */
template <>
bool tryParseNumber<s8>(s8* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseS8(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success (not null-checked for bases other than 10)
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether at least one digit was parsed without overflow
 */
bool tryParseS8(s8* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseInteger_(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether the whole leading number was parsed without overflow
 */
template <>
bool tryParseNumber<u16>(u16* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseU16(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success (not null-checked for bases other than 10)
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether at least one digit was parsed without overflow
 */
bool tryParseU16(u16* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseInteger_(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether the whole leading number was parsed without overflow
 */
template <>
bool tryParseNumber<s16>(s16* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseS16(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success (not null-checked for bases other than 10)
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether at least one digit was parsed without overflow
 */
bool tryParseS16(s16* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseInteger_(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether the whole leading number was parsed without overflow
 */
template <>
bool tryParseNumber<u32>(u32* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseU32(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success (not null-checked for bases other than 10)
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether at least one digit was parsed without overflow
 */
bool tryParseU32(u32* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseInteger_(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether the whole leading number was parsed without overflow
 */
template <>
bool tryParseNumber<s32>(s32* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseS32(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success (not null-checked for bases other than 10)
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether at least one digit was parsed without overflow
 */
bool tryParseS32(s32* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseInteger_(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether the whole leading number was parsed without overflow
 */
template <>
bool tryParseNumber<u64>(u64* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseU64(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success (not null-checked for bases other than 10)
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether at least one digit was parsed without overflow
 */
bool tryParseU64(u64* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseInteger_(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether the whole leading number was parsed without overflow
 */
template <>
bool tryParseNumber<s64>(s64* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseS64(pOut, rStr, cardinalNumber);
}

/**
 * Parses an integer, detecting a 0b/0x/0 prefix when the base is BaseAuto.
 * @param pOut receives the parsed value on success (not null-checked for bases other than 10)
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return whether at least one digit was parsed without overflow
 */
bool tryParseS64(s64* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseInteger_(pOut, rStr, cardinalNumber);
}

/**
 * Parses a decimal floating point number.
 * @param pOut receives the parsed value on success
 * @param rStr string to parse
 * @param cardinalNumber unused
 * @return whether a number was parsed
 */
template <>
bool tryParseNumber<f32>(f32* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseF32(pOut, rStr);
}

/**
 * Parses a decimal floating point number.
 * @param pOut receives the parsed value on success
 * @param rStr string to parse
 * @param cardinalNumber unused
 * @return whether a number was parsed
 */
template <>
bool tryParseNumber<f64>(f64* pOut, const SafeString& rStr, CardinalNumber cardinalNumber)
{
    return tryParseF64(pOut, rStr);
}

/**
 * Parses a number, returning 0 on failure.
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return the parsed value
 */
template <>
u8 parseNumber<u8>(const SafeString& rStr, CardinalNumber cardinalNumber)
{
    u8 value = 0;
    tryParseNumber(&value, rStr, cardinalNumber);
    return value;
}

/**
 * Parses a number, returning 0 on failure.
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return the parsed value
 */
template <>
s8 parseNumber<s8>(const SafeString& rStr, CardinalNumber cardinalNumber)
{
    s8 value = 0;
    tryParseNumber(&value, rStr, cardinalNumber);
    return value;
}

/**
 * Parses a number, returning 0 on failure.
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return the parsed value
 */
template <>
u16 parseNumber<u16>(const SafeString& rStr, CardinalNumber cardinalNumber)
{
    u16 value = 0;
    tryParseNumber(&value, rStr, cardinalNumber);
    return value;
}

/**
 * Parses a number, returning 0 on failure.
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return the parsed value
 */
template <>
s16 parseNumber<s16>(const SafeString& rStr, CardinalNumber cardinalNumber)
{
    s16 value = 0;
    tryParseNumber(&value, rStr, cardinalNumber);
    return value;
}

/**
 * Parses a number, returning 0 on failure.
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return the parsed value
 */
template <>
u32 parseNumber<u32>(const SafeString& rStr, CardinalNumber cardinalNumber)
{
    u32 value = 0;
    tryParseNumber(&value, rStr, cardinalNumber);
    return value;
}

/**
 * Parses a number, returning 0 on failure.
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return the parsed value
 */
template <>
s32 parseNumber<s32>(const SafeString& rStr, CardinalNumber cardinalNumber)
{
    s32 value = 0;
    tryParseNumber(&value, rStr, cardinalNumber);
    return value;
}

/**
 * Parses a number, returning 0 on failure.
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return the parsed value
 */
template <>
u64 parseNumber<u64>(const SafeString& rStr, CardinalNumber cardinalNumber)
{
    u64 value = 0;
    tryParseNumber(&value, rStr, cardinalNumber);
    return value;
}

/**
 * Parses a number, returning 0 on failure.
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return the parsed value
 */
template <>
s64 parseNumber<s64>(const SafeString& rStr, CardinalNumber cardinalNumber)
{
    s64 value = 0;
    tryParseNumber(&value, rStr, cardinalNumber);
    return value;
}

/**
 * Parses a number, returning 0 on failure.
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return the parsed value
 */
template <>
f32 parseNumber<f32>(const SafeString& rStr, CardinalNumber cardinalNumber)
{
    f32 value = 0;
    tryParseNumber(&value, rStr, cardinalNumber);
    return value;
}

/**
 * Parses a number, returning 0 on failure.
 * @param rStr string to parse
 * @param cardinalNumber base of the number
 * @return the parsed value
 */
template <>
f64 parseNumber<f64>(const SafeString& rStr, CardinalNumber cardinalNumber)
{
    f64 value = 0;
    tryParseNumber(&value, rStr, cardinalNumber);
    return value;
}

/**
 * Copies a null-terminated UTF-16 string, truncating it to fit.
 * @param pDst destination buffer
 * @param n size of the destination buffer in characters
 * @param pSrc source string
 * @return pDst
 */
char16* wcs16cpy(char16* pDst, size_t n, const char16* pSrc)
{
    if (n == 0)
    {
        return pDst;
    }

    size_t i = 0;
    for (; i < n - 1; ++i)
    {
        if (pSrc[i] == 0)
        {
            break;
        }
        pDst[i] = pSrc[i];
    }
    pDst[i] = 0;
    return pDst;
}

/**
 * Formats a UTF-16 string into a buffer.
 * @param pDst destination buffer
 * @param n size of the destination buffer in characters
 * @param pFormat format string
 * @return formatted length (see vsw16printf)
 */
s32 sw16printf(char16* pDst, size_t n, const char16* pFormat, ...)
{
    std::va_list args;
    va_start(args, pFormat);
    const s32 ret = vsw16printf(pDst, n, pFormat, args);
    va_end(args);
    return ret;
}

/**
 * Formats a string into a buffer, always terminating it.
 * @param pDst destination buffer
 * @param n size of the destination buffer
 * @param pFormat format string
 * @param args format arguments
 * @return length written (clamped to n - 1), or -1 if n is 0
 */
s32 vsnprintf(char* pDst, size_t n, const char* pFormat, std::va_list args)
{
    if (n == 0)
    {
        return -1;
    }

    const s32 ret = nn::util::VSNPrintf(pDst, n, pFormat, args);
    if (ret < 0 || static_cast<size_t>(ret) >= n)
    {
        pDst[n - 1] = SafeString::cNullChar;
    }
    return static_cast<size_t>(ret) < n ? ret : n - 1;
}

/**
 * Formats a string into a buffer, always terminating it.
 * @param pDst destination buffer
 * @param n size of the destination buffer
 * @param pFormat format string
 * @return length written (clamped to n - 1), or -1 if n is 0
 */
s32 snprintf(char* pDst, size_t n, const char* pFormat, ...)
{
    std::va_list args;
    va_start(args, pFormat);
    const s32 ret = vsnprintf(pDst, n, pFormat, args);
    va_end(args);
    return ret;
}

/**
 * Converts a Sjis string to Utf16.
 * @param pDst destination buffer
 * @param dstLength size of the destination buffer in characters
 * @param pSrc source string
 * @param srcLength length of the source string (-1 for null-terminated)
 * @return number of characters written
 */
s32 convertSjisToUtf16(char16* pDst, u32 dstLength, const char* pSrc, s32 srcLength)
{
    s32 length = 0;
    tryConvertSjisToUtf16(&length, pDst, dstLength, pSrc, srcLength);
    return length;
}

/**
 * Converts a Utf16 string to Sjis.
 * @param pDst destination buffer
 * @param dstLength size of the destination buffer in characters
 * @param pSrc source string
 * @param srcLength length of the source string (-1 for null-terminated)
 * @return number of characters written
 */
s32 convertUtf16ToSjis(char* pDst, u32 dstLength, const char16* pSrc, s32 srcLength)
{
    s32 length = 0;
    tryConvertUtf16ToSjis(&length, pDst, dstLength, pSrc, srcLength);
    return length;
}

/**
 * Converts a Utf8 string to Utf16.
 * @param pDst destination buffer
 * @param dstLength size of the destination buffer in characters
 * @param pSrc source string
 * @param srcLength length of the source string (-1 for null-terminated)
 * @return number of characters written
 */
s32 convertUtf8ToUtf16(char16* pDst, u32 dstLength, const char* pSrc, s32 srcLength)
{
    s32 length = 0;
    tryConvertUtf8ToUtf16(&length, pDst, dstLength, pSrc, srcLength);
    return length;
}

/**
 * Converts a Sjis string to Utf8.
 * @param pDst destination buffer
 * @param dstLength size of the destination buffer in characters
 * @param pSrc source string
 * @param srcLength length of the source string (-1 for null-terminated)
 * @return number of characters written
 */
s32 convertSjisToUtf8(char* pDst, u32 dstLength, const char* pSrc, s32 srcLength)
{
    s32 length = 0;
    tryConvertSjisToUtf8(&length, pDst, dstLength, pSrc, srcLength);
    return length;
}

/**
 * Converts a Utf8 string to Sjis.
 * @param pDst destination buffer
 * @param dstLength size of the destination buffer in characters
 * @param pSrc source string
 * @param srcLength length of the source string (-1 for null-terminated)
 * @return number of characters written
 */
s32 convertUtf8ToSjis(char* pDst, u32 dstLength, const char* pSrc, s32 srcLength)
{
    s32 length = 0;
    tryConvertUtf8ToSjis(&length, pDst, dstLength, pSrc, srcLength);
    return length;
}

/**
 * Converts the first character of a string to upper case (ASCII only).
 * @param pStr string to modify
 */
void toUpperCapitalFirstCharactor(BufferedSafeString* pStr)
{
    char* buffer = pStr->getBuffer();
    if (buffer[0] != '\0')
    {
        buffer[0] = toUpperCapital(buffer[0]);
    }
}

/**
 * Converts the first character of a string to lower case (ASCII only).
 * @param pStr string to modify
 */
void toLowerCapitalFirstCharactor(BufferedSafeString* pStr)
{
    char* buffer = pStr->getBuffer();
    if (buffer[0] != '\0')
    {
        buffer[0] = toLowerCapital(buffer[0]);
    }
}

/**
 * Compares the source character of a replacement pair with a character.
 * @param rPair replacement pair
 * @param rKey character to compare with
 * @return difference between the pair's source character and rKey
 */
s32 compareChar16Pair(const Char16Pair& rPair, const char16& rKey)
{
    return rPair.before - rKey;
}

/**
 * Replaces a character using a table sorted by source character.
 * @param c character to replace
 * @param rSortedTable replacement pairs sorted by their source character
 * @return the replacement, or c if it is not in the table
 */
char16 replace(char16 c, const Buffer<const Char16Pair>& rSortedTable)
{
    const s32 idx = rSortedTable.binarySearch(c, compareChar16Pair);
    if (idx < 0)
    {
        return c;
    }

    return rSortedTable[idx].after;
}

/**
 * Converts a character to upper case using the built-in conversion table.
 * @param c character to convert
 * @return the upper case character, or c if it has none
 */
char16 toUpperCapital(char16 c)
{
    return replace(c, Buffer<const Char16Pair>(cToUpperTable));
}

/**
 * Converts the first character of a string to upper case.
 * @param pStr string to modify
 */
void toUpperCapitalFirstCharactor(WBufferedSafeString* pStr)
{
    char16* buffer = pStr->getBuffer();
    if (buffer[0] != 0)
    {
        buffer[0] = toUpperCapital(buffer[0]);
    }
}

/**
 * Converts a character to lower case using the built-in conversion table.
 * @param c character to convert
 * @return the lower case character, or c if it has none
 */
char16 toLowerCapital(char16 c)
{
    return replace(c, Buffer<const Char16Pair>(cToLowerTable));
}

/**
 * Converts the first character of a string to lower case.
 * @param pStr string to modify
 */
void toLowerCapitalFirstCharactor(WBufferedSafeString* pStr)
{
    char16* buffer = pStr->getBuffer();
    if (buffer[0] != 0)
    {
        buffer[0] = toLowerCapital(buffer[0]);
    }
}

// NON_MATCHING: ~97%; the fraction loop index is not widened to 64 bits like in the target
template <typename T>
static bool tryParseFloat_(T* pOut, const SafeString& rStr)
{
    u32 idx = 0;
    s32 sign = 1;
    const char first = rStr[0];
    if (first == '+')
    {
        idx = 1;
    }
    else if (first == '-')
    {
        idx = 1;
        sign = -1;
    }

    if (!tryParseDecimal_(pOut, rStr, idx, sign))
    {
        return false;
    }

    if (rStr[idx] != '.')
    {
        return true;
    }

    ++idx;
    const T integer = *pOut;
    T divisor = 1;
    T fraction = 0;
    for (;;)
    {
        const s32 digit = toDecimalDigit_(rStr[idx]);
        if (digit < 0)
        {
            break;
        }
        divisor *= 10;
        fraction = fraction * 10 + static_cast<T>(sign) * digit;
        ++idx;
    }

    if (pOut && divisor != 1)
    {
        *pOut = integer + fraction / divisor;
    }
    return true;
}

/**
 * Parses a decimal single precision number with an optional fractional part.
 * @param pOut receives the parsed value
 * @param rStr string to parse
 * @return whether the integer part was parsed without overflow
 */
bool tryParseF32(f32* pOut, const SafeString& rStr)
{
    return tryParseFloat_(pOut, rStr);
}

/**
 * Parses a decimal double precision number with an optional fractional part.
 * @param pOut receives the parsed value
 * @param rStr string to parse
 * @return whether the integer part was parsed without overflow
 */
bool tryParseF64(f64* pOut, const SafeString& rStr)
{
    return tryParseFloat_(pOut, rStr);
}
}  // namespace sead::StringUtil
