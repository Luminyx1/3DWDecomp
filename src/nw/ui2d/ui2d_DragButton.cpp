#include <nn/ui2d/ui2d_DragButton.h>
#include <nn/ui2d/ui2d_Animator.h>
#include <nn/ui2d/ui2d_ControlSrc.h>
#include <nn/ui2d/ui2d_Layout.h>
namespace nn::ui2d {
DragButton::DragButton()
    : mDragPane(nullptr), mDragStart{}, mPaneStart{}, mDragX(true), mDragY(true) {
    mFlags = (mFlags & ~0xc0u) | 0x40u;
}

// device creates animators, layout provides the root pane to drag, and source
// identifies the standard and release animation resources.
void DragButton::Build(nn::gfx::Device* device, Layout* layout, const ControlSrc& source) {
    AnimButton::Build(device, layout, source);
    mCancelAnimator = layout->CreateGroupAnimatorAuto(device, source.FindFunctionalAnimName("Release"), false);
    mDragPane = layout->mRootPane;
}

// position records the pointer origin; retain the pane's position at drag start.
void DragButton::InitializeDragPosition(const nn::util::Float2& position) {
    mDragStart = position;
    mPaneStart.x = mDragPane->mPositionX;
    mPaneStart.y = mDragPane->mPositionY;
}

// position is the current pointer position, or null when no pointer is present.
void DragButton::UpdateDragPosition(const nn::util::Float2* position) {
    if (position == nullptr) return;
    float x = mPaneStart.x;

    if (mDragX) x += position->x - mDragStart.x;
    float y = mPaneStart.y;

    if (mDragY) y += position->y - mDragStart.y;
    Pane* pane = mDragPane;
    pane->mPositionX = x;
    pane->mPositionY = y;
    pane->mFlags |= 0x10;
}

bool DragButton::ProcessOff() {
    bool processed = true;

    switch (mState) {
    case cState_OnStart:
        ChangeState(cState_OffStart); StartOff(); break;
    case cState_On:
        ChangeState(cState_OffStart); StartOff(); break;
    case cState_CancelStart:
        processed = false; break;
    default: break;
    }

    return processed;
}

bool DragButton::ProcessCancel() {
    bool processed = true;

    switch (mState) {
    case cState_DownStart: processed = false; break;
    case cState_Down:
        ChangeState(cState_CancelStart); StartCancel(); break;
    case cState_CancelStart: processed = false; break;
    default: break;
    }

    return processed;
}

void DragButton::FinishCancel() { ChangeState(cState_On); }
}
