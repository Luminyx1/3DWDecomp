#pragma once
#include <nn/font/font_TagProcessorBase.h>
#include <message/seadMessageSet.h>
#include <prim/seadSafeString.h>
namespace eui {
class MessageMgr;
class FontMgr;
class TextBoxEx;
class TagProcessor : public nn::font::TagProcessorBase<u16> {
public:
    using Context = nn::font::PrintContext<u16>;
    using Rect = nn::font::Rectangle;
    using TagInfo = sead::MessageSet<char16_t>::TagInfo;
    struct PreProcessOption {
        PreProcessOption() : _0(false), _1(false) {}

        bool _0;
        bool _1;
        s32 _4;
    };
    TagProcessor(MessageMgr*, FontMgr*);
    ~TagProcessor() override = default;
    NN_RUNTIME_TYPEINFO(nn::font::TagProcessorBase<u16>);
    Operation Process(u32, Context*) override;
    Operation CalculateRect(Rect*, Context*, u32) override;
    void BeginPrint(Context*) override;
    void EndPrint(Context*) override;
    void BeginCalculateRect(Context*) override;
    void EndCalculateRect(Context*) override;
    virtual void preProcess(char16_t*, u32*, u32*, u32, const char16_t*, u32, int, u32, const PreProcessOption&, void*);
    virtual float calcAdjustTextScale(TextBoxEx*, float, float) const;
    virtual void preProcessSystemTag_(const TagInfo*, char16_t*, u32*, u32*, u32, const char16_t*, u32, void*);
    virtual void preProcessEuiTag_(const TagInfo*, char16_t*, u32*, u32*, u32, const char16_t*, u32, void*);
    virtual void preProcessEuiPictFontTag_(const TagInfo*, char16_t*, u32*, u32*, u32, const char16_t*, u32, void*);
    virtual void preProcessGrammarTag_(const TagInfo*, char16_t*, u32*, u32*, u32, const char16_t*, u32, void*);
    virtual void preProcessAppTag_(const TagInfo*, char16_t*, u32*, u32*, u32, const char16_t*, u32, void*);
    virtual Operation process_(u16, Context*, Rect*);
    virtual Operation processEuiTag_(const TagInfo*, Context*, Rect*, const char16_t*);
    virtual Operation processAppTag_(const TagInfo*, Context*, Rect*, const char16_t*);
    virtual Operation processRubyTag_(const TagInfo*, Context*, Rect*, const char16_t*);
    virtual void adjustRubyCursorY(nn::font::TextWriterBase<u16>*, const TagInfo*, Context*);
    virtual Operation processFontTag_(const TagInfo*, Context*, Rect*, const char16_t*);
    virtual Operation processSizeTag_(const TagInfo*, Context*, Rect*, const char16_t*);
    virtual Operation processColorTag_(const TagInfo*, Context*, Rect*, const char16_t*);
    virtual Operation processPageBreakTag_(const TagInfo*, Context*, Rect*, const char16_t*);
    virtual Operation processAlphaTag_(const TagInfo*, Context*, Rect*, const char16_t*);
    virtual Operation processPictFontProcessTag_(const TagInfo*, Context*, Rect*, const char16_t*);
    virtual Operation processSkipTag_(const TagInfo*, Context*, Rect*, const char16_t*);
    virtual Operation processFitWidthTag_(const TagInfo*, Context*, Rect*, const char16_t*);
    virtual float getRubyScale_() const;
    virtual float getRubyScaleMax_() const;
    virtual float getRubyBaseLinkOffset_() const;
    virtual float getRubyCharSpace_() const;
    virtual float getPictFontScale_() const;
    virtual void getPictFontCodeAndFont_(char16_t*, u16*, u8) const;
    u8 calcAlphaValueStart_();
    u8 calcAlphaValueEnd_();
    static char16_t* setAlphaTag(char16_t* pText, bool reset, u8 alpha);
    static char16_t* setPictFontProcessTag(char16_t* pText, u16 value);
    static char16_t* setSkipTag(char16_t* pText, u16 length);
    static char16_t* setFitWidthTag(char16_t* pText, u16 length, float width);
    static char16_t* setSizeTag(char16_t* pText, u16 size);
    static char16_t* setFontTag(char16_t* pText, u16 index);
    u32 mStartColor, mEndColor;
    void* _10;
    void* _18;
    void* _20;
    u32 _28;
    float _2c, _30;
    u32 mPrintDepth;
    MessageMgr* mMessageMgr;
    FontMgr* mFontMgr;
    u8 mFlags, mAlpha, mStartAlpha, mEndAlpha;
};

static_assert(sizeof(TagProcessor) == 0x50, "TagProcessor size");
}
