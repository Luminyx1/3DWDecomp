#pragma once

#include <basis/seadTypes.h>
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

private:
    void* _0 = nullptr;
    void* _8 = nullptr;
    nn::ui2d::TextBox* mTextBox;
    void* _18 = nullptr;
    void* _20 = nullptr;
    sead::WFixedSafeString<2048> mMessage;
    bool mIsAnimating = false;
    bool _1041 = false;
    s32 _1044 = 0;
    s32 _1048 = 0;
    s32 _104c = 0;
    const u16* mTextBoxString = nullptr;
    s32 mTextBoxStringLength = 0;
    s32 _105c = 0;
    s32 _1060 = 0;
    s32 mCurrentPage = 0;
    s32 mPageNum = 0;
    LayoutActor* mActor;
    s32 _1078 = 0;
    SePlayParamList* mSePlayParamList = nullptr;
    u16 _1088 = 0;
    bool _108a = false;
    const IUseAudioKeeper* mAudioKeeper = nullptr;
    bool _1098 = false;
};
}  // namespace al
