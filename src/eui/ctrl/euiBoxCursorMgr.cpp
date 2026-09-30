#include <eui/euiBoxCursorMgr.h>
#include <eui/euiBoxCursorNode.h>
#include <eui/euiBoxCursorControl.h>
#include <eui/euiScreen.h>
#include <eui/euiScreenMgr.h>
#include <eui/euiLayoutEx.h>
#include <controller/seadControllerBase.h>
namespace eui {
BoxCursorMgr::BoxCursorMgr() : mScreenMgr(nullptr), mAction(Action::cAction_None), mEnabledTargets(0), mControls{} {}
BoxCursorMgr::~BoxCursorMgr() = default;
// action is the cursor movement or decision to process on the next update.
void BoxCursorMgr::setAction(Action action) { mAction = action; }

// pScreen owns the cursor nodes; pName identifies the requested node.
BoxCursorNode* BoxCursorMgr::findNodeByName(Screen* pScreen, const char* pName) {
    return pScreen->findBoxCursorNodeByName_(pName);
}

// pScreenName identifies the screen; pName identifies a node within it.
BoxCursorNode* BoxCursorMgr::findNodeByName(const char* pScreenName, const char* pName) {
    auto* screen = mScreenMgr->findScreenByName(pScreenName);
    return screen ? screen->findBoxCursorNodeByName_(pName) : nullptr;
}

// pScreen owns the nodes; pName identifies the node; pParentParts limits the parent parts.
BoxCursorNode* BoxCursorMgr::findNodeByNameWithParentParts_(Screen* pScreen, const char* pName, const char* pParentParts) {
    return pScreen->findBoxCursorNodeByNameWithParentParts_(pName, pParentParts);
}

// pScreenName identifies the screen; pName and pParentParts identify the node and its parent parts.
BoxCursorNode* BoxCursorMgr::findNodeByNameWithParentParts_(const char* pScreenName, const char* pName, const char* pParentParts) {
    auto* screen = mScreenMgr->findScreenByName(pScreenName);
    return screen ? screen->findBoxCursorNodeByNameWithParentParts_(pName, pParentParts) : nullptr;
}

// pScreen owns the cursor nodes; tag identifies the requested node.
BoxCursorNode* BoxCursorMgr::findNodeByTag(Screen* pScreen, int tag) {
    return pScreen->findBoxCursorNodeByTag_(tag);
}

// pScreenName identifies the screen; tag identifies a node within it.
BoxCursorNode* BoxCursorMgr::findNodeByTag(const char* pScreenName, int tag) {
    auto* screen = mScreenMgr->findScreenByName(pScreenName);
    return screen ? screen->findBoxCursorNodeByTag_(tag) : nullptr;
}

// pNode becomes the reserved cursor selection on its screen's draw target.
void BoxCursorMgr::moveBoxCursor(const BoxCursorNode* pNode) {
    auto* screen = pNode->mScreen;
    if (!screen) return;
    auto* control = mControls[int(screen->getDrawTarget())];
    if (!control) return;
    control->mReservedActiveNode = pNode;
    screen->mLastActiveCursor = pNode;
}

void BoxCursorMgr::update() {
    updateDrawTarget(DrawTarget(0));
    updateDrawTarget(DrawTarget(1));
}

// target selects the display whose cursor input and visibility are updated.
void BoxCursorMgr::updateDrawTarget(DrawTarget target) {
    const int index = target;
    auto* control = mControls[index];
    if (!control) return;
    auto* screen = static_cast<LayoutEx*>(control->_20)->mScreen;
    const bool enabled = isEnable(DrawTarget(index));
    const bool closed = screen->isClosed();
    if (enabled) {
        if (closed) {
            control->updateActiveNode(DrawTarget(index));
            if (!control->mActiveNode) return;
            screen->open(Screen::OpenOption(1));
        }

        if (mAction == Action::cAction_Decide || mAction == Action::cAction_DecideRepeat)
            control->decideNode(mAction == Action::cAction_DecideRepeat);
    } else if (!closed) {
        control->setActiveNode_(nullptr);
        screen->close(Screen::CloseOption(-1));
    }
}

// target selects the display whose active node's screen is returned.
Screen* BoxCursorMgr::getActiveNodeScreen(DrawTarget target) {
    auto* control = mControls[int(target)];
    return control ? control->getActiveNodeScreen() : nullptr;
}

// target selects the display whose current cursor node is returned.
const BoxCursorNode* BoxCursorMgr::getActiveNode(DrawTarget target) const {
    auto* control = mControls[int(target)];
    return control ? control->mActiveNode : nullptr;
}

// target selects the display; rName is the parts layout name to compare with its active node.
bool BoxCursorMgr::isEqualActiveNodePartsLayoutName(DrawTarget target, const sead::SafeString& rName) const {
    auto* control = mControls[int(target)];
    return control ? control->isEqualActiveNodePartsLayoutName(rName) : false;
}

// target selects the cursor display; enabled controls whether its cursor can be shown.
void BoxCursorMgr::setEnable(DrawTarget target, bool enabled) {
    mEnabledTargets.changeBit(int(target), enabled);
}

// pController supplies trigger and repeat inputs; decisions take precedence over directions.
void BoxCursorMgr::setActionWithController(const sead::ControllerBase* pController) {
    Action action = Action::cAction_None;
    if (pController->isTrig(1)) action = Action::cAction_Decide;
    else if (pController->isRepeat(1)) action = Action::cAction_DecideRepeat;
    else if (pController->isTrigWithRepeat(0x110000)) action = Action::cAction_Up;
    else if (pController->isTrigWithRepeat(0x220000)) action = Action::cAction_Down;
    else if (pController->isTrigWithRepeat(0x440000)) action = Action::cAction_Left;
    else if (pController->isTrigWithRepeat(0x880000)) action = Action::cAction_Right;
    setAction(action);
}

// pNode is removed from screen routes and both displays' active or reserved selections.
void BoxCursorMgr::eraseNodeLinks(const BoxCursorNode* pNode) {
    mScreenMgr->eraseBoxCursorNodeFromRouteNodes(pNode);
    for (int i = 0; i < 2; ++i) {
        if (mControls[i]) mControls[i]->clearActiveAndReservedActiveNode(pNode);
    }
}

// target selects the display; pControl supplies its cursor control, or null unregisters it.
void BoxCursorMgr::registerControl(DrawTarget target, BoxCursorControl* pControl) {
    mControls[int(target)] = pControl;
}
}
