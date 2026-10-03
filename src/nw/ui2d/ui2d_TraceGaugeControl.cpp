#include <nn/ui2d/ui2d_TraceGaugeControl.h>
#include <nn/ui2d/ui2d_AnimatorEx.h>
#include <nn/ui2d/ui2d_LayoutEx.h>
#include <nn/ui2d/ui2d_ControlSrc.h>
#include <cmath>
namespace nn::ui2d {
namespace {
/**
 * @brief Read an optional tracing setting from the root pane or control source.
 * @param pValue Setting to update; preserved when neither source supplies a value.
 * @param pLayout Layout whose root-pane metadata has priority.
 * @param rSource Control resource supplying fallback metadata.
 * @param pName Null-terminated name of floating-point user data to read.
 */
inline void ReadFloatUserData(float* pValue, LayoutEx* pLayout, const nn::ui2d::ControlSrc& rSource,
                              const char* pName) {
    auto* data = pLayout->GetRootPane()->FindExtUserDataByName(pName);

    if (data == nullptr) {
        data = rSource.FindExtUserDataByName(pName);
    }

    if (data != nullptr && data->count) {
        *pValue = *static_cast<const float*>(data->GetData());
    }
}
/**
 * @brief Move a floating-point value toward its target without crossing it.
 * @param pValue Non-null value updated in place.
 * @param target Desired final value.
 * @param step Nonnegative movement allowed for this update.
 * @return True when the target has been reached, otherwise false.
 */
inline bool ChaseValue(float* pValue, float target, float step) {
    float current = *pValue;
    if (current < target) {
        float next = current + step;
        if (next < current || target <= next) {
            *pValue = target;
            return true;
        }
        *pValue = next;
        return false;
    } else if (current > target) {
        float next = current - step;
        if (current < next || target >= next) {
            *pValue = target;
            return true;
        }
        *pValue = next;
        return false;
    }
    return true;
}
} // namespace

/** @brief Construct a full gauge with default tracing speed, thresholds and no animator bindings. */
TraceGaugeControl::TraceGaugeControl()
    : mAnimators{}, mGaugeValue(100), mPreviousValue(100), mTracingValue(100), mTracingSpeed(2),
      mTracingFraction(0), mShortageUpper(20), mShortageLower(0), mWaitRemaining(0), mTracingWait(0),
      mEnabled(true) {}
/**
 * @brief Set the desired gauge percentage when it lies in the supported interval.
 * @param value Target percentage in [0, 100]; other values are ignored.
 */
void TraceGaugeControl::SetGaugeValue(float value) {
    if (value >= 0 && value <= 100) {
        mGaugeValue = value;
    }
}
/**
 * @brief Set the minimum tracing movement per animation step.
 * @param speed Movement in [0, 100]; other values are ignored.
 */
void TraceGaugeControl::SetTracingSpeed(float speed) {
    if (speed >= 0 && speed <= 100) {
        mTracingSpeed = speed;
    }
}
/**
 * @brief Set proportional tracing movement based on the remaining difference.
 * @param fraction Proportion in [0, 1]; other values are ignored.
 */
void TraceGaugeControl::SetTracingFraction(float fraction) {
    if (fraction >= 0 && fraction <= 1) {
        mTracingFraction = fraction;
    }
}
/**
 * @brief Set the delay before the trailing gauge begins to move.
 * @param wait Nonnegative delay in animation-step units; negative values are ignored.
 */
void TraceGaugeControl::SetTracingWait(float wait) {
    if (wait >= 0) {
        mTracingWait = wait;
    }
}
/**
 * @brief Set the boundaries selecting normal, low and empty gauge animations.
 * @param upper Upper shortage threshold; must not exceed 100.
 * @param lower Lower shortage threshold; must be nonnegative and no greater than upper.
 */
void TraceGaugeControl::SetShortageThreshold(float upper, float lower) {
    if (lower >= 0 && ((upper <= 100) & (lower <= upper))) {
        mShortageUpper = upper;
        mShortageLower = lower;
    }
}

/**
 * @brief Immediately position the trailing gauge and apply its animations.
 * @param value New trailing gauge percentage, used without range validation.
 */
void TraceGaugeControl::SetTracingValue(float value) {
    mTracingValue = value;
    ApplyAnimation_();
}
/**
 * @brief Release this control's reference to its owning layout.
 * @param device Unused; no graphics resources are destroyed by the base control.
 */
void ControlBase::Finalize(nn::gfx::Device* device) { mLayout = nullptr; }
/**
 * @brief Ignore user input for this display-only gauge control.
 * @param position Unused pointer position; may be nullptr.
 * @param pressed Unused press-edge state.
 * @param released Unused release-edge state.
 */
void TraceGaugeControl::UpdateControlUserInput(const nn::util::Float2* position, bool pressed,
                                               bool released) {}
/**
 * @brief Recreate a gauge's animator bindings on another layout using its tracing settings.
 * @param pDevice Graphics device owning the new animator resources.
 * @param rOther Gauge supplying animator tag names and tracing speed, fraction and delay.
 * @param pLayout Non-null destination layout containing the same animation tags.
 */
TraceGaugeControl::TraceGaugeControl(nn::gfx::Device* pDevice, const TraceGaugeControl& rOther,
                                     LayoutEx* pLayout)
    : mAnimators{}, mGaugeValue(100), mPreviousValue(100), mTracingValue(100),
      mTracingSpeed(rOther.mTracingSpeed), mTracingFraction(rOther.mTracingFraction), mShortageUpper(20),
      mShortageLower(0), mWaitRemaining(0), mTracingWait(rOther.mTracingWait), mEnabled(true) {
    mLayout = pLayout;
    mName = rOther.mName;
    mAnimators[0] = pLayout->CreateAnimatorExAuto(pDevice, rOther.mAnimators[0]->GetTagName(), true);
    mAnimators[1] = pLayout->CreateAnimatorExAuto(pDevice, rOther.mAnimators[1]->GetTagName(), true);
    if (rOther.mAnimators[2] != nullptr) {
        mAnimators[2] = pLayout->CreateAnimatorExAuto(pDevice, rOther.mAnimators[2]->GetTagName(), true);
    }
    if (rOther.mAnimators[3] != nullptr) {
        mAnimators[3] = pLayout->CreateAnimatorExAuto(pDevice, rOther.mAnimators[3]->GetTagName(), true);
    }
}
/**
 * @brief Bind gauge animations and load optional tracing settings from layout resources.
 * @param pDevice Graphics device owning the animator resources.
 * @param rSource Control resource containing animation roles and fallback user data.
 * @param pLayout Non-null layout containing the referenced animation tags.
 */
void TraceGaugeControl::Initialize(nn::gfx::Device* pDevice, const ControlSrc& rSource, LayoutEx* pLayout) {
    mLayout = pLayout;
    Pane* pRoot = pLayout->GetRootPane();
    mName = pRoot->GetParent() != nullptr ? pRoot->GetName() : pLayout->GetName();
    mAnimators[0] =
        pLayout->CreateAnimatorExAuto(pDevice, rSource.FindFunctionalAnimName("GaugeRatio"), true);
    mAnimators[1] =
        pLayout->CreateAnimatorExAuto(pDevice, rSource.FindFunctionalAnimName("TraceRatio"), true);
    const char* pName = rSource.FindFunctionalAnimName("TraceColor");
    if (pName != nullptr && *pName != '\0') {
        mAnimators[2] = pLayout->TryCreateAnimatorExAuto(pDevice, pName, true);
    }
    pName = rSource.FindFunctionalAnimName("Shortage");
    if (pName != nullptr && *pName != '\0') {
        mAnimators[3] = pLayout->TryCreateAnimatorExAuto(pDevice, pName, true);
    }
    ReadFloatUserData(&mTracingSpeed, pLayout, rSource, "TracingSpeed");
    ReadFloatUserData(&mTracingFraction, pLayout, rSource, "TracingFraction");
    ReadFloatUserData(&mTracingWait, pLayout, rSource, "TracingWait");
}
/**
 * @brief Update shortage state, delay and the trailing gauge position.
 * @param step Nonnegative elapsed animation step scaling the delay and tracing speed.
 */
void TraceGaugeControl::UpdateControl(float step) {
    bool changed = false;
    const float gauge = mGaugeValue;
    const float previous = mPreviousValue;

    if (gauge != previous) {
        if (mAnimators[3] != nullptr) {
            int frame;

            if (gauge <= mShortageLower) {
                frame = 2;
            } else if (gauge <= mShortageUpper) {
                frame = 1;
            } else {
                frame = 0;
            }

            if (frame != mAnimators[3]->mFrame) {
                mAnimators[3]->StopAt(frame);
            }
        }

        if (mTracingValue == mPreviousValue) {
            mWaitRemaining = mTracingWait;
        }

        mPreviousValue = mGaugeValue;

        if (gauge < previous) {
            ApplyAnimation_();
            return;
        }

        changed = true;
    }

    if (mPreviousValue != mTracingValue && mEnabled) {
        if (mWaitRemaining > 0) {
            ChaseValue(&mWaitRemaining, 0.0f, step);
        } else {
            const float difference = std::fabs(mTracingValue - mPreviousValue);
            const float speed = (mTracingSpeed < difference * mTracingFraction ? difference * mTracingFraction
                                                                               : mTracingSpeed);
            ChaseValue(&mTracingValue, mPreviousValue, speed * step);
            changed = true;
        }
    }

    if (changed) {
        ApplyAnimation_();
    }
}

/** @brief Position gauge, tracing and optional color animations from the current values. */
void TraceGaugeControl::ApplyAnimation_() {
    const float gauge = 1.0f - mPreviousValue / 100.0f;
    const float tracing = 1.0f - mTracingValue / 100.0f;

    if (mPreviousValue > mTracingValue) {
        mAnimators[0]->StopAt(gauge * mAnimators[0]->GetFrameSize());
        mAnimators[1]->StopAt(tracing * mAnimators[1]->GetFrameSize());

        if (mAnimators[2] != nullptr) {
            mAnimators[2]->StopAt(1);
        }
    } else if (mPreviousValue < mTracingValue) {
        mAnimators[0]->StopAt(tracing * mAnimators[0]->GetFrameSize());
        mAnimators[1]->StopAt(gauge * mAnimators[1]->GetFrameSize());

        if (mAnimators[2] != nullptr) {
            mAnimators[2]->StopAt(0);
        }
    } else {
        const float frame = gauge * mAnimators[0]->GetFrameSize();
        mAnimators[0]->StopAt(frame);
        mAnimators[1]->StopAt(frame);

        if (mAnimators[2] != nullptr) {
            mAnimators[2]->StopAt(1);
        }
    }
}
} // namespace nn::ui2d
