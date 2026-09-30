#pragma once

#include <basis/seadTypes.h>

namespace agl {
class DrawContext;
class RenderBuffer;
}  // namespace agl

namespace eui {
class DrawInfoEx;
}

namespace sead {
class Viewport;
}

namespace al {
class EffectSystem;
class ExecuteDirector;
class FontHolder;
class LayoutSystem;
struct LayoutRenderInfo;

class LayoutKit {
public:
    LayoutKit(FontHolder* pFontHolder);
    ~LayoutKit();

    void createCameraParamForIcon();
    void createExecuteDirector(s32 requestCount);
    void createEffectSystem();
    void endInit();
    void update();
    void setFrameBuffer(const agl::RenderBuffer* pRenderBuffer, const sead::Viewport* pViewport);
    void draw(const char* pTableName) const;
    void drawList(const char* pTableName, const char* pListName) const;
    void setLayoutSystem(LayoutSystem* pLayoutSystem);
    void setDrawContext(agl::DrawContext* pDrawContext);

    ExecuteDirector* getExecuteDirector() const { return mExecuteDirector; }
    EffectSystem* getEffectSystem() const { return mEffectSystem; }
    agl::DrawContext* getDrawContext() const { return mDrawContext; }
    eui::DrawInfoEx* getDrawInfo() const { return mDrawInfo; }
    LayoutRenderInfo* getRenderInfo() const { return mRenderInfo; }

    void setEffectSystem(EffectSystem* pEffectSystem) { mEffectSystem = pEffectSystem; }

    FontHolder* mFontHolder;
    ExecuteDirector* mExecuteDirector = nullptr;
    EffectSystem* mEffectSystem = nullptr;
    LayoutSystem* mLayoutSystem = nullptr;
    agl::DrawContext* mDrawContext = nullptr;
    eui::DrawInfoEx* mDrawInfo = nullptr;
    LayoutRenderInfo* mRenderInfo = nullptr;
};

static_assert(sizeof(LayoutKit) == 0x38);
}  // namespace al
