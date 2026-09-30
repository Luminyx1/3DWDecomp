#pragma once
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadRuntimeTypeInfo.h>
#include <container/seadOffsetList.h>
#include <math/seadBoundBox.h>
#include <eui/euiDrawInfoEx.h>
#include <eui/euiControlCreator.h>
#include <eui/euiButtonBase.h>
#include <eui/euiAnimator.h>
namespace xlink2 { class UserInstanceSLink; class System; }
namespace nn::ui2d { class ResourceAccessor; }
namespace eui {
class ScreenMgr; class LayoutEx; class BoxCursorNode; class PartsEx; class Animator;
class UIController; class MultiArcResourceAccessor; class TagProcessor;
class DrawTarget;
class Screen : public sead::IDisposer, public sead::hostio::Node {
public:
    class OpenOption; class CloseOption;
    enum AnimatorOperationType { cPlay, cPlayFromCurrent, cStop, cStopCurrent, cStopAtMin, cStopAtMax };
    Screen();
    ~Screen() override;
    SEAD_RTTI_BASE(Screen);
    virtual bool isEnableControl() const;
    virtual void open(OpenOption);
    virtual void close(CloseOption);
    virtual void adjstBoxCursor(sead::BoundBox2f*, const BoxCursorNode*) const;
    virtual BoxCursorNode* createBoxCursorNode(sead::Heap*);
    virtual const char* replacePartsLayoutName(const char*, PartsEx*, LayoutEx*);
    virtual void afterBuildPaneCallback(nn::ui2d::Pane*, LayoutEx*, const nn::ui2d::BuildArgSet&);
    virtual void animatorOperationCallback(AnimatorOperationType, Animator*);
    virtual void initialize(ScreenMgr*, sead::Heap*, const char*, int, s8, bool);
    virtual void update();
    virtual void draw(const DrawInfoEx::RenderBufferInfo*);
    virtual const char* getLayoutName_() const;
    virtual const char* getMessageName_() const;
    virtual const char* getArchiveName_() const;
    virtual bool isPlayPartsInOut_() const;
    virtual bool isDisallowHitLowerScreenOnButtonHit_() const;
    virtual LayoutEx* doCreateLayout_(sead::Heap*);
    virtual DrawInfoEx* doCreateDrawInfoEx_(sead::Heap*);
    virtual ButtonGroup* doCreateButtonGroup_(sead::Heap*);
    virtual void doAfterBuildLayout_(sead::Heap*);
    virtual void doSetupDrawInfo_();
    virtual UIController* doCreateUIController_(sead::Heap*);
    virtual MultiArcResourceAccessor* doCreateResourceAccessor_(sead::Heap*);
    virtual TagProcessor* doCreateTagProcessor_(sead::Heap*);
    virtual void doBuildLayout_(const sead::SafeString&, nn::ui2d::ResourceAccessor*);
    virtual void doLoadResource_(sead::Heap*);
    virtual void doInitialize_(sead::Heap*);
    virtual void doUpdate_();
    virtual float getAnimationStep_() const;
    virtual void doDraw_(const DrawInfoEx::RenderBufferInfo*);
    virtual void doOpenStart_();
    virtual void doOpenEnd_();
    virtual void doCloseStart_();
    virtual void doCloseEnd_();
    virtual void doButtonOnStart_(AnimButton*);
    virtual void doButtonOnEnd_(AnimButton*);
    virtual void doButtonOffStart_(AnimButton*);
    virtual void doButtonOffEnd_(AnimButton*);
    virtual void doButtonDownStart_(AnimButton*);
    virtual void doButtonDownEnd_(AnimButton*);
    virtual void doButtonCancelStart_(AnimButton*);
    virtual void doButtonCancelEnd_(AnimButton*);
    virtual xlink2::System* getElinkSystem_() const;
    virtual const char* getSlink2ResourceList_(xlink2::UserInstanceSLink*) const;
    virtual u32 getSlink2LocalPropertyNum_() const;
    virtual void setSlink2PropertyDefinition_(xlink2::UserInstanceSLink*);
    virtual void updateButton_();
    virtual void updateControl_();
    virtual void openStart_(OpenOption);
    virtual bool isOpenEnd_();
    virtual void openEnd_();
    virtual void closeStart_(CloseOption);
    virtual bool isCloseEnd_();
    virtual void closeEnd_();
    virtual bool isForceGlbMtxDirty_() const;
    virtual void updateAnimator_();
    virtual void registerController_();
    virtual void unregisterController_();
    virtual void setupPaneAfterBuild_(nn::ui2d::Pane*, LayoutEx*, u32*);
    virtual void countEffectLinkPane_(nn::ui2d::Pane*, u32*);
    virtual void createEffectLinkUser_(sead::Heap*, u32);
    virtual void createSoundLink2User_(sead::Heap*);
    virtual void invokeSoundLink2Event_(const char*);
    virtual void invokeSoundLink2ButtonEvent_(AnimButton*, const char*);
    virtual void invokeSoundLink2AnimPlayEvent(Animator*, const char*);
    bool isOpened() const;
    bool isClosed() const;
    bool isOpening() const;
    bool isClosing() const;
    void setOwnInitializeHeap(bool own);
    void muteNextNoOperationButtonOnSE_();
    void updateStaticControl_();
    DrawTarget getDrawTarget() const;
    void eraseBoxCursorNodeFromRouteNodes(const BoxCursorNode* pNode);
    void setAnimatorActive(Animator* pAnimator);
    void eraseAnimatorFromActiveList(Animator* pAnimator);
    bool moveBoxCursorByButton(const AnimButton* pButton);
    void buttonStateChangeCallback(AnimButton* pButton, ButtonBase::State oldState, ButtonBase::State newState);
    ScreenMgr* mScreenMgr;
    LayoutEx* mLayout;
    ButtonGroup* mButtonGroup;
    ControlList mControls;
    ControlList mStaticControls;
    UIController* mController;
    DrawInfoEx* mDrawInfo;
    BoxCursorNode* mPrimaryCursor;
    nn::util::IntrusiveList<Animator,
        nn::util::IntrusiveListMemberNodeTraits<Animator, &Animator::mActiveLink>> mActiveAnimators;
    sead::OffsetList<BoxCursorNode> mCursorNodes;
    sead::Heap* mInitializeHeap;
    int mScreenId;
    sead::SafeString mName;
    void* _c0;
    const BoxCursorNode* mLastActiveCursor;
    void* _d0;
    void* _d8;
    float _e0;
    s8 mDrawLayer;
    s8 mOpenRequest;
    u8 mState;
    u8 _e7, _e8, _e9, _ea, _eb, _ec;
    u8 mNoOperationButtonOnSE;
    u8 mFlags;
};
static_assert(sizeof(Screen) == 0xf0, "Screen size");
}
