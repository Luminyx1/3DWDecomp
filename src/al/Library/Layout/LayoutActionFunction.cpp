#include "Library/Layout/LayoutActionFunction.hpp"

#include "Library/Layout/IUseLayoutAction.hpp"
#include "Library/Layout/LayoutActor.hpp"
#include "Library/Layout/LayoutPaneGroup.hpp"
#include "Library/Layout/LayoutTextPaneAnimator.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Message/LanguageUtil.hpp"
#include "Library/Message/MessageHolder.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Layout/LayoutActionKeeper.hpp"

namespace al {
namespace {
inline LayoutPaneGroup* getLayoutPaneGroup(const IUseLayoutAction* pLayout,
                                           const char* pPaneName) {
    return pLayout->getLayoutActionKeeper()->getLayoutPaneGroup(pPaneName);
}
}  // namespace

/**
 * Starts an action.
 * @param pLayout layout action user
 * @param pActionName action name
 * @param pPaneName pane group name, or nullptr for the main group
 */
void startAction(IUseLayoutAction* pLayout, const char* pActionName, const char* pPaneName) {
    pLayout->getLayoutActionKeeper()->startAction(pActionName, pPaneName);
}

/**
 * Checks whether an action is playing but paused.
 * @param pLayout layout action user
 * @param pActionName action name
 * @param pPaneName pane group name, or nullptr for the main group
 * @return whether the action is paused
 */
bool isPausedAction(IUseLayoutAction* pLayout, const char* pActionName, const char* pPaneName) {
    LayoutPaneGroup* paneGroup = getLayoutPaneGroup(pLayout, pPaneName);

    if (paneGroup == nullptr || !paneGroup->isAnimPlaying()) {
        return false;
    }

    if (strcmp(paneGroup->getPlayingAnimName(), pActionName) != 0) {
        return false;
    }

    return paneGroup->getAnimFrameRate() == 0.0f;
}

/**
 * Pauses the action of a pane group.
 * @param pLayout layout action user
 * @param pPaneName pane group name, or nullptr for the main group
 */
void pauseAction(IUseLayoutAction* pLayout, const char* pPaneName) {
    LayoutPaneGroup* paneGroup = getLayoutPaneGroup(pLayout, pPaneName);

    if (paneGroup != nullptr) {
        paneGroup->setAnimFrameRate(0.0f);
    }
}

/**
 * Resumes the action of a pane group.
 * @param pLayout layout action user
 * @param pPaneName pane group name, or nullptr for the main group
 */
void unpauseAction(IUseLayoutAction* pLayout, const char* pPaneName) {
    LayoutPaneGroup* paneGroup = getLayoutPaneGroup(pLayout, pPaneName);

    if (paneGroup != nullptr) {
        paneGroup->setAnimFrameRate(1.0f);
    }
}

/**
 * Starts an action at a random frame.
 * @param pLayout layout action user
 * @param pActionName action name
 * @param pPaneName pane group name, or nullptr for the main group
 * @return the chosen frame
 */
s32 startActionAtRandomFrame(IUseLayoutAction* pLayout, const char* pActionName,
                             const char* pPaneName) {
    startAction(pLayout, pActionName, pPaneName);
    LayoutPaneGroup* paneGroup = getLayoutPaneGroup(pLayout, pPaneName);
    f32 frameMax = paneGroup->getAnimFrameMax(pActionName);
    f32 frame = getRandom(0.0f, frameMax);
    paneGroup->setAnimFrame(static_cast<s32>(frame));
    return frame;
}

/**
 * Starts an action frozen at a frame.
 * @param pLayout layout action user
 * @param pActionName action name
 * @param frame frame to freeze at
 * @param pPaneName pane group name, or nullptr for the main group
 */
void startFreezeAction(IUseLayoutAction* pLayout, const char* pActionName, f32 frame,
                       const char* pPaneName) {
    startAction(pLayout, pActionName, pPaneName);
    LayoutPaneGroup* paneGroup = getLayoutPaneGroup(pLayout, pPaneName);
    paneGroup->setAnimFrame(frame);
    paneGroup->setAnimFrameRate(0.0f);
}

/**
 * Starts an action frozen at its last frame.
 * @param pLayout layout action user
 * @param pActionName action name
 * @param pPaneName pane group name, or nullptr for the main group
 */
void startFreezeActionEnd(IUseLayoutAction* pLayout, const char* pActionName,
                          const char* pPaneName) {
    f32 frameMax = getActionFrameMax(pLayout, pActionName, pPaneName);
    startFreezeAction(pLayout, pActionName, frameMax, pPaneName);
}

/**
 * Returns the frame count of an action.
 * @param pLayout layout action user
 * @param pActionName action name
 * @param pPaneName pane group name, or nullptr for the main group
 * @return last frame of the action
 */
f32 getActionFrameMax(const IUseLayoutAction* pLayout, const char* pActionName,
                      const char* pPaneName) {
    return getLayoutPaneGroup(pLayout, pPaneName)->getAnimFrameMax(pActionName);
}

/**
 * Starts an action frozen at the frame matching a value in a range.
 * @param pLayout layout action user
 * @param value gauge value
 * @param minFrame value mapped to the first frame
 * @param maxFrame value mapped to the last frame
 * @param pActionName action name
 * @param pPaneName pane group name, or nullptr for the main group
 */
void startFreezeGaugeAction(IUseLayoutAction* pLayout, f32 value, f32 minFrame, f32 maxFrame,
                            const char* pActionName, const char* pPaneName) {
    f32 frame =
        calcRate01(value, minFrame, maxFrame) * getActionFrameMax(pLayout, pActionName, pPaneName);
    startFreezeAction(pLayout, pActionName, frame, pPaneName);
}

/**
 * Starts an action if it exists.
 * @param pLayout layout action user
 * @param pActionName action name
 * @param pPaneName pane group name, or nullptr for the main group
 * @return whether the action was started
 */
bool tryStartAction(IUseLayoutAction* pLayout, const char* pActionName, const char* pPaneName) {
    if (!isExistAction(pLayout, pActionName, pPaneName)) {
        return false;
    }

    startAction(pLayout, pActionName, pPaneName);
    return true;
}

/**
 * Checks whether an action exists.
 * @param pLayout layout action user
 * @param pActionName action name
 * @param pPaneName pane group name, or nullptr for the main group
 * @return whether the action exists
 */
bool isExistAction(const IUseLayoutAction* pLayout, const char* pActionName,
                   const char* pPaneName) {
    return isExistAction(pLayout, pPaneName) &&
           getLayoutPaneGroup(pLayout, pPaneName)->isAnimExist(pActionName);
}

/**
 * Checks whether the action of a pane group has ended.
 * @param pLayout layout action user
 * @param pPaneName pane group name, or nullptr for the main group
 * @return whether the action has ended
 */
bool isActionEnd(const IUseLayoutAction* pLayout, const char* pPaneName) {
    LayoutPaneGroup* paneGroup = getLayoutPaneGroup(pLayout, pPaneName);

    if (paneGroup != nullptr && paneGroup->isAnimPlaying() && paneGroup->isAnimOneTime()) {
        return paneGroup->isAnimEnd();
    }

    return true;
}

/**
 * Checks whether a pane group exists.
 * @param pLayout layout action user
 * @param pPaneName pane group name, or nullptr for the main group
 * @return whether the pane group exists
 */
bool isExistAction(const IUseLayoutAction* pLayout, const char* pPaneName) {
    return pLayout->getLayoutActionKeeper()->getLayoutPaneGroup(pPaneName) != nullptr;
}

/**
 * Checks whether an action plays only once.
 * @param pLayout layout action user
 * @param pActionName action name
 * @param pPaneName pane group name, or nullptr for the main group
 * @return whether the action doesn't loop
 */
bool isActionOneTime(const IUseLayoutAction* pLayout, const char* pActionName,
                     const char* pPaneName) {
    return getLayoutPaneGroup(pLayout, pPaneName)->isAnimOneTime(pActionName);
}

/**
 * Returns the current frame of a pane group's action.
 * @param pLayout layout action user
 * @param pPaneName pane group name, or nullptr for the main group
 * @return current frame
 */
f32 getActionFrame(const IUseLayoutAction* pLayout, const char* pPaneName) {
    return getLayoutPaneGroup(pLayout, pPaneName)->getAnimFrame();
}

/**
 * Sets the current frame of a pane group's action.
 * @param pLayout layout action user
 * @param frame new frame
 * @param pPaneName pane group name, or nullptr for the main group
 */
void setActionFrame(IUseLayoutAction* pLayout, f32 frame, const char* pPaneName) {
    getLayoutPaneGroup(pLayout, pPaneName)->setAnimFrame(frame);
}

/**
 * Returns the frame count of a pane group's action.
 * @param pLayout layout action user
 * @param pPaneName pane group name, or nullptr for the main group
 * @return last frame
 */
f32 getActionFrameMax(const IUseLayoutAction* pLayout, const char* pPaneName) {
    return getLayoutPaneGroup(pLayout, pPaneName)->getAnimFrameMax();
}

/**
 * Returns the frame rate of a pane group's action.
 * @param pLayout layout action user
 * @param pPaneName pane group name, or nullptr for the main group
 * @return frame rate
 */
f32 getActionFrameRate(const IUseLayoutAction* pLayout, const char* pPaneName) {
    return getLayoutPaneGroup(pLayout, pPaneName)->getAnimFrameRate();
}

/**
 * Sets the frame rate of a pane group's action.
 * @param pLayout layout action user
 * @param frameRate new frame rate
 * @param pPaneName pane group name, or nullptr for the main group
 */
void setActionFrameRate(IUseLayoutAction* pLayout, f32 frameRate, const char* pPaneName) {
    getLayoutPaneGroup(pLayout, pPaneName)->setAnimFrameRate(frameRate);
}

/**
 * Returns the name of a pane group's playing action.
 * @param pLayout layout action user
 * @param pPaneName pane group name, or nullptr for the main group
 * @return action name
 */
const char* getActionName(const IUseLayoutAction* pLayout, const char* pPaneName) {
    return getLayoutPaneGroup(pLayout, pPaneName)->getPlayingAnimName();
}

/**
 * Checks whether an action is playing.
 * @param pLayout layout action user
 * @param pActionName action name
 * @param pPaneName pane group name, or nullptr for the main group
 * @return whether the action is playing
 */
bool isActionPlaying(const IUseLayoutAction* pLayout, const char* pActionName,
                     const char* pPaneName) {
    return isEqualString(getLayoutPaneGroup(pLayout, pPaneName)->getPlayingAnimName(),
                         pActionName);
}

/**
 * Checks whether any action of a pane group is playing.
 * @param pLayout layout action user
 * @param pPaneName pane group name, or nullptr for the main group
 * @return whether an action is playing
 */
bool isAnyActionPlaying(const IUseLayoutAction* pLayout, const char* pPaneName) {
    return getLayoutPaneGroup(pLayout, pPaneName)->isAnimPlaying();
}

/**
 * Changes the nerve once the main action has ended.
 * @param pActor layout actor
 * @param pNerve nerve to change to
 */
void setNerveAtActionEnd(LayoutActor* pActor, const Nerve* pNerve) {
    if (isActionEnd(pActor, nullptr)) {
        setNerve(pActor, pNerve);
    }
}

/**
 * Starts the text pane animation of a message.
 * @param pActor layout actor
 * @param pMessage message
 * @param pTagDataHolder tag data holder
 * @param pReplaceTagProcessor replace tag processor
 */
void startTextPaneAnim(LayoutActor* pActor, const char16_t* pMessage,
                       const MessageTagDataHolder* pTagDataHolder,
                       const ReplaceTagProcessorBase* pReplaceTagProcessor) {
    pActor->getTextPaneAnimator()->start(pMessage, pTagDataHolder, pReplaceTagProcessor);
}

/**
 * Starts the text pane animation of a message, playing voices through an audio keeper.
 * @param pActor layout actor
 * @param pMessage message
 * @param pTagDataHolder tag data holder
 * @param pReplaceTagProcessor replace tag processor
 * @param pAudioKeeper audio keeper user
 */
void startTextPaneAnimWithAudioUser(LayoutActor* pActor, const char16_t* pMessage,
                                    const MessageTagDataHolder* pTagDataHolder,
                                    const ReplaceTagProcessorBase* pReplaceTagProcessor,
                                    const IUseAudioKeeper* pAudioKeeper) {
    pActor->getTextPaneAnimator()->setAudioKeeper(pAudioKeeper);
    pActor->getTextPaneAnimator()->start(pMessage, pTagDataHolder, pReplaceTagProcessor);
}

/**
 * Starts the text pane animation of a stage message.
 * @param pActor layout actor
 * @param pFileName message file name
 * @param pLabel text label
 * @param pTagDataHolder tag data holder
 * @param pReplaceTagProcessor replace tag processor
 */
void startAndSetTextPaneAnimStage(LayoutActor* pActor, const char* pFileName, const char* pLabel,
                                  const MessageTagDataHolder* pTagDataHolder,
                                  const ReplaceTagProcessorBase* pReplaceTagProcessor) {
    const char16_t* message = getStageMessageString(pActor, pFileName, pLabel);
    pActor->getTextPaneAnimator()->start(message, pTagDataHolder, pReplaceTagProcessor);
}

/**
 * Starts the text pane animation of a system message.
 * @param pActor layout actor
 * @param pFileName message file name
 * @param pLabel text label
 * @param pTagDataHolder tag data holder
 * @param pReplaceTagProcessor replace tag processor
 */
void startAndSetTextPaneAnimSystem(LayoutActor* pActor, const char* pFileName, const char* pLabel,
                                   const MessageTagDataHolder* pTagDataHolder,
                                   const ReplaceTagProcessorBase* pReplaceTagProcessor) {
    const char16_t* message = getSystemMessageString(pActor, pFileName, pLabel);
    pActor->getTextPaneAnimator()->start(message, pTagDataHolder, pReplaceTagProcessor);
}

/**
 * Ends the text pane animation.
 * @param pActor layout actor
 */
void endTextPaneAnim(LayoutActor* pActor) {
    pActor->getTextPaneAnimator()->end();
}

/**
 * Skips to the end of the current page of the text pane animation.
 * @param pActor layout actor
 */
void skipTextPaneAnim(LayoutActor* pActor) {
    pActor->getTextPaneAnimator()->skip();
}

/**
 * Flushes the text pane animation.
 * @param pActor layout actor
 */
void flushTextPaneAnim(LayoutActor* pActor) {
    pActor->getTextPaneAnimator()->flush();
}

/**
 * Changes to the next page of the text pane animation.
 * @param pActor layout actor
 * @param pTagDataHolder tag data holder
 * @param pReplaceTagProcessor replace tag processor
 */
void changeNextPage(LayoutActor* pActor, const MessageTagDataHolder* pTagDataHolder,
                    const ReplaceTagProcessorBase* pReplaceTagProcessor) {
    pActor->getTextPaneAnimator()->changeNextPage(pTagDataHolder, pReplaceTagProcessor);
}

/**
 * Changes to the next page of the text pane animation if there is one.
 * @param pActor layout actor
 * @param pTagDataHolder tag data holder
 * @param pReplaceTagProcessor replace tag processor
 * @return whether the page was changed
 */
bool tryChangeNextPage(LayoutActor* pActor, const MessageTagDataHolder* pTagDataHolder,
                       const ReplaceTagProcessorBase* pReplaceTagProcessor) {
    LayoutTextPaneAnimator* animator = pActor->getTextPaneAnimator();

    if (!animator->isExistNextPage()) {
        return false;
    }

    animator->changeNextPage(pTagDataHolder, pReplaceTagProcessor);
    return true;
}

/**
 * Checks whether the text pane animation has another page.
 * @param pActor layout actor
 * @return whether a next page exists
 */
bool isExistNextPage(const LayoutActor* pActor) {
    return pActor->getTextPaneAnimator()->isExistNextPage();
}

/**
 * Checks whether the text pane animation has ended.
 * @param pActor layout actor
 * @param isCheckNextPage whether remaining pages also have to be shown
 * @return whether the animation has ended
 */
bool isEndTextPaneAnim(const LayoutActor* pActor, bool isCheckNextPage) {
    LayoutTextPaneAnimator* animator = pActor->getTextPaneAnimator();
    return !animator->isAnimating() && (!isCheckNextPage || !animator->isExistNextPage());
}

/**
 * Returns the current message of the text pane animation.
 * @param pActor layout actor
 * @return current message
 */
const char16_t* getCurrentMessagePaneAnim(const LayoutActor* pActor) {
    return pActor->getTextPaneAnimator()->getCurrentMessage();
}

/**
 * Returns the number of characters of the current page of the text pane animation.
 * @param pActor layout actor
 * @return number of characters
 */
s32 calcCurrentMessageTextNum(const LayoutActor* pActor) {
    const char16_t* nextMessage = pActor->getTextPaneAnimator()->tryGetNextMessage();
    return calcMessageSizeWithoutTag(pActor->getTextPaneAnimator()->getCurrentMessage(),
                                     nextMessage);
}

/**
 * Returns the time needed to show a number of characters.
 * @param textNum number of characters
 * @return time in frames
 */
s32 calcShowTextTime(s32 textNum) {
    return (isLanguageEmQuad() ? 10 : 5) * textNum;
}

/**
 * Starts the text animation action requested by a message.
 * @param pActor layout actor
 * @param pMessage message
 * @return whether a text animation tag was found
 */
bool tryStartTextAnim(LayoutActor* pActor, const char16_t* pMessage) {
    StringTmp<64> animName;
    animName.clear();
    tryGetMessageTagTextAnim(&animName, pActor, pMessage);

    if (animName.isEmpty()) {
        startAction(pActor, "Normal", "Font");
        return false;
    }

    startAction(pActor, animName.cstr(), "Font");
    return true;
}
}  // namespace al
