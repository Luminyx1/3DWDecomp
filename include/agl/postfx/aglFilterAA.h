#pragma once

#include <container/seadBuffer.h>
#include <container/seadSafeArray.h>
#include <gfx/seadColor.h>
#include <gfx/seadGraphicsContext.h>
#include <hostio/seadHostIONode.h>
#include <math/seadMatrix.h>
#include <prim/seadBitFlag.h>
#include "common/aglGPUMemBlock.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
#include "utility/aglDebugTexturePage.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterIO.h"
#include "utility/aglParameterObj.h"

namespace sead {
class Heap;
class Viewport;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
class ShaderProgram;
}  // namespace agl

namespace agl::pfx {

class FilterAA : public utl::IParameterIO, public sead::hostio::Node {
public:
    enum Type {
        cType_FXAA = 0,
        cType_ReduceAA = 1,
    };

    struct InitializeArg {
        s32 mContextNum;
        u32 mReprojectionBufferSize;
    };

    struct Context {
        bool mIsValid;
        RenderBuffer mRenderBuffer;
        RenderTargetColor mRenderTarget;
        TextureSampler mSourceSampler;
        TextureSampler mLumaSampler;
        TextureSampler mHistorySampler;
        TextureData mHistoryTexture;
        GPUMemBlock<u8> mHistoryBuffer;
        f32 mNear;
        f32 mFar;
        sead::Matrix44f mProjMtx;
        sead::Matrix34f mViewMtx;
        sead::Matrix44f mPrevProjMtx;
        sead::Matrix34f mPrevViewMtx;
    };
    static_assert(sizeof(Context) == 0x880);

    static const sead::Color4f cDefaultLumaCoeff;

    FilterAA();
    ~FilterAA() override;

    void initialize(const InitializeArg& rArg, sead::Heap* pHeap);
    void setDrawInfo(u32 context, const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                     f32 near, f32 far);
    void draw(DrawContext* pDrawContext, u32 context, const RenderBuffer& rDst,
              const RenderBuffer& rSrc, const sead::Viewport& rViewport) const;
    void draw(DrawContext* pDrawContext, u32 context, const RenderBuffer& rRenderBuffer,
              const sead::Viewport& rViewport, const TextureSampler* pSource,
              const TextureSampler* pDepth, bool normalizedDepth) const;
    void draw(DrawContext* pDrawContext, u32 context, const RenderBuffer& rRenderBuffer,
              const sead::Viewport& rViewport, const TextureSampler* pSource,
              const TextureSampler* pDepth, bool normalizedDepth,
              const TextureSampler* pHistory) const;
    void FXAA(DrawContext* pDrawContext, u32 context, const RenderBuffer& rRenderBuffer,
              const sead::Viewport& rViewport, const TextureSampler* pSource,
              const TextureSampler* pDepth, bool normalizedDepth,
              const TextureSampler* pHistory) const;
    void ReduceAA(DrawContext* pDrawContext, u32 context, const RenderBuffer& rRenderBuffer,
                  const sead::Viewport& rViewport, const TextureSampler* pSource) const;
    void reprojection(DrawContext* pDrawContext, u32 context, const RenderBuffer& rRenderBuffer,
                      const sead::Viewport& rViewport, const TextureSampler* pSource,
                      const TextureSampler* pDepth, bool normalizedDepth,
                      const TextureSampler* pHistory) const;
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    using ProgramTable = sead::SafeArray<sead::SafeArray<const ShaderProgram*, 3>, 7>;

    Context& getContext_(u32 context) const
    {
        return const_cast<Context&>(mContexts[context]);
    }

    void resetParameters_()
    {
        *mAlphaOut = 1.0f;
        *mEdgeThresholdScale = 1.0f;
        *mDetectEdgeQuality = 0;
        *mFetchQuality = 1;
        *mLumaCoeff = cDefaultLumaCoeff;
        *mSubpixParam = 0.15f;
        *mMaxSpan = 3.0f;
        *mSpanMultiply = 1.0f;
        *mSpanMinimum = 0.04f;
        *mReprojection = true;
        *mReprojectionMoveLimit = 8.0f;
        mDebugFlag.setDirect(0);
    }

    sead::GraphicsContext mGraphicsContext;
    const ShaderProgram* mLumaPrograms[2] = {};
    sead::SafeArray<ProgramTable, 2> mFxaaPrograms = {};
    sead::SafeArray<sead::SafeArray<ProgramTable, 2>, 2> mReprojectionPrograms = {};
    utl::ParameterObj mParamObj;
    utl::Parameter<s32> mType{cType_FXAA, "antialias_type", "タイプ", &mParamObj};
    utl::Parameter<bool> mEnable{true, "enable", "有効", &mParamObj};
    utl::Parameter<f32> mAlphaOut{1.0f, "fxaa_alpha_out", "αアウト",
                                  "Min = 0, Max = 1, Mode = MinMaxLock", &mParamObj};
    utl::Parameter<s32> mDetectEdgeQuality{0, "fxaa_detect_edge_qa", "エッジ抽出クォリティ",
                                           &mParamObj};
    utl::Parameter<s32> mFetchQuality{1, "fxaa_fetcht_qa", "テクスチャフェッチのクォリティ",
                                      &mParamObj};
    utl::Parameter<f32> mEdgeThresholdScale{1.0f, "fxaa_edge_threshold_scale", "ダイナミックレンジ",
                                            "Min = 0.1, Max = 8", &mParamObj};
    utl::Parameter<sead::Color4f> mLumaCoeff{cDefaultLumaCoeff, "fxaa_luma_coeff",
                                             "輝度抽出バランス", &mParamObj};
    utl::Parameter<f32> mSubpixParam{0.15f, "subpix_param", "サブピクセル処理時のオフセット",
                                     "Min = 0.0, Max = 0.5, Mode = MinMaxLock", &mParamObj};
    utl::Parameter<f32> mMaxSpan{3.0f, "max_span", "ぼかし最大半径", "Min = 0.0, Max = 5.0",
                                 &mParamObj};
    utl::Parameter<f32> mSpanMultiply{1.0f, "span_multiply", "ぼかし時の倍率",
                                      "Min = 0.1, Max = 5.0", &mParamObj};
    utl::Parameter<f32> mSpanMinimum{0.04f, "span_minimum", "ぼかし時の最小半径（0.1 px 単位）",
                                     "Min = 0.0, Max = 0.1", &mParamObj};
    utl::Parameter<bool> mReprojection{true, "fxaa_reprojection", "リプロジェクション",
                                       &mParamObj};
    utl::Parameter<f32> mReprojectionMoveLimit{8.0f, "fxaa_reprojection_move_limit",
                                               "リプロジェクション時の最大追跡ピクセル数",
                                               "Min = 1, Max = 16", &mParamObj};
    s32 mDebugNo = 0;
    sead::BitFlag32 mDebugFlag = 0;
    bool mIsSRGB = false;
    sead::Buffer<Context> mContexts;
    utl::DebugTexturePage mDebugTexturePage;
};
static_assert(sizeof(FilterAA) == 0xad8);

}  // namespace agl::pfx
