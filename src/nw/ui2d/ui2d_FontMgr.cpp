#include <nn/ui2d/ui2d_FontMgr.h>
#include <nn/ui2d/ui2d_ScalableFontMgr.h>
namespace nn::ui2d {
// index selects the font table entry used by message resources.
const nn::font::Font* FontMgr::GetFontByMessageIndex(u32 index) const { return mMessageFonts[index]; }
// index selects the font table entry used by message resources.
nn::font::Font* FontMgr::GetFontByMessageIndex(u32 index) { return mMessageFonts[index]; }
// font supplies the ruby-annotation font.
void FontMgr::SetRubyFont(const nn::font::Font* font) { mRubyFont = font; }
// name is checked against the scalable-font registrations.
bool FontMgr::IsScalableFont(const char* name) const { return mScalableFonts && mScalableFonts->GetFont(name); }
}
