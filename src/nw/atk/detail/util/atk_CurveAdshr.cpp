#include <nn/atk/atk_CurveAdshr.h>
#include <nn/atk/atk_CurveLfo.h>
#include <nn/atk/atk_Util.h>
#include <nn/util/util_Arithmetic.h>
#include <cstring>

namespace nn::atk::detail {
namespace {
const float AttackTable[128] = {
    0.99921751f,  0.998432577f, 0.997645199f, 0.996855319f, 0.996062875f, 0.995267928f, 0.994470417f,
    0.993670404f, 0.992867708f, 0.992062509f, 0.991254628f, 0.990444124f, 0.989630878f, 0.988815129f,
    0.987996519f, 0.987175226f, 0.986351192f, 0.985524416f, 0.984694898f, 0.983862519f, 0.983027279f,
    0.982189298f, 0.981348276f, 0.980504513f, 0.979657829f, 0.978808105f, 0.97795552f,  0.977099895f,
    0.976241291f, 0.975379705f, 0.974515021f, 0.973647177f, 0.972776294f, 0.971902311f, 0.971025109f,
    0.970144808f, 0.969261229f, 0.968374372f, 0.967484415f, 0.966591001f, 0.965694427f, 0.964794397f,
    0.963891029f, 0.962984204f, 0.962073982f, 0.961160421f, 0.960243285f, 0.959322572f, 0.958398402f,
    0.957470596f, 0.956539214f, 0.955604196f, 0.954665482f, 0.953723073f, 0.952776909f, 0.95182699f,
    0.950873196f, 0.949915707f, 0.948954225f, 0.947988808f, 0.947019517f, 0.946046174f, 0.945068896f,
    0.944087505f, 0.943102002f, 0.942112386f, 0.941118598f, 0.940120578f, 0.939118385f, 0.938111782f,
    0.937100887f, 0.936085582f, 0.935065925f, 0.934041679f, 0.933013082f, 0.931979775f, 0.930941999f,
    0.929899514f, 0.92885232f,  0.927800417f, 0.926743627f, 0.925682127f, 0.924615622f, 0.923544228f,
    0.922467828f, 0.921386421f, 0.920299828f, 0.919208109f, 0.918111205f, 0.917009115f, 0.915901601f,
    0.914788723f, 0.913670301f, 0.912546515f, 0.911417127f, 0.910282075f, 0.909141421f, 0.907994926f,
    0.906842709f, 0.905684471f, 0.904520392f, 0.903350174f, 0.902173996f, 0.900991619f, 0.899802923f,
    0.898608029f, 0.897406578f, 0.896198809f, 0.894984424f, 0.890059888f, 0.882462204f, 0.875924706f,
    0.869186103f, 0.863640606f, 0.853578806f, 0.843018889f, 0.82861352f,  0.814909875f, 0.800217211f,
    0.778066278f, 0.755474985f, 0.724212527f, 0.682823896f, 0.632916927f, 0.559213519f, 0.455141097f,
    0.329876989f, 0.0f,
};

/**
 * @brief Samples the sine oscillator with the shared trigonometric lookup table.
 * @param phase Normalized oscillator phase, in the range [0, 1).
 * @return Interpolated sine value at the supplied phase.
 */
float SineCurve(float phase) { return nn::util::SinTable(static_cast<u32>(phase * 4294967296.0f)); }

/**
 * @brief Samples the piecewise linear triangle oscillator.
 * @param phase Normalized oscillator phase, in the range [0, 1].
 * @return Triangle wave value at the supplied phase.
 */
float TriangleCurve(float phase) {
    static const float slopes[] = {4.0f, -4.0f, -4.0f, 4.0f, 4.0f};
    static const float offsets[] = {0.0f, 2.0f, 2.0f, -4.0f, 0.0f};
    u32 index = static_cast<u32>(phase * 4.0f);
    return slopes[index] * phase + offsets[index];
}

/**
 * @brief Samples the rising sawtooth oscillator.
 * @param phase Normalized oscillator phase, in the range [0, 1).
 * @return The phase itself.
 */
float SawtoothCurve(float phase) { return phase; }

/**
 * @brief Samples the two-level square oscillator.
 * @param phase Normalized oscillator phase, in the range [0, 1).
 * @return Zero during the first half of the period and one during the second half.
 */
float SquareCurve(float phase) { return phase < 0.5f ? 0.0f : 1.0f; }

/**
 * @brief Produces a random oscillator sample.
 * @param phase Unused; the output depends only on the shared random generator.
 * @return A random sample in the inclusive range [0, 1].
 */
float RandomCurve(float phase) { return (Util::CalcRandom() & 0xffff) / 65535.0f; }
} // namespace

/** @brief Constructs an envelope at its minimum level with immediate attack. */
CurveAdshr::CurveAdshr() { Initialize(-90.4f); }

/**
 * @brief Restores the envelope parameters and starts the attack stage.
 * @param decibels Initial envelope level, expressed in decibels.
 */
void CurveAdshr::Initialize(float decibels) {
    mAttack = 0.0f;
    mHold = 0;
    mDecay = 65535.0f;
    mSustain = 127;
    mRelease = 65535.0f;
    Reset(decibels);
}

/**
 * @brief Selects the attack multiplier from the envelope rate table.
 * @param attack Attack parameter, in the range [0, 127].
 */
void CurveAdshr::SetAttack(int attack) { mAttack = AttackTable[attack]; }

/**
 * @brief Converts the hold parameter to its squared duration.
 * @param hold Hold parameter, in the range [0, 127].
 */
void CurveAdshr::SetHold(int hold) { mHold = static_cast<u32>((hold + 1) * (hold + 1)) / 4; }

/**
 * @brief Sets the decay rate toward the sustain level.
 * @param decay Decay parameter, in the range [0, 127].
 */
void CurveAdshr::SetDecay(int decay) { mDecay = CalcRelease(decay); }

/**
 * @brief Sets the sustain level reached after decay.
 * @param sustain Sustain table index, in the range [0, 127].
 */
void CurveAdshr::SetSustain(int sustain) { mSustain = sustain; }

/**
 * @brief Sets the rate at which the envelope is released.
 * @param release Release parameter, in the range [0, 127].
 */
void CurveAdshr::SetRelease(int release) { mRelease = CalcRelease(release); }

/**
 * @brief Restarts the attack stage at the requested level.
 * @param decibels Initial envelope level, expressed in decibels.
 */
void CurveAdshr::Reset(float decibels) {
    mValue = decibels * 10.0f;
    mState = Attack;
}

/**
 * @brief Reads the envelope level, accounting for an instantaneous attack.
 * @return Current attenuation in decibels.
 */
float CurveAdshr::GetValue() const {
    if (mState == Attack && mAttack == 0.0f) {
        return 0.0f;
    }
    return mValue / 10.0f;
}

/**
 * @brief Advances the current envelope stage by the requested duration.
 * @param step Nonnegative number of envelope update intervals to advance.
 */
void CurveAdshr::Update(int step) {
    switch (mState) {
    case Attack:
        while (step-- > 0) {
            mValue = mAttack * mValue;
            if (mValue > -0.03125f) {
                mValue = 0.0f;
                mState = Hold;
                mHoldRemaining = mHold;
                break;
            }
        }
        break;
    case Hold: {
        int remaining = mHoldRemaining;
        if (remaining > step) {
            mHoldRemaining = remaining - step;
            break;
        }
        step -= mHoldRemaining;
        mHoldRemaining = 0;
        mState = Decay;
    }
        // Exhausting hold applies the remaining duration to decay.
    case Decay: {
        float sustain = CalcDecibelSquare(mSustain);
        mValue -= mDecay * step;
        if (mValue < sustain) {
            mValue = sustain;
            mState = Sustain;
        }
        break;
    }
    case Sustain:
        break;
    case Release:
        mValue -= mRelease * step;
        break;
    }
}

/**
 * @brief Looks up the squared-volume attenuation curve.
 * @param value Volume table index, in the range [0, 127].
 * @return Attenuation in tenths of a decibel.
 */
s16 CurveAdshr::CalcDecibelSquare(int value) { return DecibelSquareTable[value]; }

/**
 * @brief Converts an envelope decay or release parameter to a linear rate.
 * @param release Rate parameter, in the range [0, 127]; 127 means instantaneous.
 * @return Level reduction per envelope interval, in tenths of a decibel.
 */
float CurveAdshr::CalcRelease(int release) {
    if (release == 127) {
        return 65535.0f;
    }
    if (release == 126) {
        return 24.0f;
    }
    if (release < 50) {
        return ((release * 2 + 1) * (1.0f / 128)) / 5.0f;
    }
    return (60.0f / (126 - release)) / 5.0f;
}

/** @brief Clears user oscillator curves and installs the five built-in curves. */
void CurveLfo::InitializeCurveTable() {
    std::memset(g_CurveLfoTable + 5, 0, sizeof(g_CurveLfoTable) - 5 * sizeof(LfoCurveFunction));
    g_CurveLfoTable[0] = SineCurve;
    g_CurveLfoTable[1] = TriangleCurve;
    g_CurveLfoTable[2] = SawtoothCurve;
    g_CurveLfoTable[3] = SquareCurve;
    g_CurveLfoTable[4] = RandomCurve;
}
} // namespace nn::atk::detail
