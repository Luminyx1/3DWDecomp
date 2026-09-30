#include <eui/euiBoxCursorNode.h>
#include <eui/euiAnimButton.h>
#include <eui/euiScreen.h>
#include <eui/euiButtonGroup.h>
namespace eui {
BoxCursorNode::BoxCursorNode() : mButton(nullptr), mScreen(nullptr), mRoutes{} {}
BoxCursorNode::~BoxCursorNode() = default;
// pButton supplies the cursor pane; pScreen owns and registers this navigation node.
void BoxCursorNode::initialize(AnimButton* pButton, Screen* pScreen) {
    mButton = pButton;
    mScreen = pScreen;
    if (pScreen) pScreen->mCursorNodes.pushBack(this);
}
// target selects the display whose cursor may navigate to this node.
bool BoxCursorNode::isMovable(DrawTarget target) const {
    Screen* screen = mScreen;
    if (!screen->isOpened()) return false;
    if (!(screen->mButtonGroup->mFlags & 2)) return false;
    if (!(mButton->mFlags & 0x10)) return false;
    return int(screen->getDrawTarget()) == int(target);
}
// target selects the display; requireDecidable additionally requires the button's repeat-on flag.
bool BoxCursorNode::isDecidable(DrawTarget target, bool requireDecidable) const {
    Screen* screen = mScreen;
    if (!screen->isOpened() || !(screen->mButtonGroup->mFlags & 2) || !(mButton->mFlags & 0x10))
        return false;
    if (int(screen->getDrawTarget()) != int(target)) return false;
    if (requireDecidable && !(mButton->mFlags & 0x80)) return false;
    return !screen->mButtonGroup->IsExistExcludingDown();
}
// pNode is removed from every directional route that currently points to it.
void BoxCursorNode::eraseNodeFromRouteNodes(const BoxCursorNode* pNode) {
    for (int i = 0; i < 4; ++i)
        if (mRoutes[i] == pNode) mRoutes[i] = nullptr;
}
// pPosition receives the cursor pane's global translation in layout coordinates.
void BoxCursorNode::getPosition(sead::Vector2f* pPosition) const {
    const auto* pPane = mButton->GetCursorPane();
    pPosition->set(pPane->mGlobalMtx[3], pPane->mGlobalMtx[7]);
}
// direction chooses this route; pNode receives the corresponding reverse route.
void BoxCursorNode::setRouteNodeEach(Direction direction, BoxCursorNode* pNode) {
    mRoutes[direction] = pNode;
    pNode->setRouteNode(GetOppositeDirection(direction), this);
}
void BoxCursorNode::clearRouteAll() { mRoutes.fill(nullptr); }
}
