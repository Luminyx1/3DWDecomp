#pragma once
#include <nn/ui2d/ui2d_Pane.h>
namespace nn::ui2d {
class AnimatorEx;
class LayoutEx;
class ControlSrc;
class ControlBase {
  public:
    NN_RUNTIME_TYPEINFO_BASE();
    /** @brief Destroy the control without taking ownership of its layout. */
    virtual ~ControlBase() = default;
    virtual void Finalize(nn::gfx::Device* device);
    virtual void UpdateControl(float step) = 0;
    virtual void UpdateControlUserInput(const nn::util::Float2* position, bool pressed, bool released) = 0;
    nn::util::IntrusiveListNode m_Link;
    const char* mName = nullptr;
    LayoutEx* mLayout = nullptr;
};
static_assert(sizeof(ControlBase) == 0x28, "ControlBase size");
class TraceGaugeControl : public ControlBase {
  public:
    TraceGaugeControl();
    TraceGaugeControl(nn::gfx::Device* pDevice, const TraceGaugeControl& rOther, LayoutEx* pLayout);
    NN_RUNTIME_TYPEINFO(ControlBase);
    void UpdateControl(float step) override;
    void UpdateControlUserInput(const nn::util::Float2* position, bool pressed, bool released) override;
    void Initialize(nn::gfx::Device* device, const ControlSrc& source, LayoutEx* layout);
    void SetGaugeValue(float value);
    void SetTracingSpeed(float speed);
    void SetTracingFraction(float fraction);
    void SetTracingWait(float wait);
    void SetShortageThreshold(float upper, float lower);
    void SetTracingValue(float value);
    void ApplyAnimation_();
    AnimatorEx* mAnimators[4];
    float mGaugeValue, mPreviousValue, mTracingValue, mTracingSpeed;
    float mTracingFraction, mShortageUpper, mShortageLower, mWaitRemaining, mTracingWait;
    bool mEnabled;
};
static_assert(sizeof(TraceGaugeControl) == 0x70, "TraceGaugeControl size");
} // namespace nn::ui2d
