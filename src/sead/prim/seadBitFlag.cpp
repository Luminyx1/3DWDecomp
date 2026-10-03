#include <prim/seadBitFlag.h>

namespace sead
{
namespace
{
/**
 * @brief Finds the requested set bit by clearing lower set bits in succession.
 * @tparam T Unsigned 32-bit or 64-bit integer type.
 * @param x Bit field to search; zero has no set bits.
 * @param num One-based set-bit ordinal; callers require a positive value.
 * @return Zero-based bit position, or -1 when the requested bit does not exist.
 */
template <class T>
inline int findSetBitFromRight(T x, int num)
{
    while (x != 0)
    {
        if (num-- <= 1)
        {
            if constexpr (sizeof(T) == sizeof(u32))
            {
                return BitFlagUtil::countContinuousOffBitFromRight(x);
            }
            else
            {
                return BitFlagUtil::countContinuousOffBitFromRight64(x);
            }
        }
        x &= x - 1;
    }
    return -1;
}
}  // namespace

/**
 * @brief Counts the set bits (population count).
 * @param x value to count in
 * @return number of bits that are 1
 */
int BitFlagUtil::countOnBit(u32 x)
{
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x += (x >> 8);
    x += (x >> 16);
    return x & 0x3f;
}

/**
 * @brief Counts the leading zero bits.
 * @param x value to look at
 * @return number of 0 bits above the highest 1 bit (32 for 0)
 */
int BitFlagUtil::countContinuousOffBitFromLeft(u32 x)
{
    return __builtin_clz(x);
}

/**
 * @brief Counts the set bits at or below a position.
 * @param x value to count in
 * @param bit Highest bit position to include, from 0 to 31.
 * @return number of bits that are 1 in bits 0 to bit
 */
int BitFlagUtil::countRightOnBit(u32 x, int bit)
{
    SEAD_ASSERT(static_cast<u32>(bit) < sizeof(u32) * 8);
    const u32 mask = ((1u << bit) - 1) | (1u << bit);
    return countOnBit(x & mask);
}

/**
 * @brief Finds the position of the num-th set bit, counting from bit 0.
 * @param x value to search
 * @param num Positive one-based set-bit ordinal (1 for the lowest).
 * @return the bit position, or -1 if x has fewer set bits
 */
int BitFlagUtil::findOnBitFromRight(u32 x, int num)
{
    SEAD_ASSERT(num > 0);
    return findSetBitFromRight(x, num);
}

/**
 * @brief Counts the leading zero bits.
 * @param x value to look at
 * @return number of 0 bits above the highest 1 bit (64 for 0)
 */
int BitFlagUtil::countContinuousOffBitFromLeft64(u64 x)
{
    return __builtin_clzll(x);
}

/**
 * @brief Counts the set bits at or below a position.
 * @param x value to count in
 * @param bit Highest bit position to include, from 0 to 63.
 * @return number of bits that are 1 in bits 0 to bit
 */
int BitFlagUtil::countRightOnBit64(u64 x, int bit)
{
    SEAD_ASSERT(static_cast<u64>(bit) < sizeof(u64) * 8);
    const u64 mask = ((1ull << bit) - 1) | (1ull << bit);
    return countOnBit64(x & mask);
}

/**
 * @brief Finds the position of the num-th set bit, counting from bit 0.
 * @param x value to search
 * @param num Positive one-based set-bit ordinal (1 for the lowest).
 * @return the bit position, or -1 if x has fewer set bits
 */
int BitFlagUtil::findOnBitFromRight64(u64 x, int num)
{
    SEAD_ASSERT(num > 0);
    return findSetBitFromRight(x, num);
}

}  // namespace sead
