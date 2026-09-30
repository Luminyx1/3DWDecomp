#pragma once
#include <eui/euiUtility.h>
#include <container/seadListImpl.h>
#include <container/seadSafeArray.h>
#include <prim/seadRuntimeTypeInfo.h>
namespace eui {
class AnimButton;
class Screen;
class DrawTarget;
class BoxCursorNode {
public:
    SEAD_RTTI_BASE(BoxCursorNode);
    BoxCursorNode();
    virtual ~BoxCursorNode();
    virtual void initialize(AnimButton* pButton, Screen* pScreen);
    void eraseNodeFromRouteNodes(const BoxCursorNode* pNode);
    virtual bool isMovable(DrawTarget target) const;
    virtual bool isDecidable(DrawTarget target, bool requireDecidable) const;
    void getPosition(sead::Vector2f* pPosition) const;
    // direction selects the outgoing route; pNode is its destination.
    void setRouteNode(Direction direction, BoxCursorNode* pNode) { mRoutes[direction] = pNode; }
    void setRouteNodeEach(Direction direction, BoxCursorNode* pNode);
    void clearRouteAll();
    sead::ListNode mListNode;
    AnimButton* mButton;
    Screen* mScreen;
    sead::SafeArray<BoxCursorNode*, 4> mRoutes;
};

static_assert(sizeof(BoxCursorNode) == 0x48, "BoxCursorNode size");
}
