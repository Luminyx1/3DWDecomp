#pragma once
#include <prim/seadRuntimeTypeInfo.h>
#include <container/seadSafeArray.h>
namespace eui {
class ScreenMgr;
class BoxCursorNode;
class BoxCursorControl;
class DrawTarget;
class BoxCursorMgr {
public:
    SEAD_RTTI_BASE(BoxCursorMgr);
    enum Action { cAction_None, cAction_Decide, cAction_DecideRepeat,
                  cAction_Up, cAction_Down, cAction_Left, cAction_Right };
    BoxCursorMgr();
    virtual ~BoxCursorMgr();
    virtual void setAction(Action action);
    virtual void moveBoxCursor(const BoxCursorNode* pNode);
    virtual void updateDrawTarget(DrawTarget target);
    virtual void update();
    ScreenMgr* mScreenMgr;
    Action mAction;
    u8 mEnabledTargets;
    sead::SafeArray<BoxCursorControl*, 2> mControls;
};
static_assert(sizeof(BoxCursorMgr) == 0x28, "BoxCursorMgr size");
}
