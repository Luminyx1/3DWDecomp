#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <gfx/seadColor.h>
#include <gfx/seadViewport.h>
#include <hostio/seadHostIONode.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>
#include <prim/seadSafeString.h>

#include "common/aglDisplayList.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureData.h"
#include "common/aglTextureEnum.h"
#include "layer/aglLayerEnum.h"
#include "layer/aglLayerJob.h"
#include "layer/aglRenderDL.h"
#include "utility/aglAtomicPtrArray.h"

namespace sead {
class Heap;
class LogicalFrameBuffer;
namespace hostio {
class Context;
class NodeEvent;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
}

namespace agl::lyr {

class DrawMethod;
class Layer;
class RenderDLBuffer;
class Renderer;

class RenderDisplay : public sead::hostio::Node {
    friend class LayerJob;
    friend class Renderer;

public:
    enum Flag {
        cFlag_FrameBufferAllocated = 1 << 2,
        cFlag_DrawDebugInfo = 1 << 3,
        cFlag_FrameBufferSizeChanged = 1 << 8,
        cFlag_ResetFrameBufferSize = 1 << 9,
        cFlag_ScanOutBufferCopied = 1 << 10,
        cFlag_11 = 1 << 11,
    };

    RenderDisplay();
    virtual ~RenderDisplay();

    void initialize(s32 displayIndex, const sead::SafeString& rName,
                    const sead::Vector2i& rVirtualSize, const sead::Vector2i& rPhysicalSize,
                    sead::LogicalFrameBuffer* pLogicalFrameBuffer, s32 layerNum, s32 renderDLNum,
                    bool createFrameBuffer, sead::Heap* pHeap);
    void freeFrameBuffer();
    void calc();
    void allocFrameBuffer(MultiSampleType multiSample, RenderDisplay* pShare,
                          TextureFormat colorFormat, s32 colorNum, TextureFormat depthFormat,
                          s32 depthNum, bool b1, bool b2);
    void clear();
    void pushBack(Layer* pLayer);
    void erase(Layer* pLayer);
    void bindAndClearRenderBuffer(DrawContext* pDrawContext) const;
    void draw(DrawContext* pDrawContext) const;
    void calcDL(sead::PtrArray<LayerJob>* pJobs);
    void sortDL() const;
    void calcGPU(sead::PtrArray<LayerJob>* pJobs) const;
    void callDisplayList(DrawContext* pDrawContext, bool b) const;
    void dumpDisplayListInfo() const;
    void dumpRawDisplayList() const;
    void destroyDisplayList() const;

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);
    void listenNodeEvent(const sead::hostio::NodeEvent* pEvent);

    u8 getDisplayIndex() const { return mDisplayIndex; }
    RenderBuffer* getFrameBuffer() const { return mFrameBuffer; }
    sead::LogicalFrameBuffer* getLogicalFrameBuffer() const { return mLogicalFrameBuffer; }
    const sead::Viewport& getViewport() const { return mViewport; }
    RenderDLBuffer* getDLBuffer() const { return mDLBuffer; }

private:
    void resetFrameBufferSize_(const sead::Vector2f& rSize);
    void copyScanOutBuffer_() const;
    void pushBackDL_(DrawContext* pDrawContext, RenderDL* pDL, const Layer* pLayer) const;
    void beginDraw_(DrawContext* pDrawContext) const;
    void drawLayerDirect_(DrawContext* pDrawContext, const Layer* pLayer,
                          FrameworkType frameworkType) const;
    void preDrawLayer_(DrawContext* pDrawContext, const Layer* pLayer,
                       FrameworkType frameworkType) const;
    void drawRenderStep_(DrawContext* pDrawContext, const Layer* pLayer, u32 flag,
                         FrameworkType frameworkType, bool useDL, s32 priority) const;
    void postDrawLayer_(DrawContext* pDrawContext, const Layer* pLayer,
                        FrameworkType frameworkType) const;
    void calcLayerDL_(DrawContext* pDrawContext, const Layer* pLayer, s32 priority) const;
    void calcSubLayerDL_(DrawContext* pDrawContext, const Layer* pLayer, s32 priority) const;
    void endDraw_(DrawContext* pDrawContext) const;
    void calcGPU_(DrawContext* pDrawContext, const Layer* pLayer, s32 priority) const;

    u8 mDisplayIndex;
    u8 _9;
    RenderBuffer* mFrameBuffer;
    TextureData* mColorTexture;
    TextureData* mDepthTexture;
    TextureData mColorTextureData;
    TextureData mDepthTextureData;
    RenderTargetColor mColorTarget;
    RenderTargetDepth mDepthTarget;
    mutable RenderTargetColor mScanOutTarget;
    mutable RenderBuffer mScanOutBuffer;
    TextureData* mResolveTexture;
    TextureData mResolveTextureData;
    RenderTargetColor mResolveTarget;
    sead::Vector2f mFrameBufferSize;
    RenderDLBuffer* mDLBuffer;
    sead::LogicalFrameBuffer* mLogicalFrameBuffer;
    sead::Viewport mViewport;
    sead::Viewport mViewportOrigin;
    mutable sead::BitFlag32 mFlag;
    sead::PtrArray<Layer> mLayer;
    sead::Buffer<LayerJob> mJob;
    u32 mClearFlag;
    sead::Color4f mClearColor;
    f32 mClearDepth;
    u32 mClearStencil;
    mutable bool mIsClear;
    mutable u64 mDLTotalSize;
    mutable u64 mDLMaxSize;
    DisplayList mDisplayList[2];
    mutable utl::AtomicPtrArray<RenderDL> mRenderDLPtr;
    mutable sead::Buffer<RenderDL> mRenderDL;
    mutable utl::AtomicPtrArray<RenderDL> mRenderDLSorted;
    DrawMethod* mBeginDrawMethod;
    DrawMethod* mEndDrawMethod;
    DrawMethod* mRenderStepDrawMethod;
};
static_assert(sizeof(RenderDisplay) == 0xfb8);

}  // namespace agl::lyr
