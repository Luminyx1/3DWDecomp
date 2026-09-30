#pragma once

#include <eui/euiButtonBase.h>
#include <math/seadVector.h>

namespace nn::ui2d { class Pane; class ControlSrc; }
namespace sead { class Heap; }

namespace eui {
class LayoutEx;
class Animator;
class AnimatorSet;

class AnimButton : public ButtonBase {
public:
    AnimButton();
    ~AnimButton() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(ButtonBase);
    void Down() override;
    void ForceOff() override;
    void ForceOn() override;
    void ForceDown() override;
    bool ProcessOn() override;
    bool ProcessOff() override;
    bool ProcessCancel() override;
    bool UpdateOn() override;
    bool UpdateOff() override;
    bool UpdateDown() override;
    void StartOn() override;
    void StartOff() override;
    void StartDown() override;
    void FinishDown() override;
    void ChangeState(State state) override;
    void ForceChangeState(State state) override;
    virtual void Build(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout);
    virtual bool HitTest(const sead::Vector2f& rPosition) const;
    virtual void StartDrag(const sead::Vector2f& rPosition);
    virtual void UpdateDrag(const sead::Vector2f* pPosition);
    virtual void FinishDrag(const sead::Vector2f* pPosition);
    virtual void PlayDisableAnim(bool disabled);
    virtual void SetDisableAnimDirect(bool disabled);
    virtual void ActivateByBoxCursor();
    virtual void InactivateByBoxCursor();
    virtual void BuildStateAnim(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout);

    nn::ui2d::Pane* GetCursorPane() const { return mCursorPane; }
    void SetTouch(bool touch);
    void DownOff(bool force);
    bool IsPlayDisableAnim() const;
    Animator* SelectStateAnim(int index);
    void CloneImpl_(const AnimButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap);

protected:
    AnimatorSet* mStateAnimators;
    Animator* mDisableAnimator;
    nn::ui2d::Pane* mHitPane;
    nn::ui2d::Pane* mCursorPane;
};
static_assert(sizeof(AnimButton) == 0x68, "AnimButton size");

}  // namespace eui
