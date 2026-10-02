#pragma once

#include <basis/seadTypes.h>
#include <eui/euiLetterAnimControl.h>
#include <nn/util/util_MathTypes.h>
#include <prim/seadSafeString.h>

namespace eui {
class LayoutEx;
}

namespace nn::ui2d {
class TextBox;
}

namespace al {
class IUseAudioKeeper;
class LayoutActor;
class MessageTagDataHolder;
class ReplaceTagProcessorBase;
class SePlayParamList;

class EuiLetterAnimCtrl : public eui::LetterAnimControl {
public:
    EuiLetterAnimCtrl() : mIsSkip(false) {}
    ~EuiLetterAnimCtrl() override = default;

    void Update(f32 step) override {
        eui::LetterAnimControl::Update(step);

        if (mIsSkip) {
            _4c = 0;
        }
    }

    void setSkip(bool isSkip) { mIsSkip = isSkip; }

private:
    bool mIsSkip;
};

static_assert(sizeof(EuiLetterAnimCtrl) == 0x88);

class LayoutTextPaneAnimator {
public:
    LayoutTextPaneAnimator(nn::ui2d::TextBox* pTextBox, LayoutActor* pActor);

    void initEuiLetterAnimCtrl(eui::LayoutEx* pLayout);
    void initEuiLetterAnimCtrlWithShadow(eui::LayoutEx* pLayout, nn::ui2d::TextBox* pShadowTextBox);
    void start(const char16_t* pMessage, const MessageTagDataHolder* pTagDataHolder,
               const ReplaceTagProcessorBase* pReplaceTagProcessor);
    void end();
    void update();
    void updateText();
    void skip();
    void flush();
    void changeNextPage(const MessageTagDataHolder* pTagDataHolder,
                        const ReplaceTagProcessorBase* pReplaceTagProcessor);
    const char16_t* getCurrentMessage() const;
    const char16_t* tryGetNextMessage() const;
    void startVoice(const char* pVoiceName);

    bool isAnimating() const { return mIsAnimating; }
    bool isExistNextPage() const { return mCurrentPage < mPageNum; }
    void setAudioKeeper(const IUseAudioKeeper* pAudioKeeper) { mAudioKeeper = pAudioKeeper; }
    LayoutActor* getActor() const { return mActor; }

private:
    const IUseAudioKeeper* getVoiceAudioKeeper() const;

    EuiLetterAnimCtrl* mLetterAnimCtrl = nullptr;
    EuiLetterAnimCtrl* mShadowLetterAnimCtrl = nullptr;
    nn::ui2d::TextBox* mTextBox;
    nn::ui2d::TextBox* mShadowTextBox = nullptr;
    const char16_t* mMessage = nullptr;
    sead::WFixedSafeString<2048> mText;
    bool mIsAnimating = false;
    bool mIsTextAnim = false;
    s32 mFrame = 0;
    s32 mInterval = 0;
    s32 mTextNum = 0;
    nn::util::Float3 mBaseTrans = {};
    f32 mTextWidth = 0.0f;
    f32 mTextHeight = 0.0f;
    s32 mCurrentPage = 0;
    s32 mPageNum = 0;
    LayoutActor* mActor;
    s32 mStep = 0;
    SePlayParamList* mSePlayParamList = nullptr;
    char16_t mLastChar = 0;
    bool mIsCharAdded = false;
    const IUseAudioKeeper* mAudioKeeper = nullptr;
    bool mIsExistVoice = false;
};

static_assert(sizeof(LayoutTextPaneAnimator) == 0x10a0);
}  // namespace al
