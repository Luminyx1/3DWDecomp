#include <eui/euiBoxCursorControl.h>
#include <eui/euiBoxCursorNode.h>
#include <eui/euiAnimButton.h>
namespace eui {
const char* BoxCursorControl::getClassName() const { return "BoxCursorControl"; }
BoxCursorControl::BoxCursorControl()
    : _28(nullptr), _30(nullptr), _38(nullptr), _40(nullptr), mActiveNode(nullptr),
      mReservedActiveNode(nullptr), mPosition(sead::Vector2f::zero) {}
// pNode is detached from either cursor selection that currently refers to it.
void BoxCursorControl::clearActiveAndReservedActiveNode(const BoxCursorNode* pNode) {
    if (mActiveNode == pNode) {
        mActiveNode = nullptr;
        if (pNode) pNode->mButton->InactivateByBoxCursor();
    }
    if (mReservedActiveNode == pNode) mReservedActiveNode = nullptr;
}
Screen* BoxCursorControl::getActiveNodeScreen() {
    return mActiveNode ? mActiveNode->mScreen : nullptr;
}
}
