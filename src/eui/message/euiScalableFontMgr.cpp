#include <eui/euiScalableFontMgr.h>
namespace eui {
// NON_MATCHING: initialization store grouping still differs.
ScalableFontMgr::ScalableFontMgr() : _28(nullptr), mTextureCache(nullptr), mUpdateThread(nullptr),
    mReservedTextBox(nullptr), _58(0), _5c(0), _5d(0), _5e(0) {}
ScalableFontMgr::FontParameter::FontParameter() : name(nullptr), size(0), face(0), _10(0) {}
// pName names the font; size and face select its glyphs; value supplies the fourth font parameter.
ScalableFontMgr::FontParameter::FontParameter(const char* pName, int size, u16 face, int value)
    : name(pName), size(size), face(face), _10(value) {}
// NON_MATCHING: branch relocation awaits registerGlyphs_ reconstruction.
// pText and length identify the text; pFont supplies the font whose glyph readiness is checked.
bool ScalableFontMgr::isGlyphsReady(const char16_t* pText, u32 length, const nn::font::ScalableFont* pFont) {
    return registerGlyphs_(pText, length, pFont, -1, true);
}
// NON_MATCHING: branch relocation awaits registerGlyphs_ reconstruction.
// pText and length identify the text; pFont selects the font; lockGroup groups cache locks.
bool ScalableFontMgr::registerGlyphs(const char16_t* pText, u32 length, const nn::font::ScalableFont* pFont, int lockGroup) {
    return registerGlyphs_(pText, length, pFont, lockGroup, false);
}
// pTextBox replaces the pending text box; the previous reservation is returned.
ScalableFontTextBoxEx* ScalableFontMgr::reserveRegisterGlyphs(ScalableFontTextBoxEx* pTextBox) {
    return __atomic_exchange_n(&mReservedTextBox, pTextBox, __ATOMIC_RELAXED);
}
}
