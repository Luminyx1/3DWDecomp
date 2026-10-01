#pragma once
#include <nn/ui2d/ui2d_ButtonBase.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/ui2d/ui2d_Pane.h>
#include <nn/font/font_Util.h>
#include <nn/util/util_IntrusiveList.h>
#include <nn/util/util_MathTypes.h>

namespace nn::ui2d {
class Animator;
class Layout;
class ControlSrc;
class Pane;

class AnimButton : public ButtonBase {
public:
    using StateChangeCallback = void (*)(AnimButton*, State, State, void*);
    AnimButton();
    ~AnimButton() override = default;
    void ForceOff() override;
    void ForceOn() override;
    void ForceDown() override;
    bool ProcessCancel() override;
    bool UpdateOn() override;
    bool UpdateOff() override;
    bool UpdateDown() override;
    bool UpdateCancel() override;
    void StartOn() override;
    void StartOff() override;
    void StartDown() override;
    void StartCancel() override;
    void ChangeState(State state) override;
    NN_RUNTIME_TYPEINFO_BASE();
    virtual void UpdateHitBox();
    virtual bool IsHit(const nn::util::Float2& position) const;
    virtual Layout* GetLayout() { return nullptr; }
    // layout is unused by controls that do not retain their owning layout.
    virtual void SetLayout(Layout* layout) {}
    virtual void InitializeDragPosition(const nn::util::Float2& position);
    virtual void UpdateDragPosition(const nn::util::Float2* position);
    virtual void PlayDisableAnim(bool disabled);
    virtual void SetAllAnimatorDisable();

    void Build(nn::gfx::Device* device, Layout* layout, const ControlSrc& source);
    void BuildEx(nn::gfx::Device* device, Layout* layout, const ControlSrc& source);
    void CloneImpl_(nn::gfx::Device* device, const AnimButton& source, Layout* layout);
    void EnableAnim(Animator* animator);
    void SetStateChangeCallback(StateChangeCallback callback, void* argument);

    // A parts layout is named after its root pane; a top-level layout uses its own name.
    static const char* GetControlName_(const Layout* layout) {
        Pane* rootPane = layout->GetRootPane();
        return rootPane->GetParent() != nullptr ? rootPane->GetName() : layout->GetName();
    }

    nn::util::IntrusiveListNode mLink;
    StateChangeCallback mCallback;
    void* mCallbackArg;
    Animator* mOnAnimator;
    Animator* mDownAnimator;
    Animator* mCancelAnimator;
    Animator* mDisableAnimator;
    Pane* mHitPane;
    nn::util::Float4 mHitBox;
    int mTag;
    const char* mName;
};
static_assert(sizeof(AnimButton) == 0x90, "AnimButton size");
}
