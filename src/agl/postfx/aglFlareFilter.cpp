#include "postfx/aglFlareFilter.h"

#include <prim/seadSafeString.h>
#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "detail/aglRootNode.h"
#include "detail/aglShaderHolder.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglDynamicTextureAllocator.h"

namespace agl::pfx {

namespace {

constexpr s32 cGhostColorNum = 8;

const char* sGhostColorName[cGhostColorNum] = {
    "ghost_color_0", "ghost_color_1", "ghost_color_2", "ghost_color_3",
    "ghost_color_4", "ghost_color_5", "ghost_color_6", "ghost_color_7",
};

}  // namespace

FlareFilterParameter::FlareFilterParameter() = default;

void FlareFilterParameter::initialize(utl::IParameterObj* pObj, sead::Heap* pHeap)
{
    mColor.init(sead::Color4f(1.0f, 1.0f, 1.0f, 0.9f), "color", "全体の色", pObj);
    mIsBlurAfterFlare.init(false, "blur_after_flare", "ブラーを最後にする", pObj);
    mIsEnableThreshold.init(true, "is_enable_threshold", "高輝度バッファを内部生成する", pObj);
    mThreshold.init(0.5f, "threshold", "高輝度バッファ閾値（輝度）", pObj);
    mGhostNum.init(6, "ghost_num", "ゴースト数", pObj);
    mGhostDispersal.init(0.33f, "ghost_dispersal", "ゴースト分離幅", pObj);
    mGhostColors.tryAllocBuffer(cGhostColorNum, pHeap);
    const char** pName = sGhostColorName;
    for (auto& rColor : mGhostColors)
    {
        rColor.init(sead::Color4f::cWhite, *pName, *pName, pObj);
        pName++;
    }
    mIsEnableHalo.init(true, "is_enable_halo", "ハロ有効", pObj);
    mHaloWidth.init(0.56f, "halo_width", "ハロ位置", pObj);
    mHaloColor.init(sead::Color4f::cWhite, "halo_color", "ハロ色", pObj);
    mIsEnableChromaDistortion.init(true, "is_enable_chroma_distortion", "色収差有効", pObj);
    mChromaDistortion.init(sead::Vector3f(-0.01f, 0.01f, 0.03f), "chroma_distortion",
                           "色収差（ XYZ が RGB に対応）", pObj);
    mChromaDistortionScale.init(-2.8f, "chroma_distortion_scale", "色収差スケール", pObj);
}

void FlareFilterParameter::genMessageFlareFilterParameter(sead::hostio::Context* pContext)
{
    mColor.genMessageParameter(pContext, mColor.getMeta());
    mIsBlurAfterFlare.genMessageParameter(pContext, mIsBlurAfterFlare.getMeta());
    mIsEnableThreshold.genMessageParameter(pContext, mIsEnableThreshold.getMeta());
    mThreshold.genMessageParameter(pContext, mThreshold.getMeta());
    mGhostNum.genMessageParameter(
        pContext,
        sead::FormatFixedSafeString<256>("Min = 0, Max = %d, Mode = MinMaxLock", cGhostColorNum));
    mGhostDispersal.genMessageParameter(pContext, "Min = -0.5, Max = 0.5");
    mIsEnableHalo.genMessageParameter(pContext, mIsEnableHalo.getMeta());
    mHaloWidth.genMessageParameter(pContext, "Min = -1, Max = 1");
    mHaloColor.genMessageParameter(pContext, mHaloColor.getMeta());
    mIsEnableChromaDistortion.genMessageParameter(pContext, mIsEnableChromaDistortion.getMeta());
    mChromaDistortion.genMessageParameter(pContext, "Min = -0.05, Max = 0.05");
    mChromaDistortionScale.genMessageParameter(pContext, "Min = -2, Max = 2");
    for (auto& rColor : mGhostColors)
    {
        rColor.genMessageParameter(pContext, rColor.getMeta());
    }
}

void FlareFilterParameter::listenPropertyEventFlareFilterParameter(
    sead::hostio::Reflexible* pReflexible, const sead::hostio::PropertyEvent* pEvent)
{
}

void FlareFilter::Tex::alloc(DrawContext* pDrawContext, TextureFormat format, u32 width,
                             u32 height, const char* pName, bool withoutContext) const
{
    auto* pAllocator = utl::DynamicTextureAllocator::instance();
    const TextureData* pTextureData;
    if (withoutContext)
    {
        pTextureData = pAllocator->allocWithoutContext(pDrawContext, pName, format, width, height,
                                                       1, nullptr,
                                                       utl::DynamicTextureAllocator::cAllocateType_0,
                                                       true, false);
    }
    else
    {
        pTextureData =
            pAllocator->alloc(pDrawContext, pName, format, width, height, 1, nullptr,
                              utl::DynamicTextureAllocator::cAllocateType_0, true, false);
    }
    refer(pDrawContext, pTextureData);
    mIsAllocated = true;
    mIsWithoutContext = withoutContext;
}

void FlareFilter::Tex::release() const
{
    if (mIsAllocated)
    {
        utl::DynamicTextureAllocator::instance()->free(mpTextureData);
    }
    mpTextureData = nullptr;
    mIsAllocated = false;
    mIsWithoutContext = false;
}

FlareFilter::FlareFilter() : IParameterIO("aglflr", 1)
{
    agl::detail::RootNode::setNodeMeta(this, "Icon=EFFECT");
}

FlareFilter::~FlareFilter()
{
    for (auto& rContext : mUnusedContexts)
    {
        rContext.mUnused.release();
    }
}

void FlareFilter::initialize(s32 contextNum, sead::Heap* pHeap)
{
    initializeContextParameterBuffer(contextNum, false, pHeap);
    mEnable.init(true, "enable", "有効", &mParameterObjs[0]);
    addObj(&mParameterObjs[0], "flare_filter");
    copyParameterToAllContext(0);
    mGraphicsContextAdd.setDepthEnable(false, false);
    mGraphicsContextAdd.setBlendEquation(0, 1);
    mGraphicsContextAdd.setBlendFactor(0, 2, 2);
    mGraphicsContextAdd.setBlendEnable(true);
    mGraphicsContext.setDepthEnable(false, false);
    mGraphicsContext.setBlendEnable(false);
    mDebugTexturePage.setUp(contextNum, "FlareFilter", pHeap);
}

void FlareFilter::initializeContext(Context* pContext, sead::Heap* pHeap) {}

void FlareFilter::calc() {}

void FlareFilter::calcView(s32 context, const cull::ViewFrustumCulling& rCulling)
{
    if (!*mEnable || !isEnableContext(context))
    {
        return;
    }
    getContext_(context).mCulling = rCulling;
}

void FlareFilter::calcGPU() const {}

void FlareFilter::calcViewGPU(s32 context) const {}

void FlareFilter::draw(DrawContext* pDrawContext, s32 context, const RenderBuffer& rRenderBuffer,
                       const sead::Viewport& rViewport, const TextureData& rTexture,
                       s32 reduceNum, s32 blurNum) const
{
    if (!*mEnable || !isEnableContext(context))
    {
        return;
    }
    Context& rContext = getContext_(context);
    rContext.mTarget.refer(pDrawContext,
                           reinterpret_cast<const TextureData*>(
                               rRenderBuffer.getRenderTargetColor()));
    drawToFlareBuffer(pDrawContext, context, rTexture, reduceNum, blurNum);
    mGraphicsContextAdd.apply(pDrawContext);
    drawCopy_(pDrawContext, rContext.mTarget, rContext.mFlare, false, false, false, 0.5f, 1.0f,
              sead::Color4f::cWhite);
    releaseFlareBuffer(context);
    rContext.mTarget.release();
}

void FlareFilter::drawToFlareBuffer(DrawContext* pDrawContext, s32 context,
                                    const TextureData& rTexture, s32 reduceNum,
                                    s32 blurNum) const
{
    if (!*mEnable || !isEnableContext(context))
    {
        return;
    }

    const FlareFilterParameter& rParam = getParameter(context);
    Context& rContext = getContext_(context);
    const Tex& rSource = rContext.mSource;
    rSource.refer(pDrawContext, &rTexture);
    mGraphicsContext.apply(pDrawContext);

    const Tex* pCurrent = &rSource;
    for (s32 i = 0; i < reduceNum; i++)
    {
        const Tex& rIn = i == 0 ? rSource : (i & 1) == 0 ? rContext.mWork0 : rContext.mWork1;
        const Tex& rOut = (i & 1) == 0 ? rContext.mWork1 : rContext.mWork0;
        rOut.alloc(pDrawContext, TextureFormat::cTextureFormat_R11_G11_B10_float, rIn.mWidth / 2,
                   rIn.mHeight / 2, "reduce_temp", false);
        drawCopy_(pDrawContext, rOut, rIn, true, i == 0,
                  i == reduceNum - 1 && *rParam.mIsEnableThreshold, *rParam.mThreshold, 1.0f,
                  sead::Color4f::cWhite);
        if (i != 0)
        {
            rIn.release();
        }
        pCurrent = &rOut;
    }

    if (!*rParam.mIsBlurAfterFlare)
    {
        mGraphicsContext.apply(pDrawContext);
        for (s32 i = 0; i < blurNum; i++)
        {
            const Tex* pIn;
            const Tex* pTemp;
            if (!rContext.mWork0.mIsAllocated && !rContext.mWork1.mIsAllocated)
            {
                rContext.mWork1.alloc(pDrawContext, TextureFormat::cTextureFormat_R11_G11_B10_float,
                                      rSource.mWidth, rSource.mHeight, "blur_temp", false);
                pIn = &rSource;
                pCurrent = &rContext.mWork1;
                pTemp = &rContext.mWork0;
            }
            else
            {
                bool isWork0 = rContext.mWork0.mIsAllocated;
                pIn = isWork0 ? &rContext.mWork0 : &rContext.mWork1;
                pTemp = isWork0 ? &rContext.mWork1 : &rContext.mWork0;
                pCurrent = pIn;
            }
            pTemp->alloc(pDrawContext, TextureFormat::cTextureFormat_R11_G11_B10_float, pIn->mWidth,
                         pIn->mHeight, "blur_temp", false);
            drawBlur_(pDrawContext, *pTemp, *pIn, 2, false, false, 1.0f, sead::Color4f::cWhite);
            drawBlur_(pDrawContext, *pCurrent, *pTemp, 2, true, false, 1.0f,
                      sead::Color4f::cWhite);
            pTemp->release();
        }
    }

    const Tex& rFlare = rContext.mFlare;
    rFlare.alloc(pDrawContext, TextureFormat::cTextureFormat_R11_G11_B10_float, pCurrent->mWidth,
                 pCurrent->mHeight, "flare_buffer", false);
    rFlare.mRenderBuffer.bind(pDrawContext);
    rFlare.mViewport.apply(pDrawContext, rFlare.mRenderBuffer);
    mGraphicsContext.apply(pDrawContext);

    {
        const ShaderProgram* pProgram =
            agl::detail::ShaderHolder::instance()->getShaderProgramUnsafe(
                agl::detail::ShaderHolder::cFlareFilterFlare);
        s32 variation = pProgram->getVariationMacroStride(0) * *rParam.mGhostNum +
                        pProgram->getVariationMacroStride(1) * *rParam.mIsEnableHalo +
                        pProgram->getVariationMacroStride(2) * *rParam.mIsEnableChromaDistortion;
        pProgram = pProgram->getVariation(variation);
        pProgram->activate(pDrawContext, true);

        sead::Color4f color = *rParam.mColor;
        sead::Color4f haloColor = *rParam.mHaloColor;
        sead::Vector3f chroma = *rParam.mChromaDistortion;
        color *= color.a;
        haloColor *= haloColor.a;
        chroma.x = *rParam.mChromaDistortionScale * chroma.x;
        chroma.y = *rParam.mChromaDistortionScale * chroma.y;
        chroma.z = *rParam.mChromaDistortionScale * chroma.z;

        pProgram->getUniformLocation(0).setUniform(pDrawContext, *rParam.mGhostDispersal);
        pProgram->getUniformLocation(2).setUniform(pDrawContext, *rParam.mHaloWidth);
        pProgram->getUniformLocation(3).setUniform(pDrawContext, 3, &haloColor);
        pProgram->getUniformLocation(4).setUniform(pDrawContext, 3, &chroma);
        pProgram->getUniformLocation(5).setUniform(pDrawContext, 3, &color);
        pCurrent->mSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);

        u32 index = 0;
        for (const auto& rGhostColor : rParam.mGhostColors)
        {
            sead::Color4f ghostColor = *rGhostColor;
            ghostColor *= ghostColor.a;
            pProgram->getUniformLocation(1).setUniform(pDrawContext, index, &ghostColor);
            index += 4;
        }
    }

    detail::drawQuadTriangle(pDrawContext);
    rFlare.mRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);

    if (*rParam.mIsBlurAfterFlare)
    {
        mGraphicsContext.apply(pDrawContext);
        for (s32 i = 0; i < blurNum; i++)
        {
            const Tex& rTemp = rContext.mWork0.mIsAllocated ? rContext.mWork1 : rContext.mWork0;
            rTemp.alloc(pDrawContext, TextureFormat::cTextureFormat_R11_G11_B10_float, rFlare.mWidth,
                        rFlare.mHeight, "blur_temp", false);
            drawBlur_(pDrawContext, rTemp, rFlare, 2, false, false, 1.0f, sead::Color4f::cWhite);
            drawBlur_(pDrawContext, rFlare, rTemp, 2, true, false, 1.0f, sead::Color4f::cWhite);
            rTemp.release();
        }
    }

    pCurrent->release();
    rSource.release();
}

void FlareFilter::drawCopy_(DrawContext* pDrawContext, const Tex& rDst, const Tex& rSrc,
                            bool isReduce, bool isFirst, bool isThreshold, f32 threshold,
                            f32 scale, const sead::Color4f& rColor) const
{
    rDst.mRenderBuffer.bind(pDrawContext);
    rDst.mViewport.apply(pDrawContext, rDst.mRenderBuffer);

    const ShaderProgram* pProgram = agl::detail::ShaderHolder::instance()->getShaderProgram(
        agl::detail::ShaderHolder::cFlareFilterCopy);
    s32 variation = pProgram->getVariationMacroStride(0) * (isReduce ? 3 : 0) +
                    pProgram->getVariationMacroStride(2) * !(rColor == sead::Color4f::cWhite) +
                    pProgram->getVariationMacroStride(3) * isFirst +
                    pProgram->getVariationMacroStride(4) * isThreshold;
    pProgram = pProgram->getVariation(variation);
    pProgram->activate(pDrawContext, true);

    {
        sead::Vector2f texelSize(1.0f / rSrc.mWidth, 1.0f / rSrc.mHeight);
        pProgram->getUniformLocation(0).setUniform(pDrawContext, 2, &texelSize);
    }
    pProgram->getUniformLocation(1).setUniform(pDrawContext, scale);
    pProgram->getUniformLocation(2).setUniform(pDrawContext, threshold);
    pProgram->getUniformLocation(3).setUniform(pDrawContext, 4, &rColor);
    rSrc.mSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);

    detail::drawQuadTriangle(pDrawContext);
    rDst.mRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);
}

void FlareFilter::releaseFlareBuffer(s32 context) const
{
    getContext_(context).mFlare.release();
}

void FlareFilter::drawBlur_(DrawContext* pDrawContext, const Tex& rDst, const Tex& rSrc,
                            s32 direction, bool isSecond, bool b2, f32 scale,
                            const sead::Color4f& rColor) const
{
    rDst.mRenderBuffer.bind(pDrawContext);
    rDst.mViewport.apply(pDrawContext, rDst.mRenderBuffer);

    const ShaderProgram* pProgram = agl::detail::ShaderHolder::instance()->getShaderProgram(
        agl::detail::ShaderHolder::cFlareFilterCopy);
    s32 variation = pProgram->getVariationMacroStride(0) * (direction == 1 ? 1 : 2) +
                    pProgram->getVariationMacroStride(1) * isSecond +
                    pProgram->getVariationMacroStride(2) * !(rColor == sead::Color4f::cWhite) +
                    pProgram->getVariationMacroStride(3) * b2;
    pProgram = pProgram->getVariation(variation);
    pProgram->activate(pDrawContext, true);

    sead::Vector2f texelSize(1.0f / rSrc.mWidth, 1.0f / rSrc.mHeight);
    pProgram->getUniformLocation(0).setUniform(pDrawContext, 2, &texelSize);
    pProgram->getUniformLocation(1).setUniform(pDrawContext, scale);
    pProgram->getUniformLocation(3).setUniform(pDrawContext, 4, &rColor);
    rSrc.mSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);

    detail::drawQuadTriangle(pDrawContext);
    rDst.mRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);
}

void FlareFilter::drawDebug(DrawContext* pDrawContext, s32 context) const {}

void FlareFilter::genMessage(sead::hostio::Context* pContext)
{
    genMessageIO(pContext, 0xf);
    mDebugTexturePage.genMessagePage(pContext, this);
    mEnable.genMessageParameter(pContext, mEnable.getMeta());
    genMessageFlareFilterParameter(pContext);
}

void FlareFilter::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    listenPropertyEventIO(this, pEvent);
    copyParameterToAllContext(0);
}

}  // namespace agl::pfx
