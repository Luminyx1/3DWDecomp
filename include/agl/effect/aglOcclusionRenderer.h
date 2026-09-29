#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <gfx/seadColor.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <hostio/seadHostIONode.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "common/aglGPUMemAddr.h"
#include "common/aglGPUMemBlock.h"
#include "common/aglIndexStream.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
#include "common/aglUniformBlock.h"
#include "common/aglVertexAttribute.h"
#include "common/aglVertexBuffer.h"
#include "cull/aglViewFrustumCulling.h"
#include "utility/aglDebugTexturePage.h"

namespace sead {
class Heap;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
}  // namespace agl

namespace agl::fx {

class OcclusionRenderer : public sead::hostio::Node {
public:
    struct CalcResult {
        sead::Vector3f mScreenPos;
        f32 mDepth;
    };

    struct OcclVtx {
        f32 mRate;
        f32 mSin;
        f32 mCos;
    };

    struct SubContext {
        TextureData* mTexture = nullptr;
        GPUMemVoidAddr mAddr;
        TextureSampler mSampler;
        RenderTargetColor mRenderTarget;
    };
    static_assert(sizeof(SubContext) == 0x308);

    struct Context {
        UniformBlock mUniformBlock;
        sead::Matrix34f mOccluderMtx;
        sead::Matrix34f mViewMtx;
        sead::Matrix44f mProjMtx;
        sead::Matrix44f mOccluderProjMtx{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                         0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
        f32 mSin = 0.0f;
        f32 mCos = 0.0f;
        f32 mScreenRadius = 0.0f;
        f32 mAspect = 0.0f;
        f32 mRadiusScale = 0.0f;
        f32 mSampleRate = 0.0f;
        f32 mSampleScale = 0.0f;
        sead::Vector3f mOffset;
        sead::Vector3f mPos;
        cull::ViewFrustumCulling mViewFrustumCulling;
        TextureSampler mSampler;
        sead::Viewport mViewport;
        RenderBuffer mRenderBuffer;
        sead::Buffer<SubContext> mSub;
    };
    static_assert(sizeof(Context) == 0x5d8);

    class OcclVtxStream {
    public:
        void initialize(s32 num, s32 divNum, f32 angle, sead::Heap* pHeap);
        void create(s32 num, s32 divNum, f32 angle);

        s32 mNum = 0x80;
        f32 mScale = 0.0f;
        VertexAttribute mVertexAttribute;
        GPUMemBlock<OcclVtx> mVertexBlock;
        VertexBuffer mVertexBuffer;
        GPUMemBlock<u16> mIndexBlock;
        IndexStream mIndexStream;
    };
    static_assert(sizeof(OcclVtxStream) == 0x3d8);

    class ClearBufVtxStream {
    public:
        void initialize(sead::Heap* pHeap);

        VertexAttribute mVertexAttribute;
        GPUMemBlock<u32> mVertexBlock;
        VertexBuffer mVertexBuffer;
        GPUMemBlock<u16> mIndexBlock;
        IndexStream mIndexStream;
    };
    static_assert(sizeof(ClearBufVtxStream) == 0x3d0);

    OcclusionRenderer();
    virtual ~OcclusionRenderer();

    f32 getOcclusionRate(s32 index) const;
    f32 getCoreOcclusionRate(s32 index) const;
    void initialize(s32 contextNum, sead::Heap* pHeap);
    void calc();
    void calcContext(s32 index, const sead::Matrix34f& rView, const sead::Matrix44f& rProj,
                     f32 near, f32 far, f32 fovy, f32 aspect, const sead::Vector2f& rOffset,
                     CalcResult* pResult);
    void updateGPU();
    void updateViewGPU(s32 index, const RenderBuffer& rRenderBuffer);
    void draw(DrawContext* pDrawContext, s32 index, const RenderTargetDepth& rDepth) const;
    void release(DrawContext* pDrawContext, s32 index) const;
    void drawDebug(DrawContext* pDrawContext, s32 index, const sead::Color4f& rColor0,
                   const sead::Color4f& rColor1, const sead::Color4f& rColor2) const;
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    bool isEnable() const { return mEnable; }

private:
    sead::Buffer<Context> mContext;
    sead::GraphicsContext mClearGraphicsContext;
    sead::GraphicsContext mDrawGraphicsContext;
    utl::DebugTexturePage mDebugTexturePage;
    bool mEnable = true;
    bool mBorderBlack = true;
    bool mUseCore = false;
    bool mUseSoft = false;
    bool mAutoDirection = false;
    s32 mBufferIndex = 0;
    sead::Vector3f mOffset{40.0f, 0.0f, 0.0f};
    f32 mSize = 40.0f;
    f32 mSampleSize = 10.0f;
    f32 mPower = 1.0f;
    s32 mDivNum = 8;
    f32 mThreshold = 0.0f;
    f32 mAngle = 2.8159999f;
    s32 mRingNum = 25;
    OcclVtxStream mOcclVtxStream;
    ClearBufVtxStream mClearBufVtxStream;
};
static_assert(sizeof(OcclusionRenderer) == 0xb20);

}  // namespace agl::fx
