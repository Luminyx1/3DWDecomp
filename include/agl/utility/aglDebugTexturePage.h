#pragma once

#include <container/seadBuffer.h>
#include <container/seadTList.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

namespace sead {
class Heap;
class LogicalFrameBuffer;
class Viewport;
namespace hostio {
class Context;
class NodeEvent;
class PropertyEvent;
class PropertyEventListener;
class Reflexible;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
class RenderTargetDepth;
class TextureData;
}  // namespace agl

namespace agl::utl {

class DebugTextureDrawer;

class DebugTexture : public sead::TListNode<DebugTexture*> {
    friend class DebugTexturePage;
    friend class DebugTextureDrawer;

public:
    enum Type {
        cType_Color = 0,
        cType_1 = 1,
        cType_2 = 2,
        cType_3 = 3,
    };

    DebugTexture();
    virtual ~DebugTexture();

    void initialize(const sead::SafeString& rName);
    bool copyTexture(DrawContext* pDrawContext, const TextureData& rTexture, Type type, f32 min,
                     f32 max, bool isLineBreakBefore, bool isLineBreakAfter, bool isSpecial);
    void setReference(const TextureData& rTexture, Type type, f32 min, f32 max,
                      bool isLineBreakBefore, bool isLineBreakAfter, bool isSpecial);
    bool expand(DrawContext* pDrawContext, const RenderTargetDepth& rDepth, Type type, f32 min,
                f32 max, u8 index, bool isLineBreakBefore, bool isLineBreakAfter, bool isSpecial);
    bool copyCurrentRenderTargetColor(DrawContext* pDrawContext, s32 index);
    bool copyCurrentRenderTargetDepth(DrawContext* pDrawContext);
    void freeTexture();
    void allocTexture(DrawContext* pDrawContext, const TextureData& rTexture, Type type, f32 min,
                      f32 max, bool isLineBreakBefore, bool isLineBreakAfter, bool isSpecial);

    const sead::SafeString& getName() const { return mName; }
    const TextureData* getTexture() const { return mTexture; }

private:
    sead::FixedSafeString<32> mName;
    const TextureData* mTexture = nullptr;
    bool mIsLineBreakBefore = false;
    bool mIsLineBreakAfter = false;
    bool mIsSpecial;
    Type mType = cType_Color;
    f32 mMin = 0.0f;
    f32 mMax = 0.0f;
    u8 mIndex = 0xff;
    bool mIsAllocated = false;
};
static_assert(sizeof(DebugTexture) == 0x80);

class DebugTexturePage : public sead::IDisposer,
                         public sead::hostio::Node,
                         public sead::TListNode<DebugTexturePage*> {
    friend class DebugTextureDrawer;

public:
    struct DrawOption {
        bool mIsDrawLabel = true;
        bool mIsDrawLabelBg = false;
        bool _2 = false;
        s32 mBlendType = 0;
        s32 _8 = 0;
        s32 _c;
    };
    static_assert(sizeof(DrawOption) == 0x10);

    class Context {
    public:
        Context();
        ~Context();

        void setUp(const DebugTexturePage* pPage, sead::Heap* pHeap);
        void pushBack(DebugTexture* pTexture, const sead::SafeString& rSelectName) const;
        void draw(DrawContext* pDrawContext, const sead::LogicalFrameBuffer& rFrameBuffer,
                  const sead::Viewport& rViewport, const DrawOption& rOption) const;
        void clearEntryTextures() const;
        const DebugTexture* searchFullScreenTexture() const;
        void copyTextureLabel() const;
        void genMessageContextComboBox(sead::hostio::Context* pContext, bool isActive,
                                       sead::hostio::PropertyEventListener* pListener);
        void genMessageContextParameter(sead::hostio::Context* pContext, bool isActive,
                                        sead::hostio::PropertyEventListener* pListener);
        void listenPropertyEventContext(const sead::hostio::PropertyEvent* pEvent);

    private:
        sead::Vector2f calcTextureDrawSize_(const TextureData& rTexture) const;
        void drawTexture_(DrawContext* pDrawContext, const DebugTexture& rTexture,
                          const sead::Viewport& rViewport, const sead::Vector2f& rPos,
                          const sead::Vector2f& rScale, const DrawOption& rOption,
                          bool isSpecial) const;
        void drawLabel_(DrawContext* pDrawContext, const sead::SafeString& rLabel,
                        const TextureData& rTexture, const sead::Viewport& rViewport,
                        const sead::Vector2f& rPos, bool isDrawLabel, bool isDrawLabelBg) const;

        mutable s32 mLabelNum;
        mutable sead::TList<DebugTexture*> mTextures;
        sead::Vector2f mScroll = {0.0f, 0.0f};
        f32 mScale = 1.0f;
        s32 mSliceIndex = -1;
        s32 mMipLevel = -1;
        mutable s32 mSelectIndex = -1;
        mutable f32 mContentHeight = 100.0f;
        const DebugTexturePage* mPage = nullptr;
    };
    static_assert(sizeof(Context) == 0x48);

    DebugTexturePage();
    ~DebugTexturePage() override { cleanUp(); }

    void setUp(u32 num, const sead::SafeString& rName, sead::Heap* pHeap);
    void cleanUp();
    void setActive(bool active);
    bool isActive() const { return mIsActive; }

    void entryBoundRenderBuffer(DrawContext* pDrawContext, s32 index,
                                const sead::SafeString& rName) const;

    void genMessage(sead::hostio::Context* pContext);
    void genMessagePage(sead::hostio::Context* pContext, sead::hostio::Reflexible* pReflexible);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);
    void listenNodeEvent(const sead::hostio::NodeEvent* pEvent);
    void invalidateHostIoNode();

private:
    bool entryTexture_(DrawContext* pDrawContext, s32 index, const TextureData& rTexture,
                       const sead::SafeString& rName, DebugTexture::Type type, f32 min, f32 max,
                       bool isLineBreakBefore, bool isLineBreakAfter, bool isSpecial,
                       bool isCopy) const;
    bool entryRenderTargetDepth_(DrawContext* pDrawContext, s32 index,
                                 const RenderTargetDepth& rDepth, const sead::SafeString& rName,
                                 DebugTexture::Type type, f32 min, f32 max, u8 depthIndex,
                                 bool isLineBreakBefore, bool isLineBreakAfter,
                                 bool isSpecial) const;
    void clearEntryTextures_() const;
    void draw_(DrawContext* pDrawContext, const sead::LogicalFrameBuffer& rFrameBuffer,
               const sead::Viewport& rViewport) const;
    void genNodePage_(sead::hostio::Context* pContext);
    void genMessagePage_(sead::hostio::Context* pContext, sead::hostio::Reflexible* pReflexible,
                         sead::hostio::PropertyEventListener* pListener);
    bool updateNodeMeta_();

    const Context& getCurrentContext_() const { return mContexts[mCurrentContext]; }
    Context& getCurrentContext_() { return mContexts[mCurrentContext]; }

    sead::FixedSafeString<128> mName{sead::SafeString("")};
    sead::FixedSafeString<128> mSelectTextureName;
    sead::FixedSafeString<128> mNodeMeta;
    bool mIsActive = false;
    s32 mCurrentContext = 0;
    sead::Buffer<Context> mContexts;
    DrawOption mDrawOption;
    sead::hostio::Reflexible* mOwner = nullptr;
};
static_assert(sizeof(DebugTexturePage) == 0x240);

}  // namespace agl::utl
