#pragma once
#include <nn/font/font_Font.h>
namespace nn::ui2d {
class ScalableFontMgr;
// Partial interface for existing instances; allocation and virtual hooks remain unreconstructed.
class FontMgr {
public:
    virtual ~FontMgr();
    const nn::font::Font* GetFontByMessageIndex(u32 index) const;
    nn::font::Font* GetFontByMessageIndex(u32 index);
    void SetRubyFont(const char* name);
    void SetRubyFont(const nn::font::Font* font);
    const nn::font::Font* GetFont(const char* name) const;
    nn::font::Font* GetFont(const char* name);
    bool IsScalableFont(const char* name) const;
    void* mArchive;
    int mFontCount;
    void* mResourceFonts;
    int mMessageFontCount;
    nn::font::Font** mMessageFonts;
    const nn::font::Font* mRubyFont;
    ScalableFontMgr* mScalableFonts;
};
}
