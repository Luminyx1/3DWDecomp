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
    void initialize(AnimButton* pButton, Screen* pScreen);
    void eraseNodeFromRouteNodes(const BoxCursorNode* pNode);
    bool isMovable(DrawTarget target) const;
    bool isDecidable(DrawTarget target, bool requireDecidable) const;
    void getPosition(sead::Vector2f* pPosition) const;
    void setRouteNodeEach(Direction direction, BoxCursorNode* pNode);
    void clearRouteAll();
    sead::ListNode mListNode;
    AnimButton* mButton;
    Screen* mScreen;
    sead::SafeArray<BoxCursorNode*, 4> mRoutes;
};
static_assert(sizeof(BoxCursorNode) == 0x48, "BoxCursorNode size");
}
