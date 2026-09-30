#include <nn/ui2d/ui2d_ScalableFontMgr.h>
#include <cstring>
namespace nn::ui2d {
ScalableFontMgr::FontParameter::FontParameter() : name(nullptr), size(0), face(0), alternateSize(0), alternateFace(0) {}
// name identifies the font; size/face and alternateSize/alternateFace select its two faces.
ScalableFontMgr::FontParameter::FontParameter(const char* name, int size, u16 face, int alternateSize, u16 alternateFace)
    : name(name), size(size), face(face), alternateSize(alternateSize), alternateFace(alternateFace) {}
ScalableFontMgr::InitializeArg::InitializeArg() : textureCacheArgs(nullptr), fonts(nullptr), fontCount(0) {}
// name is the registered scalable-font name to find.
const nn::font::ScalableFont* ScalableFontMgr::GetFont(const char* name) const {
    for (int i = 0; i < mFontCount; ++i) if (std::strcmp(mFonts[i].name, name) == 0) return &mFonts[i].font;
    return nullptr;
}

// name is the registered scalable-font name to find.
nn::font::ScalableFont* ScalableFontMgr::GetFont(const char* name) {
    return const_cast<nn::font::ScalableFont*>(static_cast<const ScalableFontMgr*>(this)->GetFont(name));
}

// font identifies an entry whose registration name is returned.
const char* ScalableFontMgr::FindFontName(const nn::font::ScalableFont* font) const {
    for (int i = 0; i < mFontCount; ++i) if (&mFonts[i].font == font) return mFonts[i].name;
    return nullptr;
}

// text contains length UTF-16 code units; font selects the glyph cache to check.
bool ScalableFontMgr::IsGlyphsReady(const u16* text, u32 length, const nn::font::ScalableFont* font) {
    return RegisterGlyphs(text, length, font, -1, true);
}
}
