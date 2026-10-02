#include "Library/Layout/LayoutTextPaneAnimator.hpp"

#include <cstring>
#include <eui/euiScreen.h>
#include <eui/euiScreenMgr.h>
#include <eui/euiTagProcessor.h>
#include <eui/euiTextBoxEx.h>
#include <nn/ui2d/ui2d_TextBox.h>

#include "Library/Layout/LayoutActionFunction.hpp"
#include "Library/Layout/LayoutActor.hpp"
#include "Library/Layout/LayoutActorUtil.hpp"
#include "Library/Layout/LayoutKeeper.hpp"
#include "Library/Memory/Util.hpp"
#include "Library/Message/MessageHolder.hpp"
#include "Library/Message/ReplaceTagProcessorBase.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Se/Project/SePlayParamList.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
using TagInfo = sead::MessageSet<char16_t>::TagInfo;

class TextPaneAppTagCallback : public eui::LetterAnimControl::AppTagCallback {
public:
    TextPaneAppTagCallback(LayoutTextPaneAnimator* pAnimator) : mAnimator(pAnimator) {}

    void invoke(const TagInfo* pTagInfo) override;

private:
    LayoutTextPaneAnimator* mAnimator;
};

class TextPaneTagProcessor : public eui::TagProcessor {
public:
    TextPaneTagProcessor(eui::MessageMgr* pMessageMgr, eui::FontMgr* pFontMgr)
        : eui::TagProcessor(pMessageMgr, pFontMgr) {}

    void preProcessSystemTag_(const TagInfo* pTagInfo, char16_t* pDst, u32* pDstLength,
                              u32* pCharNum, u32 dstSize, const char16_t* pTag, u32 tagLength,
                              void* pUserData) override {
        copyTag(pDst, pDstLength, dstSize, pTag, tagLength);
    }

    void preProcessEuiTag_(const TagInfo* pTagInfo, char16_t* pDst, u32* pDstLength,
                           u32* pCharNum, u32 dstSize, const char16_t* pTag, u32 tagLength,
                           void* pUserData) override {
        copyTag(pDst, pDstLength, dstSize, pTag, tagLength);
    }

    void preProcessGrammarTag_(const TagInfo* pTagInfo, char16_t* pDst, u32* pDstLength,
                               u32* pCharNum, u32 dstSize, const char16_t* pTag, u32 tagLength,
                               void* pUserData) override {
        copyTag(pDst, pDstLength, dstSize, pTag, tagLength);
    }

    void preProcessAppTag_(const TagInfo* pTagInfo, char16_t* pDst, u32* pDstLength,
                           u32* pCharNum, u32 dstSize, const char16_t* pTag, u32 tagLength,
                           void* pUserData) override {
        copyTag(pDst, pDstLength, dstSize, pTag, tagLength);
    }

private:
    static void copyTag(char16_t* pDst, u32* pDstLength, u32 dstSize, const char16_t* pTag,
                        u32 tagLength) {
        if (*pDstLength + tagLength < dstSize) {
            memcpy(pDst + *pDstLength, pTag, tagLength * 2);
            *pDstLength += tagLength;
        }
    }
};

EuiLetterAnimCtrl* createEuiLetterAnimCtrl(nn::ui2d::TextBox* pTextBox, eui::LayoutEx* pLayout,
                                           LayoutTextPaneAnimator* pAnimator) {
    EuiLetterAnimCtrl* letterAnimCtrl = new EuiLetterAnimCtrl();
    letterAnimCtrl->initialize(getCurrentHeap(), static_cast<eui::TextBoxEx*>(pTextBox), pLayout);
    letterAnimCtrl->setSpeed(16.0f);
    letterAnimCtrl->setAppTagCallback(new TextPaneAppTagCallback(pAnimator));
    return letterAnimCtrl;
}

bool startLetterAnim(EuiLetterAnimCtrl* pLetterAnimCtrl, const char16_t* pText,
                     LayoutActor* pActor) {
    pLetterAnimCtrl->changeText(pText, calcMessageSizeWithoutNullCharacter(pText, nullptr), 0);
    pLetterAnimCtrl->reset();
    pLetterAnimCtrl->setSkip(false);
    pLetterAnimCtrl->start();
    StringTmp<64> animName;
    animName.clear();
    tryGetMessageTagTextAnim(&animName, pActor, pText);

    if (animName.isEmpty()) {
        if (pActor->getLayoutKeeper()->getGroup("Font") != nullptr) {
            startAction(pActor, "Normal", "Font");
        }

        return false;
    }

    startAction(pActor, animName.cstr(), "Font");
    return true;
}
}  // namespace

/**
 * Creates a text pane animator.
 * @param pTextBox animated text box
 * @param pActor layout actor owning the text box
 */
LayoutTextPaneAnimator::LayoutTextPaneAnimator(nn::ui2d::TextBox* pTextBox, LayoutActor* pActor)
    : mTextBox(pTextBox), mActor(pActor) {
    mSePlayParamList = new SePlayParamList();
    mBaseTrans = mTextBox->GetTranslate();
}

/**
 * Creates the letter animation control of the text box.
 * @param pLayout layout containing the text box
 */
void LayoutTextPaneAnimator::initEuiLetterAnimCtrl(eui::LayoutEx* pLayout) {
    mLetterAnimCtrl = createEuiLetterAnimCtrl(mTextBox, pLayout, this);
}

/**
 * Creates the letter animation controls of the text box and of its shadow text box.
 * @param pLayout layout containing the text boxes
 * @param pShadowTextBox shadow text box
 */
void LayoutTextPaneAnimator::initEuiLetterAnimCtrlWithShadow(eui::LayoutEx* pLayout,
                                                             nn::ui2d::TextBox* pShadowTextBox) {
    mShadowTextBox = pShadowTextBox;
    mLetterAnimCtrl = createEuiLetterAnimCtrl(mTextBox, pLayout, this);
    mShadowLetterAnimCtrl = createEuiLetterAnimCtrl(mShadowTextBox, pLayout, this);
}

/**
 * Starts animating a message from its first page.
 * @param pMessage message
 * @param pTagDataHolder tag data used to replace the message tags, or nullptr
 * @param pReplaceTagProcessor processor used to replace the message tags, or nullptr
 */
void LayoutTextPaneAnimator::start(const char16_t* pMessage,
                                   const MessageTagDataHolder* pTagDataHolder,
                                   const ReplaceTagProcessorBase* pReplaceTagProcessor) {
    mIsAnimating = true;
    mIsTextAnim = false;
    mFrame = 0;
    mInterval = 0;
    mTextNum = 0;
    mCurrentPage = 0;
    mStep = 0;
    mPageNum = countMessagePage(mActor, pMessage, 0);

    if (mLetterAnimCtrl != nullptr) {
        mMessage = pMessage;
        eui::ScreenMgr* screenMgr = mActor->getLayoutKeeper()->getScreen()->getScreenMgr();
        TextPaneTagProcessor tagProcessor(screenMgr->getMessageMgr(), screenMgr->getFontMgr());
        u32 length = 0;
        u32 charNum = 0;
        tagProcessor.preProcess(mText.getBuffer(), &length, &charNum, mText.getBufferSize(),
                                mMessage, calcMessageSizeWithoutNullCharacter(mMessage, nullptr),
                                mCurrentPage, 10, eui::TagProcessor::PreProcessOption(),
                                nullptr);

        if (pTagDataHolder != nullptr) {
            replaceMessageTagData(&mText, mActor, pTagDataHolder, mText.cstr());
        }

        if (pReplaceTagProcessor != nullptr) {
            char16_t message[0x800];
            copyMessageWithTag(message, 0x800, mText.cstr());
            pReplaceTagProcessor->replace(mText.getBuffer(), mActor, message);
        }

        mIsTextAnim = startLetterAnim(mLetterAnimCtrl, mText.cstr(), mActor);

        if (mShadowLetterAnimCtrl != nullptr) {
            startLetterAnim(mShadowLetterAnimCtrl, mText.cstr(), mActor);
        }

        StringTmp<64> voiceName;
        voiceName.clear();
        tryGetMessageTagVoiceNameInPage(&voiceName, mActor, pMessage);
        mIsExistVoice = !voiceName.isEmpty();

        if (mAudioKeeper != nullptr && !mIsExistVoice) {
            tryStartSe(mAudioKeeper, "VoiceDefault");
        }

        return;
    }

    nn::font::Rectangle rect = mTextBox->GetTextDrawRect();
    mTextWidth = rect.right - rect.left;
    rect = mTextBox->GetTextDrawRect();
    mTextHeight = rect.top - rect.bottom;
    mText.format(pMessage);
    mTextBox->SetString(reinterpret_cast<const u16*>(u""), 0, 1);

    if (mShadowTextBox != nullptr) {
        mShadowTextBox->SetString(reinterpret_cast<const u16*>(u""), 0, 1);
    }

    mTextBox->SetTranslate(mBaseTrans);

    if (mShadowTextBox != nullptr) {
        mShadowTextBox->SetTranslate(mBaseTrans);
    }
}

/**
 * Stops the letter animation.
 */
void LayoutTextPaneAnimator::end() {
    mIsAnimating = false;
    mLetterAnimCtrl->stop();

    if (mShadowLetterAnimCtrl != nullptr) {
        mShadowLetterAnimCtrl->stop();
    }
}

/**
 * Advances the letter animation by one frame.
 */
void LayoutTextPaneAnimator::update() {
    if (mLetterAnimCtrl != nullptr) {
        mFrame++;

        if (mInterval > mFrame) {
            return;
        }

        mLetterAnimCtrl->Update(1.0f);

        if (mShadowLetterAnimCtrl != nullptr) {
            mShadowLetterAnimCtrl->Update(1.0f);
        }

        if (mLetterAnimCtrl->mVisibleLength != mLetterAnimCtrl->mTextLength ||
            mLetterAnimCtrl->mPlaying || mIsTextAnim) {
            requestCaptureRecursive(mActor);
        }

        const u16* string = mLetterAnimCtrl->mTextBox->GetStringBuffer();
        s32 prevTextNum = mTextNum;
        s32 textNum = 0;

        if (!isMessageTagMark(string[0])) {
            while (string[textNum] != 0) {
                textNum++;

                if (isMessageTagMark(string[textNum])) {
                    break;
                }
            }
        }

        mTextNum = textNum;

        if (calcMessageSizeWithoutNullCharacter(mText.cstr(), nullptr) >= 1) {
            if (mTextNum > prevTextNum) {
                mLastChar = string[mTextNum - 1];
                mIsCharAdded = true;
            } else {
                mIsCharAdded = false;
            }
        }

        if (mLetterAnimCtrl->mVisibleLength == mLetterAnimCtrl->mTextLength &&
            !mLetterAnimCtrl->mPlaying) {
            mLetterAnimCtrl->stop();

            if (mShadowLetterAnimCtrl != nullptr) {
                mShadowLetterAnimCtrl->stop();
            }

            mIsAnimating = false;
        }

        mStep++;
        return;
    }

    if (!mIsAnimating) {
        return;
    }

    if (mFrame++ < 2) {
        return;
    }

    mFrame = 0;
    mTextNum++;
    updateText();

    if (mText.calcLength() <= mTextNum) {
        mIsAnimating = false;
    }
}

/**
 * Shows the first characters of the text and aligns the text box.
 */
void LayoutTextPaneAnimator::updateText() {
    sead::WFixedSafeString<256> text;
    text.copy(mText, mTextNum);
    mTextBox->SetString(reinterpret_cast<const u16*>(text.cstr()), 0, mTextNum);

    if (mShadowTextBox != nullptr) {
        mShadowTextBox->SetString(reinterpret_cast<const u16*>(text.cstr()), 0, mTextNum);
    }

    nn::util::Float3 trans = mBaseTrans;

    if (mTextBox->GetTextPositionH() == 0) {
        trans.x = trans.x + (mTextWidth - mTextBox->GetTextDrawRect().GetWidth()) * -0.5f;
    }

    if (mTextBox->IsTextFlag12() || mTextBox->GetTextPositionV() == 0) {
        trans.y = (mTextHeight - -mTextBox->GetTextDrawRect().GetHeight()) * 0.5f + trans.y;
    }

    mTextBox->SetTranslate(trans);

    if (mShadowTextBox != nullptr) {
        mShadowTextBox->SetTranslate(trans);
    }

    requestCaptureRecursive(mActor);
}

/**
 * Speeds up the letter animation, or shows the whole text if there is none.
 */
void LayoutTextPaneAnimator::skip() {
    if (!mIsAnimating) {
        return;
    }

    if (mLetterAnimCtrl != nullptr) {
        mLetterAnimCtrl->setSkip(true);
        mLetterAnimCtrl->setSpeedTemporarily(120.0f);

        if (mShadowLetterAnimCtrl != nullptr) {
            mShadowLetterAnimCtrl->setSkip(true);
            mShadowLetterAnimCtrl->setSpeedTemporarily(120.0f);
        }

        return;
    }

    mTextNum = mText.calcLength();
    updateText();
}

/**
 * Shows the whole text immediately.
 */
void LayoutTextPaneAnimator::flush() {
    if (!mIsAnimating) {
        return;
    }

    if (mLetterAnimCtrl != nullptr) {
        mLetterAnimCtrl->flush();

        if (mShadowLetterAnimCtrl != nullptr) {
            mShadowLetterAnimCtrl->flush();
        }

        return;
    }

    mTextNum = mText.calcLength();
    updateText();
}

/**
 * Starts animating the next page of the message.
 * @param pTagDataHolder tag data used to replace the message tags, or nullptr
 * @param pReplaceTagProcessor processor used to replace the message tags, or nullptr
 */
void LayoutTextPaneAnimator::changeNextPage(const MessageTagDataHolder* pTagDataHolder,
                                            const ReplaceTagProcessorBase* pReplaceTagProcessor) {
    mCurrentPage++;
    mIsAnimating = true;
    mTextNum = 0;
    mStep = 0;
    mFrame = 0;
    mInterval = 20;
    eui::ScreenMgr* screenMgr = mActor->getLayoutKeeper()->getScreen()->getScreenMgr();
    TextPaneTagProcessor tagProcessor(screenMgr->getMessageMgr(), screenMgr->getFontMgr());
    u32 length = 0;
    u32 charNum = 0;
    tagProcessor.preProcess(mText.getBuffer(), &length, &charNum, mText.getBufferSize(), mMessage,
                            calcMessageSizeWithoutNullCharacter(mMessage, nullptr), mCurrentPage,
                            10, eui::TagProcessor::PreProcessOption(), nullptr);

    if (pTagDataHolder != nullptr) {
        replaceMessageTagData(&mText, mActor, pTagDataHolder, mText.cstr());
    }

    if (pReplaceTagProcessor != nullptr) {
        char16_t message[0x800];
        copyMessageWithTag(message, 0x800, mText.cstr());
        pReplaceTagProcessor->replace(mText.getBuffer(), mActor, message);
    }

    startLetterAnim(mLetterAnimCtrl, mText.cstr(), mActor);

    if (mShadowLetterAnimCtrl != nullptr) {
        startLetterAnim(mShadowLetterAnimCtrl, mText.cstr(), mActor);
    }

    requestCaptureRecursive(mActor);
}

/**
 * Gets the current page of the message.
 * @return the current page
 */
const char16_t* LayoutTextPaneAnimator::getCurrentMessage() const {
    return getMessageWithPage(mActor, mMessage, mCurrentPage);
}

/**
 * Gets the page following the current page of the message.
 * @return the next page, or nullptr if there is none
 */
const char16_t* LayoutTextPaneAnimator::tryGetNextMessage() const {
    const IUseMessageSystem* messageSystem = mActor;
    return getNextMessagePage(messageSystem,
                              getMessageWithPage(messageSystem, mMessage, mCurrentPage));
}

inline const IUseAudioKeeper* LayoutTextPaneAnimator::getVoiceAudioKeeper() const {
    if (mAudioKeeper != nullptr) {
        return mAudioKeeper;
    }

    return mActor;
}

/**
 * Plays a voice sound effect.
 * @param pVoiceName sound effect name
 */
void LayoutTextPaneAnimator::startVoice(const char* pVoiceName) {
    if (!isExistSeKeeper(getVoiceAudioKeeper())) {
        return;
    }

    startSe(getVoiceAudioKeeper(), pVoiceName);
}

namespace {
/**
 * Plays the voice of a "PlaySe" message tag.
 * @param pTagInfo message tag
 */
void TextPaneAppTagCallback::invoke(const TagInfo* pTagInfo) {
    if (!isEqualString(getMessageTagGroupName(mAnimator->getActor(), pTagInfo->mGroup),
                       "PlaySe")) {
        return;
    }

    StringTmp<64> voiceName;
    getMessageTagVoiceName(&voiceName, mAnimator->getActor(),
                           reinterpret_cast<const char16_t*>(pTagInfo));
    mAnimator->startVoice(voiceName.cstr());
}
}  // namespace
}  // namespace al
