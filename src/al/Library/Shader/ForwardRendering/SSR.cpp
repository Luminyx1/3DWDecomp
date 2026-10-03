#include "Library/Shader/ForwardRendering/SSR.hpp"

#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <common/aglShaderLocation.h>
#include <common/aglShaderProgram.h>
#include <common/aglTextureData.h>
#include <common/aglTextureSampler.h>
#include <gfx/seadCamera.h>
#include <gfx/seadColor.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <nvn/nvn_FuncPtrInline.h>
#include <utility/aglDynamicTextureAllocator.h>

#include "Library/Debug/Render/RenderBufferAttacher.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/FullScreenQuadModel.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"

namespace al {
namespace {
const sead::Vector2f cReflectionParam = {600.0f, 1.08f};

const UniformBlockLayout cUniformBlockLayout[] = {
    {0, agl::UniformBlock::cType_Vec3, 1},
    {1, agl::UniformBlock::cType_Float, 1},
    {2, agl::UniformBlock::cType_Float, 1},
    {3, agl::UniformBlock::cType_Float, 1},
    {4, agl::UniformBlock::cType_Float, 1},
    {5, agl::UniformBlock::cType_Float, 1},
    {6, agl::UniformBlock::cType_Vec2, 1},
    {7, agl::UniformBlock::cType_Vec2, 1},
    {8, agl::UniformBlock::cType_Bool, 1},
    {9, agl::UniformBlock::cType_Float, 1},
    {10, agl::UniformBlock::cType_Float, 1},
    {11, agl::UniformBlock::cType_Float, 1},
    {12, agl::UniformBlock::cType_Float, 1},
    {13, agl::UniformBlock::cType_Float, 1},
    {14, agl::UniformBlock::cType_Vec2, 1},
};

/**
 * Starts a named debug group on a command buffer.
 * @param pCommandBuffer Command buffer.
 * @param pName Group name.
 */
inline void pushDebugGroup(NVNcommandBuffer* pCommandBuffer, const char* pName) {
    reinterpret_cast<void (*)(NVNcommandBuffer*, const char*)>(
        pfnc_nvnCommandBufferPushDebugGroup)(pCommandBuffer, pName);
}

/**
 * Binds the current block of a uniform block to a shader location.
 * @param pBlock Uniform block.
 * @param location Shader location to bind the block to.
 */
inline void activateUniformBlock(const UniformBlock* pBlock, agl::ShaderLocation location) {
    pBlock->activate(GameFrameworkNx::getAglDrawContext(), location);
}

/**
 * Clears the color of a render buffer to black over its whole area.
 * @param rRenderBuffer Render buffer to clear.
 */
inline void clearRenderBuffer(const agl::RenderBuffer& rRenderBuffer) {
    rRenderBuffer.fastClear(GameFrameworkNx::getAglDrawContext(), 0, 1, sead::Color4f::cBlack,
                            0.0f, 0, sead::Viewport(rRenderBuffer), true);
}

/**
 * Sets the filtering used to sample the reflection textures.
 * @param pSampler Sampler to set up.
 */
inline void setupSampler(agl::TextureSampler* pSampler) {
    pSampler->setFilter(1, 1, 2);
    pSampler->setWrap(7, 7, 7);
}

/**
 * Searches a uniform by name and sets a two component value to it.
 * @param pDrawContext Draw context.
 * @param pProgram Shader program to search the uniform in.
 * @param pName Uniform name.
 * @param rValue Value to set.
 */
inline void setUniform(agl::DrawContext* pDrawContext, const agl::ShaderProgram* pProgram,
                       const char* pName, const sead::Vector2f& rValue) {
    agl::UniformLocation location(pName);
    location.search(*pProgram);
    location.setUniform(pDrawContext, 2, &rValue);
}

/**
 * Searches a uniform by name and sets a float value to it.
 * @param pDrawContext Draw context.
 * @param pProgram Shader program to search the uniform in.
 * @param pName Uniform name.
 * @param value Value to set.
 */
inline void setUniform(agl::DrawContext* pDrawContext, const agl::ShaderProgram* pProgram,
                       const char* pName, f32 value) {
    agl::UniformLocation location(pName);
    location.search(*pProgram);
    location.setUniform(pDrawContext, value);
}
}  // namespace

/**
 * Creates the screen space reflection uniform block and the full screen quad.
 * @param pInfo Graphics system info.
 * @param bufferNum Unused number of buffers.
 */
SSR::SSR(GraphicsSystemInfo* pInfo, s32 bufferNum) : mGraphicsSystemInfo(pInfo) {
    mUniformBlock = createUniformBlock(cUniformBlockLayout, 15, nullptr, 2);
    mFullScreenQuadModel = new FullScreenQuadModel();
    mIsEnable = false;
}

/**
 * Destroys the full screen quad and the uniform block.
 */
SSR::~SSR() {
    if (mFullScreenQuadModel != nullptr) {
        delete mFullScreenQuadModel;
        mFullScreenQuadModel = nullptr;
    }

    if (mUniformBlock != nullptr) {
        delete mUniformBlock;
        mUniformBlock = nullptr;
    }
}

/**
 * Sets the camera the reflections are traced from.
 * @param pCamera Camera.
 */
void SSR::setCam(const sead::LookAtCamera* pCamera) {
    mCamera = pCamera;
}

/**
 * Traces the reflections, blurs them and combines them with the scene.
 * @param pTexture Scene color texture.
 */
void SSR::draw(const agl::TextureData* pTexture) const {
    if (!mIsEnable) {
        return;
    }

    sead::GraphicsContext graphicsContext;
    const agl::RenderBuffer* renderBuffer =
        GameFrameworkNx::getAglDrawContext()->getBoundRenderBuffer();
    mUniformBlock->swap();
    pushDebugGroup(GameFrameworkNx::getDrawContext()->getNvnCommandBuffer(), "SSR");

    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setCullingMode(0);
    graphicsContext.setBlendEnableMask(0);
    graphicsContext.setColorMask(0xf);
    graphicsContext.setStencilTestEnable(false);
    graphicsContext.setStencilTestRef(1);
    graphicsContext.setStencilTestMask(0xff);
    graphicsContext.setStencilOp(1, 1, 3);
    graphicsContext.setStencilTestFunc(3);
    graphicsContext.setStencilWriteMask(0);
    graphicsContext.apply(GameFrameworkNx::getDrawContext());

    const agl::ShaderProgram* program = ShaderHolder::instance()->getShaderProgram("SSR");

    if (program != nullptr) {
        agl::utl::DynamicTextureAllocator* allocator =
            agl::utl::DynamicTextureAllocator::instance();

        agl::SamplerLocation sceneLocation("cScene");
        sceneLocation.search(*program);
        agl::SamplerLocation normalLocation("cNormal");
        normalLocation.search(*program);
        agl::SamplerLocation depthLocation("cViewDepth");
        depthLocation.search(*program);

        agl::TextureSampler sceneSampler;
        sceneSampler.applyTextureData(*pTexture);
        setupSampler(&sceneSampler);
        sceneSampler.activate(GameFrameworkNx::getAglDrawContext(), sceneLocation, -1, false);

        mGraphicsSystemInfo->getDrawGBufferArray()->getGBufNrmViewTex();
        mGraphicsSystemInfo->getDrawGBufferArray()->getGBufDepthViewTex();
        mGraphicsSystemInfo->getDrawGBufferArray()->activateSamplerNrmView(normalLocation);
        mGraphicsSystemInfo->getDrawGBufferArray()->activateSamplerNearestDepthView(depthLocation);

        UniformBlockSetter setter(mUniformBlock, 0);
        mUniformBlock->setData(0, &mCamera->getPos(), 0, 1);
        mUniformBlock->setValue(1, 100000.0f);
        mUniformBlock->setValue(2, 0.115f);
        mUniformBlock->setValue(3, _1c);
        mUniformBlock->setValue(4, 0.01f);
        mUniformBlock->setValue(5, 6.0f);
        mUniformBlock->setData(6, &cReflectionParam, 0, 1);
        mUniformBlock->setValue(8, 0);
        mUniformBlock->setValue(9, 2120.0f);
        mUniformBlock->setValue(10, 175000.0f);
        mUniformBlock->setValue(11, 0.7f);
        mUniformBlock->setValue(12, 223200.0f);
        mUniformBlock->setValue(13, 15.0f);
        mUniformBlock->setValueRef(14, sead::Vector2f(0.75f, 0.85f));

        const agl::TextureData* albedoTexture =
            mGraphicsSystemInfo->getDrawGBufferArray()->getGBufAlbedoTex();
        u32 width = albedoTexture->getWidth(0) / 3;
        sead::Vector2f size(width, albedoTexture->getHeight(0) / 3);

        agl::TextureData* reflectionTexture = allocator->alloc(
            GameFrameworkNx::getAglDrawContext(), "reflection_texture",
            agl::TextureFormat(albedoTexture->getTextureFormat()), width, size.y, 1, nullptr,
            agl::utl::DynamicTextureAllocator::cAllocateType_0, true, false);

        agl::ShaderLocation blockLocation;
        blockLocation.setLocation(agl::cShaderType_Vertex, 2);
        blockLocation.setLocation(agl::cShaderType_Fragment, 2);
        blockLocation.setLocation(agl::cShaderType_Geometry, 2);

        agl::TextureData* filterTexture = allocator->alloc(
            GameFrameworkNx::getAglDrawContext(), "reflection_texture_filter",
            agl::TextureFormat(albedoTexture->getTextureFormat()), width, size.y, 1, nullptr,
            agl::utl::DynamicTextureAllocator::cAllocateType_0, true, false);
        agl::TextureData* filterTexture2 = allocator->alloc(
            GameFrameworkNx::getAglDrawContext(), "reflection_texture_filter2",
            agl::TextureFormat(albedoTexture->getTextureFormat()), width, size.y, 1, nullptr,
            agl::utl::DynamicTextureAllocator::cAllocateType_0, true, false);

        mUniformBlock->setValueRef(7, sead::Vector2f(size.x, size.y));
        activateUniformBlock(mUniformBlock, blockLocation);

        {
            agl::RenderBuffer reflectionBuffer;
            RenderBufferAttacher attacher(&reflectionBuffer, reflectionTexture, nullptr, nullptr,
                                          nullptr, nullptr);
            reflectionBuffer.adjustPhysicalAreaAndVirtualSizeFromColorTarget(0);
            clearRenderBuffer(reflectionBuffer);
            reflectionBuffer.bind(GameFrameworkNx::getDrawContext());
            program->activate(GameFrameworkNx::getAglDrawContext(), true);
            mFullScreenQuadModel->drawQuad();
        }

        applyBlur(filterTexture, reflectionTexture, width, size.y,
                  "Water Rendering:Deferred(SSR - Blur Horizontal)", BlurType_Horizontal);
        applyBlur(filterTexture2, filterTexture, width, size.y,
                  "Water Rendering:Deferred(SSR - Blur Vertical)", BlurType_Vertical);

        const agl::ShaderProgram* combiner =
            ShaderHolder::instance()->getShaderProgram("WaterCombiner");

        if (combiner != nullptr) {
            graphicsContext.setStencilTestEnable(true);
            graphicsContext.apply(GameFrameworkNx::getDrawContext());

            sead::Viewport viewport(*renderBuffer);
            viewport.apply(GameFrameworkNx::getDrawContext(), *renderBuffer);
            renderBuffer->bind(GameFrameworkNx::getDrawContext());

            agl::SamplerLocation reflectionLocation("cReflectionSampler");
            reflectionLocation.search(*combiner);
            agl::SamplerLocation sceneSamplerLocation("cSceneSampler");
            sceneSamplerLocation.search(*combiner);

            agl::TextureSampler reflectionSampler;
            agl::TextureSampler combineSceneSampler;
            reflectionSampler.applyTextureData(*filterTexture2);
            setupSampler(&reflectionSampler);
            reflectionSampler.activate(GameFrameworkNx::getAglDrawContext(), reflectionLocation,
                                       -1, false);
            combineSceneSampler.applyTextureData(*pTexture);
            setupSampler(&combineSceneSampler);
            combineSceneSampler.activate(GameFrameworkNx::getAglDrawContext(),
                                         sceneSamplerLocation, -1, false);

            combiner->activate(GameFrameworkNx::getAglDrawContext(), true);
            mFullScreenQuadModel->drawQuad();
        }

        allocator->free(reflectionTexture);
        allocator->free(filterTexture);
        allocator->free(filterTexture2);
    }

    nvnCommandBufferPopDebugGroup(GameFrameworkNx::getDrawContext()->getNvnCommandBuffer());
}

/**
 * Blurs a texture in one direction.
 * @param pDst Texture to render to.
 * @param pSrc Texture to blur.
 * @param width Texture width.
 * @param height Texture height.
 * @param pName Debug group name.
 * @param blurType Blur direction.
 */
void SSR::applyBlur(agl::TextureData* pDst, agl::TextureData* pSrc, u32 width, u32 height,
                    const char* pName, BlurType blurType) const {
    pushDebugGroup(GameFrameworkNx::getDrawContext()->getNvnCommandBuffer(), pName);
    const agl::ShaderProgram* program =
        ShaderHolder::instance()->getShaderProgram("UpscaleBlurTexture")->getVariation(blurType);

    agl::RenderBuffer renderBuffer;
    RenderBufferAttacher attacher(&renderBuffer, pDst, nullptr, nullptr, nullptr, nullptr);
    renderBuffer.adjustPhysicalAreaAndVirtualSizeFromColorTarget(0);
    clearRenderBuffer(renderBuffer);
    renderBuffer.bind(GameFrameworkNx::getDrawContext());

    agl::SamplerLocation location("cTextureSampler");
    location.search(*program);
    agl::TextureSampler sampler;
    sampler.applyTextureData(*pSrc);
    setupSampler(&sampler);
    sampler.activate(GameFrameworkNx::getAglDrawContext(), location, -1, false);

    setUniform(GameFrameworkNx::getAglDrawContext(), program, "uTextureDimensions",
               sead::Vector2f(width, height));
    setUniform(GameFrameworkNx::getAglDrawContext(), program, "uStepSize", 0.002f);
    setUniform(GameFrameworkNx::getAglDrawContext(), program, "uSamples", 2.0f);

    program->activate(GameFrameworkNx::getAglDrawContext(), true);
    mFullScreenQuadModel->drawQuad();
    nvnCommandBufferPopDebugGroup(GameFrameworkNx::getDrawContext()->getNvnCommandBuffer());
}

/**
 * Binds a block of a uniform block to the uniform block location of a shader program.
 * @param pBlock Uniform block.
 * @param pContext Draw context.
 * @param rProgram Shader program.
 * @param pName Name of the uniform block in the shader program.
 * @param index Index of the block to bind.
 */
void setUniformBlockToShader(UniformBlock* pBlock, agl::DrawContext* pContext,
                             const agl::ShaderProgram& rProgram, const char* pName, s32 index) {
    agl::UniformBlockLocation location(pName);
    location.search(rProgram);
    u64 address =
        nvnBufferGetAddress(pBlock->getNvnBuffer()) + pBlock->getCurrentBlockOffset(index);
    pBlock->setUniform(pContext, address, location, 0, pBlock->getBlockSize());
}
}  // namespace al
