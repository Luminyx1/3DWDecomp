#include <eui/euiTagProcessor.h>
namespace eui {
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
}
