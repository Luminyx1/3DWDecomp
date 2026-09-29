#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <gfx/seadColor.h>
#include <hostio/seadHostIONode.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>

#include "common/aglGPUMemAddr.h"
#include "common/aglGPUMemBlock.h"
#include "common/aglIndexStream.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
#include "common/aglVertexAttribute.h"
#include "common/aglVertexBuffer.h"
#include "utility/aglDebugTexturePage.h"

namespace sead {
class Heap;
namespace hostio {
class Context;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
}  // namespace agl

namespace agl::fx {

class RadialBlur : public sead::hostio::Node {
public:
    struct BlurParameter {
        BlurParameter();

        sead::Vector3f mCenter;
        f32 mRadius;
        f32 mPower;
        sead::Color4f mColor0;
        sead::Color4f mColor1;
        sead::Color4f mColor2;
        sead::Color4f mColor3;
        bool mDepthTestEnable;
        f32 mDepthOffset;
        bool mBlendEnable;
        u32 mBlendFactorSrc;
        u32 mBlendFactorDst;
        u32 mBlendEquation;
        f32 mReduceScale;
        s32 mSampleNum;
    };
    static_assert(sizeof(BlurParameter) == 0x74);

    struct SharedData {
        sead::Buffer<sead::BitFlag32> mViewMask;
        sead::Buffer<BlurParameter> mParam;
        sead::BitFlag32 mDefaultViewMask;
        BlurParameter mDefaultParam;
        s32 mNum;
        utl::DebugTexturePage mDebugTexturePage;
        VertexAttribute mVertexAttribute;
        IndexStream mIndexStream;
    };
    static_assert(sizeof(SharedData) == 0x520);

    struct Vertex {
        sead::Vector2f mPos;
    };

    class Context {
    public:
        struct ScreenParameter {
            sead::Vector3f mPos;
            f32 mRadius;
        };

        Context();
        ~Context();

        void initialize(s32 blurNum, sead::Heap* pHeap);
        void calc(const SharedData& rShared, const sead::Matrix34f& rView,
                  const sead::Matrix44f& rProj);
        void updateGPU();
        void draw(DrawContext* pDrawContext, s32 view, const SharedData& rShared,
                  const RenderBuffer& rRenderBuffer, const TextureData& rTexture) const;
        void allocHalfBufferTexture(DrawContext* pDrawContext, const TextureData& rTexture) const;
        void updateHalfBuffer(DrawContext* pDrawContext, s32 view, const SharedData& rShared,
                              const RenderBuffer& rRenderBuffer,
                              const TextureData& rTexture) const;
        void draw2D(DrawContext* pDrawContext, s32 view, const SharedData& rShared,
                    const RenderBuffer& rRenderBuffer, const TextureData& rTexture) const;
        void freeHalfBufferTexture() const;
        void allocReduceTexture(DrawContext* pDrawContext, const TextureData& rTexture) const;
        void freeReduceTexture() const;

    private:
        friend class RadialBlur;

        void drawToReduceTexture_(DrawContext* pDrawContext, s32 view,
                                  const BlurParameter& rParam, const SharedData& rShared,
                                  const ScreenParameter& rScreen,
                                  const TextureData& rTexture) const;
        void drawToRenderBuffer_(DrawContext* pDrawContext, s32 view, const BlurParameter& rParam,
                                 const SharedData& rShared, const ScreenParameter& rScreen) const;

        sead::Buffer<ScreenParameter> mScreen;
        ScreenParameter mDefaultScreen;
        sead::Buffer<s32> mOrder;
        mutable TextureData* mReduceTexture = nullptr;
        mutable GPUMemVoidAddr mReduceAddr;
        mutable TextureData mReduceTarget;
        mutable RenderBuffer mRenderBuffer;
        mutable RenderTargetColor mRenderTarget;
        mutable TextureSampler mSampler;
        mutable TextureData* mHalfBuffer = nullptr;
        mutable GPUMemVoidAddr mHalfBufferAddr;
    };
    static_assert(sizeof(Context) == 0x4e8);

    RadialBlur();
    virtual ~RadialBlur();

    void initialize(s32 contextNum, s32 blurNum, sead::Heap* pHeap);
    void calc();
    void calcView(s32 index, const sead::Matrix34f& rView, const sead::Matrix44f& rProj);
    void updateGPU();
    void updateViewGPU(s32 index);
    void draw(DrawContext* pDrawContext, s32 index, const RenderBuffer& rRenderBuffer,
              const TextureData& rTexture) const;
    void draw2D(DrawContext* pDrawContext, s32 index, const RenderBuffer& rRenderBuffer,
                const TextureData& rTexture, const TextureData& rUnused) const;
    void allocReduceTexture(DrawContext* pDrawContext, s32 index,
                            const TextureData& rTexture) const;
    void freeReduceTexture(s32 index) const;
    void genMessage(sead::hostio::Context* pContext);

private:
    void initVertex_(s32 divNum, s32 ringNum, sead::Heap* pHeap);
    void initIndex_(s32 divNum, s32 ringNum, sead::Heap* pHeap);

    SharedData mShared;
    sead::Buffer<Context> mContext;
    GPUMemBlock<Vertex> mVertexBlock;
    GPUMemBlock<u16> mIndexBlock;
    VertexBuffer mVertexBuffer;
};

}  // namespace agl::fx
