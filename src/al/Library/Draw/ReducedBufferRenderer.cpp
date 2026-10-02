#include "Library/Draw/ReducedBufferRenderer.hpp"

#include <common/aglDrawContext.h>
#include <common/aglGPUMemAddr.h>
#include <common/aglRenderBuffer.h>
#include <common/aglShaderEnum.h>
#include <common/aglShaderLocation.h>
#include <common/aglShaderProgram.h>
#include <common/aglTextureData.h>
#include <gfx/seadColor.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <prim/seadSafeString.h>
#include <utility/aglDynamicTextureAllocator.h>

#include "Library/Debug/Render/RenderBufferAttacher.hpp"
#include "Library/Debug/Render/RenderBufferDepthAttacher.hpp"
#include "Library/Effect/EffectShaderHolder.hpp"
#include "Library/Effect/EffectSystem.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/LiveActor/Common/LiveActorKit.hpp"
#include "Library/Play/Draw/RenderVariables.hpp"
#include "Library/Shader/DeferredRendering/FullScreenTriangle.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/PostProcessing/PostProcessingDrawerUtil.hpp"

namespace al {
namespace {

/**
 * Searches a uniform by name and sets a two component value to it.
 * @param pDrawContext Draw context.
 * @param pProgram Shader program to search the uniform in.
 * @param pName Uniform name.
 * @param rValue Value to set.
 */
inline void setUniformVector2f(agl::DrawContext* pDrawContext, const agl::ShaderProgram* pProgram,
                               const char* pName, const sead::Vector2f& rValue) {
    agl::UniformLocation location(pName);
    location.search(*pProgram);
    location.setUniform(pDrawContext, 2, &rValue);
}

/**
 * Clears the color of a render buffer over its whole area.
 * @param pDrawContext Draw context.
 * @param rRenderBuffer Render buffer to clear.
 * @param rColor Clear color.
 */
inline void clearRenderBuffer(agl::DrawContext* pDrawContext,
                              const agl::RenderBuffer& rRenderBuffer, const sead::Color4f& rColor) {
    rRenderBuffer.fastClear(pDrawContext, 0, 1, rColor, 0.0f, 0, sead::Viewport(rRenderBuffer),
                            true);
}

}  // namespace

/**
 * Gets the reduced buffer shaders from the shader holder.
 * @param pShaderHolder Shader holder to get the shaders from.
 * @param pFullScreenTriangle Full screen triangle used to compose the buffers.
 */
ReducedBufferRenderer::ReducedBufferRenderer(const ShaderHolder* pShaderHolder,
                                             const FullScreenTriangle* pFullScreenTriangle)
    : mFullScreenTriangle(pFullScreenTriangle) {
    mQuarterResBufferShader = pShaderHolder->getShaderProgram("alQuaterResBuffer");
    mQuarterResBufferLinearDepthShader =
        pShaderHolder->getShaderProgram("alQuaterResBufferLinearDepth");
    mQuarterResBufferComposeShader = pShaderHolder->getShaderProgram("alQuaterResBufferCompose");
    mHalfResBufferDepthShader = pShaderHolder->getShaderProgram("alHalfResBufferDepth");
    mHalfResBufferComposeShader = pShaderHolder->getShaderProgram("alHalfResBufferCompose");
    mQuarterResBufferComposeShader2 = pShaderHolder->getShaderProgram("QuaterResBufferCompose");
    mFullResBufferComposeShader = pShaderHolder->getShaderProgram("alFullResBufferCompose");
}

/**
 * Frees all the allocated buffers.
 */
ReducedBufferRenderer::~ReducedBufferRenderer() {
    freeReducedBuffer();
    freeReducedBufferHdr();
    freeReducedDepthBuffers();
}

/**
 * Frees the reduced color buffer.
 */
void ReducedBufferRenderer::freeReducedBuffer() {
    if (mReducedBuffer != nullptr) {
        agl::utl::DynamicTextureAllocator::instance()->free(mReducedBuffer);
        mReducedBuffer = nullptr;
    }
}

/**
 * Frees the reduced HDR color buffer.
 */
void ReducedBufferRenderer::freeReducedBufferHdr() {
    if (mReducedBufferHdr != nullptr) {
        agl::utl::DynamicTextureAllocator::instance()->free(mReducedBufferHdr);
        mReducedBufferHdr = nullptr;
    }
}

/**
 * Frees the reduced depth buffers.
 */
void ReducedBufferRenderer::freeReducedDepthBuffers() {
    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();

    if (mHalfDepthTexture != nullptr) {
        allocator->free(mHalfDepthTexture);
    }

    if (mHalfLinearDepth != nullptr) {
        allocator->free(mHalfLinearDepth);
    }

    mHalfDepthTexture = nullptr;
    mHalfLinearDepth = nullptr;

    if (mQuarterDepthTexture != nullptr) {
        allocator->free(mQuarterDepthTexture);
        mQuarterDepthTexture = nullptr;
    }

    if (mQuarterLinearDepth != nullptr) {
        allocator->free(mQuarterLinearDepth);
        mQuarterLinearDepth = nullptr;
    }
}

/**
 * Allocates the half resolution depth target and linear depth buffer.
 * @param pDrawContext Draw context.
 * @param rSize Full resolution size.
 */
void ReducedBufferRenderer::createReducedDepthBuffers(agl::DrawContext* pDrawContext,
                                                      const sead::Vector2i& rSize) {
    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();
    agl::GPUMemVoidAddr zCullBuffer;
    mHalfDepthTexture =
        allocator->alloc(pDrawContext, "Half Depth Target", agl::TextureFormat(0x3c),
                         rSize.x * 0.5f, rSize.y * 0.5f, 1, &zCullBuffer,
                         agl::utl::DynamicTextureAllocator::AllocateType(0), true, false);
    mHalfDepthTarget.applyTextureData(*mHalfDepthTexture, 0, 0);
    mHalfDepthTarget.setZCullBuffer(agl::GPUMemVoidAddr(zCullBuffer));
    mHalfDepthSampler.applyTextureData(*mHalfDepthTexture);
    mHalfLinearDepth = allocator->alloc(pDrawContext, "Half Linear Depth", agl::TextureFormat(9),
                                        rSize.x * 0.5f, rSize.y * 0.5f, 1, nullptr,
                                        agl::utl::DynamicTextureAllocator::AllocateType(0), true,
                                        false);
}

/**
 * Allocates the reduced color buffer if it is not allocated yet.
 * @param pDrawContext Draw context.
 * @param pName Texture name.
 * @param format Texture format.
 * @param width Texture width.
 * @param height Texture height.
 * @return Reduced color buffer.
 */
agl::TextureData* ReducedBufferRenderer::createReducedBuffer(agl::DrawContext* pDrawContext,
                                                             const char* pName,
                                                             agl::TextureFormat format, s32 width,
                                                             s32 height) {
    if (mReducedBuffer == nullptr) {
        mReducedBuffer = agl::utl::DynamicTextureAllocator::instance()->alloc(
            pDrawContext, pName, format, width, height, 1, nullptr,
            agl::utl::DynamicTextureAllocator::AllocateType(0), true, false);
    }

    return mReducedBuffer;
}

/**
 * Allocates the reduced HDR color buffer if it is not allocated yet.
 * @param pDrawContext Draw context.
 * @param pName Texture name.
 * @param format Texture format.
 * @param width Texture width.
 * @param height Texture height.
 * @return Reduced color buffer.
 */
agl::TextureData* ReducedBufferRenderer::createReducedBufferHdr(agl::DrawContext* pDrawContext,
                                                                const char* pName,
                                                                agl::TextureFormat format,
                                                                s32 width, s32 height) {
    if (mReducedBufferHdr == nullptr) {
        mReducedBufferHdr = agl::utl::DynamicTextureAllocator::instance()->alloc(
            pDrawContext, pName, format, width, height, 1, nullptr,
            agl::utl::DynamicTextureAllocator::AllocateType(0), true, false);
    }
    return mReducedBufferHdr;
}

/**
 * Draws the reduced depth into the half resolution depth target.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param rSrcDepthTarget Full resolution depth target.
 * @param pSrcDepthTexture Full resolution depth texture.
 * @param isOutLinear Whether to also output the half resolution linear depth.
 */
void ReducedBufferRenderer::drawReducedDepth(f32 near, f32 far,
                                             const agl::RenderTargetDepth& rSrcDepthTarget,
                                             const agl::TextureData* pSrcDepthTexture,
                                             bool isOutLinear) {
    drawReducedDepth(near, far, rSrcDepthTarget, pSrcDepthTexture, mHalfDepthTarget,
                     isOutLinear ? mHalfLinearDepth : nullptr);
}

/**
 * Draws the reduced depth into a depth target.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param rSrcDepthTarget Source depth target.
 * @param pSrcDepthTexture Source depth texture.
 * @param rDstDepthTarget Destination depth target.
 * @param pDstLinearDepth Destination linear depth texture, or nullptr.
 */
void ReducedBufferRenderer::drawReducedDepth(f32 near, f32 far,
                                             const agl::RenderTargetDepth& rSrcDepthTarget,
                                             const agl::TextureData* pSrcDepthTexture,
                                             const agl::RenderTargetDepth& rDstDepthTarget,
                                             const agl::TextureData* pDstLinearDepth) {
    agl::DrawContext* drawContext = GameFrameworkNx::getAglDrawContext();
    rSrcDepthTarget.expandHiZBuffer(drawContext);
    rSrcDepthTarget.invalidateGPUCache(drawContext);

    sead::GraphicsContext graphicsContext;
    graphicsContext.setColorMask(0, true, false, false, false);
    graphicsContext.setDepthFunc(8);
    graphicsContext.setDepthEnable(true, true);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(drawContext);

    const char* macros[] = {"IS_OUT_LINEAR", "SAMPLING_TYPE"};
    const char* values[] = {"0", "0"};

    if (pDstLinearDepth != nullptr) {
        values[0] = "1";
        values[1] = "1";
    }

    const agl::ShaderProgram* program =
        mHalfResBufferDepthShader->searchVariation(2, macros, values);
    program->activate(drawContext, true);

    agl::RenderBuffer renderBuffer;
    RenderBufferDepthAttacher attacher(&renderBuffer, &rDstDepthTarget,
                                       rDstDepthTarget.getZCullBuffer(), pDstLinearDepth,
                                       nullptr, nullptr, nullptr);

    sead::Vector2f texelSize(1.0f / rSrcDepthTarget.getWidth(0),
                             1.0f / rSrcDepthTarget.getHeight(0));
    setUniformVector2f(drawContext, program, "uTexelSize", texelSize);

    setPostProcessingUniform(drawContext, program, "uNear", near);
    setPostProcessingUniform(drawContext, program, "uFar", far);
    setPostProcessingUniform(drawContext, program, "uRange", far - near);
    setPostProcessingUniform(drawContext, program, "uInvRange", 1.0f / (far - near));

    agl::TextureSampler sampler;
    agl::SamplerLocation location("uDepth");
    location.search(*program);
    sampler.applyTextureData(*pSrcDepthTexture);
    sampler.activate(drawContext, location, -1, false);

    drawPostProcessingQuad(drawContext);
    rDstDepthTarget.expandHiZBuffer(drawContext);
}

/**
 * Composes a reduced color buffer onto a full resolution target.
 * @param pTarget Full resolution target texture.
 * @param pViewDepth Full resolution view depth.
 * @param pHalfViewDepth Half resolution view depth.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param isHdr Whether to compose the HDR buffer.
 * @param isAdjust Whether to adjust the composition with the depth buffers.
 */
void ReducedBufferRenderer::drawCompose(const agl::TextureData* pTarget,
                                        const agl::TextureData* pViewDepth,
                                        const agl::TextureData* pHalfViewDepth, f32 near, f32 far,
                                        bool isHdr, bool isAdjust) {
    if (isHdr) {
        if (mReducedBufferHdr == nullptr) {
            return;
        }
    } else if (mReducedBuffer == nullptr) {
        return;
    }

    agl::DrawContext* drawContext = GameFrameworkNx::getAglDrawContext();
    agl::RenderBuffer renderBuffer;
    RenderBufferAttacher attacher(&renderBuffer, pTarget, nullptr, nullptr, nullptr, nullptr);

    const char* macros[] = {"IS_ADJUST"};
    const char* values[] = {"IS_ADJUST"};
    s32 index = ShaderSearchImpl::searchMacroIndex(macros, "IS_ADJUST");

    if (index != -1) {
        values[index] = isAdjust ? "1" : "0";
    }

    const agl::ShaderProgram* program =
        mHalfResBufferComposeShader->searchVariation(1, macros, values);
    program->activate(drawContext, true);

    if (isAdjust) {
        activateAdjustParam(program, pViewDepth, pHalfViewDepth);
    }

    setPostProcessingUniform(drawContext, program, "uNear", near);
    setPostProcessingUniform(drawContext, program, "uInvRange", 1.0f / (far - near));

    agl::SamplerLocation location("cReduceBuf");
    location.search(*program);
    agl::TextureSampler sampler;
    sampler.applyTextureData(isHdr ? *mReducedBufferHdr : *mReducedBuffer);
    sampler.activate(drawContext, location, -1, false);
    mFullScreenTriangle->drawFar(drawContext);
}

/**
 * Activates the depth samplers and uniforms used to adjust the composition.
 * @param pProgram Compose shader program.
 * @param pViewDepth Full resolution view depth.
 * @param pHalfViewDepth Half resolution view depth.
 */
void ReducedBufferRenderer::activateAdjustParam(const agl::ShaderProgram* pProgram,
                                                const agl::TextureData* pViewDepth,
                                                const agl::TextureData* pHalfViewDepth) {
    agl::DrawContext* drawContext = GameFrameworkNx::getAglDrawContext();
    agl::SamplerLocation viewDepthLocation("cViewDepth");
    viewDepthLocation.search(*pProgram);
    agl::SamplerLocation halfViewDepthLocation("cHalfViewDepth");
    halfViewDepthLocation.search(*pProgram);

    agl::TextureSampler viewDepthSampler;
    viewDepthSampler.setFilter(0, 0, 0);
    viewDepthSampler.applyTextureData(*pViewDepth);
    viewDepthSampler.activate(drawContext, viewDepthLocation, -1, false);

    agl::TextureSampler halfViewDepthSampler;
    halfViewDepthSampler.setFilter(0, 0, 0);
    halfViewDepthSampler.applyTextureData(*pHalfViewDepth);
    halfViewDepthSampler.activate(drawContext, halfViewDepthLocation, -1, false);

    sead::Vector2f texel(pViewDepth->getWidth(0) * 0.5f, pViewDepth->getHeight(0) * 0.5f);
    sead::Vector2f texelSizeInv(1.0f / texel.x, 1.0f / texel.y);

    setUniformVector2f(drawContext, pProgram, "uReducedBufferTexel", texel);
    setUniformVector2f(drawContext, pProgram, "uReducedBufferTexelSizeInv", texelSizeInv);
    setPostProcessingUniform(drawContext, pProgram, "uReducedBufferDepthCoeff", 2.0f);
}

/**
 * Composes a full resolution source texture onto the bound render buffer.
 * @param pSource Source texture.
 * @param near Near clip distance.
 * @param far Far clip distance.
 */
void ReducedBufferRenderer::drawComposeFullScreen(const agl::TextureData* pSource, f32 near,
                                                  f32 far) {
    agl::DrawContext* drawContext = GameFrameworkNx::getAglDrawContext();

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setAlphaTestEnable(false);
    graphicsContext.setCullingMode(0);
    graphicsContext.setBlendEnable(false);
    graphicsContext.setBlendFactorSrcRGB(0, 2);
    graphicsContext.setBlendFactorDstRGB(0, 1);
    graphicsContext.setBlendEquationRGB(0, 1);
    graphicsContext.apply(drawContext);

    const char* macros[] = {"IS_ADJUST"};
    const char* values[] = {"IS_ADJUST"};
    s32 index = ShaderSearchImpl::searchMacroIndex(macros, "IS_ADJUST");

    if (index != -1) {
        values[index] = "0";
    }

    const agl::ShaderProgram* program =
        mFullResBufferComposeShader->searchVariation(1, macros, values);
    program->activate(drawContext, true);

    setPostProcessingUniform(drawContext, program, "uNear", near);
    setPostProcessingUniform(drawContext, program, "uInvRange", 1.0f / (far - near));

    agl::SamplerLocation location("cSourceTexture");
    location.search(*program);
    agl::TextureSampler sampler;
    sampler.applyTextureData(*pSource);
    sampler.activate(drawContext, location, -1, false);
    mFullScreenTriangle->drawFar(drawContext);
}

/**
 * Renders effects into a reduced color buffer and composes it onto the color buffer.
 * @param rRenderVariables Render variables of the view.
 * @param pViewDepth Full resolution view depth.
 * @param pTarget Reduced color buffer to render the effects into.
 * @param drawFunc Effect draw function.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param fovy Vertical field of view.
 * @param isUseLinearDepth Whether the effects use the half resolution linear depth.
 * @param isHdr Whether the HDR buffer is rendered.
 */
void ReducedBufferRenderer::RenderEffects(const RenderVariables& rRenderVariables,
                                          const agl::TextureData* pViewDepth,
                                          const agl::TextureData* pTarget, DrawEffectFunc drawFunc,
                                          const sead::Matrix44f& rProjMtx,
                                          const sead::Matrix34f& rViewMtx, f32 near, f32 far,
                                          f32 fovy, bool isUseLinearDepth, bool isHdr) {
    agl::DrawContext* drawContext = GameFrameworkNx::getAglDrawContext();
    agl::RenderBuffer renderBuffer;

    {
        RenderBufferAttacher attacher(&renderBuffer, pTarget, nullptr, nullptr, nullptr,
                                      &mHalfDepthTarget);
        clearRenderBuffer(drawContext, renderBuffer, sead::Color4f(0.0f, 0.0f, 0.0f, 1.0f));
        tryChangeShaderMode(drawContext, agl::cShaderMode_UniformBlock);

        const EffectSystem* effectSystem = rRenderVariables.getLiveActorKit()->getEffectSystem();
        effectSystem->getShaderHolder()->setupTextureDepth(
            isUseLinearDepth ? mHalfLinearDepth : mHalfDepthTexture);
        drawFunc(rRenderVariables.getLiveActorKit()->getEffectSystem(), rProjMtx, rViewMtx, near,
                 far, fovy);
    }

    rRenderVariables.bindRenderBuffer();

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setAlphaTestEnable(false);
    graphicsContext.setCullingMode(0);
    graphicsContext.setBlendEnable(true);
    graphicsContext.setBlendFactorSrcRGB(0, 2);
    graphicsContext.setBlendFactorDstRGB(0, 5);
    graphicsContext.setBlendEquationRGB(0, 1);
    graphicsContext.apply(drawContext);

    drawCompose(rRenderVariables.getColorTexture(), pViewDepth, mHalfLinearDepth, near, far,
                isHdr, true);
}

/**
 * Draws the reduced effects of a view.
 * @param rRenderVariables Render variables of the view.
 * @param pViewDepth Full resolution view depth.
 * @param rProjMtx Projection matrix.
 * @param rViewMtx View matrix.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param fovy Vertical field of view.
 * @param isUseLinearDepth Whether the effects use the half resolution linear depth.
 * @return Whether anything was drawn.
 */
bool ReducedBufferRenderer::draw(const RenderVariables& rRenderVariables,
                                 const agl::TextureData* pViewDepth,
                                 const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx,
                                 f32 near, f32 far, f32 fovy, bool isUseLinearDepth) {
    if (!mIsEnableHdr && !mIsEnable) {
        return false;
    }

    agl::DrawContext* drawContext = GameFrameworkNx::getAglDrawContext();
    createReducedDepthBuffers(drawContext,
                              *reinterpret_cast<const sead::Vector2i*>(&rRenderVariables.mWidth));
    drawReducedDepth(near, far, rRenderVariables.getDepthTarget(),
                     rRenderVariables.getDepthTexture(), isUseLinearDepth);

    s32 width = rRenderVariables.getWidth() / 2;
    s32 height = rRenderVariables.getHeight() / 2;

    if (mIsEnable) {
        const agl::TextureData* buffer = createReducedBuffer(
            drawContext, "Reduced Res", agl::TextureFormat(29), width, height);
        RenderEffects(rRenderVariables, pViewDepth, buffer,
                      alEffectSystemFunction::drawEffectForwardReduced, rProjMtx, rViewMtx, near,
                      far, fovy, isUseLinearDepth, false);
        freeReducedBuffer();
    }

    if (mIsEnableHdr) {
        const agl::TextureData* buffer = createReducedBufferHdr(
            drawContext, "Reduced Res HDR", agl::TextureFormat(43), width, height);
        RenderEffects(rRenderVariables, pViewDepth, buffer,
                      alEffectSystemFunction::drawEffectForwardReducedHDR, rProjMtx, rViewMtx,
                      near, far, fovy, isUseLinearDepth, true);
        freeReducedBufferHdr();
    }

    freeReducedDepthBuffers();
    return true;
}

}  // namespace al
