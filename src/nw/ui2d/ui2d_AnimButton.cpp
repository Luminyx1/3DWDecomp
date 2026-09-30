#include <nn/ui2d/ui2d_AnimButton.h>
#include <nn/ui2d/ui2d_AnimatorEx.h>
#include <nn/ui2d/ui2d_ControlSrc.h>
#include <nn/ui2d/ui2d_LayoutEx.h>
#include <nn/ui2d/ui2d_Screen.h>
#include <cmath>

namespace nn::ui2d {
namespace {
// layout is the candidate whose runtime ancestry is checked before downcasting.
inline LayoutEx* AsLayoutEx(Layout* layout) {
    const auto* target = LayoutEx::GetRuntimeTypeInfoStatic();
    if (layout) {
        for (auto* type = layout->GetRuntimeTypeInfo(); type; type = type->m_ParentTypeInfo)
            if (type == target) return static_cast<LayoutEx*>(layout);
    }
    return nullptr;
}
}
AnimButton::AnimButton()
    : mCallback(nullptr), mCallbackArg(nullptr), mOnAnimator(nullptr),
      mDownAnimator(nullptr), mCancelAnimator(nullptr), mDisableAnimator(nullptr),
      mHitPane(nullptr), mHitBox{}, mTag(0), mName(nullptr) {}

// device creates cloned animators, source supplies their tags and hit pane,
// and layout is the destination layout for all cloned bindings.
void AnimButton::CloneImpl_(nn::gfx::Device* device, const AnimButton& source, Layout* layout) {
    SetLayout(layout);
    LayoutEx* extended = AsLayoutEx(layout);
    if (source.mOnAnimator) mOnAnimator = extended->CreateAnimatorExAuto(device, source.mOnAnimator->GetTagName(), true);
    if (source.mDownAnimator) mDownAnimator = extended->CreateAnimatorExAuto(device, source.mDownAnimator->GetTagName(), false);
    if (source.mCancelAnimator) mCancelAnimator = extended->CreateAnimatorExAuto(device, source.mCancelAnimator->GetTagName(), false);
    if (source.mDisableAnimator) mDisableAnimator = extended->CreateAnimatorExAuto(device, source.mDisableAnimator->GetTagName(), false);
    if (source.mHitPane) mHitPane = layout->mRootPane->FindPaneByName(source.mHitPane->mPanelName, true);
    mName = layout->mRootPane->mParent ? layout->mRootPane->mPanelName : static_cast<const char*>(layout->_30);
}
// device creates animators, layout owns the panes, and source maps functional
// control names to the layout's animation and pane resources.
void AnimButton::Build(nn::gfx::Device* device, Layout* layout, const ControlSrc& source) {
    SetLayout(layout);
    mOnAnimator = layout->CreateGroupAnimatorAuto(device, source.FindFunctionalAnimName("OnOff"), true);
    mOnAnimator->StopAtStartFrame();
    mDownAnimator = layout->CreateGroupAnimatorAuto(device, source.FindFunctionalAnimName("Down"), false);
    const char* disable = source.FindFunctionalAnimName("Disable");
    if (disable && *disable) mDisableAnimator = layout->CreateGroupAnimatorAuto(device, disable, false);
    mHitPane = layout->mRootPane->FindPaneByName(source.FindFunctionalPaneName("Hit"), true);
    mName = layout->mRootPane->mParent ? layout->mRootPane->mPanelName : static_cast<const char*>(layout->_30);
}
// device creates animators, layout owns the panes, and source maps functional
// control names to the layout's animation and pane resources.
void AnimButton::BuildEx(nn::gfx::Device* device, Layout* layout, const ControlSrc& source) {
    SetLayout(layout);
    LayoutEx* extended = AsLayoutEx(layout);
    mOnAnimator = extended->TryCreateAnimatorExAuto(device, source.FindFunctionalAnimName("OnOff"), true);
    mOnAnimator->StopAtStartFrame();
    mDownAnimator = extended->TryCreateAnimatorExAuto(device, source.FindFunctionalAnimName("Down"), false);
    const char* disable = source.FindFunctionalAnimName("Disable");
    if (disable && *disable) mDisableAnimator = extended->TryCreateAnimatorExAuto(device, disable, false);
    mHitPane = layout->mRootPane->FindPaneByName(source.FindFunctionalPaneName("Hit"), true);
    mName = layout->mRootPane->mParent ? layout->mRootPane->mPanelName : static_cast<const char*>(layout->_30);
}
void AnimButton::UpdateHitBox() {
    if (!mHitPane) return;
    float32x4_t row0 = vld1q_f32(mHitPane->mGlobalMtx);
    float32x4_t row1 = vld1q_f32(mHitPane->mGlobalMtx + 4);
    float width = std::fabs(mHitPane->mSizeX * vgetq_lane_f32(row0, 0)) * 0.5f;
    float height = std::fabs(mHitPane->mSizeY * vgetq_lane_f32(row1, 1)) * 0.5f;
    float x = vgetq_lane_f32(row0, 3);
    float y = vgetq_lane_f32(row1, 3);
    int originX = mHitPane->mOriginFlags & 3;
    int originY = (mHitPane->mOriginFlags >> 2) & 3;
    if (originX == 1) x += width;
    else if (originX == 2) x -= width;
    if (originY == 1) y -= height;
    else if (originY == 2) y += height;
    mHitBox = {x - width, y - height, x + width, y + height};
}
// position is tested against the inclusive bounds calculated from the hit pane.
bool AnimButton::IsHit(const nn::util::Float2& position) const {
    if (!mHitPane) return false;
    return mHitBox.x <= position.x && position.x <= mHitBox.z &&
           mHitBox.y <= position.y && position.y <= mHitBox.w;
}
// position is unused by the base class; draggable controls override these hooks.
void AnimButton::InitializeDragPosition(const nn::util::Float2& position) {}
void AnimButton::UpdateDragPosition(const nn::util::Float2* position) {}

// callback observes transitions; argument is passed to it without modification.
void AnimButton::SetStateChangeCallback(StateChangeCallback callback, void* argument) {
    mCallback = callback;
    mCallbackArg = argument;
}
// disabled selects the direction of the disable animation.
void AnimButton::PlayDisableAnim(bool disabled) {
    if (mDisableAnimator) {
        mDisableAnimator->SetEnabled(true);
        mDisableAnimator->PlayFromCurrent(Animator::PlayType_Once, disabled ? 1.0f : -1.0f);
    }
}
void AnimButton::SetAllAnimatorDisable() {
    if (mOnAnimator) mOnAnimator->SetEnabled(false);
    if (mDownAnimator) mDownAnimator->SetEnabled(false);
    if (mCancelAnimator) mCancelAnimator->SetEnabled(false);
    if (mDisableAnimator) mDisableAnimator->SetEnabled(false);
}
// animator is the sole enabled state animator; null disables all three.
void AnimButton::EnableAnim(Animator* animator) {
    if (mOnAnimator) mOnAnimator->SetEnabled(mOnAnimator == animator);
    if (mDownAnimator) mDownAnimator->SetEnabled(mDownAnimator == animator);
    if (mCancelAnimator) mCancelAnimator->SetEnabled(mCancelAnimator == animator);
}
void AnimButton::ForceOff() {
    ButtonBase::ForceOff();
    if (mOnAnimator) {
        EnableAnim(mOnAnimator);
        mOnAnimator->StopAtStartFrame();
    }
}
void AnimButton::ForceOn() {
    ButtonBase::ForceOn();
    if (mOnAnimator) {
        EnableAnim(mOnAnimator);
        mOnAnimator->StopAtEndFrame();
    }
}
void AnimButton::ForceDown() {
    ButtonBase::ForceDown();
    if (mDownAnimator) {
        EnableAnim(mDownAnimator);
        mDownAnimator->StopAtEndFrame();
    }
}
void AnimButton::StartOn() {
    if (mOnAnimator) {
        EnableAnim(mOnAnimator);
        mOnAnimator->PlayFromCurrent(Animator::PlayType_Once, 1.0f);
    }
}
bool AnimButton::UpdateOn() { return !mOnAnimator || (mOnAnimator->mFlags & 1); }
void AnimButton::StartOff() {
    if (mOnAnimator) {
        EnableAnim(mOnAnimator);
        mOnAnimator->PlayFromCurrent(Animator::PlayType_Once, -1.0f);
    }
}
bool AnimButton::UpdateOff() { return !mOnAnimator || (mOnAnimator->mFlags & 1); }
void AnimButton::StartDown() {
    if (mDownAnimator) {
        EnableAnim(mDownAnimator);
        mDownAnimator->Play(Animator::PlayType_Once, 1.0f);
    }
}
bool AnimButton::UpdateDown() { return !mDownAnimator || (mDownAnimator->mFlags & 1); }
void AnimButton::StartCancel() {
    if (mCancelAnimator) {
        EnableAnim(mCancelAnimator);
        mCancelAnimator->Play(Animator::PlayType_Once, 1.0f);
    }
}
bool AnimButton::UpdateCancel() { return !mCancelAnimator || (mCancelAnimator->mFlags & 1); }
bool AnimButton::ProcessCancel() { return true; }
// state is reported to the callback and owning screen before it becomes current.
void AnimButton::ChangeState(State state) {
    if (mState == state) return;
    if (mCallback) mCallback(this, mState, state, mCallbackArg);
    LayoutEx* layout = AsLayoutEx(GetLayout());
    if (layout && layout->mScreen) layout->mScreen->HandleEventOnButtonStateChanged(this, mState, state);
    mState = state;
}
}
