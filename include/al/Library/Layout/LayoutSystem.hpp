#pragma once

#include <basis/seadTypes.h>

namespace agl {
class DrawContext;
class RenderBuffer;
}  // namespace agl

namespace eui {
class DrawInfoEx;
class ScreenMgr;
}  // namespace eui

namespace nn::font {
class Font;
}

namespace nn::ui2d {
class DrawInfo;
class GraphicsResource;
}  // namespace nn::ui2d

namespace sead {
class GraphicsContext;
class Viewport;
}  // namespace sead

namespace al {
class EffectSystem;
class ExecuteDirector;
class FontHolder;

struct LayoutFontList {
    u8 _0[0x5e];
    bool isInvalid;
};

class LayoutSystem {
public:
    LayoutSystem();

    void init(bool isInitEui);
    void initGraphicsResource();
    nn::font::Font* tryFindFont(const char* pFontName) const;
    void beginDraw() const;
    void endDraw() const;

    nn::ui2d::GraphicsResource* getGraphicsResource() const { return mGraphicsResource; }
    eui::ScreenMgr* getScreenMgr() const { return mScreenMgr; }
    LayoutFontList* getFontList() const { return mFontList; }

private:
    nn::ui2d::GraphicsResource* mGraphicsResource = nullptr;
    void* _8 = nullptr;
    s32 _10 = 0;
    eui::ScreenMgr* mScreenMgr = nullptr;
    void* _20 = nullptr;
    LayoutFontList* mFontList = nullptr;
    bool _30 = false;
    void* _38 = nullptr;
    void* _40 = nullptr;
    s32 _48 = 0;
};

struct LayoutRenderInfo {
    const agl::RenderBuffer* renderBuffer = nullptr;
    sead::GraphicsContext* graphicsContext = nullptr;
    sead::Viewport* viewport = nullptr;
    void* _18 = nullptr;
    agl::DrawContext* drawContext = nullptr;
};

inline LayoutRenderInfo* getLayoutRenderInfo(eui::DrawInfoEx* pDrawInfo) {
    return *reinterpret_cast<LayoutRenderInfo**>(reinterpret_cast<u8*>(pDrawInfo) + 0x1a0);
}
}  // namespace al
