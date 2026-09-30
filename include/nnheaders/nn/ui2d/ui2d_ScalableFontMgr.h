#pragma once
#include <nn/font/font_ScalableFont.h>
namespace nn::ui2d {
// Partial interface: virtual extension hooks are not yet reconstructed.
// Use existing instances through pointers until initialization is implemented.
class ScalableFontMgr {
public:
    struct FontParameter {
        FontParameter();
        FontParameter(const char* name, int size, u16 face, int alternateSize, u16 alternateFace);
        const char* name;
        int size;
        u16 face;
        int alternateSize;
        u16 alternateFace;
    };
    struct InitializeArg {
        InitializeArg();
        const void* textureCacheArgs;
        const FontParameter* fonts;
        int fontCount;
    };
    struct FontEntry {
        char name[128];
        void* _80;
        nn::font::ScalableFont font;
    };
    virtual ~ScalableFontMgr();
    const nn::font::ScalableFont* GetFont(const char* name) const;
    nn::font::ScalableFont* GetFont(const char* name);
    const char* FindFontName(const nn::font::ScalableFont* font) const;
    bool IsGlyphsReady(const u16* text, u32 length, const nn::font::ScalableFont* font);
    bool RegisterGlyphs(const u16* text, u32 length, const nn::font::ScalableFont* font, int lockGroup, bool checkOnly);
    void* _08;
    nn::font::TextureCache* mTextureCache;
    int mFontCount;
    FontEntry* mFonts;
};
static_assert(sizeof(ScalableFontMgr::FontEntry) == 0xc0, "FontEntry size");
}
