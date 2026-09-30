#pragma once
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <container/seadBuffer.h>
#include <nn/ui2d/ui2d_GraphicsResource.h>
#include <nn/ui2d/ui2d_ControlCreator.h>
#include <eui/euiSharcArchive.h>
#include <eui/euiDrawInfoEx.h>
namespace eui {
class Screen;
class ArcResourceMgr;
class BoxCursorMgr;
class FontMgr;
class BoxCursorNode;
class ScreenMgr : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(ScreenMgr);
public:
    ScreenMgr();
    virtual ~ScreenMgr();
    void updateViewer_();
    void inactivateScreen(int index);
    void activateScreen(int index);
    void updateSystem();
    void draw(s8 layer, const DrawInfoEx::RenderBufferInfo* pInfo);
    void unloadScreen(int index);
    void resetScreenId(int index);
    void eraseBoxCursorNodeFromRouteNodes(const BoxCursorNode* pNode);
    sead::Buffer<Screen*> mScreens;
    sead::Buffer<s8> mScreenLayers;
    void* _48;
    nn::ui2d::GraphicsResource mGraphicsResource;
    sead::hostio::Node mHostIONode;
    ArcResourceMgr* mArcResourceMgr;
    BoxCursorMgr* mBoxCursorMgr;
    float mAnimationStep;
    SharcArchive mArchive;
    void* _430;
    FontMgr* mFontMgr;
    bool _440, _441, _442;
    void* _448;
};
static_assert(sizeof(nn::ui2d::GraphicsResource) == 0x3b8, "GraphicsResource size");
static_assert(sizeof(ScreenMgr) == 0x450, "ScreenMgr size");
}
