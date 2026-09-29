#include "postfx/aglFilterAA.h"

#include <gfx/seadViewport.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "detail/aglRootNode.h"
#include "detail/aglShaderHolder.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglDynamicTextureAllocator.h"
#include "utility/aglImageFilter2D.h"

namespace agl::pfx {

const sead::Color4f FilterAA::cDefaultLumaCoeff(0.299f, 0.587f, 0.114f, 0.0f);

FilterAA::FilterAA() : IParameterIO("aglfila", 0)
{
    mGraphicsContext.setDepthEnable(false, false);
    mGraphicsContext.setBlendEnable(false);
    mGraphicsContext.setAlphaTestEnable(false);
    mGraphicsContext.setColorMask(true, true, true, true);
    mGraphicsContext.setCullingMode(0);
    addObj(&mParamObj, "filter_aa");
    agl::detail::RootNode::setNodeMeta(this, "Icon=CIRCLE_BLACK");
    resetParameters_();
}

FilterAA::~FilterAA()
{
    for (s32 i = 0; i < mContexts.size(); i++)
    {
        GPUMemBlock<u8>& buffer = mContexts[i].mHistoryBuffer;
        if (buffer.isAllocated())
        {
            buffer.freeBuffer();
        }
    }
    mContexts.freeBuffer();
    mDebugTexturePage.cleanUp();
}

void FilterAA::initialize(const InitializeArg& rArg, sead::Heap* pHeap)
{
    mContexts.tryAllocBuffer(rArg.mContextNum, pHeap);
    for (s32 i = 0; i < mContexts.size(); i++)
    {
        mContexts[i].mIsValid = true;
        if (rArg.mReprojectionBufferSize != 0)
        {
            GPUMemBlock<u8>& buffer = mContexts[i].mHistoryBuffer;
            buffer.allocBuffer_(rArg.mReprojectionBufferSize, pHeap, 0x2000,
                                MemoryAttribute::Default);
            GPUMemAddr<u8> addr(buffer, 0);
        }
        mContexts[i].mProjMtx = sead::Matrix44f(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        mContexts[i].mViewMtx = sead::Matrix34f(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                                0.0f, 0.0f, 1.0f, 0.0f);
        mContexts[i].mPrevProjMtx = sead::Matrix44f(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                                    0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                                    0.0f, 1.0f);
        mContexts[i].mPrevViewMtx = sead::Matrix34f(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                                    0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    }

    {
        const char* macro = "FETCH_TYPE";
        const char* value = "0";
        const ShaderProgram* program = agl::detail::ShaderHolder::instance()->getShaderProgram(
            agl::detail::ShaderHolder::cFxaaLuma);
        mLumaPrograms[0] = program->searchVariationShaderProgram(1, &macro, &value);
        value = "5";
        mLumaPrograms[1] = program->searchVariationShaderProgram(1, &macro, &value);
    }

    {
        const char* macros[] = {"ENABLE_ALPHA_OUT", "FETCH_TYPE", "DEBUG_NO"};
        static const char* const cValues[] = {"0", "1", "2", "3", "4", "5", "6"};
        const ShaderProgram* program = agl::detail::ShaderHolder::instance()->getShaderProgram(
            agl::detail::ShaderHolder::cFxaa);
        for (s32 alpha = 0; alpha < 2; alpha++)
        {
            for (s32 fetch = 0; fetch < 7; fetch++)
            {
                for (s32 debug = 0; debug < 3; debug++)
                {
                    const char* values[] = {cValues[alpha], cValues[fetch], cValues[debug]};
                    mFxaaPrograms[alpha][fetch][debug] =
                        program->searchVariationShaderProgram(3, macros, values);
                }
            }
        }
    }

    {
        const char* macros[] = {"USE_NORMALIZED_LINEAR_DEPTH", "ENABLE_ALPHA_OUT", "FETCH_TYPE",
                                "DEBUG_NO"};
        static const char* const cValues[] = {"0", "1", "2", "3", "4", "5", "6"};
        const ShaderProgram* program = agl::detail::ShaderHolder::instance()->getShaderProgram(
            agl::detail::ShaderHolder::cFxaaReprojection);
        for (s32 depth = 0; depth < 2; depth++)
        {
            for (s32 alpha = 0; alpha < 2; alpha++)
            {
                for (s32 fetch = 0; fetch < 7; fetch++)
                {
                    for (s32 debug = 0; debug < 3; debug++)
                    {
                        const char* values[] = {cValues[depth], cValues[alpha], cValues[fetch],
                                                cValues[debug]};
                        mReprojectionPrograms[depth][alpha][fetch][debug] =
                            program->searchVariationShaderProgram(4, macros, values);
                    }
                }
            }
        }
    }

    mDebugTexturePage.setUp(rArg.mContextNum, "FilterAA", pHeap);
}

void FilterAA::setDrawInfo(u32 context, const sead::Matrix44f& rProjMtx,
                           const sead::Matrix34f& rViewMtx, f32 near, f32 far)
{
    Context& ctx = mContexts[context];
    ctx.mPrevProjMtx = ctx.mProjMtx;
    ctx.mPrevViewMtx = ctx.mViewMtx;
    ctx.mProjMtx = rProjMtx;
    ctx.mViewMtx = rViewMtx;
    ctx.mNear = near;
    ctx.mFar = far;
}

void FilterAA::draw(DrawContext* pDrawContext, u32 context, const RenderBuffer& rDst,
                    const RenderBuffer& rSrc, const sead::Viewport& rViewport) const
{
    TextureSampler source(*rSrc.getRenderTargetColor());
    TextureSampler depth(*rSrc.getRenderTargetDepth());
    draw(pDrawContext, context, rDst, rViewport, &source, &depth, false);
}

void FilterAA::draw(DrawContext* pDrawContext, u32 context, const RenderBuffer& rRenderBuffer,
                    const sead::Viewport& rViewport, const TextureSampler* pSource,
                    const TextureSampler* pDepth, bool normalizedDepth) const
{
    Context& ctx = getContext_(context);
    sead::Vector2f size;
    rViewport.getOnFrameBufferSize(&size, rRenderBuffer);
    if (ctx.mHistoryTexture.getWidth(0) != size.x || ctx.mHistoryTexture.getHeight(0) != size.y)
    {
        ctx.mHistoryTexture.initialize_(
            TextureType::cTextureType_2D,
            TextureFormat(rRenderBuffer.getRenderTargetColor()->getTextureFormat()), u32(size.x),
            u32(size.y), 1, 1, TextureAttribute(0), MultiSampleType(0), true);
        ctx.mHistoryTexture.setImagePtr(GPUMemVoidAddr(ctx.mHistoryBuffer, 0));
        ctx.mHistorySampler.applyTextureData(ctx.mHistoryTexture);
    }

    draw(pDrawContext, context, rRenderBuffer, rViewport, pSource, pDepth, false,
         &ctx.mHistorySampler);

    ctx.mRenderTarget.applyTextureData(ctx.mHistoryTexture);
    ctx.mRenderBuffer.setRenderTargetColorNullAll();
    ctx.mRenderBuffer.setRenderTargetDepth(nullptr);
    ctx.mRenderBuffer.setRenderTargetColor(&ctx.mRenderTarget);
    ctx.mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, size.x, size.y));
    ctx.mRenderBuffer.setVirtualSize(size);
    ctx.mRenderBuffer.bind(pDrawContext);
    ctx.mRenderTarget.invalidateGPUCache(pDrawContext);

    ctx.mSourceSampler.applyTextureData(
        *reinterpret_cast<const TextureData*>(rRenderBuffer.getRenderTargetColor()));
    sead::Viewport viewport(ctx.mRenderBuffer);
    viewport.apply(pDrawContext, ctx.mRenderBuffer);
    sead::Vector2f offset(-rViewport.getMin().x, -rViewport.getMin().y);
    utl::ImageFilter2D::drawTexture(pDrawContext, ctx.mSourceSampler, viewport,
                                    sead::Vector2f::ones, offset);
    ctx.mRenderTarget.invalidateGPUCache(pDrawContext);
}

void FilterAA::draw(DrawContext* pDrawContext, u32 context, const RenderBuffer& rRenderBuffer,
                    const sead::Viewport& rViewport, const TextureSampler* pSource,
                    const TextureSampler* pDepth, bool normalizedDepth,
                    const TextureSampler* pHistory) const
{
    mGraphicsContext.apply(pDrawContext);
    if (*mEnable && getContext_(context).mIsValid && *mAlphaOut > 0.0f)
    {
        switch (*mType)
        {
        case cType_FXAA:
            FXAA(pDrawContext, context, rRenderBuffer, rViewport, pSource, pDepth,
                 normalizedDepth, pHistory);
            return;
        case cType_ReduceAA:
            ReduceAA(pDrawContext, context, rRenderBuffer, rViewport, pSource);
            return;
        default:
            return;
        }
    }
    utl::ImageFilter2D::drawTexture(pDrawContext, *pSource, rViewport, sead::Vector2f::ones,
                                    sead::Vector2f::zero);
}

void FilterAA::FXAA(DrawContext* pDrawContext, u32 context, const RenderBuffer& rRenderBuffer,
                    const sead::Viewport& rViewport, const TextureSampler* pSource,
                    const TextureSampler* pDepth, bool normalizedDepth,
                    const TextureSampler* pHistory) const
{
    if (!(*mAlphaOut <= 0.0f))
    {
        Context& ctx = getContext_(context);
        bool useReprojection = pDepth && pHistory && *mReprojection;
        pSource->getTextureData().getHeight(0);

        sead::Vector4f rcpFrame;
        sead::Vector4f rcpFrameOpt;
        sead::Vector4f lumaCoeff;
        sead::Vector4f threshold;
        sead::Vector4f span;
        sead::Vector2f size;
        rViewport.getOnFrameBufferSize(&size, rRenderBuffer);
        const TextureData* pLuma = utl::DynamicTextureAllocator::instance()->alloc(
            pDrawContext, "luma", TextureFormat::cTextureFormat_R8_uNorm, u32(size.x),
            u32(size.y), 1, nullptr, utl::DynamicTextureAllocator::AllocateType(0), true, false);

        f32 rcpWidth = 1.0f / size.x;
        f32 rcpHeight = 1.0f / size.y;
        rcpFrame.z = rcpWidth * 0.5f;
        rcpFrame.w = rcpHeight * 0.5f;
        rcpFrameOpt.x = rcpWidth * 0.5f;
        rcpFrameOpt.y = rcpHeight * 0.5f;
        rcpFrame.x = rcpWidth;
        rcpFrame.y = rcpHeight;
        rcpFrameOpt.z = rcpWidth * 0.15f;
        rcpFrameOpt.w = rcpHeight * 0.15f;

        f32 edgeThreshold;
        f32 edgeThresholdMin;
        switch (*mDetectEdgeQuality)
        {
        case 2:
            edgeThreshold = 0.1f;
            edgeThresholdMin = 0.05f;
            break;
        case 1:
            edgeThreshold = 0.125f;
            edgeThresholdMin = 0.0625f;
            break;
        default:
            edgeThreshold = 0.2f;
            edgeThresholdMin = 0.1f;
            break;
        }
        if (*mFetchQuality > 4)
        {
            edgeThreshold *= 1.5f;
            edgeThresholdMin *= 1.5f;
        }
        f32 scale = *mEdgeThresholdScale;
        threshold.x = edgeThreshold / scale;
        threshold.y = edgeThresholdMin / scale;
        threshold.z = scale;
        threshold.w = 1.0f / scale;
        span.x = *mSubpixParam;
        span.y = *mMaxSpan;
        span.z = 1.0f / (*mSpanMultiply * 8.0f);
        span.w = *mSpanMinimum * 0.1f;
        f32 lumaSum = mLumaCoeff->r + mLumaCoeff->g;
        if (lumaSum == 0.0f)
        {
            lumaSum = 1.0f;
        }
        lumaCoeff.x = mLumaCoeff->r / lumaSum;
        lumaCoeff.y = mLumaCoeff->g / lumaSum;
        lumaCoeff.z = mLumaCoeff->b / lumaSum;
        lumaCoeff.w = 0.0f;
        f32 alphaOut = *mAlphaOut;
        f32 alphaIn = 1.0f - alphaOut;

        ctx.mSourceSampler.applyTextureData(pSource->getTextureData());
        ctx.mLumaSampler.applyTextureData(*pLuma);
        ctx.mRenderTarget.applyTextureData(*pLuma);
        ctx.mRenderBuffer.setRenderTargetColorNullAll();
        ctx.mRenderBuffer.setRenderTargetDepth(nullptr);
        ctx.mRenderBuffer.setRenderTargetColor(&ctx.mRenderTarget);
        ctx.mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, size.x, size.y));
        ctx.mRenderBuffer.setVirtualSize(size);
        ctx.mRenderBuffer.bind(pDrawContext);
        ctx.mRenderTarget.invalidateGPUCache(pDrawContext);
        {
            sead::Viewport viewport(ctx.mRenderBuffer);
            viewport.apply(pDrawContext, ctx.mRenderBuffer);
        }

        {
            const ShaderProgram* program = mLumaPrograms[*mFetchQuality > 4];
            program->activate(pDrawContext, true);
            program->getUniformLocation(1).setUniform(pDrawContext, 4, &rcpFrame);
            program->getUniformLocation(2).setUniform(pDrawContext, 4, &rcpFrameOpt);
            program->getUniformLocation(3).setUniform(pDrawContext, 4, &lumaCoeff);
            program->getUniformLocation(4).setUniform(pDrawContext, 4, &threshold);
            program->getUniformLocation(5).setUniform(pDrawContext, 4, &span);
            ctx.mSourceSampler.activate(pDrawContext, program->getSamplerLocation(0), -1, false);
            detail::drawQuadTriangle(pDrawContext);
            ctx.mRenderTarget.invalidateGPUCache(pDrawContext);
        }

        if (mIsSRGB)
        {
            rRenderBuffer.bind_(pDrawContext, 0xff);
        }
        else
        {
            rRenderBuffer.bind(pDrawContext);
        }
        rViewport.apply(pDrawContext, rRenderBuffer);

        if (useReprojection)
        {
            const ShaderProgram* program =
                mReprojectionPrograms[normalizedDepth][alphaOut != 1.0f][*mFetchQuality]
                                     [mDebugNo];
            f32 moveScale = 1.41421354f / *mReprojectionMoveLimit;
            sead::Vector4f historySize;
            historySize.x = pHistory->getTextureData().getWidth(0);
            historySize.y = pHistory->getTextureData().getHeight(0);
            historySize.z = moveScale * historySize.x;
            historySize.w = moveScale * historySize.y;

            sead::Matrix44f viewProj;
            viewProj.setMul(ctx.mProjMtx, ctx.mViewMtx);
            sead::Matrix44f invViewProj;
            invViewProj.setInverse(viewProj);
            sead::Matrix44f prevViewProj;
            prevViewProj.setMul(ctx.mPrevProjMtx, ctx.mPrevViewMtx);
            const sead::Matrix44f bias(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, -0.5f, 0.0f, 0.5f, 0.0f,
                                       0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
            sead::Matrix44f texProj;
            detail::multiplyMtx44(texProj, bias, prevViewProj);
            sead::Matrix44f reprojMtx;
            detail::multiplyMtx44(reprojMtx, texProj, invViewProj);

            program->activate(pDrawContext, true);
            program->getUniformLocation(1).setUniform(pDrawContext, 4, &rcpFrame);
            program->getUniformLocation(2).setUniform(pDrawContext, 4, &rcpFrameOpt);
            program->getUniformLocation(3).setUniform(pDrawContext, 4, &lumaCoeff);
            program->getUniformLocation(4).setUniform(pDrawContext, 4, &threshold);
            program->getUniformLocation(5).setUniform(pDrawContext, 4, &span);
            f32 value = alphaIn;
            program->getUniformLocation(6).setUniform(pDrawContext, 1, &value);
            value = mIsSRGB ? 1.0f / 2.2f : 1.0f;
            program->getUniformLocation(7).setUniform(pDrawContext, 1, &value);
            program->getUniformLocation(8).setUniform(pDrawContext, 0x10, &reprojMtx);
            program->getUniformLocation(0).setUniform(pDrawContext, 4, &historySize);
            program->getUniformLocation(9).setUniform(pDrawContext, 4, &value);
            program->getUniformLocation(10).setUniform(pDrawContext, 4, &viewProj);
            ctx.mSourceSampler.activate(pDrawContext, program->getSamplerLocation(0), -1, false);
            ctx.mLumaSampler.activate(pDrawContext, program->getSamplerLocation(1), -1, false);
            pHistory->activate(pDrawContext, program->getSamplerLocation(4), -1, false);
            pDepth->activate(pDrawContext, program->getSamplerLocation(3), -1, false);
        }
        else
        {
            const ShaderProgram* program =
                mFxaaPrograms[alphaOut != 1.0f][*mFetchQuality][mDebugNo];
            program->activate(pDrawContext, true);
            program->getUniformLocation(1).setUniform(pDrawContext, 4, &rcpFrame);
            program->getUniformLocation(2).setUniform(pDrawContext, 4, &rcpFrameOpt);
            program->getUniformLocation(3).setUniform(pDrawContext, 4, &lumaCoeff);
            program->getUniformLocation(4).setUniform(pDrawContext, 4, &threshold);
            program->getUniformLocation(5).setUniform(pDrawContext, 4, &span);
            f32 value = alphaIn;
            program->getUniformLocation(6).setUniform(pDrawContext, 1, &value);
            value = mIsSRGB ? 1.0f / 2.2f : 1.0f;
            program->getUniformLocation(7).setUniform(pDrawContext, 1, &value);
            ctx.mSourceSampler.activate(pDrawContext, program->getSamplerLocation(0), -1, false);
            ctx.mLumaSampler.activate(pDrawContext, program->getSamplerLocation(1), -1, false);
        }
        detail::drawQuadTriangle(pDrawContext);
        ctx.mRenderTarget.invalidateGPUCache(pDrawContext);
        utl::DynamicTextureAllocator::instance()->free(pLuma);
    }
    else
    {
        pSource->getTextureData().getHeight(0);
        utl::ImageFilter2D::drawTexture(pDrawContext, *pSource, rViewport, sead::Vector2f::ones,
                                        sead::Vector2f::zero);
    }
}

void FilterAA::ReduceAA(DrawContext* pDrawContext, u32 context, const RenderBuffer& rRenderBuffer,
                        const sead::Viewport& rViewport, const TextureSampler* pSource) const
{
    Context& ctx = getContext_(context);
    sead::Vector2f size;
    rViewport.getOnFrameBufferSize(&size, rRenderBuffer);
    const TextureData* pReduce = utl::DynamicTextureAllocator::instance()->alloc(
        pDrawContext, "reduce", TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm,
        u32(size.x * 0.5f), u32(size.y * 0.5f), 1, nullptr,
        utl::DynamicTextureAllocator::AllocateType(0), true, false);

    ctx.mSourceSampler.applyTextureData(*pReduce);
    ctx.mRenderTarget.applyTextureData(*pReduce);
    ctx.mRenderBuffer.setRenderTargetColor(&ctx.mRenderTarget);
    sead::Vector2f reduceSize(size.x * 0.5f, size.y * 0.5f);
    ctx.mRenderBuffer.setVirtualSize(reduceSize);
    ctx.mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, reduceSize.x, reduceSize.y));
    ctx.mRenderBuffer.bind(pDrawContext);
    {
        sead::Viewport viewport(ctx.mRenderBuffer);
        viewport.apply(pDrawContext, ctx.mRenderBuffer);
        utl::ImageFilter2D::drawTexture(pDrawContext, *pSource, rViewport, sead::Vector2f::ones,
                                        sead::Vector2f::zero);
    }

    if (mIsSRGB)
    {
        rRenderBuffer.bind_(pDrawContext, 0xff);
    }
    else
    {
        rRenderBuffer.bind(pDrawContext);
    }
    {
        sead::Viewport viewport(ctx.mRenderBuffer);
        viewport.applyViewport(pDrawContext, rRenderBuffer);
    }
    rViewport.applyScissor(pDrawContext, rRenderBuffer);

    sead::Vector2f texel(1.0f / pSource->getTextureData().getWidth(0),
                         1.0f / pSource->getTextureData().getHeight(0));
    const ShaderProgram* program = agl::detail::ShaderHolder::instance()->getShaderProgram(
        agl::detail::ShaderHolder::cReduceAa);
    program->activate(pDrawContext, true);
    program->getUniformLocation(0).setUniform(pDrawContext, 2, &texel);
    pSource->activate(pDrawContext, program->getSamplerLocation(0), -1, false);
    ctx.mSourceSampler.activate(pDrawContext, program->getSamplerLocation(1), -1, false);
    detail::drawQuad(pDrawContext);
    utl::DynamicTextureAllocator::instance()->free(pReduce);
}

void FilterAA::reprojection(DrawContext* pDrawContext, u32 context,
                            const RenderBuffer& rRenderBuffer, const sead::Viewport& rViewport,
                            const TextureSampler* pSource, const TextureSampler* pDepth,
                            bool normalizedDepth, const TextureSampler* pHistory) const
{
    Context& ctx = getContext_(context);
    pSource->getTextureData().getHeight(0);

    sead::Matrix44f viewProj;
    viewProj.setMul(ctx.mProjMtx, ctx.mViewMtx);
    sead::Matrix44f invViewProj;
    invViewProj.setInverse(viewProj);
    sead::Matrix44f prevViewProj;
    prevViewProj.setMul(ctx.mPrevProjMtx, ctx.mPrevViewMtx);
    const sead::Matrix44f bias(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, -0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f,
                               0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    sead::Matrix44f texProj;
    detail::multiplyMtx44(texProj, bias, prevViewProj);
    sead::Matrix44f reprojMtx;
    detail::multiplyMtx44(reprojMtx, texProj, invViewProj);

    f32 moveScale = 1.41421354f / *mReprojectionMoveLimit;
    sead::Vector2f size;
    rViewport.getOnFrameBufferSize(&size, rRenderBuffer);
    sead::Vector4f screen(size.x, size.y, moveScale * size.x, moveScale * size.y);
    f32 near = ctx.mNear;
    f32 far = ctx.mFar;
    sead::Vector4f nearFar(near, far, 0.0f, 0.0f);
    f32 rcp = near / (far + near);
    sead::Vector4f depthParam(far - near, far + near, rcp, (far + far) * rcp);

    if (mIsSRGB)
    {
        rRenderBuffer.bind_(pDrawContext, 0xff);
    }
    else
    {
        rRenderBuffer.bind(pDrawContext);
    }
    rViewport.apply(pDrawContext, ctx.mRenderBuffer);

    const ShaderProgram* program =
        agl::detail::ShaderHolder::instance()
            ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cFilterAaReprojection)
            ->getVariation(normalizedDepth ? 1 : 0);
    program->activate(pDrawContext, true);
    program->getUniformLocation(0).setUniform(pDrawContext, 4, &screen);
    program->getUniformLocation(8).setUniform(pDrawContext, 0x10, &reprojMtx);
    program->getUniformLocation(9).setUniform(pDrawContext, 4, &nearFar);
    program->getUniformLocation(10).setUniform(pDrawContext, 4, &depthParam);
    pSource->activate(pDrawContext, program->getSamplerLocation(0), -1, false);
    pHistory->activate(pDrawContext, program->getSamplerLocation(4), -1, false);
    pDepth->activate(pDrawContext, program->getSamplerLocation(3), -1, false);
    detail::drawQuadTriangle(pDrawContext);
    ctx.mRenderTarget.invalidateGPUCache(pDrawContext);
}

void FilterAA::genMessage(sead::hostio::Context* pContext)
{
    genMessageIO(pContext, 0xf);
    mEnable.genMessageParameter(pContext, mEnable.getMeta());
    mMaxSpan.genMessageParameter(pContext, mMaxSpan.getMeta());
    mSpanMultiply.genMessageParameter(pContext, mSpanMultiply.getMeta());
    mSubpixParam.genMessageParameter(pContext, mSubpixParam.getMeta());
    mLumaCoeff.genMessageParameter(pContext, mLumaCoeff.getMeta());
    mAlphaOut.genMessageParameter(pContext, mAlphaOut.getMeta());
    mEdgeThresholdScale.genMessageParameter(pContext, mEdgeThresholdScale.getMeta());
    mReprojection.genMessageParameter(pContext, mReprojection.getMeta());
    mReprojectionMoveLimit.genMessageParameter(pContext, mReprojectionMoveLimit.getMeta());
    mDebugTexturePage.genMessagePage(pContext, this);
}

/**
 * Handles a host property edit, updating the debug mode or resetting the luma coefficients.
 * @param pEvent Property event received from the host.
 */
void FilterAA::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if (listenPropertyEventIO(this, pEvent) == 0)
    {
        uintptr_t id = pEvent->getIdValue();
        if (id == 100)
        {
            *mLumaCoeff = cDefaultLumaCoeff;
        }
        else if (id == reinterpret_cast<uintptr_t>(&mDebugFlag))
        {
            if (mDebugFlag.isOn(1 << 2))
            {
                mDebugNo = 2;
            }
            else if (mDebugFlag.isOn(1 << 1))
            {
                mDebugNo = 1;
            }
            else
            {
                mDebugNo = 0;
            }
        }
    }
}

}  // namespace agl::pfx
