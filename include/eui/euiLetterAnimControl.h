#pragma once
#include <eui/euiControlBase.h>
#include <message/seadMessageSet.h>
#include <prim/seadDelegate.h>
namespace eui {
class TextBoxEx;
class LayoutEx;
class MessageString;
class LetterAnimControl : public ControlBase {
public:
    LetterAnimControl();
    LetterAnimControl(const LetterAnimControl& rOther, LayoutEx* pLayout, sead::Heap* pHeap);
    ~LetterAnimControl() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(ControlBase);
    void Update(float step) override;
    void reset();
    void initialize(sead::Heap* pHeap, TextBoxEx* pTextBox, LayoutEx* pLayout);
    void setAppTagCallback(sead::IDelegate1<const sead::MessageSet<char16_t>::TagInfo*>* pCallback);
    void setChoiceExcludeCallback(sead::IDelegate2R<const char16_t*, u16, bool>* pCallback);
    void finishAnim_();
    void start();
    void stop();
    void setSpeed(float speed);
    void setSpeedTemporarily(float speed);
    void flush();
    void flushAllowWait();
    void changeText(const char16_t* pText, u16 length, u32 flags);
    void changeText(const MessageString& rText, u32 flags);
    TextBoxEx* mTextBox;
    const char16_t* mText;
    char16_t* mWorkBuffer;
    float mSpeed;
    float mDefaultSpeed;
    float mWaitRemaining;
    u32 _4c;
    const char16_t* mCurrentText;
    u16 mVisibleLength;
    u16 _5a;
    u16 mTextLength;
    u8 mPlaying;
    u8 mFlags;
    u8 mFlushMode;
    u8 _61;
    u8 _62;
    u8 mAlpha;
    u32 _64;
    u64 _68;
    sead::IDelegate1<const sead::MessageSet<char16_t>::TagInfo*>* mAppTagCallback;
    sead::IDelegate2R<const char16_t*, u16, bool>* mChoiceExcludeCallback;
};
static_assert(sizeof(LetterAnimControl) == 0x80, "LetterAnimControl size");
}
