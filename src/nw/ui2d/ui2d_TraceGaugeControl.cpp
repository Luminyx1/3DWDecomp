#include <nn/ui2d/ui2d_TraceGaugeControl.h>
namespace nn::ui2d {
TraceGaugeControl::TraceGaugeControl() : mAnimators{}, mGaugeValue(100), mPreviousValue(100), mTracingValue(100), mTracingSpeed(2), mTracingFraction(0), mShortageUpper(20), mShortageLower(0), mWaitElapsed(0), mTracingWait(0), mEnabled(true) {}
// value is the gauge percentage in the inclusive 0..100 interval.
void TraceGaugeControl::SetGaugeValue(float value) { if (value >= 0 && value <= 100) mGaugeValue = value; }
// speed is the tracing step in the inclusive 0..100 interval.
void TraceGaugeControl::SetTracingSpeed(float speed) { if (speed >= 0 && speed <= 100) mTracingSpeed = speed; }
// fraction controls proportional tracing in the inclusive 0..1 interval.
void TraceGaugeControl::SetTracingFraction(float fraction) { if (fraction >= 0 && fraction <= 1) mTracingFraction = fraction; }
// wait is the nonnegative delay before the trailing gauge begins to move.
void TraceGaugeControl::SetTracingWait(float wait) { if (wait >= 0) mTracingWait = wait; }
// upper and lower bound the shortage interval, with 0 <= lower <= upper <= 100.
void TraceGaugeControl::SetShortageThreshold(float upper, float lower) {
    if (lower >= 0 && upper <= 100 && lower <= upper) { mShortageUpper = upper; mShortageLower = lower; }
}

// value immediately positions the trailing gauge and updates the animations.
void TraceGaugeControl::SetTracingValue(float value) { mTracingValue = value; ApplyAnimation_(); }
// device is unused; controls release their reference to the layout.
void ControlBase::Finalize(nn::gfx::Device* device) { mLayout = nullptr; }
// position, pressed, and released are unused by this display-only control.
void TraceGaugeControl::UpdateControlUserInput(const nn::util::Float2* position, bool pressed, bool released) {}
}
