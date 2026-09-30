#include <eui/euiTagProcessor.h>
#include <eui/euiTextBoxEx.h>
#include <nn/font/font_TextWriterBase.h>
namespace eui {
// pText receives a tag header; kind selects its operation and bytes gives its payload size.
static void WriteTagHeader(char16_t* pText, u16 kind, u16 bytes) {
    pText[0] = 0xe;
    pText[1] = 0;
    pText[2] = kind;
    pText[3] = bytes;
}
// pText receives the alpha tag; reset restores full opacity and alpha supplies its value.
char16_t* TagProcessor::setAlphaTag(char16_t* pText, bool reset, u8 alpha) {
    WriteTagHeader(pText, 0x80, 2);
    reinterpret_cast<u8*>(pText)[8] = !reset;
    reinterpret_cast<u8*>(pText)[9] = alpha;
    return pText + 5;
}
// pText receives a pictograph-processing tag containing value.
char16_t* TagProcessor::setPictFontProcessTag(char16_t* pText, u16 value) {
    WriteTagHeader(pText, 0x81, 2);
    pText[4] = value;
    return pText + 5;
}
// pText receives a tag that skips length characters.
char16_t* TagProcessor::setSkipTag(char16_t* pText, u16 length) {
    WriteTagHeader(pText, 0x82, 2);
    pText[4] = length;
    return pText + 5;
}
// pText receives a fit-width tag; length selects the text span and width uses 1/256-unit precision.
char16_t* TagProcessor::setFitWidthTag(char16_t* pText, u16 length, float width) {
    WriteTagHeader(pText, 0x83, 4);
    const float scaledWidth = width * 256.0f;
    const int roundedWidth = scaledWidth + (scaledWidth >= 0 ? 0.5f : -0.5f);
    pText[4] = length;
    pText[5] = roundedWidth;
    return pText + 6;
}
// pText receives a size-selection tag; size is the encoded text size.
char16_t* TagProcessor::setSizeTag(char16_t* pText, u16 size) {
    WriteTagHeader(pText, 2, 2);
    pText[4] = size;
    return pText + 5;
}
// pText receives a font-selection tag; index selects the message font.
char16_t* TagProcessor::setFontTag(char16_t* pText, u16 index) {
    WriteTagHeader(pText, 1, 2);
    pText[4] = index;
    return pText + 5;
}
// pTextBox supplies available width; width is measured text width and minimum bounds the scale.
float TagProcessor::calcAdjustTextScale(TextBoxEx* pTextBox, float width, float minimum) const {
    const float scale = pTextBox->mSizeX / width;
    return scale > minimum ? scale : minimum;
}
// pContext supplies the current font and the outermost horizontal/vertical text scales.
void TagProcessor::BeginCalculateRect(Context* pContext) {
    _10 = _18 = const_cast<nn::font::Font*>(pContext->writer->GetFont());
    if (!mPrintDepth) {
        _2c = pContext->hScale;
        _30 = pContext->vScale;
    }
    ++mPrintDepth;
}
u8 TagProcessor::calcAlphaValueStart_() { return (u32(mStartAlpha) * mAlpha) / 255; }
u8 TagProcessor::calcAlphaValueEnd_() { return (u32(mEndAlpha) * mAlpha) / 255; }
float TagProcessor::getRubyScaleMax_() const { return 1; }
float TagProcessor::getRubyBaseLinkOffset_() const { return 0; }
float TagProcessor::getRubyCharSpace_() const { return 0; }
float TagProcessor::getPictFontScale_() const { return 1; }
// pCode and pFont receive the selected pictograph and font; the base lookup ignores index.
void TagProcessor::getPictFontCodeAndFont_(char16_t* pCode, u16* pFont, u8 index) const {
    *pCode = 0; *pFont = 0;
}
// pTag and pRect are unused here; pContext resumes scanning at pNext after the page break.
TagProcessor::Operation TagProcessor::processPageBreakTag_(const TagInfo* pTag, Context* pContext,
    Rect* pRect, const char16_t* pNext) {
    pContext->str = reinterpret_cast<const u16*>(pNext);
    return Operation_NoCharSpace;
}
// code identifies the control character; pContext holds the current text rendering state.
TagProcessor::Operation TagProcessor::Process(u32 code, Context* pContext) { return process_(code, pContext, nullptr); }
// pRect receives bounds; pContext supplies text state; code identifies the control character.
TagProcessor::Operation TagProcessor::CalculateRect(Rect* pRect, Context* pContext, u32 code) {
    return process_(code, pContext, pRect);
}
// pContext is unused when unwinding the current print nesting level.
void TagProcessor::EndPrint(Context* pContext) { --mPrintDepth; }
// pContext is unused when unwinding the current bounds-calculation nesting level.
void TagProcessor::EndCalculateRect(Context* pContext) { --mPrintDepth; }
float TagProcessor::getRubyScale_() const { return 0.35f; }

}
