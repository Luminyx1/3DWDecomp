#include <eui/euiBoxCursorNode.h>
#include <eui/euiAnimButton.h>
namespace eui {
BoxCursorNode::BoxCursorNode() : mButton(nullptr), mScreen(nullptr), mRoutes{} {}
BoxCursorNode::~BoxCursorNode() = default;
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
// NON_MATCHING: temporary Direction stack slots are reversed.
void BoxCursorNode::setRouteNodeEach(Direction direction, BoxCursorNode* pNode) {
    Direction opposite;
    mRoutes[direction] = pNode;
    opposite = GetOppositeDirection(direction);
    pNode->mRoutes[opposite] = this;
}
void BoxCursorNode::clearRouteAll() { mRoutes.fill(nullptr); }
}
