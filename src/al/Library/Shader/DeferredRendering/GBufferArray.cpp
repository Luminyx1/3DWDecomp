#include "Library/Shader/DeferredRendering/GBufferArray.hpp"

#include <common/aglRenderBuffer.h>
#include <common/aglShaderLocation.h>
#include <gfx/seadCamera.h>
#include <gfx/seadGraphicsContextMRT.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>
#include <lighting/aglLightPrePass.h>
#include <utility/aglDynamicTextureAllocator.h>

#include "Library/Draw/GraphicsFunction.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GraphicsStressDirector.hpp"

namespace {
/**
 * @brief Gets the draw context of the game framework.
 */
agl::DrawContext* getDrawContext() {
    return al::GameFrameworkNx::getAglDrawContext();
}

/**
 * @brief Sizes a render buffer to cover a whole texture.
 */
template <typename T>
void setRenderBufferSize(agl::RenderBuffer* pRenderBuffer, T width, T height) {
    pRenderBuffer->setVirtualSize(sead::Vector2f(width, height));
    pRenderBuffer->setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, width, height));
}

/**
 * @brief Frees a G-buffer texture made by the dynamic texture allocator.
 */
void freeGBufferTexture(al::GBuffer* pGBuffer) {
    if (pGBuffer->mTexture != nullptr) {
        agl::utl::DynamicTextureAllocator::instance()->free(pGBuffer->mTexture);
        pGBuffer->mTexture = nullptr;
    }
}
}  // namespace

namespace al {

/**
 * @brief Sets up the G-buffer formats; the textures are allocated per frame.
 */
GBufferArray::GBufferArray(const agl::RenderTargetDepth* pDepthTarget,
                           const GraphicsSystemInfo* pGraphicsSystemInfo,
                           bool isHighPrecisionDepth, bool isClearDepthView)
    : mGraphicsSystemInfo(pGraphicsSystemInfo), mDepthTarget(pDepthTarget),
      mIsHighPrecisionDepth(isHighPrecisionDepth), mIsClearDepthView(isClearDepthView) {
    for (s32 i = 0; i < cIndex_Num; i++) {
        mGBuffers[i].mTexture = nullptr;
    }

    mGBuffers[cIndex_Albedo].mFormat = agl::TextureFormat(0x1d);
    mGBuffers[cIndex_NrmView].mFormat = agl::TextureFormat(0x22);
    mGBuffers[cIndex_DepthView].mFormat = agl::TextureFormat(isHighPrecisionDepth ? 0x19 : 9);
}

/**
 * @brief Frees the owned G-buffer textures (the light buffer belongs to the light pre-pass).
 */
GBufferArray::~GBufferArray() {
    freeGBufAlbedo();
    freeGBufNrmView();
    freeGBufDepthView();
}

/**
 * @brief Frees one owned G-buffer texture.
 */
void GBufferArray::freeGBuf(s32 index) {
    if (index == cIndex_LightBuffer) {
        return;
    }

    freeGBufferTexture(&mGBuffers[index]);
}

/**
 * @brief Allocates the owned G-buffer textures at the size of the depth target.
 */
void GBufferArray::allocGBuffer(s32 subIndex) {
    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();
    u32 width = mDepthTarget->getWidth(0);
    u32 height = mDepthTarget->getHeight(0);

    for (s32 i = 0; i < cIndex_Num; i++) {
        GBuffer& gbuffer = mGBuffers[i];
        gbuffer.mIsClear = false;
        const char* name;
        agl::utl::DynamicTextureAllocator::AllocateType type;

        switch (i) {
        case cIndex_Albedo:
            name = subIndex == 0 ? "gbuf_albedo" : "gbuf_albedo_sub";
            type = agl::utl::DynamicTextureAllocator::cAllocateType_1;
            break;
        case cIndex_NrmView: {
            const GraphicsStressDirector* stress = mGraphicsSystemInfo->getGraphicsStressDirector();
            gbuffer.mIsClear = stress->isForceStressOff() || stress->isClearGBufferViewNrm();
            name = subIndex == 0 ? "gbuf_view_normal" : "gbuf_view_normal_sub";
            type = agl::utl::DynamicTextureAllocator::cAllocateType_1;
            break;
        }
        case cIndex_DepthView: {
            const GraphicsStressDirector* stress = mGraphicsSystemInfo->getGraphicsStressDirector();
            gbuffer.mIsClear = mIsClearDepthView || stress->isForceStressOff() ||
                               stress->isClearGBufferViewDepth();
            name = subIndex == 0 ? "gbuf_view_depth" : "gbuf_view_depth_sub";
            type = agl::utl::DynamicTextureAllocator::cAllocateType_0;
            break;
        }
        case cIndex_LightBuffer:
            continue;
        default:
            return;
        }

        gbuffer.mTexture = allocator->alloc(getDrawContext(), name, gbuffer.mFormat, width,
                                            height, 1, nullptr, type, true, false);
        gbuffer.mRenderTarget.applyTextureData(*gbuffer.mTexture);
        gbuffer.mSampler.applyTextureData(*gbuffer.mTexture);
        gbuffer.mNearestSampler.applyTextureData(*gbuffer.mTexture);
        gbuffer.mNearestSampler.setFilter(0, 0, 0);
    }
}

/**
 * @brief Clears the G-buffers flagged for clearing.
 */
void GBufferArray::clearGBuffer() {
    for (s32 i = 0; i != cIndex_Num; i++) {
        s32 width = mDepthTarget->getMipWidth(0);
        s32 height = mDepthTarget->getMipHeight(0);

        if (i == cIndex_LightBuffer) {
            break;
        }

        if (!mGBuffers[i].mIsClear) {
            continue;
        }

        agl::RenderTargetColor renderTarget;
        renderTarget.applyTextureData(*mGBuffers[i].mTexture);
        agl::RenderBuffer renderBuffer;
        setRenderBufferSize(&renderBuffer, width, height);
        renderBuffer.setRenderTargetColorNullAll();
        renderBuffer.setRenderTargetColor(&renderTarget);
        renderBuffer.bind(GameFrameworkNx::getDrawContext());
        sead::Viewport viewport(renderBuffer);
        viewport.apply(GameFrameworkNx::getDrawContext(), renderBuffer);
        renderBuffer.clear(GameFrameworkNx::getDrawContext(), sead::FrameBuffer::cColor,
                           sead::Color4f::cBlack, 1.0f, 0);
    }
}

/**
 * @brief Calculates the light pre-pass context of a view and takes its light buffer.
 */
void GBufferArray::createLightBufferAndCalcContext(
    agl::lght::LightPrePass* pLightPrePass, s32 view, s32 width, s32 height,
    const sead::LookAtCamera& rCamera, const sead::PerspectiveProjection& rProjection,
    bool isUseMultiTarget) {
    pLightPrePass->calcContext(view, rCamera.getMatrix(), rProjection.getProjectionMatrix(),
                               rProjection.getNear(), rProjection.getFar(),
                               rProjection.getFovy(), rProjection.getAspect(),
                               rProjection.getOffsetDirect());
    GBuffer& lightBuffer = mGBuffers[cIndex_LightBuffer];
    lightBuffer.mTexture = pLightPrePass->createLightBuffer(getDrawContext(), view, width, height,
                                                            isUseMultiTarget, true);
    lightBuffer.mSampler = pLightPrePass->getContext(view).mLightBufferSampler;
    lightBuffer.mRenderTarget.applyTextureData(*lightBuffer.mTexture);
}

/**
 * @brief Gets the albedo G-buffer.
 */
const GBuffer* GBufferArray::getGBufAlbedo() const {
    return &mGBuffers[cIndex_Albedo];
}

/**
 * @brief Gets the view normal G-buffer.
 */
const GBuffer* GBufferArray::getGBufNrmView() const {
    return &mGBuffers[cIndex_NrmView];
}

/**
 * @brief Gets the view depth G-buffer.
 */
const GBuffer* GBufferArray::getGBufDepthView() const {
    return &mGBuffers[cIndex_DepthView];
}

/**
 * @brief Gets the light buffer.
 */
const GBuffer* GBufferArray::getGBufLightBuffer() const {
    return &mGBuffers[cIndex_LightBuffer];
}

/**
 * @brief Gets the albedo texture.
 */
agl::TextureData* GBufferArray::getGBufAlbedoTex() const {
    return mGBuffers[cIndex_Albedo].mTexture;
}

/**
 * @brief Gets the view normal texture.
 */
agl::TextureData* GBufferArray::getGBufNrmViewTex() const {
    return mGBuffers[cIndex_NrmView].mTexture;
}

/**
 * @brief Gets the view depth texture.
 */
agl::TextureData* GBufferArray::getGBufDepthViewTex() const {
    return mGBuffers[cIndex_DepthView].mTexture;
}

/**
 * @brief Gets the light buffer texture.
 */
agl::TextureData* GBufferArray::getGBufLightBufferTex() const {
    return mGBuffers[cIndex_LightBuffer].mTexture;
}

/**
 * @brief Activates the albedo sampler.
 */
void GBufferArray::activateSamplerAlbedo(const agl::SamplerLocation& rLocation) const {
    mGBuffers[cIndex_Albedo].mSampler.activate(getDrawContext(), rLocation, -1, false);
}

/**
 * @brief Activates the nearest-filtered albedo sampler.
 */
void GBufferArray::activateSamplerNearestAlbedo(const agl::SamplerLocation& rLocation) const {
    mGBuffers[cIndex_Albedo].mNearestSampler.activate(getDrawContext(), rLocation, -1, false);
}

/**
 * @brief Activates the view normal sampler.
 */
void GBufferArray::activateSamplerNrmView(const agl::SamplerLocation& rLocation) const {
    mGBuffers[cIndex_NrmView].mSampler.activate(getDrawContext(), rLocation, -1, false);
}

/**
 * @brief Activates the view depth sampler.
 */
void GBufferArray::activateSamplerDepthView(const agl::SamplerLocation& rLocation) const {
    mGBuffers[cIndex_DepthView].mSampler.activate(getDrawContext(), rLocation, -1, false);
}

/**
 * @brief Activates the nearest-filtered view depth sampler.
 */
void GBufferArray::activateSamplerNearestDepthView(const agl::SamplerLocation& rLocation) const {
    mGBuffers[cIndex_DepthView].mNearestSampler.activate(getDrawContext(), rLocation, -1, false);
}

/**
 * @brief Activates the light buffer sampler.
 */
void GBufferArray::activateSamplerLightBuffer(const agl::SamplerLocation& rLocation) const {
    mGBuffers[cIndex_LightBuffer].mSampler.activate(getDrawContext(), rLocation, -1, false);
}

/**
 * @brief Enables the stencil test that marks the G-buffer pixels.
 */
void GBufferArray::setStencilTest(sead::GraphicsContextMRT* pContext) {
    pContext->setStencilTestEnable(true);
    pContext->setStencilTestFunc(8);
    pContext->setStencilTestRef(0);
    pContext->setStencilTestMask(0xff);
    pContext->setStencilOp(1, 1, 3);
    pContext->setStencilWriteMask(0xff);
}

/**
 * @brief Frees the albedo texture.
 */
void GBufferArray::freeGBufAlbedo() {
    freeGBufferTexture(&mGBuffers[cIndex_Albedo]);
}

/**
 * @brief Frees the view normal texture.
 */
void GBufferArray::freeGBufNrmView() {
    freeGBufferTexture(&mGBuffers[cIndex_NrmView]);
}

/**
 * @brief Frees the view depth texture.
 */
void GBufferArray::freeGBufDepthView() {
    freeGBufferTexture(&mGBuffers[cIndex_DepthView]);
}

/**
 * @brief Binds the first G-buffers (up to the given index) and the depth target.
 */
void GBufferArray::bindRenderBuffer(s32 num) {
    const agl::TextureData* texture = mGBuffers[cIndex_Albedo].mTexture;
    u32 width = texture->getWidth(0);
    u32 height = texture->getHeight(0);
    agl::RenderBuffer renderBuffer;
    setRenderBufferSize(&renderBuffer, width, height);
    renderBuffer.setRenderTargetColorNullAll();

    for (s32 i = 0; i < cIndex_Num; i++) {
        if (i <= num) {
            renderBuffer.setRenderTargetColor(&mGBuffers[i].mRenderTarget, i);
        }
    }

    renderBuffer.setRenderTargetDepth(const_cast<agl::RenderTargetDepth*>(mDepthTarget));
    renderBuffer.bind(GameFrameworkNx::getDrawContext());
    sead::Viewport viewport(renderBuffer);
    viewport.apply(GameFrameworkNx::getDrawContext(), renderBuffer);
}

/**
 * @brief Binds the light buffer and the depth target.
 */
void GBufferArray::bindRenderBufferLightBuf() {
    const agl::TextureData* texture = mGBuffers[cIndex_Albedo].mTexture;
    u32 width = texture->getWidth(0);
    u32 height = texture->getHeight(0);
    agl::RenderBuffer renderBuffer;
    setRenderBufferSize(&renderBuffer, width, height);
    renderBuffer.setRenderTargetColorNullAll();
    renderBuffer.setRenderTargetColor(&mGBuffers[cIndex_LightBuffer].mRenderTarget);
    renderBuffer.setRenderTargetDepth(const_cast<agl::RenderTargetDepth*>(mDepthTarget));
    renderBuffer.bind(GameFrameworkNx::getDrawContext());
    sead::Viewport viewport(renderBuffer);
    viewport.apply(GameFrameworkNx::getDrawContext(), renderBuffer);
}

/**
 * @brief Sets up a context drawing opaque models into the G-buffers.
 */
void GBufferArray::setContextMRT(sead::GraphicsContextMRT* pContext) {
    pContext->setColorMask(0xffff);
    setStencilTest(pContext);
    pContext->setBlendEnableMask(0);
}

/**
 * @brief Sets up a context drawing translucent models into the G-buffers.
 */
void GBufferArray::setContextMRTXlu(sead::GraphicsContextMRT* pContext) {
    pContext->setBlendEnableMask(9);
    pContext->setColorMask(0xf007);
    pContext->setDepthEnable(true, true);
    pContext->setBlendFactorSrcA(0, 1);
    pContext->setBlendFactorDstA(0, 1);
    pContext->setBlendEquationA(0, 1);
    pContext->setBlendFactorSrcRGB(3, 5);
    pContext->setBlendFactorDstRGB(3, 6);
    pContext->setBlendEquationRGB(3, 1);
}

/**
 * @brief Sets up a context drawing translucent models that also write normals.
 */
void GBufferArray::setContextMRTXluNrm(sead::GraphicsContextMRT* pContext) {
    pContext->setBlendEnableMask(0xb);
    pContext->setColorMask(0xfff7);
    pContext->setDepthEnable(true, true);
    pContext->setBlendFactorSrcA(0, 1);
    pContext->setBlendFactorDstA(0, 1);
    pContext->setBlendEquationA(0, 1);
    pContext->setBlendFactorSrcRGB(1, 5);
    pContext->setBlendFactorDstRGB(1, 6);
    pContext->setBlendEquationRGB(1, 1);
    pContext->setBlendFactorSrcRGB(3, 5);
    pContext->setBlendFactorDstRGB(3, 6);
    pContext->setBlendEquationRGB(3, 1);
}

/**
 * @brief Sets up a context from the render state of a material.
 */
void GBufferArray::setContextMRTCustom(sead::GraphicsContextMRT* pContext,
                                       const nn::g3d::MaterialObj* pMaterial, bool isNoNrm) {
    pContext->setBlendEnableMask(0);
    pContext->setColorMask(0);

    if (!isUseBlend(pMaterial)) {
        setContextMRT(pContext);
        return;
    }

    const char* blendMode = getBlendMode(pMaterial);
    bool isCustom = isEqualString(blendMode, "Custom");

    if (getAlphaTestEnable(pMaterial)) {
        setContextMRTAlphaMask(pContext);
        return;
    }

    bool isXlu = isXluBlend(pMaterial);

    if (!isCustom && isXlu) {
        setContextMRTXlu(pContext);
        return;
    }

    if (!isEqualString(blendMode, "Custom")) {
        setContextMRT(pContext);
        return;
    }

    if (!isNoNrm) {
        pContext->setBlendEnable(0, true);
        pContext->setBlendEnable(3, true);
        pContext->setColorMask(0, true, true, true, false);
        pContext->setColorMask(1, true, true, true, true);
        pContext->setColorMask(2, true, true, true, true);
        pContext->setColorMask(3, true, true, true, true);
    } else {
        pContext->setBlendEnable(3, true);
        pContext->setColorMask(0, false, false, false, false);
        pContext->setColorMask(1, false, false, false, false);
        pContext->setColorMask(2, false, false, false, false);
        pContext->setColorMask(3, true, true, true, true);
    }

    setPolygonCtrlToContext(pContext, pMaterial);
    setDepthCtrlToContext(pContext, pMaterial);
    setAlphaTestToContext(pContext, pMaterial);
    pContext->setBlendFactorSrcA(0, 1);
    pContext->setBlendFactorDstA(0, 1);
    pContext->setBlendEquationA(0, 1);
    u8 srcFunc = getBlendFunc(pMaterial, true, false);
    u8 dstFunc = getBlendFunc(pMaterial, false, false);
    u8 equation = getBlendEquation(pMaterial, false);
    pContext->setBlendFactorSrcRGB(0, srcFunc);
    pContext->setBlendFactorDstRGB(0, dstFunc);
    pContext->setBlendEquationRGB(0, equation);
    pContext->setBlendFactorSrcRGB(3, srcFunc);
    pContext->setBlendFactorDstRGB(3, dstFunc);
    pContext->setBlendEquationRGB(3, equation);
    sead::Color4f color = sead::Color4f::cWhite;
    getConstantColor(&color, pMaterial);
    pContext->setBlendConstantColor(color);
}

/**
 * @brief Sets up a context drawing alpha-tested models into the G-buffers.
 */
void GBufferArray::setContextMRTAlphaMask(sead::GraphicsContextMRT* pContext) {
    setStencilTest(pContext);
    pContext->setAlphaTestEnable(true);
    pContext->setBlendFactorSrcRGB(0, 2);
    pContext->setBlendFactorSrcA(0, 1);
    pContext->setBlendFactorDstRGB(0, 1);
    pContext->setBlendFactorDstA(0, 1);
    pContext->setBlendEquation(0, 1);
    pContext->setBlendEnableMask(1);
    pContext->setColorMask(0xffff);
}

/**
 * @brief Sets up a context drawing translucent Mii faces.
 */
void GBufferArray::setContextMRTMiiFaceXlu(sead::GraphicsContextMRT* pContext) {
    pContext->setBlendEnableMask(9);
    pContext->setColorMask(0xfff7);
    pContext->setDepthEnable(true, false);
    pContext->setBlendFactorSrcA(0, 1);
    pContext->setBlendFactorDstA(0, 1);
    pContext->setBlendEquationA(0, 1);
    pContext->setBlendFactorSrcRGB(3, 5);
    pContext->setBlendFactorDstRGB(3, 6);
    pContext->setBlendEquationRGB(3, 1);
}

/**
 * @brief Sets up a context writing only depth.
 */
void GBufferArray::setContextMRTOnlyDepth(sead::GraphicsContextMRT* pContext) {
    pContext->setDepthEnable(true, true);
    pContext->setBlendEnableMask(0);
    pContext->setColorMask(0);
}

/**
 * @brief Binds all the G-buffers and applies the opaque G-buffer context.
 */
void GBufferArray::bindRenderBufferAndContextMRT() {
    const agl::TextureData* texture = mGBuffers[cIndex_Albedo].mTexture;
    {
        u32 width = texture->getWidth(0);
        u32 height = texture->getHeight(0);
        agl::RenderBuffer renderBuffer;
        setRenderBufferSize(&renderBuffer, width, height);
        renderBuffer.setRenderTargetColorNullAll();

        for (s32 i = 0; i < cIndex_Num; i++) {
            renderBuffer.setRenderTargetColor(&mGBuffers[i].mRenderTarget, i);
        }

        renderBuffer.setRenderTargetDepth(const_cast<agl::RenderTargetDepth*>(mDepthTarget));
        renderBuffer.bind(GameFrameworkNx::getDrawContext());
        sead::Viewport viewport(renderBuffer);
        viewport.apply(GameFrameworkNx::getDrawContext(), renderBuffer);
    }

    sead::GraphicsContextMRT context;
    setContextMRT(&context);
    context.apply(GameFrameworkNx::getDrawContext());
}

}  // namespace al
