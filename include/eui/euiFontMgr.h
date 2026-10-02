#pragma once
#include <eui/euiSharcArchive.h>
#include <container/seadBuffer.h>
#include <heap/seadDisposer.h>
#include <nn/font/font_ResFont.h>
namespace eui {
class ScalableFontMgr;
class FontMgr {
    SEAD_SINGLETON_DISPOSER(FontMgr);
public:
    FontMgr();
    virtual ~FontMgr();
    void initialize(sead::Heap* pHeap, void* pArchiveData, u32 archiveSize,
                    const sead::Buffer<const char*>& rFontNames, ScalableFontMgr* pScalableFontMgr);
    void finalize();
    nn::font::Font* tryGetFont(const sead::SafeString& rName);
    void setRubyFont(const sead::SafeString& rName);
    nn::font::Font* getFont(const sead::SafeString& rName);
    const nn::font::Font* getFont(const sead::SafeString& rName) const;
    nn::font::Font* getFontByMessageIndex(u32 index);
    const nn::font::Font* getFontByMessageIndex(u32 index) const;
    void setRubyFont(const nn::font::Font* pFont);
    nn::font::Font* tryGetFont(const sead::SafeString& rName) const;

    ScalableFontMgr* getScalableFontMgr() const { return mScalableFontMgr; }

    SharcArchive mArchive;
    sead::Buffer<nn::font::ResFont> mFonts;
    sead::Buffer<nn::font::Font*> mMessageFonts;
    const nn::font::Font* mRubyFont;
    ScalableFontMgr* mScalableFontMgr;
};

static_assert(sizeof(FontMgr) == 0x60, "FontMgr size");
}
