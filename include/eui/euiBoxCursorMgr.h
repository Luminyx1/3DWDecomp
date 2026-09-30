#pragma once
#include <prim/seadRuntimeTypeInfo.h>
#include <container/seadSafeArray.h>
#include <prim/seadSafeString.h>
#include <eui/euiUtility.h>
#include <prim/seadBitFlag.h>
namespace sead { class ControllerBase; }
namespace eui {
class ScreenMgr;
class BoxCursorNode;
class BoxCursorControl;
class Screen;
class BoxCursorMgr {
public:
    SEAD_RTTI_BASE(BoxCursorMgr);
    SEAD_ENUM(Action, cAction_None, cAction_Decide, cAction_DecideRepeat,
              cAction_Up, cAction_Down, cAction_Left, cAction_Right)
    BoxCursorMgr();
    virtual ~BoxCursorMgr();
    virtual void setAction(Action action);
    virtual void moveBoxCursor(const BoxCursorNode* pNode);
    virtual void updateDrawTarget(DrawTarget target);
    virtual void update();
    void registerControl(DrawTarget target, BoxCursorControl* pControl);
    BoxCursorNode* findNodeByName(Screen* pScreen, const char* pName);
    BoxCursorNode* findNodeByName(const char* pScreenName, const char* pName);
    BoxCursorNode* findNodeByNameWithParentParts_(Screen* pScreen, const char* pName, const char* pParentParts);
    BoxCursorNode* findNodeByNameWithParentParts_(const char* pScreenName, const char* pName, const char* pParentParts);
    BoxCursorNode* findNodeByTag(Screen* pScreen, int tag);
    BoxCursorNode* findNodeByTag(const char* pScreenName, int tag);
    Screen* getActiveNodeScreen(DrawTarget target);
    const BoxCursorNode* getActiveNode(DrawTarget target) const;
    bool isEqualActiveNodePartsLayoutName(DrawTarget target, const sead::SafeString& rName) const;
    void setEnable(DrawTarget target, bool enabled);
    // target selects the display whose enable bit is tested.
    bool isEnable(DrawTarget target) const { return mEnabledTargets.isOnBit(int(target)); }
    void setActionWithController(const sead::ControllerBase* pController);
    void eraseNodeLinks(const BoxCursorNode* pNode);
    ScreenMgr* mScreenMgr;
    Action mAction;
    sead::BitFlag8 mEnabledTargets;
    sead::SafeArray<BoxCursorControl*, 2> mControls;
};

static_assert(sizeof(BoxCursorMgr) == 0x28, "BoxCursorMgr size");
}
