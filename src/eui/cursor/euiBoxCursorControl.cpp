#include <eui/euiBoxCursorControl.h>
#include <eui/euiBoxCursorNode.h>
#include <eui/euiAnimButton.h>
#include <eui/euiLayoutEx.h>
#include <eui/euiScreen.h>
#include <eui/euiScreenMgr.h>
#include <eui/euiBoxCursorMgr.h>
#include <nn/ui2d/ui2d_ControlSrc.h>
namespace eui {
const char* BoxCursorControl::getClassName() const { return "BoxCursorControl"; }
BoxCursorControl::BoxCursorControl()
    : mTopLeft(nullptr), mTopRight(nullptr), mBottomLeft(nullptr), mBottomRight(nullptr), mActiveNode(nullptr),
      mReservedActiveNode(nullptr), mPosition(sead::Vector2f::zero) {}
BoxCursorControl::~BoxCursorControl() {
    auto* screen = static_cast<LayoutEx*>(_20)->mScreen;
    auto* manager = screen->mScreenMgr->mBoxCursorMgr;
    manager->registerControl(screen->getDrawTarget(), nullptr);
}

// rSource names the four cursor panes; pLayout supplies their root and owning screen.
void BoxCursorControl::initialize(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) {
    _20 = pLayout;
    const char* topLeft = rSource.FindFunctionalPaneName("TopLeft");
    mTopLeft = pLayout->mRootPane->FindPaneByName(topLeft, true);
    const char* topRight = rSource.FindFunctionalPaneName("TopRight");
    mTopRight = pLayout->mRootPane->FindPaneByName(topRight, true);
    const char* bottomLeft = rSource.FindFunctionalPaneName("BottomLeft");
    mBottomLeft = pLayout->mRootPane->FindPaneByName(bottomLeft, true);
    const char* bottomRight = rSource.FindFunctionalPaneName("BottomRight");
    mBottomRight = pLayout->mRootPane->FindPaneByName(bottomRight, true);
    auto* screen = static_cast<LayoutEx*>(_20)->mScreen;
    auto* manager = screen->mScreenMgr->mBoxCursorMgr;
    manager->registerControl(screen->getDrawTarget(), this);
    static_cast<LayoutEx*>(_20)->mScreen->mFlags |= 4;
}

// NON_MATCHING: the tail-call relocation awaits selectActiveNode_ reconstruction.
// target selects the display on which the active node must remain movable.
void BoxCursorControl::updateActiveNode(DrawTarget target) {
    if (mActiveNode != nullptr && mActiveNode->isMovable(target)) return;
    selectActiveNode_();
}

// pNode becomes active; null clears the selection, and unchanged nodes keep their activation state.
void BoxCursorControl::setActiveNode_(const BoxCursorNode* pNode) {
    const auto* previous = mActiveNode;
    mActiveNode = pNode;

    if (previous != nullptr && previous != pNode) previous->mButton->InactivateByBoxCursor();

    if (pNode != nullptr) {
        pNode->getPosition(&mPosition);
        pNode->mScreen->mLastActiveCursor = pNode;

        if (previous != pNode) pNode->mButton->ActivateByBoxCursor();
    }
}

// pNode is detached from either cursor selection that currently refers to it.
void BoxCursorControl::clearActiveAndReservedActiveNode(const BoxCursorNode* pNode) {
    if (mActiveNode == pNode) {
        mActiveNode = nullptr;

        if (pNode != nullptr) pNode->mButton->InactivateByBoxCursor();
    }

    if (mReservedActiveNode == pNode) mReservedActiveNode = nullptr;
}

Screen* BoxCursorControl::getActiveNodeScreen() {
    return (mActiveNode != nullptr) ? mActiveNode->mScreen : nullptr;
}
}
