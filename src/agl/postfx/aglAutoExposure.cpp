#include "postfx/aglAutoExposure.h"

#include <cmath>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "detail/aglRootNode.h"
#include "detail/aglShaderHolder.h"
#include "driver/aglGraphicsDriverMgr.h"
#include "postfx/aglPostFxUtil.h"

namespace agl::pfx {

void AutoExposureVtxStream::initialize(sead::Heap* pHeap)
{
    mNum = 0x800;
    mVertexBlock.allocBuffer_(sizeof(AutoExposureVtx) * 0x800, pHeap, 8,
                              MemoryAttribute::Default);
    GPUMemAddr<AutoExposureVtx> vertexAddr(mVertexBlock, 0);
    mIndexBlock.allocBuffer_(sizeof(u16) * mNum, pHeap, 8, MemoryAttribute::Default);
    GPUMemAddr<u16> indexAddr(mIndexBlock, 0);
    mVertexAttribute.create(1, pHeap);
    create(mNum);
}

void AutoExposureVtxStream::create(s32 num)
{
    f32 scale = 0.5f / f32(num);

    for (s32 i = 0; i < num; i++)
    {
        f32 fi = f32(i);
        f32 angle = fi * 70.7738037f;
        AutoExposureVtx* pVtx = detail::getBufferPtr<AutoExposureVtx>(mVertexBlock) + i;
        pVtx->mPos.x = scale * fi * std::sin(angle);
        pVtx->mPos.y = scale * fi * std::cos(angle);
        pVtx->mPos.z = 0.0f;
        detail::getBufferPtr<u16>(mIndexBlock)[i] = i;
    }

    mVertexBuffer.setUpBuffer(ConstGPUMemVoidAddr(mVertexBlock, 0),
                              sizeof(AutoExposureVtx), sizeof(AutoExposureVtx) * num);
    mVertexBuffer.setUpStream(0, VertexStreamFormat(0x22), 0, false);
    mVertexAttribute.setVertexStream(0, &mVertexBuffer, 0);
    mVertexAttribute.setUp();
    mIndexStream.setUpStream(GPUMemAddr<u16>(mIndexBlock, 0), num);
    mIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_POINTS);
}

void AutoExposure::ResultBuffer::Create(sead::Heap* pHeap, s32 width, s32 height,
                                        TextureFormat format)
{
    mWidth = width;
    mHeight = height;
    mTexture.initialize_(TextureType::cTextureType_2D, format, width, height, 1, 1,
                         TextureAttribute(0), MultiSampleType(0), true);
    u32 size = mTexture.getSurface().mStorageSize;
    u32 alignment = mTexture.getSurface().mAlignment;
    auto* block = new (pHeap, 8) GPUMemBlock<u8>;
    block->allocBuffer_(size, pHeap, alignment, MemoryAttribute::Default);
    mImage = GPUMemAddr<u8>(*block, 0);
    mTexture.setImagePtr(mImage);
    mRenderTarget.applyTextureData(mTexture);
    mRenderBuffer.setVirtualSize(sead::Vector2f(mWidth, mHeight));
    mRenderBuffer.setRenderTargetColor(&mRenderTarget);
    mRenderBuffer.setRenderTargetDepth(nullptr);
    mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, mWidth, mHeight));
    mSampler.applyTextureData(mTexture);
}

void AutoExposure::ResultBuffer::Bind(DrawContext* pDrawContext)
{
    mRenderTarget.applyTextureData(mTexture);
    mRenderBuffer.setVirtualSize(sead::Vector2f(mWidth, mHeight));
    mRenderBuffer.setRenderTargetColor(&mRenderTarget);
    mRenderBuffer.setRenderTargetDepth(nullptr);
    mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, mWidth, mHeight));
    mRenderBuffer.bind(pDrawContext);
    sead::Viewport viewport(mRenderBuffer);
    viewport.apply(pDrawContext, mRenderBuffer);
    mRenderTarget.invalidateGPUCache(pDrawContext);
}

void AutoExposure::ResultBuffer::Clear(DrawContext* pDrawContext, const sead::Color4f& rColor)
{
    mRenderBuffer.clear(pDrawContext, 1, rColor, 1.0f, 0);
}

AutoExposure::AutoExposure() : IParameterIO("aglatex", 0)
{
    addObj(&mParamObj, "AutoExposure");
    agl::detail::RootNode::setNodeMeta(this, "Icon=EFFECT");
    *mIsEnable = true;
}

AutoExposure::~AutoExposure()
{
    for (auto& context : mContexts)
    {
        for (s32 j = 0; j < cResult_Num; j++)
        {
            context.mResults[j].mImage.deleteGPUMemBlock();
        }
    }

    mContexts.freeBuffer();
}

void AutoExposure::initialize(s32 contextNum, sead::Heap* pHeap)
{
    mContexts.tryAllocBuffer(contextNum, pHeap);

    for (auto& context : mContexts)
    {
        context.mResults[cResult_Prev0].Create(
            pHeap, 1, 1, TextureFormat::cTextureFormat_R32_G32_B32_A32_float);
        context.mResults[cResult_Prev1].Create(
            pHeap, 1, 1, TextureFormat::cTextureFormat_R32_G32_B32_A32_float);
        context.mResults[cResult_Histogram].Create(
            pHeap, 128, 1, TextureFormat::cTextureFormat_R32_G32_B32_A32_float);
        context.mResults[cResult_HistogramDebug].Create(
            pHeap, 128, 128, TextureFormat::cTextureFormat_R32_G32_B32_A32_float);
        context.mCurrent = 0;
        context.mPrev = 1;
        context.mIsUpdated = false;
    }

    mDebugTexturePage.setUp(contextNum, "AutoExposure", pHeap);
    mVtxStream.initialize(pHeap);
}

void AutoExposure::calc()
{
    for (auto& context : mContexts)
    {
        if (context.mIsUpdated)
        {
            context.mCurrent = context.mPrev;
            context.mPrev = 1 - context.mPrev;
            context.mIsUpdated = false;
        }
    }
}

void AutoExposure::calcGPU() const {}

void AutoExposure::calcGPU(s32 context) const {}

void AutoExposure::setCommonShaderParam(DrawContext* pDrawContext,
                                        const ShaderProgram* pProgram) const
{
    sead::Vector4f exposure;
    sead::Vector4f vertex;
    sead::Vector4f luminance;
    sead::Vector4f range;

    exposure.x = *mExposureMid;
    exposure.y = *mBlendRateUp;
    exposure.z = 1.0f / mLuminanceScale;
    exposure.w = *mBlendRateDown;
    pProgram->getUniformLocation(0).setUniform(pDrawContext, 4, &exposure);

    vertex.x = *mVertexOffsetX;
    vertex.y = *mVertexOffsetY;
    vertex.z = *mVertexScaleX;
    vertex.w = *mVertexScaleY;
    pProgram->getUniformLocation(1).setUniform(pDrawContext, 4, &vertex);

    luminance.x = mLuminanceMax;
    luminance.y = mLuminanceMin;
    luminance.z = mLuminanceRange;
    luminance.w = 1.0f / mLuminanceRange;
    pProgram->getUniformLocation(2).setUniform(pDrawContext, 4, &luminance);

    range.x = *mRangeMax;
    range.y = *mRangeMin;
    range.z = 0.0f;
    range.w = 0.0f;
    pProgram->getUniformLocation(3).setUniform(pDrawContext, 4, &range);
}

void AutoExposure::draw(DrawContext* pDrawContext, s32 context, const RenderBuffer& rRenderBuffer,
                        const TextureData* pTexture) const
{
    if (!*mIsEnable)
    {
        return;
    }

    Context& ctx = getContext_(context);
    drawHistogram(pDrawContext, context, rRenderBuffer, pTexture);
    drawHistogramCalc(pDrawContext, context, rRenderBuffer,
                      &ctx.mResults[cResult_Histogram].mTexture);
    drawSimple(pDrawContext, context, rRenderBuffer, pTexture);
    ctx.mIsUpdated = true;
}

void AutoExposure::drawHistogram(DrawContext* pDrawContext, s32 context,
                                 const RenderBuffer& rRenderBuffer,
                                 const TextureData* pTexture) const
{
    if (!*mIsEnable || mFlags.isOn(cFlag_Simple))
    {
        return;
    }

    Context& ctx = getContext_(context);
    ResultBuffer& histogram = ctx.mResults[cResult_Histogram];
    histogram.Bind(pDrawContext);
    histogram.mRenderBuffer.clear(pDrawContext, 1, sead::Color4f(0.0f, 0.0f, 0.0f, 1.0f), 1.0f,
                                  0);

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(true);
    graphicsContext.setBlendFactor(0, 5, 2);
    graphicsContext.setBlendEquation(0, 1);
    graphicsContext.apply(pDrawContext);

    const ShaderProgram* program =
        agl::detail::ShaderHolder::instance()
            ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cAutoExposure)
            ->getVariation(1);
    program->activate(pDrawContext, true);
    setCommonShaderParam(pDrawContext, program);

    ctx.mSampler.applyTextureData((pTexture != nullptr) ? *pTexture :
                                             *reinterpret_cast<const TextureData*>(
                                                 rRenderBuffer.getRenderTargetColor()));
    ctx.mSampler.activate(pDrawContext, program->getSamplerLocationValidate(0), -1, false);
    ctx.mResults[ctx.mCurrent].mSampler.activate(
        pDrawContext, program->getSamplerLocationValidate(1), -1, false);

    driver::GraphicsDriverMgr::instance()->setPointSize(pDrawContext, 1.0f);
    mVtxStream.mVertexAttribute.activate(pDrawContext);
    detail::drawIndexStream(pDrawContext, mVtxStream.mIndexStream);

    histogram.mRenderTarget.invalidateGPUCache(pDrawContext);
    histogram.mSampler.applyTextureData(histogram.mTexture);
}

void AutoExposure::drawHistogramCalc(DrawContext* pDrawContext, s32 context,
                                     const RenderBuffer& rRenderBuffer,
                                     const TextureData* pTexture) const
{
    if (!*mIsEnable || mFlags.isOn(cFlag_Simple))
    {
        return;
    }

    Context& ctx = getContext_(context);
    const ShaderProgram* program =
        agl::detail::ShaderHolder::instance()
            ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cAutoExposure)
            ->getVariation(2);

    if ((mFlags.getDirect() & (cFlag_Cleared | cFlag_ForceClear)) != cFlag_Cleared)
    {
        mFlags.set(cFlag_Cleared);
        sead::Color4f color(0.5f, 0.5f, 0.5f, 0.5f);
        ctx.mResults[ctx.mCurrent].Bind(pDrawContext);
        ctx.mResults[ctx.mCurrent].mRenderBuffer.clear(pDrawContext, 1, color, 1.0f, 0);
    }

    ctx.mResults[ctx.mPrev].Bind(pDrawContext);

    sead::GraphicsContext graphicsContext;
    graphicsContext.setBlendEnable(false);
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.apply(pDrawContext);

    program->activate(pDrawContext, true);
    setCommonShaderParam(pDrawContext, program);

    ctx.mSampler2.setMagFilter(0);
    ctx.mSampler2.setMinFilter(0);
    ctx.mSampler2.applyTextureData(*pTexture);
    ctx.mSampler2.activate(pDrawContext, program->getSamplerLocationValidate(0), -1, false);
    ctx.mResults[ctx.mCurrent].mSampler.activate(
        pDrawContext, program->getSamplerLocationValidate(1), -1, false);
    detail::drawQuadTriangle(pDrawContext);

    ctx.mResults[ctx.mPrev].mRenderTarget.invalidateGPUCache(pDrawContext);
    ctx.mResults[ctx.mPrev].mSampler.applyTextureData(ctx.mResults[ctx.mPrev].mTexture);
}

void AutoExposure::drawSimple(DrawContext* pDrawContext, s32 context,
                              const RenderBuffer& rRenderBuffer,
                              const TextureData* pTexture) const
{
    if (!*mIsEnable || !mFlags.isOn(cFlag_Simple))
    {
        return;
    }

    Context& ctx = getContext_(context);
    const ShaderProgram* program =
        agl::detail::ShaderHolder::instance()
            ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cAutoExposure)
            ->getVariation(0);

    if ((mFlags.getDirect() & (cFlag_Cleared | cFlag_ForceClear)) != cFlag_Cleared)
    {
        mFlags.set(cFlag_Cleared);
        sead::Color4f color(0.5f, 0.5f, 0.5f, 0.5f);
        ctx.mResults[ctx.mCurrent].Bind(pDrawContext);
        ctx.mResults[ctx.mCurrent].mRenderBuffer.clear(pDrawContext, 1, color, 1.0f, 0);
    }

    ctx.mResults[ctx.mPrev].Bind(pDrawContext);

    sead::GraphicsContext graphicsContext;
    graphicsContext.setBlendEnable(false);
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.apply(pDrawContext);

    program->activate(pDrawContext, true);
    setCommonShaderParam(pDrawContext, program);

    ctx.mSampler.applyTextureData((pTexture != nullptr) ? *pTexture :
                                             *reinterpret_cast<const TextureData*>(
                                                 rRenderBuffer.getRenderTargetColor()));
    ctx.mSampler.activate(pDrawContext, program->getSamplerLocationValidate(0), -1, false);
    ctx.mResults[ctx.mCurrent].mSampler.activate(
        pDrawContext, program->getSamplerLocationValidate(1), -1, false);
    detail::drawQuadTriangle(pDrawContext);

    ctx.mResults[ctx.mPrev].mRenderTarget.invalidateGPUCache(pDrawContext);
    ctx.mResults[ctx.mPrev].mSampler.applyTextureData(ctx.mResults[ctx.mPrev].mTexture);
}

void AutoExposure::drawDebug(DrawContext* pDrawContext, s32 context,
                             const RenderBuffer& rRenderBuffer, const TextureData* pTexture) const
{
    if (!*mIsEnable)
    {
        return;
    }

    drawHistogramDebugVertex(pDrawContext, context, rRenderBuffer, pTexture);
    drawHistogramDebug(pDrawContext, context);
}

void AutoExposure::drawHistogramDebugVertex(DrawContext* pDrawContext, s32 context,
                                            const RenderBuffer& rRenderBuffer,
                                            const TextureData* pTexture) const
{
    if (!*mIsEnable || !mFlags.isOn(cFlag_DebugVertex))
    {
        return;
    }

    Context& ctx = getContext_(context);
    sead::GraphicsContext graphicsContext;
    graphicsContext.setBlendEnable(false);
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.apply(pDrawContext);

    const ShaderProgram* program =
        agl::detail::ShaderHolder::instance()
            ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cAutoExposure)
            ->getVariation(4);
    program->activate(pDrawContext, true);
    setCommonShaderParam(pDrawContext, program);

    ctx.mSampler.applyTextureData((pTexture != nullptr) ? *pTexture :
                                             *reinterpret_cast<const TextureData*>(
                                                 rRenderBuffer.getRenderTargetColor()));
    ctx.mSampler.activate(pDrawContext, program->getSamplerLocationValidate(0), -1, false);
    ctx.mResults[ctx.mCurrent].mSampler.activate(
        pDrawContext, program->getSamplerLocationValidate(1), -1, false);

    driver::GraphicsDriverMgr::instance()->setPointSize(pDrawContext, 2.0f);
    mVtxStream.mVertexAttribute.activate(pDrawContext);
    detail::drawIndexStream(pDrawContext, mVtxStream.mIndexStream);

    ResultBuffer& histogram = ctx.mResults[cResult_Histogram];
    histogram.mRenderTarget.invalidateGPUCache(pDrawContext);
    histogram.mSampler.applyTextureData(histogram.mTexture);
}

void AutoExposure::drawHistogramDebug(DrawContext* pDrawContext, s32 context) const
{
    if (!*mIsEnable || !mDebugTexturePage.isActive())
    {
        return;
    }

    Context& ctx = getContext_(context);
    const ShaderProgram* program =
        agl::detail::ShaderHolder::instance()
            ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cAutoExposure)
            ->getVariation(3);

    ResultBuffer& debug = ctx.mResults[cResult_HistogramDebug];
    debug.Bind(pDrawContext);

    sead::GraphicsContext graphicsContext;
    graphicsContext.setBlendEnable(false);
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.apply(pDrawContext);

    program->activate(pDrawContext, true);
    setCommonShaderParam(pDrawContext, program);

    ctx.mSampler.applyTextureData(ctx.mResults[cResult_Histogram].mTexture);
    ctx.mSampler.activate(pDrawContext, program->getSamplerLocationValidate(0), -1, false);
    ctx.mResults[ctx.mPrev].mSampler.activate(pDrawContext,
                                              program->getSamplerLocationValidate(1), -1, false);
    detail::drawQuadTriangle(pDrawContext);

    debug.mRenderTarget.invalidateGPUCache(pDrawContext);
    debug.mSampler.applyTextureData(debug.mTexture);
}

void AutoExposure::enableSingleChannel(bool enable, TextureCompSel compSel)
{
    for (auto& context : mContexts)
    {
        if (enable)
        {
            context.mSampler.setUseTextureView(true);
            context.mSampler.setCompSel(compSel, compSel, compSel, compSel);
            context.mSampler2.setUseTextureView(true);
            context.mSampler2.setCompSel(compSel, compSel, compSel, compSel);
        }
        else
        {
            context.mSampler.setUseTextureView(false);
            context.mSampler2.setUseTextureView(false);
        }
    }
}

void AutoExposure::genMessage(sead::hostio::Context* pContext)
{
    genMessageIO(pContext, 0xf);
    mDebugTexturePage.genMessagePage(pContext, this);
}

void AutoExposure::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    listenPropertyEventIO(this, pEvent);
}

}  // namespace agl::pfx
