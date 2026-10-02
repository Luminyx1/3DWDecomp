#include "Library/Draw/DeferredRendering.hpp"

#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <common/aglRenderTarget.h>
#include <common/aglShaderLocation.h>
#include <common/aglShaderProgram.h>
#include <common/aglTextureSampler.h>
#include <cull/aglViewFrustumCulling.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <lighting/aglLightPrePass.h>
#include <math/seadMathCalcCommon.h>
#include <shadow/aglDepthShadow.h>
#include <shadow/aglPrimitiveOcclusion.h>
#include <shadow/aglSSAO.h>
#include <shadow/aglShadowMap.h>
#include <shadow/aglShadowPrePass.h>
#include <utility/aglDynamicTextureAllocator.h>

#include "Library/Debug/Render/RenderBufferAttacher.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Fog/FogDirector.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Light/PrePassLightKeeper.hpp"
#include "Library/LiveActor/Common/LiveActorKit.hpp"
#include "Library/Model/SimpleModelEnv.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDirector.hpp"
#include "Library/Shader/DeferredRendering/FullScreenQuadModel.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Library/Shadow/Depth/DepthShadowDrawer.hpp"
#include "Library/Shadow/ShadowDirector.hpp"
#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace {
/**
 * @brief Layout of the deferred shading scene uniform block: nine floats.
 */
const al::UniformBlockLayout cSceneUboLayout[] = {
    {0, agl::UniformBlock::cType_Float, 1}, {1, agl::UniformBlock::cType_Float, 1},
    {2, agl::UniformBlock::cType_Float, 1}, {3, agl::UniformBlock::cType_Float, 1},
    {4, agl::UniformBlock::cType_Float, 1}, {5, agl::UniformBlock::cType_Float, 1},
    {6, agl::UniformBlock::cType_Float, 1}, {7, agl::UniformBlock::cType_Float, 1},
    {8, agl::UniformBlock::cType_Float, 1},
};

/**
 * @brief Gets the draw context of the game framework.
 * @return The agl draw context.
 */
agl::DrawContext* getDrawContext() {
    return al::GameFrameworkNx::getAglDrawContext();
}
}  // namespace

namespace al {

/**
 * @brief Returns whether the depth shadow map is used this frame.
 * @return True when the depth shadow map is used.
 */
bool DeferredRendering::isUsingDepthShadowMap() const {
    return mIsUsingDepthShadowMap;
}

/**
 * @brief Creates the scene uniform blocks, gets the deferred shaders and creates the shadow
 * pre-pass.
 * @param pGraphicsSystemInfo Graphics system info.
 * @param viewNum Number of views rendered.
 */
DeferredRendering::DeferredRendering(GraphicsSystemInfo* pGraphicsSystemInfo, s32 viewNum)
    : mGraphicsSystemInfo(pGraphicsSystemInfo) {
    mSceneUbo = createUniformBlock(cSceneUboLayout, 9, nullptr, 2);
    mMakeHalfTextureUbo = createMakeHalfTextureUbo();
    mDeferredShadingShader = ShaderHolder::sInstance->getShaderProgram("DeferredShading");
    mFillDeferredSkyShader = ShaderHolder::sInstance->getShaderProgram("FillDeferredSky");
    mMakeHalfDepthTextureShader = ShaderHolder::sInstance->getShaderProgram("MakeHalfDepthTexture");
    mFullScreenQuadModel = new FullScreenQuadModel();
    mShadowPrePass = new agl::sdw::ShadowPrePass();
    mShadowPrePass->initialize(viewNum, nullptr);
}

/**
 * @brief Destroys the shadow pre-pass, the quad model and the uniform blocks.
 */
DeferredRendering::~DeferredRendering() {
    if (mShadowPrePass != nullptr) {
        delete mShadowPrePass;
        mShadowPrePass = nullptr;
    }

    if (mFullScreenQuadModel != nullptr) {
        delete mFullScreenQuadModel;
        mFullScreenQuadModel = nullptr;
    }

    if (mMakeHalfTextureUbo != nullptr) {
        delete mMakeHalfTextureUbo;
        mMakeHalfTextureUbo = nullptr;
    }

    if (mSceneUbo != nullptr) {
        delete mSceneUbo;
        mSceneUbo = nullptr;
    }
}

/**
 * @brief Computes the shadow pre-pass for the view, stores the view matrices for the light
 * pre-pass, then clears and binds the G-buffers.
 * @param width Buffer width (unused).
 * @param height Buffer height (unused).
 * @param pGBufferArray G-buffers to render to.
 * @param viewIndex Index of the rendered view.
 * @param rViewMtx View matrix.
 * @param rProjMtx Projection matrix.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param fovy Vertical field of view.
 * @param aspect Aspect ratio.
 * @param rOffset Projection offset.
 * @param pLiveActorKit Live actor kit of the scene.
 * @param pDepthShadowDrawer Depth shadow drawer (unused).
 */
void DeferredRendering::prepareRenderGBuffer(s32 width, s32 height, GBufferArray* pGBufferArray,
                                             s32 viewIndex, const sead::Matrix34f& rViewMtx,
                                             const sead::Matrix44f& rProjMtx, f32 near, f32 far,
                                             f32 fovy, f32 aspect, const sead::Vector2f& rOffset,
                                             LiveActorKit* pLiveActorKit,
                                             DepthShadowDrawer* pDepthShadowDrawer) {
    f32 tanHalfFovy = tanf(fovy * 0.5f);
    f32 tanHalfFovyAspect = tanHalfFovy * aspect;
    ShadowDirector* shadowDirector =
        pLiveActorKit->getGraphicsSystemInfo()->getShadowDirector();

    if (shadowDirector->isEnableShadowPrePass()) {
        agl::sdw::DepthShadow* depthShadow = shadowDirector->getDepthShadow();
        sead::Matrix44f shadowMtx[3];
        f32 nearFar[3];
        s32 cascadeNum = depthShadow->getCascadeNumParam();

        for (s32 i = 0; i < cascadeNum; i++) {
            shadowMtx[i] = depthShadow->getUnit(i).getTexMtx();
            nearFar[i] = 100000.0f;
        }

        agl::cull::ViewFrustumCulling culling(rViewMtx, rProjMtx, near, far, fovy, aspect,
                                              rOffset);
        mShadowPrePass->calcGPU(viewIndex, rViewMtx, rProjMtx, shadowMtx, nearFar, near, far,
                                tanHalfFovy, aspect, culling);
    }

    sead::Matrix44f viewProjMtx;
    viewProjMtx.setMul(rProjMtx, rViewMtx);
    mGraphicsSystemInfo->getPrePassLightKeeper()->mDrawViewMtx = rViewMtx;
    mGraphicsSystemInfo->getPrePassLightKeeper()->mDrawViewProjMtx = viewProjMtx;

    PrePassLightKeeper* prePassLightKeeper = mGraphicsSystemInfo->getPrePassLightKeeper();
    prePassLightKeeper->mDrawNear = near;
    prePassLightKeeper->mDrawFar = far;
    prePassLightKeeper->mDrawTanHalfFovy = tanHalfFovy;
    prePassLightKeeper->mDrawTanHalfFovyAspect = tanHalfFovyAspect;

    pGBufferArray->clearGBuffer();
    pGBufferArray->bindRenderBufferAndContextMRT();
}

/**
 * @brief Restores the render state after the G-buffer pass and waits for its textures.
 */
void DeferredRendering::endRenderGBuffer() {
    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setColorMask(0u);
    graphicsContext.setBlendEnableMask(0);
    graphicsContext.setCullingMode(2);
    graphicsContext.apply(getDrawContext());
    getDrawContext()->barrierTexture(1);
}

/**
 * @brief Draws the light pre-pass, the shadow pre-pass and the primitive occlusion.
 * @param pLiveActorKit Live actor kit of the scene.
 * @param pGBufferArray G-buffers of the view.
 * @param viewIndex Index of the rendered view.
 * @param rRenderBuffer Render buffer of the view (unused).
 * @param rDepthTarget Depth target of the view.
 * @param shaderMode Current shader mode.
 * @return The shader mode after drawing.
 */
agl::ShaderMode DeferredRendering::drawPrePass(LiveActorKit* pLiveActorKit,
                                               GBufferArray* pGBufferArray, s32 viewIndex,
                                               const agl::RenderBuffer& rRenderBuffer,
                                               const agl::RenderTargetDepth& rDepthTarget,
                                               agl::ShaderMode shaderMode) {
    GraphicsSystemInfo* graphicsSystemInfo = pLiveActorKit->getGraphicsSystemInfo();
    PrePassLightKeeper* prePassLightKeeper = graphicsSystemInfo->getPrePassLightKeeper();
    prePassLightKeeper->updateViewGPU(viewIndex, graphicsSystemInfo->getShadowDirector());
    shaderMode = prePassLightKeeper->drawLpp(viewIndex, pGBufferArray, rDepthTarget, shaderMode);

    ShadowDirector* shadowDirector =
        pLiveActorKit->getGraphicsSystemInfo()->getShadowDirector();

    if (shadowDirector->isEnableShadowPrePass()) {
        rDepthTarget.expandHiZBuffer(getDrawContext());
        agl::TextureSampler staticShadowMap;
        agl::sdw::DepthShadow* depthShadow = shadowDirector->getDepthShadow();
        s32 cascadeNum = depthShadow->getCascadeNumParam();
        mShadowPrePass->draw(getDrawContext(), viewIndex, &rDepthTarget, &rDepthTarget,
                             &rDepthTarget,
                             &depthShadow->getShadowMap().getDepthSampler()->getTextureData(),
                             &staticShadowMap, cascadeNum);
    }

    agl::sdw::PrimitiveOcclusion* primitiveOcclusion =
        pLiveActorKit->getGraphicsSystemInfo()->mPrimitiveOcclusion;

    if (primitiveOcclusion != nullptr) {
        shaderMode = primitiveOcclusion->drawPrecomputeSphereDo(viewIndex, shaderMode);

        {
            agl::RenderBuffer renderBuffer;
            RenderBufferAttacher attacher(&renderBuffer, pGBufferArray->getGBufAlbedoTex(),
                                          nullptr, nullptr, nullptr, nullptr);
            sead::GraphicsContext graphicsContext;
            graphicsContext.setColorMask(0, false, false, false, true);
            graphicsContext.setDepthEnable(true, false);
            graphicsContext.setDepthFunc(5);
            graphicsContext.setCullingMode(1);
            graphicsContext.setBlendEnable(true);
            graphicsContext.setBlendFactorSrcRGB(0, 9);
            graphicsContext.setBlendFactorSrcA(0, 2);
            graphicsContext.setBlendFactorDstRGB(0, 1);
            graphicsContext.setBlendFactorDstA(0, 2);
            graphicsContext.setBlendEquationRGB(0, 1);
            graphicsContext.setBlendEquationA(0, 5);
            graphicsContext.apply(getDrawContext());
            shaderMode = primitiveOcclusion->drawDo(viewIndex, *pGBufferArray->getGBufNrmViewTex(),
                                                    *pGBufferArray->getGBufDepthViewTex(), true,
                                                    shaderMode);
        }

        shaderMode = primitiveOcclusion->drawAo(viewIndex, *pGBufferArray->getGBufNrmViewTex(),
                                                *pGBufferArray->getGBufDepthViewTex(), true,
                                                shaderMode);
        sead::GraphicsContext graphicsContext;
        graphicsContext.setBlendEnable(false);
        graphicsContext.apply(getDrawContext());
    }

    return shaderMode;
}

/**
 * @brief Fills the sky pixels of the light buffer.
 * @param pGBufferArray G-buffers of the view.
 * @param shaderMode Current shader mode.
 * @return The shader mode after drawing.
 */
agl::ShaderMode DeferredRendering::fillSky(GBufferArray* pGBufferArray,
                                           agl::ShaderMode shaderMode) const {
    sead::GraphicsContext graphicsContext;
    graphicsContext.setBlendEnable(false);
    graphicsContext.setDepthEnable(true, false);
    graphicsContext.setDepthFunc(3);
    graphicsContext.apply(getDrawContext());

    mFillDeferredSkyShader->activate(getDrawContext(), true);
    const agl::ShaderProgram* program = mFillDeferredSkyShader;
    agl::SamplerLocation location("cSkyLight");
    location.search(*program);
    pGBufferArray->activateSamplerLightBuffer(location);
    mFullScreenQuadModel->drawQuad();
    return shaderMode;
}

/**
 * @brief Draws the screen space ambient occlusion of the main view from a half resolution depth.
 * @param viewIndex Index of the rendered view.
 * @param pGBufferArray G-buffers of the view.
 * @param rViewMtx View matrix.
 * @param rProjMtx Projection matrix.
 * @param shaderMode Current shader mode.
 * @return The shader mode after drawing.
 */
agl::ShaderMode DeferredRendering::drawSSAO(s32 viewIndex, GBufferArray* pGBufferArray,
                                            const sead::Matrix34f& rViewMtx,
                                            const sead::Matrix44f& rProjMtx,
                                            agl::ShaderMode shaderMode) {
    if (viewIndex != 0) {
        return shaderMode;
    }

    const agl::sdw::SSAO* ssao = mGraphicsSystemInfo->mSSAOParamKeeper->getParam();

    if (!ssao->isEnable()) {
        return shaderMode;
    }

    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();
    const agl::TextureData* albedo =
        pGBufferArray->getGBuffer(GBufferArray::cIndex_Albedo).mTexture;
    f32 halfWidth = albedo->getWidth(0) * 0.5f;
    f32 halfHeight = albedo->getHeight(0) * 0.5f;

    sead::GraphicsContext graphicsContext;
    graphicsContext.setColorMask(0, true, false, false, false);
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(getDrawContext());

    agl::TextureData* halfDepth =
        allocator->alloc(getDrawContext(), "Half Texture Target(for SSAO)",
                         agl::TextureFormat(9), halfWidth, halfHeight, 1, nullptr,
                         agl::utl::DynamicTextureAllocator::cAllocateType_0, true, false);

    {
        agl::RenderBuffer renderBuffer;
        RenderBufferAttacher attacher(&renderBuffer, halfDepth, nullptr, nullptr, nullptr,
                                      nullptr);
        sead::Viewport viewport(renderBuffer);
        viewport.apply(getDrawContext(), renderBuffer);
        renderBuffer.bind(getDrawContext());

        const char* macros[] = {"RENDER_TYPE"};
        const char* values[] = {"0"};
        const agl::ShaderProgram* program =
            mMakeHalfDepthTextureShader->searchVariation(1, macros, values);
        program->activate(getDrawContext(), true);

        agl::UniformBlockLocation uboLocation("TexAdjustInfo");
        uboLocation.search(*program);

        const agl::TextureData* depth =
            pGBufferArray->getGBuffer(GBufferArray::cIndex_Albedo).mTexture;
        sead::Vector2f texelSize(1.0f / depth->getWidth(0), 1.0f / depth->getHeight(0));
        mMakeHalfTextureUbo->setValue(1, 0.5f);
        mMakeHalfTextureUbo->setData(0, &texelSize, 0, 1);
        mMakeHalfTextureUbo->activate(getDrawContext(), uboLocation);
        mMakeHalfTextureUbo->flushAndSwap();

        agl::SamplerLocation samplerLocation("uTexture");
        samplerLocation.search(*program);

        if (samplerLocation.isValid()) {
            pGBufferArray->activateSamplerDepthView(samplerLocation);
        }

        pGBufferArray->getGBufDepthViewTex();
        mFullScreenQuadModel->drawQuad();
    }

    if (halfDepth != nullptr) {
        ssao->drawToAOBuffer(getDrawContext(), 0, halfWidth, halfHeight, *halfDepth, rViewMtx,
                             rProjMtx);
        allocator->free(halfDepth);
    } else {
        ssao->drawToAOBuffer(getDrawContext(), 0, halfWidth, halfHeight,
                             *pGBufferArray->getGBufDepthViewTex(), rViewMtx, rProjMtx);
    }

    return shaderMode;
}

/**
 * @brief Draws the deferred shading pass, fills the sky and releases the shadow pre-pass.
 * @param pLiveActorKit Live actor kit of the scene.
 * @param rRenderBuffer Render buffer of the view (unused).
 * @param rViewport Viewport of the view (unused).
 * @param pDepthShadowDrawer Depth shadow drawer, or nullptr.
 * @param rViewMtx View matrix.
 * @param rProjMtx Projection matrix.
 * @param pGBufferArray G-buffers of the view.
 * @param viewIndex Index of the rendered view.
 * @param skyFillType How the sky is filled.
 * @param shaderMode Current shader mode.
 * @param pSimpleModelEnv Simple model environment.
 * @param isFast Whether the fast (light buffer less) shading is used.
 * @return The shader mode after drawing.
 */
agl::ShaderMode DeferredRendering::drawDeferredShading(
    LiveActorKit* pLiveActorKit, const agl::RenderBuffer& rRenderBuffer,
    const sead::Viewport& rViewport, DepthShadowDrawer* pDepthShadowDrawer,
    const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx, GBufferArray* pGBufferArray,
    s32 viewIndex, SkyFillType skyFillType, agl::ShaderMode shaderMode,
    const SimpleModelEnv* pSimpleModelEnv, bool isFast) {
    shaderMode = prepareDeferredShading(pLiveActorKit, rViewMtx, rProjMtx, pGBufferArray,
                                        viewIndex, pDepthShadowDrawer, shaderMode,
                                        pSimpleModelEnv, isFast);
    getDrawContext()->barrierTexture(1);
    mFullScreenQuadModel->drawQuad();

    if (skyFillType == SkyFillType_Fill && !isFast) {
        fillSky(pGBufferArray, shaderMode);
    }

    mShadowPrePass->release(viewIndex);
    return shaderMode;
}

/**
 * @brief Selects the deferred shading shader variation and sets up its uniforms and samplers.
 * @param pLiveActorKit Live actor kit of the scene.
 * @param rViewMtx View matrix.
 * @param rProjMtx Projection matrix (unused).
 * @param pGBufferArray G-buffers of the view.
 * @param viewIndex Index of the rendered view.
 * @param pDepthShadowDrawer Depth shadow drawer, or nullptr.
 * @param shaderMode Current shader mode.
 * @param pSimpleModelEnv Simple model environment.
 * @param isFast Whether the fast (light buffer less) shading is used.
 * @return The shader mode after the setup.
 */
agl::ShaderMode DeferredRendering::prepareDeferredShading(
    LiveActorKit* pLiveActorKit, const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
    GBufferArray* pGBufferArray, s32 viewIndex, DepthShadowDrawer* pDepthShadowDrawer,
    agl::ShaderMode shaderMode, const SimpleModelEnv* pSimpleModelEnv, bool isFast) {
    // The camera position is read but no longer used by the shading setup.
    sead::Vector3f cameraPos;
    rViewMtx.getTranslation(cameraPos);

    sead::GraphicsContext graphicsContext;

    if (isFast) {
        graphicsContext.setBlendEnable(true);
        graphicsContext.setBlendFactor(0, 2, 5);
        graphicsContext.setBlendEquation(0, 1);
    } else {
        graphicsContext.setBlendEnable(false);
    }

    graphicsContext.setDepthEnable(true, false);
    graphicsContext.setDepthFunc(6);
    graphicsContext.apply(getDrawContext());

    const char* macros[] = {"IS_ENABLE_Y_FOG",
                            "FOG_TYPE",
                            "DEPTH_SHADOW_CASCADE_NUM",
                            "IS_ENABLE_SHADOW_DIST_DAMP",
                            "IS_ENABLE_SHADOW_PCF",
                            "DEPTH_SHADOW_TYPE",
                            "SHADOW_COMPOSE_TYPE",
                            "USING_SSAO",
                            "IS_FAST"};
    const char* values[] = {"1", "1", "1", "0", "0", "1", "0", "0", "0"};

    const FogDirector* fogDirector = pLiveActorKit->getGraphicsSystemInfo()->getFogDirector();

    if (fogDirector != nullptr) {
        if (!(*fogDirector->getYFogParam().mIntensityMax > 0.0f)) {
            values[0] = "0";
        }

        if (*fogDirector->getFogParam().mIntensityMax > 0.0f) {
            if (mGraphicsSystemInfo->getInitArg().isUsingCubeMapAtmosScatter()) {
                values[1] = "2";
            }
        } else {
            values[1] = "0";
        }
    }

    ShadowDirector* shadowDirector =
        pLiveActorKit->getGraphicsSystemInfo()->getShadowDirector();

    if (shadowDirector != nullptr) {
        if (shadowDirector->mIsEnableDistDamp) {
            values[3] = "1";
        }

        if (shadowDirector->mPcfType == 1) {
            values[4] = "1";
        } else if (shadowDirector->mPcfType == 2) {
            values[4] = "2";
        }
    }

    bool isUsingShadowPrePass = false;

    if (shadowDirector != nullptr && shadowDirector->isEnableShadowPrePass()) {
        values[5] = "2";
        isUsingShadowPrePass = true;
    } else if (shadowDirector != nullptr && shadowDirector->isEnableShadowForLightPrePass()) {
        values[5] = "0";
    } else if (pDepthShadowDrawer != nullptr && pDepthShadowDrawer->isEnable()) {
        if (shadowDirector != nullptr && shadowDirector->isEnableVarianceShadow()) {
            values[5] = "3";
        }
    } else {
        values[5] = "0";
    }

    if (shadowDirector != nullptr) {
        switch (shadowDirector->mComposeType) {
        case 1:
            values[6] = "1";
            break;
        case 2:
            values[6] = "2";
            break;
        case 3:
            values[6] = "3";
            break;
        case 4:
            values[6] = "4";
            break;
        }

        mSceneUbo->setValue(2, shadowDirector->_400);
    }

    SSAOParamKeeper* ssaoParamKeeper = mGraphicsSystemInfo->mSSAOParamKeeper;
    const agl::sdw::SSAO* ssao = ssaoParamKeeper != nullptr ? ssaoParamKeeper->getParam() : nullptr;

    if (viewIndex == 0 && ssao->isEnable()) {
        values[7] = "1";
    }

    if (isFast) {
        values[8] = "1";
    }

    const agl::ShaderProgram* program = mDeferredShadingShader->searchVariation(9, macros, values);
    program->activate(getDrawContext(), true);
    pSimpleModelEnv->prepareModelDraw(viewIndex);

    agl::UniformBlockLocation sceneLocation("DeferredRenderingScene");
    agl::UniformBlockLocation materialLocation("DeferredRenderingMaterial");
    sceneLocation.search(*program);
    materialLocation.search(*program);

    mSceneUbo->setValue(0, 0.5f);
    mSceneUbo->setValue(1, 0.0f);
    mSceneUbo->setValue(3, 80.0f);

    if (shadowDirector != nullptr) {
        f32 dampStart = shadowDirector->mDistDampStart;
        f32 dampEnd = shadowDirector->mDistDampEnd;
        mSceneUbo->setValue(4, -dampStart);

        if (dampEnd >= dampStart) {
            mSceneUbo->setValue(5, 1.0f / (dampStart + (1.0f - dampEnd)));
        } else {
            mSceneUbo->setValue(5, 1.0f / (dampStart - dampEnd));
        }

        mSceneUbo->setValue(6, 1.0f);
        mSceneUbo->setValue(7, shadowDirector->_1b0);
    } else {
        mSceneUbo->setValue(6, 1.0f);
    }

    mSceneUbo->setValue(8, mShadowDensityScale);
    mSceneUbo->activate(getDrawContext(), sceneLocation);
    mSceneUbo->flushOnly();

    agl::SamplerLocation albedoLocation("cAlbedo");
    albedoLocation.search(*program);
    agl::SamplerLocation normalLocation("cViewNormal");
    normalLocation.search(*program);
    agl::SamplerLocation depthLocation("cViewDepth");
    depthLocation.search(*program);
    agl::SamplerLocation lightBufferLocation("cLightBuffer");
    lightBufferLocation.search(*program);
    agl::SamplerLocation irradianceLocation("cTexCubeMapIrradiance");
    irradianceLocation.search(*program);

    if (viewIndex == 0 && mGraphicsSystemInfo->mSSAOParamKeeper->getParam()->isEnable()) {
        agl::SamplerLocation ssaoLocation("cSSAO");
        ssaoLocation.search(*program);
        mGraphicsSystemInfo->mSSAOParamKeeper->getParam()->getAOSampler().activate(
            getDrawContext(), ssaoLocation, -1, false);
    }

    pGBufferArray->activateSamplerAlbedo(albedoLocation);
    pGBufferArray->activateSamplerNrmView(normalLocation);

    if (depthLocation.isValid()) {
        pGBufferArray->activateSamplerDepthView(depthLocation);
    }

    if (!isFast) {
        pLiveActorKit->getGraphicsSystemInfo()
            ->getPrePassLightKeeper()
            ->getLightPrePass()
            ->getContext(viewIndex)
            .mLightBufferSampler.activate(getDrawContext(), lightBufferLocation, -1, false);
    }

    if (irradianceLocation.isValid()) {
        pLiveActorKit->getGraphicsSystemInfo()
            ->getCubeMapDirector()
            ->getIrradianceSampler(0)
            ->activate(getDrawContext(), irradianceLocation, -1, false);
    }

    if (isUsingShadowPrePass) {
        agl::SamplerLocation shadowLocation("cPrePassShadow");
        shadowLocation.search(*program);

        if (shadowLocation.isValid()) {
            mShadowPrePass->getContext(viewIndex).mMipSampler.activate(
                getDrawContext(), shadowLocation, -1, false);
        }
    } else if (pDepthShadowDrawer != nullptr) {
        pDepthShadowDrawer->setupShaderProgram(viewIndex, program);
    }

    pLiveActorKit->getGraphicsSystemInfo()->activateDirLitColorTex();
    return shaderMode;
}

/**
 * @brief Resets the per-frame state and swaps the scene uniform block.
 */
void DeferredRendering::preDrawGraphics() {
    mIsUsingDepthShadowMap = false;
    mSceneUbo->swap();
}

}  // namespace al
