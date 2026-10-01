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
    auto* screen = getLayout()->getScreen();
    auto* manager = screen->getScreenMgr()->getBoxCursorMgr();
    manager->registerControl(screen->getDrawTarget(), nullptr);
}

// rSource names the four cursor panes; pLayout supplies their root and owning screen.
void BoxCursorControl::initialize(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout) {
    _20 = pLayout;
    const char* topLeft = rSource.FindFunctionalPaneName("TopLeft");
    mTopLeft = pLayout->findPaneByName(topLeft);
    const char* topRight = rSource.FindFunctionalPaneName("TopRight");
    mTopRight = pLayout->findPaneByName(topRight);
    const char* bottomLeft = rSource.FindFunctionalPaneName("BottomLeft");
    mBottomLeft = pLayout->findPaneByName(bottomLeft);
    const char* bottomRight = rSource.FindFunctionalPaneName("BottomRight");
    mBottomRight = pLayout->findPaneByName(bottomRight);
    auto* screen = getLayout()->getScreen();
    auto* manager = screen->getScreenMgr()->getBoxCursorMgr();
    manager->registerControl(screen->getDrawTarget(), this);
    getLayout()->getScreen()->mFlags |= 4;
}

// NON_MATCHING: the tail-call relocation awaits selectActiveNode_ reconstruction.
// target selects the display on which the active node must remain movable.
void BoxCursorControl::updateActiveNode(DrawTarget target) {
    if (mActiveNode != nullptr && mActiveNode->isMovable(target)) {
        return;
    }

    selectActiveNode_();
}

// pNode becomes active; null clears the selection, and unchanged nodes keep their activation state.
void BoxCursorControl::setActiveNode_(const BoxCursorNode* pNode) {
    const auto* previous = mActiveNode;
    mActiveNode = pNode;

    if (previous != nullptr && previous != pNode) {
        previous->getButton()->InactivateByBoxCursor();
    }

    if (pNode != nullptr) {
        pNode->getPosition(&mPosition);
        pNode->getScreen()->setLastActiveCursor(pNode);

        if (previous != pNode) {
            pNode->getButton()->ActivateByBoxCursor();
        }
    }
}

// pNode is detached from either cursor selection that currently refers to it.
void BoxCursorControl::clearActiveAndReservedActiveNode(const BoxCursorNode* pNode) {
    if (mActiveNode == pNode) {
        mActiveNode = nullptr;

        if (pNode != nullptr) {
            pNode->getButton()->InactivateByBoxCursor();
        }
    }

    if (mReservedActiveNode == pNode) {
        mReservedActiveNode = nullptr;
    }
}

Screen* BoxCursorControl::getActiveNodeScreen() {
    return (mActiveNode != nullptr) ? mActiveNode->getScreen() : nullptr;
}
}
