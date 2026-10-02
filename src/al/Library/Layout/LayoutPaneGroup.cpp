#include "Library/Layout/LayoutPaneGroup.hpp"

#include <eui/euiAnimator.h>
#include <eui/euiCapturePane.h>
#include <eui/euiLayoutEx.h>
#include <eui/euiScalableFontTextBoxEx.h>
#include <eui/euiTagProcessor.h>
#include <eui/euiTextBoxEx.h>
#include <eui/euiUtility.h>
#include <gfx/nin/seadGraphicsNvn.h>
#include <nn/font/font_Font.h>
#include <nn/ui2d/ui2d_ExtUserData.h>
#include <nn/ui2d/ui2d_Group.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/ui2d/ui2d_Parts.h>
#include <nn/ui2d/ui2d_TextBox.h>
#include <prim/seadSafeString.h>

#include "Library/Math/MatrixUtil.hpp"
#include "Library/Message/MessageHolder.hpp"
#include "Library/Message/MessageSystem.hpp"
#include "Library/Message/ReplaceTagProcessorBase.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
nn::gfx::Device* getGfxDevice() {
    return reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice());
}

s32 calcStringLength(const char16_t* pString) {
    s32 length = 0;

    while (pString[length] != u'\0') {
        length++;
    }

    return length;
}

void formatPartsTextLabel(sead::BufferedSafeString* pLabel, const nn::ui2d::Layout* pLayout,
                          const char* pTextId) {
    sead::FixedSafeString<128> textId;
    tryReplaceString(&textId, pTextId, "#", "");
    pLabel->format("%s_%s", pLayout->GetName(), textId.cstr());
}

void forEachChildPane(nn::ui2d::Pane* pPane, void (*pFunc)(nn::ui2d::Pane*)) {
    for (auto* node = pPane->m_Children.GetNext(); node != &pPane->m_Children;
         node = node->GetNext()) {
        pFunc(nn::ui2d::Pane::FromLink(node));
    }
}
}  // namespace

/**
 * Creates an empty pane group.
 * @param pGroupName name of the layout group
 */
LayoutPaneGroup::LayoutPaneGroup(const char* pGroupName) : mGroupName(pGroupName) {}

/**
 * Starts the animation with the given name, stopping the one currently playing.
 * @param pAnimName animation name
 */
void LayoutPaneGroup::startAnim(const char* pAnimName) {
    eui::Animator* animator = getAnimator(pAnimName);

    if (animator == nullptr) {
        return;
    }

    if (mPlayingAnimator != nullptr) {
        mPlayingAnimator->disableKeepActive();
    }

    mPlayingAnimator = animator;
    animator->PlayAuto(1.0f);
}

/**
 * Gets the animator with the given name.
 * @param pAnimName animation name
 * @return the animator, or nullptr if there is none
 */
eui::Animator* LayoutPaneGroup::getAnimator(const char* pAnimName) const {
    for (s32 i = 0; i < mAnimatorNum; i++) {
        if (isEqualString(mAnimators[i]->getName(), pAnimName)) {
            return mAnimators[i];
        }
    }

    return nullptr;
}

/**
 * Sets the frame of the playing animation.
 * @param frame new frame
 */
void LayoutPaneGroup::setAnimFrame(f32 frame) {
    if (mPlayingAnimator != nullptr) {
        mPlayingAnimator->mFrame = frame;
    }
}

/**
 * Restarts the playing animation from its current frame with a new frame rate.
 * @param frameRate new frame rate
 */
void LayoutPaneGroup::setAnimFrameRate(f32 frameRate) {
    if (mPlayingAnimator != nullptr) {
        mPlayingAnimator->PlayFromCurrent(
            static_cast<eui::Animator::PlayType>(mPlayingAnimator->IsLoopData()), frameRate);
    }
}

/**
 * Gets the frame of the playing animation.
 * @return the frame, or 0 if nothing is playing
 */
f32 LayoutPaneGroup::getAnimFrame() const {
    if (mPlayingAnimator != nullptr) {
        return mPlayingAnimator->getFrame();
    }

    return 0.0f;
}

/**
 * Gets the last frame of the playing animation.
 * @return the last frame, or 0 if nothing is playing
 */
f32 LayoutPaneGroup::getAnimFrameMax() const {
    if (mPlayingAnimator != nullptr) {
        return mPlayingAnimator->GetFrameSize();
    }

    return 0.0f;
}

/**
 * Gets the last frame of the animation with the given name.
 * @param pAnimName animation name
 * @return the last frame
 */
f32 LayoutPaneGroup::getAnimFrameMax(const char* pAnimName) const {
    return getAnimator(pAnimName)->GetFrameSize();
}

/**
 * Gets the frame rate of the playing animation.
 * @return the frame rate, or 0 if nothing is playing
 */
f32 LayoutPaneGroup::getAnimFrameRate() const {
    if (mPlayingAnimator != nullptr) {
        return mPlayingAnimator->getStep();
    }

    return 0.0f;
}

/**
 * Checks whether an animation with the given name exists.
 * @param pAnimName animation name
 * @return whether the animation exists
 */
bool LayoutPaneGroup::isAnimExist(const char* pAnimName) const {
    return tryGetAnimator(pAnimName) != nullptr;
}

/**
 * Gets the animator with the given name if it exists.
 * @param pAnimName animation name
 * @return the animator, or nullptr if there is none
 */
eui::Animator* LayoutPaneGroup::tryGetAnimator(const char* pAnimName) const {
    for (s32 i = 0; i < mAnimatorNum; i++) {
        if (isEqualString(mAnimators[i]->getName(), pAnimName)) {
            return mAnimators[i];
        }
    }

    return nullptr;
}

/**
 * Checks whether the playing animation has reached its last frame.
 * @return whether the animation has ended, or true if nothing is playing
 */
bool LayoutPaneGroup::isAnimEnd() const {
    if (mPlayingAnimator != nullptr) {
        return mPlayingAnimator->isFrameMax();
    }

    return true;
}

/**
 * Checks whether the playing animation plays only once.
 * @return whether the animation is not looping, or false if nothing is playing
 */
bool LayoutPaneGroup::isAnimOneTime() const {
    if (mPlayingAnimator != nullptr) {
        return !mPlayingAnimator->IsLoopData();
    }

    return false;
}

/**
 * Checks whether the animation with the given name plays only once.
 * @param pAnimName animation name
 * @return whether the animation is not looping
 */
bool LayoutPaneGroup::isAnimOneTime(const char* pAnimName) const {
    return !getAnimator(pAnimName)->IsLoopData();
}

/**
 * Checks whether an animation is playing.
 * @return whether an animation is playing
 */
bool LayoutPaneGroup::isAnimPlaying() const {
    return mPlayingAnimator != nullptr;
}

/**
 * Gets the name of the playing animation.
 * @return the animation name, or an empty string if nothing is playing
 */
const char* LayoutPaneGroup::getPlayingAnimName() const {
    if (mPlayingAnimator != nullptr) {
        return mPlayingAnimator->getName();
    }

    return "";
}

/**
 * Appends an animation name whose animator is created by createAnimator.
 * @param pAnimName animation name
 */
void LayoutPaneGroup::pushAnimName(const char* pAnimName) {
    AnimNameNode* node = new AnimNameNode{pAnimName, nullptr};
    AnimNameNode** tail = &mAnimNames;

    if (mAnimNames != nullptr) {
        AnimNameNode* last = mAnimNames;

        while (last->mNext != nullptr) {
            last = last->mNext;
        }

        tail = &last->mNext;
    }

    *tail = node;
}

/**
 * Creates an animator for every pushed animation name, bound to this group.
 * @param pLayout layout containing the group
 */
void LayoutPaneGroup::createAnimator(nn::ui2d::Layout* pLayout) {
    mAnimatorNum = 0;

    if (mAnimNames == nullptr) {
        return;
    }

    AnimNameNode* node = mAnimNames;

    while (node != nullptr) {
        node = node->mNext;
        mAnimatorNum++;
    }

    mAnimators = new eui::Animator*[mAnimatorNum];
    eui::LayoutEx* layout = eui::DynamicCast<eui::LayoutEx>(pLayout);
    s32 i = 0;

    for (node = mAnimNames; node != nullptr; node = node->mNext) {
        const char* groupName = mGroupName;
        mAnimators[i] = layout->createAnimatorWithGroup(
            node->mName, layout->getGroupContainer()->FindGroupByName(groupName), false);
        i++;
    }
}

/**
 * Applies the playing animation and advances it.
 * @param isUpdateFrame whether to advance the animation frame
 */
void LayoutPaneGroup::animate(bool isUpdateFrame) {
    if (mPlayingAnimator == nullptr || !mPlayingAnimator->mEnabled) {
        return;
    }

    mPlayingAnimator->Animate();

    if (!mPlayingAnimator->IsLoopData() && mPlayingAnimator->isFrameMax()) {
        mPlayingAnimator->disableKeepActive();
    }

    if (isUpdateFrame) {
        mPlayingAnimator->UpdateFrame(1.0f);
    }
}

/**
 * Sets the string of a text box.
 * @param pTextBox text box
 * @param pString string to set
 * @param charNum number of message characters
 */
void setTextBoxString(nn::ui2d::TextBox* pTextBox, const char16_t* pString, u16 charNum) {
    setTextBoxStringLength(pTextBox, pString, charNum, calcStringLength(pString), -1);
}

/**
 * Sets the string of a text box, requests a capture of its capture pane parents and copies the
 * string to the text boxes listed in its "CopyTextString" user data.
 * @param pTextBox text box
 * @param pString string to set
 * @param charNum number of message characters
 * @param length string length
 * @param page page to show
 */
void setTextBoxStringLength(nn::ui2d::TextBox* pTextBox, const char16_t* pString, u16 charNum,
                            u16 length, u32 page) {
    eui::TextBoxEx* textBox = eui::DynamicCast<eui::TextBoxEx>(pTextBox);
    textBox->SetFontSize(textBox->GetFontSize());
    textBox->setStringWithPage(pString, length, nullptr, page, false, nullptr);
    textBox->adjustText_(nullptr);

    for (nn::ui2d::Pane* parent = textBox->GetParent(); parent != nullptr;
         parent = parent->GetParent()) {
        eui::CapturePane* capturePane = eui::DynamicCast<eui::CapturePane>(parent);

        if (capturePane != nullptr) {
            capturePane->mCaptureRequired = true;
        }
    }

    const nn::ui2d::ResExtUserData* userData = textBox->FindExtUserDataByName("CopyTextString");

    if (userData == nullptr) {
        return;
    }

    nn::ui2d::Pane* rootPane = textBox->GetParent();

    while (eui::DynamicCast<nn::ui2d::Parts>(rootPane) == nullptr &&
           rootPane->GetParent() != nullptr) {
        rootPane = rootPane->GetParent();
    }

    StringTmp<512> copyNames(static_cast<const char*>(userData->GetData()));
    sead::SafeString delimiter("\n");

    for (auto it = copyNames.tokenBegin(delimiter); copyNames.tokenEnd(delimiter) != it; ++it) {
        StringTmp<512> copyName;
        it.get(&copyName);
        nn::ui2d::Pane* copyPane = rootPane->FindPaneByName(copyName.cstr(), true);
        setTextBoxStringLength(eui::DynamicCast<eui::TextBoxEx>(copyPane), pString, charNum,
                               length, -1);
    }
}

/**
 * Sets the tag processor of a text box.
 * @param pTextBox text box
 * @param pTagProcessor tag processor
 */
void setTextBoxTagProcessor(nn::ui2d::TextBox* pTextBox,
                            nn::font::TagProcessorBase<u16>* pTagProcessor) {
    pTextBox->SetTagProcessor(pTagProcessor);
}

/**
 * Initializes the string of a text box from a message label.
 * @param pTextBox text box
 * @param pHolder message holder containing the label
 * @param pLabel message label, "@raw" to keep the text or "@style" to fill the box with a pattern
 * @param bufferSize string buffer size used for "@style"
 */
void initTextBoxPane(nn::ui2d::TextBox* pTextBox, const MessageHolder* pHolder,
                     const char* pLabel, u32 bufferSize) {
    if (pLabel == nullptr || isEqualString("@raw", pLabel)) {
        return;
    }

    if (isEqualString("@style", pLabel)) {
        u32 size = bufferSize < 0x800 ? bufferSize : 0x800;
        pTextBox->AllocateStringBuffer(getGfxDevice(), size);
        sead::WFormatFixedSafeString<2048> string(u"");
        f32 scale = pTextBox->GetFontSize().width / pTextBox->GetFont()->GetWidth();
        f32 width = pTextBox->GetSizeX();
        s32 charWidth = pTextBox->GetFont()->GetWidth();
        s32 lineNum;

        if (pTextBox->GetSizeY() > scale * pTextBox->GetFont()->GetHeight()) {
            lineNum = (pTextBox->GetSizeY() - scale * pTextBox->GetFont()->GetHeight()) /
                          (scale * pTextBox->GetFont()->GetHeight() + pTextBox->GetLineSpace()) +
                      1.0f;
        } else {
            lineNum = 1;
        }

        s32 charNum = width / (scale * charWidth);

        for (u16 line = 0; line < static_cast<u32>(lineNum); line++) {
            for (u16 i = 0; i < static_cast<u32>(charNum); i++) {
                string.appendWithFormat(u"/");
            }

            if (line < lineNum - 1) {
                string.appendWithFormat(u"\n");
            }
        }

        const char16_t* text = string.cstr();
        s32 bufferLength = pTextBox->GetStringBufferLength();
        s32 length = string.calcLength();
        setTextBoxStringLength(pTextBox, text, 0, length > bufferLength ? bufferLength : length,
                               -1);
        return;
    }

    const char16_t* text = pHolder != nullptr ? pHolder->tryGetText(pLabel) : nullptr;

    if (text == nullptr) {
        setTextBoxStringLength(pTextBox, u"NULL", 0, 4, -1);
        return;
    }

    s32 charNum = pHolder->calcCharacterNum(pLabel);
    u32 outLength = static_cast<u16>(charNum);
    sead::WFixedSafeString<2048> buffer;
    u32 outCharNum = 0;
    auto* tagProcessor = static_cast<eui::TagProcessor*>(pTextBox->GetTagProcessor());
    tagProcessor->preProcess(buffer.getBuffer(), &outLength, &outCharNum, 0x800, text,
                             static_cast<u16>(charNum), -1, 100,
                             eui::TagProcessor::PreProcessOption(), nullptr);
    s32 size = static_cast<u16>(charNum) > static_cast<s32>(outLength) ? charNum : outLength;
    pTextBox->AllocateStringBuffer(getGfxDevice(), size + 1);
    setTextBoxStringLength(pTextBox, text, 0, charNum, -1);
}

/**
 * Reallocates the string buffer of a text box.
 * @param pTextBox text box
 * @param bufferSize new buffer size
 */
void reallocateTextBoxStringBuffer(nn::ui2d::TextBox* pTextBox, u32 bufferSize) {
    pTextBox->AllocateStringBuffer(getGfxDevice(), bufferSize);
}

/**
 * Initializes the strings of all text boxes in a pane tree from one message label.
 * @param pPane root pane
 * @param pHolder message holder containing the label
 * @param pLabel message label
 * @param bufferSize string buffer size used for "@style"
 */
void initTextBoxRecursive(nn::ui2d::Pane* pPane, const MessageHolder* pHolder,
                          const char* pLabel, u32 bufferSize) {
    nn::ui2d::TextBox* textBox = eui::DynamicCast<nn::ui2d::TextBox>(pPane);

    if (textBox != nullptr) {
        initTextBoxPane(textBox, pHolder, pLabel, bufferSize);
    }

    for (auto* node = pPane->m_Children.GetNext(); node != &pPane->m_Children;
         node = node->GetNext()) {
        initTextBoxRecursive(nn::ui2d::Pane::FromLink(node), pHolder, pLabel, bufferSize);
    }
}

/**
 * Initializes the strings of all text boxes in a pane tree from their own text ids.
 * @param pPane root pane
 * @param bufferSize string buffer size used for "@style"
 * @param pMessageSystem message system providing the layout messages
 * @param pLayout layout whose messages are used for plain text ids
 * @param pPartsLayout layout of the enclosing parts, used for "#" text ids
 */
void initTextBoxRecursiveWithSelfTextId(nn::ui2d::Pane* pPane, u32 bufferSize,
                                        const MessageSystem* pMessageSystem,
                                        const nn::ui2d::Layout* pLayout,
                                        const nn::ui2d::Layout* pPartsLayout) {
    nn::ui2d::TextBox* textBox = eui::DynamicCast<nn::ui2d::TextBox>(pPane);
    nn::ui2d::Parts* parts = eui::DynamicCast<nn::ui2d::Parts>(pPane);

    if (parts != nullptr) {
        pPartsLayout = parts->m_pLayout;
    } else if (pPartsLayout == nullptr) {
        pPartsLayout = pLayout;
    }

    if (textBox != nullptr) {
        StringTmp<256> label;
        const char* textId = textBox->GetTextId();
        const MessageHolder* holder = nullptr;
        const char* labelName = textId;

        if (textId == nullptr) {
        } else if (isMatchString(textId, MatchStr("@*"))) {
            if (isEqualString("@raw", textId)) {
            } else if (isEqualString("@unused", textId)) {
                labelName = nullptr;
            } else if (!isEqualString("@style", textId)) {
                labelName = nullptr;
            }
        } else if (isMatchString(textId, MatchStr("#*"))) {
            formatPartsTextLabel(&label, pPartsLayout, textId);
            labelName = label.cstr();
            holder = pMessageSystem->getLayoutMessageHolder(pPartsLayout->GetName());
        } else {
            label.format("%s_%s", pLayout->GetName(), textId);
            labelName = label.cstr();
            holder = pMessageSystem->getLayoutMessageHolder(pLayout->GetName());
        }

        initTextBoxPane(textBox, holder, labelName, bufferSize);
    }

    for (auto* node = pPane->m_Children.GetNext(); node != &pPane->m_Children;
         node = node->GetNext()) {
        initTextBoxRecursiveWithSelfTextId(nn::ui2d::Pane::FromLink(node), bufferSize,
                                           pMessageSystem, pLayout, pPartsLayout);
    }
}

/**
 * Replaces the string of a text box with a message.
 * @param pTextBox text box
 * @param pHolder message holder containing the label
 * @param pLabel message label
 */
void replaceTextBoxMessage(nn::ui2d::TextBox* pTextBox, const MessageHolder* pHolder,
                           const char* pLabel) {
    const char16_t* text = pHolder->getText(pLabel);
    s32 charNum = pHolder->calcCharacterNum(pLabel);
    setTextBoxStringLength(pTextBox, text, charNum, calcStringLength(text), -1);
}

/**
 * Replaces the string of a text box with a message after replacing its tags.
 * @param pTextBox text box
 * @param pMessage message
 * @param pReplaceTagProcessor tag processor used to replace the tags
 * @param pMessageSystem message system
 */
void replaceTextBoxMessage(nn::ui2d::TextBox* pTextBox, const char16_t* pMessage,
                           const ReplaceTagProcessorBase* pReplaceTagProcessor,
                           const IUseMessageSystem* pMessageSystem) {
    char16_t buffer[0x200];
    pReplaceTagProcessor->replace(buffer, pMessageSystem, pMessage);
    s32 charNum = calcMessageSizeWithoutNullCharacter(buffer, nullptr);
    setTextBoxStringLength(pTextBox, buffer, charNum, calcStringLength(buffer), -1);
}

/**
 * Does nothing.
 * @param pPane root pane
 * @param index tag argument index
 * @param number tag argument value
 */
void setTextBoxNumberTagArgRecursive(nn::ui2d::Pane* pPane, s32 index, s32 number) {}

/**
 * Calculates the corners of a pane in global coordinates.
 * @param pTopLeft output top left corner
 * @param pBottomRight output bottom right corner
 * @param pPane pane
 */
void calcPaneRectPos(sead::Vector2f* pTopLeft, sead::Vector2f* pBottomRight,
                     const nn::ui2d::Pane* pPane) {
    sead::Matrix34f mtx;
    makeMtx34f(&mtx, *reinterpret_cast<const nn::util::neon::MatrixColumnMajor4x3fType*>(
                         pPane->GetGlobalMtx()));
    const nn::font::Rectangle rect = pPane->GetPaneRect();
    sead::Vector3f topLeft = {rect.left, rect.top, 0.0f};
    sead::Vector3f bottomRight = {rect.right, rect.bottom, 0.0f};
    calcMtxMul(&topLeft, mtx, topLeft);
    calcMtxMul(&bottomRight, mtx, bottomRight);
    pTopLeft->set(topLeft.x, topLeft.y);
    pBottomRight->set(bottomRight.x, bottomRight.y);
}

/**
 * Gets the name of a pane.
 * @param pPane pane
 * @return the pane name
 */
const char* getPaneName(const nn::ui2d::Pane* pPane) {
    return pPane->GetName();
}

/**
 * Shows a pane and all of its descendants.
 * @param pPane root pane
 */
void showPaneRecursive(nn::ui2d::Pane* pPane) {
    pPane->SetVisible(true);
    forEachChildPane(pPane, showPaneRecursive);
}

/**
 * Hides a pane and all of its descendants.
 * @param pPane root pane
 */
void hidePaneRecursive(nn::ui2d::Pane* pPane) {
    pPane->SetVisible(false);
    forEachChildPane(pPane, hidePaneRecursive);
}

/**
 * Sets the font of a text box pane, keeping its font size.
 * @param pPane text box pane
 * @param pFont font
 */
void setTextBoxPaneFont(nn::ui2d::Pane* pPane, const nn::font::Font* pFont) {
    if (pFont->GetType() == nn::font::FontType_PackedTexture) {
        eui::ScalableFontTextBoxEx* textBox =
            eui::DynamicCast<eui::ScalableFontTextBoxEx>(pPane);
        nn::ui2d::Size fontSize = textBox->GetFontSize();
        textBox->SetFont(pFont);
        textBox->SetFontSize(fontSize);
        textBox->setStringNoPreproces(reinterpret_cast<const char16_t*>(textBox->GetStringBuffer()),
                                      textBox->GetStringLength());
        return;
    }

    eui::DynamicCast<nn::ui2d::TextBox>(pPane)->SetFont(pFont);
}

/**
 * Requests a capture for every capture pane in a pane tree.
 * @param pPane root pane
 */
void requestCaptureRecursive(nn::ui2d::Pane* pPane) {
    eui::CapturePane* capturePane = eui::DynamicCast<eui::CapturePane>(pPane);

    if (capturePane != nullptr) {
        capturePane->mCaptureRequired = true;
    }

    forEachChildPane(pPane, requestCaptureRecursive);
}
}  // namespace al
