#include "random/seadRandom.h"
#include "time/seadTickTime.h"

namespace sead
{
/**
 * Seeds the generator from the current tick.
 */
void Random::init()
{
    TickTime now;
    init(static_cast<u32>(now.toTicks()));
}

/**
 * Seeds the four xorshift words from one value.
 * @param seed seed value
 */
void Random::init(u32 seed)
{
    const u32 mtConstant = 0x6C078965;
    mX = mtConstant * (seed ^ (seed >> 30u)) + 1;
    mY = mtConstant * (mX ^ (mX >> 30u)) + 2;
    mZ = mtConstant * (mY ^ (mY >> 30u)) + 3;
    mW = mtConstant * (mZ ^ (mZ >> 30u)) + 4;
}

/**
 * Sets the four xorshift words directly (not all zero; zero seeds fall back to fixed values).
 * @param seedX first word
 * @param seedY second word
 * @param seedZ third word
 * @param seedW fourth word
 */
void Random::init(u32 seedX, u32 seedY, u32 seedZ, u32 seedW)
{
    if ((seedX | seedY | seedZ | seedW) == 0)
    {
        SEAD_ASSERT_MSG(false, "seeds must not be all zero.");
        seedW = 0x48077044;
        seedZ = 0x714ACB41;
        seedY = 0x6C078967;
        seedX = 1;
    }
    mX = seedX;
    mY = seedY;
    mZ = seedZ;
    mW = seedW;
}

/**
 * @return the next 32 random bits (xorshift128)
 */
u32 Random::getU32()
{
    u32 x = mX ^ (mX << 11u);
    mX = mY;
    mY = mZ;
    mZ = mW;
    mW = mW ^ (mW >> 19u) ^ x ^ (x >> 8u);
    return mW;
}

/**
 * @return the next 64 random bits
 */
u64 Random::getU64()
{
    return u64(getU32()) << 32u | getU32();
}

/**
 * Reads the generator state, e.g. to restore it later with init(x, y, z, w).
 * @param pX receives the first word
 * @param pY receives the second word
 * @param pZ receives the third word
 * @param pW receives the fourth word
 */
void Random::getContext(u32* pX, u32* pY, u32* pZ, u32* pW) const
{
    *pX = mX;
    *pY = mY;
    *pZ = mZ;
    *pW = mW;
}
}  // namespace sead
