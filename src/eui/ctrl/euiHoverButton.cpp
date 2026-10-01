#include <eui/euiHoverButton.h>

#include <eui/euiLayoutEx.h>
#include <eui/euiScreen.h>
#include <eui/euiAnimatorSet.h>
#include <eui/euiAnimator.h>
#include <nn/ui2d/ui2d_Pane.h>
#include <eui/euiBoundingEx.h>
#include <eui/euiButtonGroup.h>
#include <eui/euiUtility.h>
#include <nn/ui2d/ui2d_ExtUserData.h>
namespace eui {
const char* HoverButton::getClassName() const { return "HoverButton"; }

// A touch press ends the hover state; cursor activation does not press this button.
void HoverButton::Down() {
    if (mFlags & 0x40) Off();
}

// pHeap stores the animator set; pPane supplies hit geometry; pAnimator supplies the
// interaction animation, and pLayout provides ownership and the input mode.
void HoverButton::Initialize(sead::Heap* pHeap, nn::ui2d::Pane* pPane, Animator* pAnimator, LayoutEx* pLayout) {
    _20 = pLayout;
    SetTouch((pLayout->mScreen != nullptr) ? pLayout->mScreen->_eb != 0 : false);
    mFlags &= ~0x2000;
    mHitPane = pPane;
    mStateAnimators = new (pHeap, 8) AnimatorSet;
    mStateAnimators->allocBuffer(4, pHeap);
    pAnimator->mFlags |= 0x10;
    pAnimator->mFlags &= ~0x20;
    mStateAnimators->setAnimator(0, pAnimator);
    _18 = pPane->mPanelName;
    mFlags |= 0x100;
}

// pPane supplies the interaction metadata; pLayout owns animations and pGroup receives the button.
void HoverButton::CreateHoverButton(nn::ui2d::Pane* pPane, LayoutEx* pLayout, ButtonGroup* pGroup) {
    const auto* boundingType = BoundingEx::GetRuntimeTypeInfoStatic();

    if (pPane == nullptr) return;
    bool isBounding = false;

    for (auto* type = pPane->GetRuntimeTypeInfo(); type != nullptr; type = type->m_ParentTypeInfo) {
        if (type == boundingType) { isBounding = true; break; }
    }

    if (!isBounding) return;
    const auto* data = pPane->FindExtUserDataByName("HoverButtonAnim");

    if (data == nullptr) return;
    Animator* animator = pLayout->tryCreateAnimatorAuto(static_cast<const char*>(data->GetData()), true);

    if (animator == nullptr) return;
    sead::Heap* heap = GetNwAllocatorHeap();
    auto* button = new (heap, 8) HoverButton;
    button->Initialize(heap, pPane, animator, pLayout);
    pGroup->mButtons.push_back(*button);
}
}
