#pragma once
#include <eui/euiControlBase.h>
namespace nn::ui2d { class ControlSrc; }
namespace sead { class Heap; }
namespace eui {
class Animator;
class LayoutEx;
class TraceGaugeControl : public ControlBase {
public:
    TraceGaugeControl();
    TraceGaugeControl(const TraceGaugeControl& rOther, LayoutEx* pLayout, sead::Heap* pHeap);
    ~TraceGaugeControl() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(ControlBase);
    void initialize(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout);
    void Update(float step) override;
    void applyAnimation_();
    void setGaugeValue(float value);
    void setTracingSpeed(float speed);
    void setTracingFraction(float fraction);
    void setTracingWait(float wait);
    void setShortageThreshold(float upper, float lower);
    void setTracingValue(float value);
    Animator* mGaugeAnimator;
    Animator* mTracingAnimator;
    Animator* mTraceColorAnimator;
    Animator* mShortageAnimator;
    float mGaugeValue;
    float mPreviousValue;
    float mTracingValue;
    float mTracingSpeed;
    float mTracingFraction;
    float mShortageUpper;
    float mShortageLower;
    float mWaitRemaining;
    float mTracingWait;
    u8 mFlags;
};
static_assert(sizeof(TraceGaugeControl) == 0x70, "TraceGaugeControl size");
}
