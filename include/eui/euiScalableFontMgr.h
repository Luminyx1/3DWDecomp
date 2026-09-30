#pragma once
#include <heap/seadDisposer.h>
#include <container/seadBuffer.h>
#include <prim/seadSafeString.h>
#include <nn/font/font_ScalableFont.h>
namespace eui {
class ScalableFontTextBoxEx;
class ScalableFontMgr {
    SEAD_SINGLETON_DISPOSER(ScalableFontMgr);
public:
    struct FontParameter {
        FontParameter();
        FontParameter(const char* pName, int size, u16 face, int value);
        const char* name;
        int size;
        u16 face;
        int _10;
    };
    struct FontEntry { sead::SafeString name; nn::font::ScalableFont font; };
    ScalableFontMgr();
    virtual ~ScalableFontMgr();
    bool isGlyphsReady(const char16_t* pText, u32 length, const nn::font::ScalableFont* pFont);
    bool registerGlyphs(const char16_t* pText, u32 length, const nn::font::ScalableFont* pFont, int lockGroup);
    bool registerGlyphs_(const char16_t* pText, u32 length, const nn::font::ScalableFont* pFont, int lockGroup, bool checkOnly);
    ScalableFontTextBoxEx* reserveRegisterGlyphs(ScalableFontTextBoxEx* pTextBox);
    void* _28;
    nn::font::TextureCache* mTextureCache;
    sead::Buffer<FontEntry> mFonts;
    void* mUpdateThread;
    ScalableFontTextBoxEx* mReservedTextBox;
    u32 _58;
    u8 _5c, _5d, _5e;
};
static_assert(sizeof(ScalableFontMgr) == 0x60, "ScalableFontMgr size");
static_assert(sizeof(ScalableFontMgr::FontEntry) == 0x48, "FontEntry size");
}
