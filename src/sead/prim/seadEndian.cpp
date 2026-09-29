#include "prim/seadEndian.h"

namespace
{
/**
 * Returns the value unchanged.
 * @param value value to convert
 * @return converted value
 */
u8 Null8(u8 value)
{
    return value;
}

/**
 * Reverses the byte order of the value.
 * @param value value to convert
 * @return converted value
 */
u8 Swap8(u8 value)
{
    return value;
}

/**
 * Returns the value unchanged.
 * @param value value to convert
 * @return converted value
 */
u16 Null16(u16 value)
{
    return value;
}

/**
 * Reverses the byte order of the value.
 * @param value value to convert
 * @return converted value
 */
u16 Swap16(u16 value)
{
    return __builtin_bswap16(value);
}

/**
 * Returns the value unchanged.
 * @param value value to convert
 * @return converted value
 */
u32 Null32(u32 value)
{
    return value;
}

/**
 * Reverses the byte order of the value.
 * @param value value to convert
 * @return converted value
 */
u32 Swap32(u32 value)
{
    return __builtin_bswap32(value);
}

/**
 * Returns the value unchanged.
 * @param value value to convert
 * @return converted value
 */
u64 Null64(u64 value)
{
    return value;
}

/**
 * Reverses the byte order of the value.
 * @param value value to convert
 * @return converted value
 */
u64 Swap64(u64 value)
{
    return __builtin_bswap64(value);
}
}  // namespace

namespace sead
{
const Endian::Types Endian::cHostEndian = Endian::cLittle;

const Endian::ConvFuncTable Endian::cConvFuncTable = {
    {&Null8, &Swap8},
    {&Null16, &Swap16},
    {&Null32, &Swap32},
    {&Null64, &Swap64},
};
}  // namespace sead
