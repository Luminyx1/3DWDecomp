#pragma once

#include <basis/seadTypes.h>

namespace eui {
class Screen;
}

namespace sead {
class DrawContext;
}

namespace nn::ui2d {
class DrawInfo;
class Layout;
}  // namespace nn::ui2d

namespace al {
class CustomTagProcessor;
class LayoutPaneGroup;
class LayoutResource;

class LayoutKeeper {
public:
    LayoutKeeper();

    void reinitializeShader();
    void initScreen(eui::Screen* pScreen);
    void initLayout(nn::ui2d::Layout* pLayout, LayoutResource* pResource);
    void initDrawInfo(nn::ui2d::DrawInfo* pDrawInfo);
    void initTagProcessor(CustomTagProcessor* pTagProcessor);
    LayoutPaneGroup* getGroup(const char* pGroupName) const;
    LayoutPaneGroup* getGroup(s32 index) const;
    s32 getGroupNum() const;
    void calcAnim(bool isRecursive);
    void draw();

    CustomTagProcessor* getTagProcessor() const { return mTagProcessor; }
    nn::ui2d::DrawInfo* getDrawInfo() const { return mDrawInfo; }
    nn::ui2d::Layout* getLayout() const { return mLayout; }
    eui::Screen* getScreen() const { return mScreen; }

    void setDrawContext(sead::DrawContext* pDrawContext) { mDrawContext = pDrawContext; }

private:
    CustomTagProcessor* mTagProcessor = nullptr;
    nn::ui2d::DrawInfo* mDrawInfo = nullptr;
    nn::ui2d::Layout* mLayout = nullptr;
    LayoutPaneGroup** mGroups = nullptr;
    s32 mGroupNum = 0;
    eui::Screen* mScreen = nullptr;
    sead::DrawContext* mDrawContext = nullptr;
};
}  // namespace al
