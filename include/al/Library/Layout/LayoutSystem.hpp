#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace eui {
class ScalableFontMgr;
class ScreenMgr;
}  // namespace eui

namespace nn::font {
class Font;
}

namespace nn::ui2d {
class GraphicsResource;
}

namespace sead {
class Heap;
}

namespace al {
class Resource;

struct FontNamePair {
    FontNamePair();

    nn::font::Font* font;
    sead::FixedSafeString<128> name;
    bool isLoaded;
};

static_assert(sizeof(FontNamePair) == 0xa8);

class LayoutSystem {
public:
    typedef sead::FixedSafeString<128> FontFileName;

    LayoutSystem();

    void init(bool isSmallFontHeap);
    void initGraphicsResource();
    void initFont();
    void initEui();
    nn::font::Font* tryFindFont(const char* pFontName) const;
    FontNamePair* getFontNamePair(s32 index) const;
    void finalizeFontData();
    void initFontForChangeLanguage();
    void reinitFont(sead::Heap* pHeap);
    void beginDraw() const;
    void endDraw() const;
    void initFontList();

    nn::ui2d::GraphicsResource* getGraphicsResource() const { return mGraphicsResource; }

    eui::ScreenMgr* getScreenMgr() const { return mScreenMgr; }

    eui::ScalableFontMgr* getScalableFontMgr() const { return mScalableFontMgr; }

    s32 getFontNamePairNum() const { return mFontNamePairNum; }

private:
    nn::ui2d::GraphicsResource* mGraphicsResource = nullptr;
    FontNamePair* mFontNamePairs = nullptr;
    s32 mFontNamePairNum = 0;
    eui::ScreenMgr* mScreenMgr = nullptr;
    sead::Heap* mFontHeap = nullptr;
    eui::ScalableFontMgr* mScalableFontMgr = nullptr;
    bool mIsSmallFontHeap = false;
    Resource* mFontResource = nullptr;
    FontFileName* mFontFileNames = nullptr;
    s32 mFontFileNameNum = 0;
};

static_assert(sizeof(LayoutSystem) == 0x50);
}  // namespace al
