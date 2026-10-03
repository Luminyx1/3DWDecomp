#include "Project/Math/FractalGenerator.hpp"

#include "Library/Math/MathUtil.hpp"

namespace al {

/**
 * Constructs a fractal noise generator.
 * @param permutations Number of noise octaves plus one.
 * @param amplitude Amplitude of the first octave.
 * @param scale Coordinate scale of the first octave.
 * @param nextOrderAmplitude Amplitude multiplier applied per octave.
 */
FractalGenerator::FractalGenerator(u32 permutations, f32 amplitude, f32 scale,
                                   f32 nextOrderAmplitude)
    : mPermutations(permutations), mAmplitude(amplitude), mScale(scale),
      mNextOrderAmplitude(nextOrderAmplitude) {}

/**
 * Sets the generator parameters.
 * @param permutations Number of noise octaves plus one.
 * @param amplitude Amplitude of the first octave.
 * @param scale Coordinate scale of the first octave.
 * @param nextOrderAmplitude Amplitude multiplier applied per octave.
 */
void FractalGenerator::setParam(u32 permutations, f32 amplitude, f32 scale,
                                f32 nextOrderAmplitude) {
    mPermutations = permutations;
    mAmplitude = amplitude;
    mScale = scale;
    mNextOrderAmplitude = nextOrderAmplitude;
}

/**
 * Sums several octaves of Perlin noise.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param useSmoothPerlingNoise Whether to use the smoothed noise.
 * @return Fractal noise value.
 */
f32 FractalGenerator::calcFractal(f32 x, f32 y, bool useSmoothPerlingNoise) {
    f32 value = 0.0f;

    f32 amplitude = mAmplitude;
    f32 scale = mScale;

    for (s32 i = 0; i < static_cast<s32>(mPermutations - 1); i++) {
        if (useSmoothPerlingNoise) {
            value += makeSmoothPerlinNoise(scale * x, scale * y) * amplitude;
        } else {
            value += makePerlinNoise(scale * x, scale * y) * amplitude;
        }

        scale *= 2;
        amplitude *= mNextOrderAmplitude;
    }

    return value;
}

/**
 * Calculates Perlin noise from smoothed random values.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return Noise value.
 */
f32 FractalGenerator::makeSmoothPerlinNoise(f32 x, f32 y) {
    s32 gridX = static_cast<s32>(x);
    s32 gridY = static_cast<s32>(y);
    f32 fracX = x - gridX;
    f32 fracY = y - gridY;

    // NOTE: the original calls makeSmoothRandom on an undefined pointer. This works because
    // makeSmoothRandom does not use any members, and is required to match.
    FractalGenerator* gen;
    f32 r00 = gen->makeSmoothRandom(gridX, gridY);
    f32 r10 = gen->makeSmoothRandom(gridX + 1, gridY);
    f32 r01 = gen->makeSmoothRandom(gridX, gridY + 1);
    f32 r11 = gen->makeSmoothRandom(gridX + 1, gridY + 1);

    return cosInterpolation(fracY, cosInterpolation(fracX, r00, r10),
                            cosInterpolation(fracX, r01, r11));
}

/**
 * Calculates Perlin noise.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return Noise value.
 */
f32 FractalGenerator::makePerlinNoise(f32 x, f32 y) {
    s32 gridX = static_cast<s32>(x);
    s32 gridY = static_cast<s32>(y);
    f32 fracX = x - gridX;
    f32 fracY = y - gridY;

    f32 r00 = makeRandom(gridX, gridY);
    f32 r10 = makeRandom(gridX + 1, gridY);
    f32 r01 = makeRandom(gridX, gridY + 1);
    f32 r11 = makeRandom(gridX + 1, gridY + 1);

    return cosInterpolation(fracY, cosInterpolation(fracX, r00, r10),
                            cosInterpolation(fracX, r01, r11));
}

/**
 * Multiplies several octaves of Perlin noise.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param baseAmplitude Factor applied to every octave.
 * @param useSmoothPerlingNoise Whether to use the smoothed noise.
 * @return Multifractal noise value.
 */
f32 FractalGenerator::calcMultiFractal(f32 x, f32 y, f32 baseAmplitude,
                                       bool useSmoothPerlingNoise) {
    f32 value = 1.0f;

    f32 amplitude = mAmplitude;
    f32 scale = mScale;

    for (s32 i = 0; i < static_cast<s32>(mPermutations - 1); i++) {
        if (useSmoothPerlingNoise) {
            value *= baseAmplitude * makeSmoothPerlinNoise(scale * x, scale * y) * amplitude;
        } else {
            value *= baseAmplitude * makePerlinNoise(scale * x, scale * y) * amplitude;
        }

        scale *= 2;
        amplitude *= mNextOrderAmplitude;
    }

    return value;
}

/**
 * Hashes grid coordinates into a seed.
 * @param x Grid X coordinate.
 * @param y Grid Y coordinate.
 * @return Seed value.
 */
inline s32 makeSeed(s32 x, s32 y) {
    s32 seed = x + y * 57;
    seed ^= seed << 13;

    return seed;
}

/**
 * Turns a seed into a pseudo-random value.
 * @param seed Seed value.
 * @return Value in the range (-1, 0].
 */
inline f32 makeNoise(s32 seed) {
    // NOTE: The only true random are all these magic values
    s32 hash = (seed * seed * 0x4bd9 + 0x2e59b) * seed + 0x4bcb4b;
    return static_cast<f32>(hash & 0x4b746f) / -3354521.0f;
}

/**
 * Calculates a pseudo-random value for a grid point.
 * @param x Grid X coordinate.
 * @param y Grid Y coordinate.
 * @return Random value.
 */
f32 FractalGenerator::makeRandom(s32 x, s32 y) {
    return makeNoise(makeSeed(x, y)) + 1.0f;
}

/**
 * Calculates a random value for a grid point, blurred with its neighbors.
 * @param x Grid X coordinate.
 * @param y Grid Y coordinate.
 * @return Smoothed random value.
 */
f32 FractalGenerator::makeSmoothRandom(s32 x, s32 y) {
    constexpr f32 weightCorners = 0.25f;
    constexpr f32 weightSides = 0.5f;
    constexpr f32 weightCenter = 0.25f;

    f32 corners = (makeRandom(x - 1, y - 1) + makeRandom(x + 1, y - 1) + makeRandom(x - 1, y + 1) +
                   makeRandom(x + 1, y + 1)) *
                  ((1.0f / 4.0f) * weightCorners);
    f32 sides = (makeRandom(x - 1, y) + makeRandom(x + 1, y) + makeRandom(x, y - 1) +
                 makeRandom(x, y + 1)) *
                ((1.0f / 4.0f) * weightSides);
    f32 center = makeRandom(x, y) * weightCenter;

    return corners + sides + center;
}

}  // namespace al
