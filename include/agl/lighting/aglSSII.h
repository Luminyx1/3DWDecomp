#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <hostio/seadHostIONode.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>

#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
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
}

namespace agl::lght {

class SSII : public sead::hostio::Node {
public:
    enum BufType {
        cBufType_Albedo = 0,
        cBufType_Light = 1,
        cBufType_Normal = 2,
        cBufType_Depth = 3,
        cBufType_Num = 4,
    };

    struct RedBufRenderSetting {
        s32 mWidth;
        s32 mHeight;
        s32 mSampleNum;
        s32 mMode;
        f32 mIntensity;
        s32 mRefLevel;
        s32 mRefMode;
        f32 mRefIntensity;
    };
    static_assert(sizeof(RedBufRenderSetting) == 0x20);

    class TexBuf {
    public:
        TexBuf()
            : mTextureData(nullptr), mWidth(0), mHeight(0), mIsAllocated(false),
              mIsWithoutContext(false)
        {
        }
        ~TexBuf();

        void init(DrawContext* pDrawContext, BufType type, u32 width, u32 height,
                  const char* pName, bool withoutContext) const;
        void init(DrawContext* pDrawContext, const TextureData* pTextureData, u32 width,
                  u32 height) const;
        void free() const;

        void setTextureData_(DrawContext* pDrawContext, const TextureData* pTextureData) const
        {
            mTextureData = pTextureData;
            mSampler.applyTextureData(*pTextureData);
        }

        mutable const TextureData* mTextureData;
        mutable TextureSampler mSampler;
        mutable RenderBuffer mRenderBuffer;
        mutable RenderTargetColor mRenderTarget;
        mutable sead::Viewport mViewport;
        mutable u32 mWidth;
        mutable u32 mHeight;
        mutable bool mIsAllocated;
        mutable bool mIsWithoutContext;
    };
    static_assert(sizeof(TexBuf) == 0x390);

    class ReduceBuffer {
    public:
        ~ReduceBuffer();

        void free() const;

        sead::Buffer<TexBuf> mBuffers;
        u32 mWidth;
        u32 mHeight;
    };
    static_assert(sizeof(ReduceBuffer) == 0x18);

    struct Context {
        s32 mIndex;
        cull::ViewFrustumCulling mViewFrustum;
        sead::Buffer<ReduceBuffer> mDifBuffers;
        sead::Buffer<ReduceBuffer> mRefBuffers;
        ReduceBuffer mSrcBuffer;
        ReduceBuffer mDifExpandBuffer;
        ReduceBuffer mRefExpandBuffer;
        TexBuf mReprojectionBuffer;
        u32 mReprojectionWidth;
        u32 mReprojectionHeight;
        RenderBuffer mRenderBuffer;
        sead::Vector2f mTanFovyHalf;
        sead::Vector2f mProjOffset;
    };
    static_assert(sizeof(Context) == 0x6b8);

    SSII();
    virtual ~SSII();

    void free();
    void initialize(s32 viewNum, sead::Heap* pHeap);
    void calcView(s32 view, const cull::ViewFrustumCulling& rViewFrustum);
    void calcGPU();
    void calcViewGPU(s32 view);
    void draw(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
              const sead::Viewport& rViewport, const TextureData& rLight,
              const TextureData& rAlbedo, const TextureData& rNormal,
              const TextureData& rDepth) const;
    void draw(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
              const RenderBuffer& rRefRenderBuffer, const sead::Viewport& rViewport,
              const TextureData& rLight, const TextureData& rAlbedo, const TextureData& rNormal,
              const TextureData& rDepth) const;
    void draw(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
              const sead::Viewport& rViewport, const TextureData& rNormal,
              const TextureData& rDepth) const;
    void draw(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
              const RenderBuffer& rRefRenderBuffer, const sead::Viewport& rViewport,
              const TextureData& rNormal, const TextureData& rDepth) const;
    void drawAlbedoMode(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
                        const sead::Viewport& rViewport, const TextureData& rLight,
                        const TextureData& rAlbedo, const TextureData& rNormal,
                        const TextureData& rDepth, bool refOnly) const;
    void setReprojectionBuffer(DrawContext* pDrawContext, s32 view,
                               const TextureData& rTextureData) const;
    void drawDebug(DrawContext* pDrawContext, s32 view) const;
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    void applyQualitySetting_();
    void draw_(DrawContext* pDrawContext, s32 view, const RenderBuffer* pRenderBuffer,
               const RenderBuffer* pRefRenderBuffer, const sead::Viewport& rViewport,
               const TextureData* pLight, const TextureData* pLightAlbedo,
               const TextureData* pAlbedo, const TextureData* pNormal,
               const TextureData* pDepth) const;
    bool allocReprojectionBuffer_(DrawContext* pDrawContext, s32 view) const;
    void drawReduce_(DrawContext* pDrawContext, s32 view, s32 level,
                     const RenderBuffer& rRenderBuffer, const TextureSampler& rSampler,
                     bool isDepth) const;
    void drawReduceWithPreRender_(DrawContext* pDrawContext, s32 view, s32 level,
                                  const RenderBuffer& rRenderBuffer,
                                  const TextureSampler& rLight,
                                  const TextureSampler& rAlbedo) const;
    void drawSSII_(DrawContext* pDrawContext, s32 view, const RedBufRenderSetting& rSetting,
                   const RenderBuffer& rRenderBuffer, const sead::Viewport& rViewport,
                   const ReduceBuffer& rSrc, const ReduceBuffer& rDst, bool useLight) const;
    void drawRef_(DrawContext* pDrawContext, s32 view, f32 intensity, s32 level, s32 mode,
                  const RenderBuffer& rRenderBuffer, const sead::Viewport& rViewport,
                  const ReduceBuffer& rLight, const ReduceBuffer& rSrc,
                  const ReduceBuffer& rDst) const;
    void drawAntiHowling_(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
                          const TextureSampler& rSrc0, const TextureSampler& rSrc1) const;
    void drawExpand_(DrawContext* pDrawContext, s32 view, const RenderBuffer& rRenderBuffer,
                     const TextureSampler& rSrcLight, const TextureSampler& rSrcNormal,
                     const TextureSampler& rSrcDepth, const TextureSampler& rDstNormal,
                     const TextureSampler& rDstDepth, const TextureSampler* pDstAlbedo,
                     const TextureSampler* pSrcLight1, s32 level, bool isFinal) const;

    const Context& getContext_(s32 view) const { return mContexts[view]; }

    sead::BitFlag32 mFlags;
    mutable sead::Buffer<Context> mContexts;
    sead::GraphicsContext mGraphicsContext[4];
    s32 mReduceLevel[3];
    s32 mRefReduceLevel[3];
    s32 mDifLevelNum;
    s32 mRefLevelNum;
    s32 mRedBufLevel;
    s32 mRefStartLevel;
    f32 mDifIntensity;
    f32 mRefIntensity;
    f32 mDifSubIntensity;
    f32 mDifWeightStep;
    f32 mRefWeightStep;
    f32 mSteep;
    f32 mAntiHowlingThreshold;
    f32 mRefPow;
    f32 mRefInflate;
    sead::Buffer<RedBufRenderSetting> mRedBufSettings;
    s32 mRedBufQuality;
    s32 mSampleQuality;
    s32 mDifQuality;
    s32 mRefQuality;
    s32 mRefStartOffset;
    s32 mResolutionLevel;
    mutable s32 mCurrentResolutionLevel;
    utl::DebugTexturePage mDebugPage;
    utl::DebugTexturePage mDifDebugPage;
    utl::DebugTexturePage mRefDebugPage;
};
static_assert(sizeof(SSII) == 0x930);

}  // namespace agl::lght
