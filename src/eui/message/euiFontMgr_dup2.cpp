#include <eui/euiFontMgr.h>
namespace eui {
SEAD_SINGLETON_DISPOSER_IMPL(FontMgr);
FontMgr::FontMgr() : mRubyFont(nullptr), mScalableFontMgr(nullptr) {}
FontMgr::~FontMgr() = default;
// index selects the font associated with a message font index.
nn::font::Font* FontMgr::getFontByMessageIndex(u32 index) { return mMessageFonts[index]; }
// index selects the font associated with a message font index.
const nn::font::Font* FontMgr::getFontByMessageIndex(u32 index) const { return mMessageFonts[index]; }
// pFont is the font used for ruby annotations, or null to clear it.
void FontMgr::setRubyFont(const nn::font::Font* pFont) { mRubyFont = pFont; }
}
