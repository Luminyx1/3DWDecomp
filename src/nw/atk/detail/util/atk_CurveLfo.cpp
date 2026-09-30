#include <nn/atk/atk_CurveLfo.h>

namespace nn::atk::detail {
LfoCurveFunction g_CurveLfoTable[128];

// function replaces user curve index (0-63); returns the previous callback.
LfoCurveFunction CurveLfo::RegisterUserCurve(LfoCurveFunction function, u32 index) {
    int slot = index + 64;
    auto previous = g_CurveLfoTable[slot];
    g_CurveLfoTable[slot] = function;
    return previous;
}
// index selects the user curve (0-63) to clear; returns its previous callback.
LfoCurveFunction CurveLfo::UnregisterUserCurve(u32 index) {
    int slot = index + 64;
    auto previous = g_CurveLfoTable[slot];
    g_CurveLfoTable[slot] = nullptr;
    return previous;
}
void CurveLfoParam::Initialize() { depth = 0; speed = 6.25f; delay = 0; range = 1; curve = 0; phase = 0; }
void CurveLfo::Reset() {
    mRandomValue = 1;
    mPhase = 0;
    mWrapped = false;
    mElapsedDelay = 0;
    mStarted = false;
}
// step advances the delay and oscillator phase in milliseconds.
void CurveLfo::Update(int step) {
    if (mElapsedDelay < mParameter.delay) {
        if (mElapsedDelay + step > mParameter.delay) {
            step -= mParameter.delay - mElapsedDelay;
            mElapsedDelay = mParameter.delay;
        } else { mElapsedDelay += step; return; }
    }
    if (!(mParameter.speed > 0)) return;
    if (!mStarted) {
        mPhase = mParameter.phase / 127.0f;
        mStarted = true;
    }
    float phase = mPhase + mParameter.speed * step / 1000.0f;
    mWrapped = phase >= 1;
    mPhase = phase - static_cast<int>(phase);
}
float CurveLfo::GetValue() const {
    if (mParameter.depth == 0 || mElapsedDelay < mParameter.delay) return 0;
    auto function = g_CurveLfoTable[mParameter.curve];
    float value = 1;
    if (function) {
        if (mParameter.curve == 4) {
            if (mWrapped) mRandomValue = function(mPhase);
            value = mRandomValue;
        } else value = function(mPhase);
    }
    value *= mParameter.depth;
    value *= mParameter.range;
    return value;
}
}
