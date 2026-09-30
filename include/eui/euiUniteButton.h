#pragma once
#include <eui/euiAnimButton.h>
#include <prim/seadEnum.h>
namespace eui {
class UniteButton : public AnimButton {
public:
    SEAD_ENUM(ButtonType, cButtonType_0, cButtonType_1, cButtonType_2, cButtonType_3, cButtonType_4)
    UniteButton();
    ~UniteButton() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(AnimButton);
    bool ProcessOn() override;
    bool ProcessOff() override;
    bool ProcessCancel() override;
    bool UpdateDown() override;
    bool UpdateCancel() override;
    void StartOn() override;
    void StartDown() override;
    void StartCancel() override;
    void FinishDown() override;
    void FinishCancel() override;
    void Build(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) override;
    bool HitTest(const sead::Vector2f& rPosition) const override;
    void StartDrag(const sead::Vector2f& rPosition) override;
    void UpdateDrag(const sead::Vector2f* pPosition) override;
    void FinishDrag(const sead::Vector2f* pPosition) override;
    void BuildStateAnim(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) override;
    virtual void Uncheck();
    void ForceSetChecked(bool checked);
    Animator* mCheckAnimator;
    Animator* mDragAnimator;
    sead::Vector2f mDragStart;
    sead::Vector2f mPaneStart;
    u8 mButtonType;
    bool mChecked;
    bool mDragHorizontal;
    bool mDragVertical;
};

static_assert(sizeof(UniteButton) == 0x90, "UniteButton size");
}
