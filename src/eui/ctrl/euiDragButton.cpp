#include <eui/euiDragButton.h>
#include <eui/euiAnimator.h>
#include <nn/ui2d/ui2d_Pane.h>
namespace eui {
const char* DragButton::getClassName() const { return "DragButton"; }
DragButton::DragButton() : mDragPane(nullptr), mDragStart(0, 0), mPaneStart(0, 0),
                           mDragX(true), mDragY(true) { mFlags |= 0x400; }
// rPosition is the initial pointer location; retain the pane origin for relative dragging.
void DragButton::StartDrag(const sead::Vector2f& rPosition) {
    mDragStart = rPosition;
    mPaneStart.set(mDragPane->mPositionX, mDragPane->mPositionY);
}
// pPosition is the current pointer location, or null when no position is available.
void DragButton::UpdateDrag(const sead::Vector2f* pPosition) {
    if (!pPosition) return;
    float x = mPaneStart.x;
    if (mDragX) x += pPosition->x - mDragStart.x;
    float y = mPaneStart.y;
    if (mDragY) y += pPosition->y - mDragStart.y;
    mDragPane->mPositionX = x;
    mDragPane->mPositionY = y;
    mDragPane->mFlags |= 0x10;
}
// pPosition is unused; input mode decides whether the completed drag turns off or cancels.
void DragButton::FinishDrag(const sead::Vector2f* pPosition) {
    if (mFlags & 0x40) Off();
    else Cancel();
}
void DragButton::StartCancel() { SelectStateAnim(6)->Play(Animator::cPlayType_OneTime, 1); }
void DragButton::FinishCancel() {
    bool touch = (mFlags & 0x40) != 0;
    Animator* pAnimator = SelectStateAnim(0);
    if (touch) { pAnimator->StopAtMin(); ChangeState(cState_Off); }
    else { pAnimator->StopAtMax(); ChangeState(cState_On); }
}
}
