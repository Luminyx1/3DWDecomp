#include <eui/euiScreen.h>
#include <eui/euiScreenMgr.h>
#include <eui/euiBoxCursorNode.h>
#include <eui/euiAnimator.h>
#include <eui/euiLayoutEx.h>
#include <eui/euiButtonGroup.h>
#include <eui/euiUIController.h>
#include <eui/euiTagProcessor.h>
namespace eui {
Screen::Screen()
    : mScreenMgr(nullptr), mLayout(nullptr), mButtonGroup(nullptr), mController(nullptr),
      mDrawInfo(nullptr), mPrimaryCursor(nullptr), mInitializeHeap(nullptr), mScreenId(-1),
      mName(), _c0(nullptr), mLastActiveCursor(nullptr),
      _d0(nullptr), _d8(nullptr), _e0(0.7853981852531433f), mDrawLayer(-1), mOpenRequest(0),
      mState(0), _e7(0), _e8(0), _e9(0), _ea(0), _eb(0), _ec(0),
      mNoOperationButtonOnSE(1), mFlags(3) {
    mCursorNodes.initOffset(offsetof(BoxCursorNode, mListNode));
}

// pHeap supplies storage for the screen's navigation node.
BoxCursorNode* Screen::createBoxCursorNode(sead::Heap* pHeap) { return new (pHeap, 8) BoxCursorNode; }
// pHeap supplies storage for the layout owned by this screen.
LayoutEx* Screen::doCreateLayout_(sead::Heap* pHeap) { return new (pHeap, 8) LayoutEx(this); }
// pHeap supplies aligned storage for rendering state.
DrawInfoEx* Screen::doCreateDrawInfoEx_(sead::Heap* pHeap) { return new (pHeap, 16) DrawInfoEx; }
// pHeap supplies storage for the screen's button group.
ButtonGroup* Screen::doCreateButtonGroup_(sead::Heap* pHeap) { return new (pHeap, 8) ButtonGroup; }
// pHeap supplies storage for the screen's input controller.
UIController* Screen::doCreateUIController_(sead::Heap* pHeap) { return new (pHeap, 8) UIController; }
// pHeap supplies storage; the screen manager supplies message and font resources.
TagProcessor* Screen::doCreateTagProcessor_(sead::Heap* pHeap) {
    return new (pHeap, 8) TagProcessor(static_cast<MessageMgr*>(mScreenMgr->_430), mScreenMgr->mFontMgr);
}

// pBounds and pNode describe the cursor geometry; the base screen leaves it unchanged.
void Screen::adjstBoxCursor(sead::BoundBox2f* pBounds, const BoxCursorNode* pNode) const {}
// pName is returned unchanged; pParts and pLayout provide context for overrides.
const char* Screen::replacePartsLayoutName(const char* pName, PartsEx* pParts, LayoutEx* pLayout) { return pName; }
// pPane is the new pane, pLayout its owner, and rArgs the construction options.
void Screen::afterBuildPaneCallback(nn::ui2d::Pane* pPane, LayoutEx* pLayout, const nn::ui2d::BuildArgSet& rArgs) {}
// operation identifies the change to pAnimator; the base screen has no additional handling.
void Screen::animatorOperationCallback(AnimatorOperationType operation, Animator* pAnimator) {}
// pHeap is available to derived screens for resources created after layout construction.
void Screen::doAfterBuildLayout_(sead::Heap* pHeap) {}
// pHeap is available to derived screens for their initialization resources.
void Screen::doInitialize_(sead::Heap* pHeap) {}
void Screen::doUpdate_() {}
void Screen::doOpenStart_() {}
void Screen::doOpenEnd_() {}
void Screen::doCloseStart_() {}
void Screen::doCloseEnd_() {}
// pButton is entering its highlighted state.
void Screen::doButtonOnStart_(AnimButton* pButton) {}
// pButton has completed its highlight transition.
void Screen::doButtonOnEnd_(AnimButton* pButton) {}
// pButton is leaving its highlighted state.
void Screen::doButtonOffStart_(AnimButton* pButton) {}
// pButton has completed its unhighlight transition.
void Screen::doButtonOffEnd_(AnimButton* pButton) {}
// pButton is entering its pressed state.
void Screen::doButtonDownStart_(AnimButton* pButton) {}
// pButton has completed its press transition.
void Screen::doButtonDownEnd_(AnimButton* pButton) {}
// pButton is starting cancellation.
void Screen::doButtonCancelStart_(AnimButton* pButton) {}
// pButton has completed cancellation.
void Screen::doButtonCancelEnd_(AnimButton* pButton) {}
xlink2::System* Screen::getElinkSystem_() const { return nullptr; }
// pUser is the sound-link user for which a derived screen can supply a resource list.
const char* Screen::getSlink2ResourceList_(xlink2::UserInstanceSLink* pUser) const { return nullptr; }
u32 Screen::getSlink2LocalPropertyNum_() const { return 0; }
// pUser receives any sound properties defined by a derived screen.
void Screen::setSlink2PropertyDefinition_(xlink2::UserInstanceSLink* pUser) {}
bool Screen::isForceGlbMtxDirty_() const { return false; }
// pAnimator is appended to this screen's active animation list.
void Screen::setAnimatorActive(Animator* pAnimator) { mActiveAnimators.push_back(*pAnimator); }
// pAnimator is detached from its current active animation list.
void Screen::eraseAnimatorFromActiveList(Animator* pAnimator) {
    mActiveAnimators.erase(mActiveAnimators.iterator_to(*pAnimator));
}

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
bool Screen::isOpening() const {
    if (mState == 1) return true;
    return mOpenRequest >= 1 && ((mState == 0) | (mState == 3));
}

// NON_MATCHING: the closing-state check still uses different boolean instructions.
bool Screen::isClosing() const {
    if (mState == 3) return true;
    return mOpenRequest < 0 && ((mState == 1) | (mState == 2));
}

// own determines whether this screen owns its initialization heap.
void Screen::setOwnInitializeHeap(bool own) {
    if (own) mFlags |= 1;
    else mFlags &= ~1;
}

void Screen::muteNextNoOperationButtonOnSE_() { mNoOperationButtonOnSE = 0; }
}
