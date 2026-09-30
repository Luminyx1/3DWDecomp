#include <eui/euiScreen.h>
#include <eui/euiScreenMgr.h>
#include <eui/euiBoxCursorNode.h>
namespace eui {
// pNode is removed from every cursor node's navigation routes.
void Screen::eraseBoxCursorNodeFromRouteNodes(const BoxCursorNode* pNode) {
    for (auto& node : mCursorNodes) node.eraseNodeFromRouteNodes(pNode);
}

bool Screen::isEnableControl() const { return false; }
const char* Screen::getLayoutName_() const { return nullptr; }
const char* Screen::getMessageName_() const { return getLayoutName_(); }
const char* Screen::getArchiveName_() const { return getLayoutName_(); }
bool Screen::isPlayPartsInOut_() const { return false; }
bool Screen::isDisallowHitLowerScreenOnButtonHit_() const { return true; }
float Screen::getAnimationStep_() const { return mScreenMgr->mAnimationStep; }
void Screen::updateControl_() {
    const float step = getAnimationStep_();
    for (auto& control : mControls) control.Update(step);
}
void Screen::updateStaticControl_() {
    const float step = getAnimationStep_();
    for (auto& control : mStaticControls) control.Update(step);
}
bool Screen::isOpened() const { return mState == 2 && mOpenRequest >= 0; }
bool Screen::isClosed() const { return mState == 0 && mOpenRequest < 1; }
// NON_MATCHING: the state/request expression compiles to different branches.
bool Screen::isOpening() const {
    return mState == 1 || ((mState == 0 || mState == 3) && mOpenRequest >= 1);
}
// NON_MATCHING: the state/request expression compiles to different branches.
bool Screen::isClosing() const {
    return mState == 3 || ((mState == 1 || mState == 2) && mOpenRequest < 0);
}
// own determines whether this screen owns its initialization heap.
void Screen::setOwnInitializeHeap(bool own) {
    if (own) mFlags |= 1;
    else mFlags &= ~1;
}
void Screen::muteNextNoOperationButtonOnSE_() { mNoOperationButtonOnSE = 0; }
}
