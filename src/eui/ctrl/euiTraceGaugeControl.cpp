#include <eui/euiTraceGaugeControl.h>
#include <eui/euiAnimator.h>
#include <eui/euiLayoutEx.h>
#include <nn/ui2d/ui2d_ControlSrc.h>
#include <math/seadMathCalcCommon.hpp>
namespace eui {
namespace {
// pFlags is the animator's flag byte; mask selects the flags to clear.
inline void ResetFlags(u8* pFlags, u8 mask) { *pFlags &= ~mask; }

// pValue receives the first float of the user data named pName, looked up on the root pane of pLayout
// first and on rSource second; it is left unchanged when neither has a value.
inline void ReadFloatUserData(float* pValue, LayoutEx* pLayout, const nn::ui2d::ControlSrc& rSource,
                              const char* pName) {
    auto* data = pLayout->getRootPane()->FindExtUserDataByName(pName);

    if (data == nullptr) {
        data = rSource.FindExtUserDataByName(pName);
    }

    if (data != nullptr && data->count) {
        *pValue = *static_cast<const float*>(data->GetData());
    }
}

}

const char* TraceGaugeControl::getClassName() const { return "TraceGaugeControl"; }
TraceGaugeControl::TraceGaugeControl()
    : mGaugeAnimator(nullptr), mTracingAnimator(nullptr), mTraceColorAnimator(nullptr),
      mShortageAnimator(nullptr), mGaugeValue(100), mPreviousValue(100), mTracingValue(100),
      mTracingSpeed(2), mTracingFraction(0), mShortageUpper(20), mShortageLower(0),
      mWaitRemaining(0), mTracingWait(0), mFlags(1) {}
// rOther supplies resource names and tracing settings; pLayout owns the clones; pHeap is unused.
TraceGaugeControl::TraceGaugeControl(const TraceGaugeControl& rOther, LayoutEx* pLayout, sead::Heap* pHeap)
    : mGaugeAnimator(nullptr), mTracingAnimator(nullptr), mTraceColorAnimator(nullptr),
      mShortageAnimator(nullptr), mGaugeValue(100), mPreviousValue(100), mTracingValue(100),
      mTracingSpeed(rOther.mTracingSpeed), mTracingFraction(rOther.mTracingFraction),
      mShortageUpper(20), mShortageLower(0), mWaitRemaining(0),
      mTracingWait(rOther.mTracingWait), mFlags(1) {
    _20 = pLayout;
    mName = rOther.mName;
    mGaugeAnimator = pLayout->createAnimatorAuto(rOther.mGaugeAnimator->getName(), true);
    mTracingAnimator = pLayout->createAnimatorAuto(rOther.mTracingAnimator->getName(), true);
    ResetFlags(&mGaugeAnimator->mFlags, 0x20);
    ResetFlags(&mTracingAnimator->mFlags, 0x20);

    if (rOther.mTraceColorAnimator != nullptr) {
        mTraceColorAnimator = pLayout->createAnimatorAuto(rOther.mTraceColorAnimator->getName(), true);
        ResetFlags(&mTraceColorAnimator->mFlags, 0x20);
    }

    if (rOther.mShortageAnimator != nullptr) {
        mShortageAnimator = pLayout->createAnimatorAuto(rOther.mShortageAnimator->getName(), true);
        ResetFlags(&mShortageAnimator->mFlags, 0x20);
    }
}

// rSource supplies animation names and fallback settings; pLayout supplies pane metadata and ownership.
void TraceGaugeControl::initialize(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) {
    _20 = pLayout;
    mName = pLayout->getRootName();
    mGaugeAnimator = pLayout->createAnimatorAuto(rSource.FindFunctionalAnimName("GaugeRatio"), true);
    mTracingAnimator = pLayout->createAnimatorAuto(rSource.FindFunctionalAnimName("TraceRatio"), true);
    ResetFlags(&mGaugeAnimator->mFlags, 0x20);
    ResetFlags(&mTracingAnimator->mFlags, 0x20);
    const char* name = rSource.FindFunctionalAnimName("TraceColor");

    if (name != nullptr && *name) {
        mTraceColorAnimator = pLayout->tryCreateAnimatorAutoWithWarning(name, true);

        if (mTraceColorAnimator != nullptr) {
            ResetFlags(&mTraceColorAnimator->mFlags, 0x20);
        }
    }

    name = rSource.FindFunctionalAnimName("Shortage");

    if (name != nullptr && *name) {
        mShortageAnimator = pLayout->tryCreateAnimatorAutoWithWarning(name, true);

        if (mShortageAnimator != nullptr) {
            ResetFlags(&mShortageAnimator->mFlags, 0x20);
        }
    }

    ReadFloatUserData(&mTracingSpeed, pLayout, rSource, "TracingSpeed");
    ReadFloatUserData(&mTracingFraction, pLayout, rSource, "TracingFraction");
    ReadFloatUserData(&mTracingWait, pLayout, rSource, "TracingWait");
}

// NON_MATCHING: conditional selection and chase branches differ from the original.
// step scales the delay countdown and the speed at which the trailing gauge approaches its target.
void TraceGaugeControl::Update(float step) {
    bool changed = false;
    const float gauge = mGaugeValue;
    const float previous = mPreviousValue;

    if (gauge != previous) {
        if (mShortageAnimator != nullptr) {
            float frame;

            if (gauge <= mShortageLower) {
                frame = 2;
            } else if (gauge <= mShortageUpper) {
                frame = 1;
            } else {
                frame = 0;
            }

            if (frame != mShortageAnimator->getFrame()) {
                mShortageAnimator->Stop(frame);
            }
        }

        if (mTracingValue == mPreviousValue) {
            mWaitRemaining = mTracingWait;
        }

        mPreviousValue = mGaugeValue;

        if (gauge < previous) {
            applyAnimation_();
            return;
        }

        changed = true;
    }

    if (mPreviousValue != mTracingValue && mFlags) {
        if (mWaitRemaining > 0) {
            sead::Mathf::chase(&mWaitRemaining, 0.0f, step);
        } else {
            const float difference = sead::Mathf::abs(mTracingValue - mPreviousValue);
            const float speed = sead::Mathf::max(mTracingSpeed, difference * mTracingFraction);
            sead::Mathf::chase(&mTracingValue, mPreviousValue, speed * step);
            changed = true;
        }
    }

    if (changed) {
        applyAnimation_();
    }
}

// value is the target gauge percentage; values outside [0, 100] are ignored.
void TraceGaugeControl::setGaugeValue(float value) {
    if (value >= 0 && value <= 100) {
        mGaugeValue = value;
    }
}

// speed is the tracing percentage increment, constrained to [0, 100].
void TraceGaugeControl::setTracingSpeed(float speed) {
    if (speed >= 0 && speed <= 100) {
        mTracingSpeed = speed;
    }
}

// fraction is the proportion of the remaining difference traced each update, in [0, 1].
void TraceGaugeControl::setTracingFraction(float fraction) {
    if (fraction >= 0 && fraction <= 1) {
        mTracingFraction = fraction;
    }
}

// wait is a nonnegative delay before the tracing gauge starts moving.
void TraceGaugeControl::setTracingWait(float wait) {
    if (wait >= 0) {
        mTracingWait = wait;
    }
}

// upper and lower bound the shortage range, with 0 <= lower <= upper <= 100.
void TraceGaugeControl::setShortageThreshold(float upper, float lower) {
    if (!(lower >= 0)) {
        return;
    }

    if ((upper <= 100) & (lower <= upper)) {
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

        if (mTraceColorAnimator != nullptr) {
            mTraceColorAnimator->Stop(1);
        }
    } else if (mPreviousValue < mTracingValue) {
        mGaugeAnimator->Stop(tracing * mGaugeAnimator->GetFrameSize());
        mTracingAnimator->Stop(gauge * mTracingAnimator->GetFrameSize());

        if (mTraceColorAnimator != nullptr) {
            mTraceColorAnimator->Stop(0);
        }
    } else {
        const float frame = gauge * mGaugeAnimator->GetFrameSize();
        mGaugeAnimator->Stop(frame);
        mTracingAnimator->Stop(frame);

        if (mTraceColorAnimator != nullptr) {
            mTraceColorAnimator->Stop(1);
        }
    }
}
}
