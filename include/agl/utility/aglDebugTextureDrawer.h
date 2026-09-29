#pragma once

#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <container/seadSafeArray.h>
#include <container/seadTList.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadSafeString.h>
#include <thread/seadCriticalSection.h>

#include "utility/aglDebugTexturePage.h"

namespace agl::utl {

class DebugTextureDrawer : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(DebugTextureDrawer)

    friend class DebugTexturePage;

public:
    DebugTextureDrawer();
    virtual ~DebugTextureDrawer();

    void initialize(sead::Heap* pHeap);
    void destroy();
    void inactivateAll();
    void draw(DrawContext* pDrawContext, const sead::LogicalFrameBuffer& rFrameBuffer,
              const sead::Viewport& rViewport) const;
    void clearEntryTextures();
    DebugTexturePage* searchPage(const sead::SafeString& rName);

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);
    void listenNodeEvent(const sead::hostio::NodeEvent* pEvent);

private:
    static constexpr s32 cTextureNum = 128;
    static constexpr s32 cActivePageNum = 16;
    static constexpr s32 cTextureLabelNum = 128;

    void entryPage_(DebugTexturePage* pPage);
    void erasePage_(DebugTexturePage* pPage);
    void eraseActivePage_(DebugTexturePage* pPage);
    void pushBack_(DebugTexture* pTexture);
    DebugTexture* popBack_();
    void invalidateHostIoNode_(DebugTexturePage* pPage);
    bool copyTextureLabel_(s32 index, const sead::SafeString& rLabel);

    sead::Buffer<DebugTexture> mTextures;
    sead::TList<DebugTexture*> mFreeTextures;
    sead::TList<DebugTexturePage*> mPages;
    sead::FixedPtrArray<DebugTexturePage, cActivePageNum> mActivePages;
    sead::SafeArray<sead::FixedSafeString<128>, cTextureLabelNum> mTextureLabels;
    bool mIsEnableDraw = true;
    bool _4cf9 = true;
    sead::CriticalSection mActivePageCS;
    sead::CriticalSection mTextureCS;
    sead::CriticalSection mPageCS;
};
static_assert(sizeof(DebugTextureDrawer) == 0x4dc0);

}  // namespace agl::utl
