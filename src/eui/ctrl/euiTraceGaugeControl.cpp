#include <eui/euiTraceGaugeControl.h>
#include <eui/euiAnimator.h>
namespace eui {
const char* TraceGaugeControl::getClassName() const { return "TraceGaugeControl"; }
TraceGaugeControl::TraceGaugeControl()
    : mGaugeAnimator(nullptr), mTracingAnimator(nullptr), mTraceColorAnimator(nullptr),
      mShortageAnimator(nullptr), mGaugeValue(100), mPreviousValue(100), mTracingValue(100),
      mTracingSpeed(2), mTracingFraction(0), mShortageUpper(20), mShortageLower(0),
      mWaitRemaining(0), mTracingWait(0), mFlags(1) {}
// value is the target gauge percentage; values outside [0, 100] are ignored.
void TraceGaugeControl::setGaugeValue(float value) {
    if (value >= 0 && value <= 100) mGaugeValue = value;
}
// speed is the tracing percentage increment, constrained to [0, 100].
void TraceGaugeControl::setTracingSpeed(float speed) {
    if (speed >= 0 && speed <= 100) mTracingSpeed = speed;
}
// fraction is the proportion of the remaining difference traced each update, in [0, 1].
void TraceGaugeControl::setTracingFraction(float fraction) {
    if (fraction >= 0 && fraction <= 1) mTracingFraction = fraction;
}
// wait is a nonnegative delay before the tracing gauge starts moving.
void TraceGaugeControl::setTracingWait(float wait) {
    if (wait >= 0) mTracingWait = wait;
}
// upper and lower bound the shortage range, with 0 <= lower <= upper <= 100.
// NON_MATCHING: the compiler reorders the range checks.
void TraceGaugeControl::setShortageThreshold(float upper, float lower) {
    if (lower >= 0) {
        if (!(upper <= 100)) return;
        if (!(lower <= upper)) return;
        mShortageUpper = upper;
        mShortageLower = lower;
    }
}
// value immediately positions the tracing gauge and reapplies its animations.
void TraceGaugeControl::setTracingValue(float value) {
    mTracingValue = value;
    applyAnimation_();
}
void TraceGaugeControl::applyAnimation_() {
    const float gauge = 1.0f - mPreviousValue / 100.0f;
    const float tracing = 1.0f - mTracingValue / 100.0f;
    if (mPreviousValue > mTracingValue) {
        mGaugeAnimator->Stop(gauge * mGaugeAnimator->GetFrameSize());
        mTracingAnimator->Stop(tracing * mTracingAnimator->GetFrameSize());
        if (mTraceColorAnimator) mTraceColorAnimator->Stop(1);
    } else if (mPreviousValue < mTracingValue) {
        mGaugeAnimator->Stop(tracing * mGaugeAnimator->GetFrameSize());
        mTracingAnimator->Stop(gauge * mTracingAnimator->GetFrameSize());
        if (mTraceColorAnimator) mTraceColorAnimator->Stop(0);
    } else {
        const float frame = gauge * mGaugeAnimator->GetFrameSize();
        mGaugeAnimator->Stop(frame);
        mTracingAnimator->Stop(frame);
        if (mTraceColorAnimator) mTraceColorAnimator->Stop(1);
    }
}
}
