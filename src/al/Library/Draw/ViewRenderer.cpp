#include "Library/Draw/ViewRenderer.hpp"

#include <common/aglRenderBuffer.h>
#include <common/aglTextureData.h>
#include <common/aglTextureDataInitializer.h>
#include <gfx/seadColor.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadPrimitiveRenderer.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>
#include <lighting/aglSSII.h>
#include <nn/oe.h>
#include <nvn/nvn_FuncPtrBase.h>
#include <postfx/aglColorCorrection.h>
#include <postfx/aglFilterAA.h>
#include <postfx/aglFlareFilter.h>
#include <resource/seadResource.h>
#include <resource/seadResourceMgr.h>
#include <shadow/aglDepthShadow.h>
#include <utility/aglDynamicTextureAllocator.h>

#include "Library/Draw/DeferredRendering.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Draw/ReducedBufferRenderer.hpp"
#include "Library/Effect/EffectShaderHolder.hpp"
#include "Library/Effect/EffectSystem.hpp"
#include "Library/Fog/FogDirector.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Light/LightIntensityDirector.hpp"
#include "Library/Light/PrePassLightKeeper.hpp"
#include "Library/Light/SSIIKeeper.hpp"
#include "Library/LiveActor/Common/LiveActorKit.hpp"
#include "Library/Memory/Util.hpp"
#include "Library/Model/SimpleModelEnv.hpp"
#include "Library/Play/Draw/RenderVariables.hpp"
#include "Library/PostProcessing/DepthOfFieldDrawer.hpp"
#include "Library/PostProcessing/FlareFilterDirector.hpp"
#include "Library/PostProcessing/GodRayDirector.hpp"
#include "Library/PostProcessing/HdrCompose.hpp"
#include "Library/PostProcessing/LightStreakDirector.hpp"
#include "Library/PostProcessing/OccludedEffectDirector.hpp"
#include "Library/PostProcessing/PostProcessingFilter.hpp"
#include "Library/PostProcessing/RadialBlurDirector.hpp"
#include "Library/Screen/ScreenFader.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDirector.hpp"
#include "Library/Shader/DeferredRendering/FullScreenQuadModel.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/CubeMapKeeper.hpp"
#include "Library/Shader/ForwardRendering/SSR.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Library/Shader/ForwardRendering/ShaderMirrorDirector.hpp"
#include "Library/Shadow/Depth/DepthShadowDrawer.hpp"
#include "Library/Shadow/ShadowDirector.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Camera/Info/SceneCameraInfo.hpp"
#include "Project/Draw/GraphicsParamKeeper.hpp"
#include "Project/Draw/GraphicsStressDirector.hpp"
#include "Project/LiveActor/ActorExecuteFunction.hpp"
#include "Project/PostProcessing/EdgeDrawer.hpp"

namespace {

// Color the screen fades in from.
const sead::Color4f cScreenFadeColor(0.0f, 0.0f, 0.0f, 1.0f);

/**
 * Opens a named GPU debug group.
 * @param pCommandBuffer The command buffer.
 * @param pName The name of the group.
 */
inline void pushDebugGroup(NVNcommandBuffer* pCommandBuffer, const char* pName) {
    using PushDebugGroupFunc = void (*)(NVNcommandBuffer*, const char*);
    reinterpret_cast<PushDebugGroupFunc>(pfnc_nvnCommandBufferPushDebugGroup)(pCommandBuffer,
                                                                             pName);
}

/**
 * Closes the current GPU debug group.
 * @param pCommandBuffer The command buffer.
 */
inline void popDebugGroup(NVNcommandBuffer* pCommandBuffer) {
    pfnc_nvnCommandBufferPopDebugGroup(pCommandBuffer);
}

}  // namespace

namespace al {

/**
 * Constructs the view renderer and all of its post effect resources.
 * @param pInfo The graphics system info of the scene.
 */
ViewRenderer::ViewRenderer(GraphicsSystemInfo* pInfo) : mGraphicsSystemInfo(pInfo) {
    mReducedEffectRenderCount[0] = 0;
    mReducedEffectRenderCount[1] = 0;

    s32 viewNum = mGraphicsSystemInfo->getInitArg()._20;
    s32 bufferNum = mGraphicsSystemInfo->getInitArg().getViewNumWithStereo() + 6;

    mSimpleModelEnv = new SimpleModelEnv();
    mSimpleModelEnv->initialize(bufferNum, mGraphicsSystemInfo, nullptr);
    mDeferredRendering = new DeferredRendering(mGraphicsSystemInfo, bufferNum);
    mScreenFader = new ScreenFader();
    mBloom.initialize(viewNum, nullptr);
    mBloom.setEnable(false);
    mHDRCompose.initialize(viewNum, nullptr);
    mReducedBufferRenderer = new ReducedBufferRenderer(ShaderHolder::sInstance,
                                                       mGraphicsSystemInfo->mFullScreenTriangle);

    sead::ResourceMgr* resourceMgr = sead::ResourceMgr::instance();
    sead::ResourceMgr::LoadArg loadArg;
    loadArg.load_data_alignment = 0x2000;
    loadArg.path = "SystemData/DitherImage.tga";
    // The heap is fetched twice, as in the original.
    loadArg.instance_heap = getCurrentHeap();
    loadArg.instance_heap = getCurrentHeap();
    mDitherImageResource =
        sead::DynamicCast<sead::DirectResource>(resourceMgr->tryLoadWithoutDecomp(loadArg));

    mDitherTexture = new agl::TextureData();
    agl::TextureDataInitializerTGA::initialize(mDitherTexture, getCurrentHeap(),
                                               mDitherImageResource->getRawData(),
                                               getCurrentHeap());
    mDitherSampler.applyTextureData(*mDitherTexture);
    mDitherSampler.setWrap(1, 1, 1);
    mDitherSampler.setFilter(0, 0, 0);

    mSSR = new SSR(mGraphicsSystemInfo, bufferNum);
    mFullScreenQuadModel = new FullScreenQuadModel();
}

/**
 * Destroys the view renderer and the objects it owns.
 */
ViewRenderer::~ViewRenderer() {
    if (mReducedBufferRenderer != nullptr) {
        delete mReducedBufferRenderer;
        mReducedBufferRenderer = nullptr;
    }

    if (mFullScreenQuadModel != nullptr) {
        delete mFullScreenQuadModel;
        mFullScreenQuadModel = nullptr;
    }

    if (mDeferredRendering != nullptr) {
        delete mDeferredRendering;
        mDeferredRendering = nullptr;
    }

    if (mSimpleModelEnv != nullptr) {
        delete mSimpleModelEnv;
        mSimpleModelEnv = nullptr;
    }

    if (mSSR != nullptr) {
        delete mSSR;
        mSSR = nullptr;
    }

    if (mDitherTexture != nullptr) {
        delete mDitherTexture;
        mDitherTexture = nullptr;
    }
}

/**
 * Updates the bloom and HDR compose contexts of every view before drawing.
 * @param pCameraInfo The scene camera info, used for the near/far planes of each view.
 */
void ViewRenderer::preDrawGraphics(const SceneCameraInfo* pCameraInfo) {
    mDeferredRendering->preDrawGraphics();

    s32 viewNum = mGraphicsSystemInfo->getInitArg()._20;
    HdrCompose* hdrCompose = mGraphicsSystemInfo->mHdrCompose;
    LightIntensityDirector* lightIntensityDirector =
        mGraphicsSystemInfo->getLightIntensityDirector();
    f32 reduceScale = lightIntensityDirector->getCurrentReduceScale();

    if (!lightIntensityDirector->isLoadedBloomParam()) {
        mBloom.setEnable(false);
    }

    mHDRCompose.offFlag(agl::pfx::HDRCompose::cFlag_EnableSampler2 |
                        agl::pfx::HDRCompose::cFlag_EnableSampler1);

    for (s32 i = 0; i < viewNum; i++) {
        mHDRCompose.getContext(i).mExposure = lightIntensityDirector->getExposureExp();
        lightIntensityDirector->applyBloomParameter(&mBloom, i);
        mHDRCompose.getContext(i).mpSampler1 = nullptr;
        mHDRCompose.getContext(i).mSampler1Param = sead::Vector2f::zero;

        if (i > 0) {
            mBloom.setNearFar(i, pCameraInfo->_38->getNear(), pCameraInfo->_38->getFar());
        } else {
            mBloom.setNearFar(i, pCameraInfo->mProjection->getNear(),
                              pCameraInfo->mProjection->getFar());
        }

        bool isUsingMyHdrCompose = hdrCompose->isUsingMyHdrCompose();
        agl::pfx::Bloom::Context& context = mBloom.getContext(i);
        f32 scale = isUsingMyHdrCompose ? reduceScale * 4.0f : reduceScale;
        context.mScale.x = scale;
        context.mScale.y = scale;
    }
}

/**
 * Does nothing.
 */
void ViewRenderer::updatePreDraw() {}

/**
 * Counts reduced buffer effect render requests and enables the reduced buffer renderer while at
 * least one is active.
 * @param isEnable Whether to add (true) or remove (false) a request.
 * @param isHdr Whether the request is for the HDR reduced buffer.
 */
void ViewRenderer::setReducedEffectRender(bool isEnable, bool isHdr) {
    s32& count = mReducedEffectRenderCount[isHdr];

    if (isEnable) {
        if (count < 2) {
            count++;
        }
    } else if (count > 0) {
        count--;
    }

    if (isHdr) {
        mReducedBufferRenderer->setEnableHdr(count != 0);
    } else {
        mReducedBufferRenderer->setEnable(count != 0);
    }
}

/**
 * Configures the renderer for single mode rendering.
 */
void ViewRenderer::setSingleModeRendering() {
    mGraphicsSystemInfo->mHdrCompose->setEnableDangerIndicator(true);
    _e73 = true;
    _e74 = true;
    _e78 = 2;
    _e7c = true;
}

/**
 * Enables or disables the danger indicator of the HDR compose.
 * @param isEnable Whether to enable the danger indicator.
 */
void ViewRenderer::dangerIndicatorEnable(bool isEnable) {
    mGraphicsSystemInfo->mHdrCompose->setEnableDangerIndicator(isEnable);
}

/**
 * Draws one view of the scene: G-buffer, lights, HDR passes, post effects and the final
 * composition into the given render buffer.
 * @param viewIndex The index of the view.
 * @param index Unused.
 * @param pKit The live actor kit of the scene.
 * @param pCameraInfo The scene camera info.
 * @param pBuffer The render buffer the view is composed into.
 * @param rViewport The viewport of the view.
 * @param isDrawHdrEffect Passed on to drawHdr.
 * @param isFadeScreen Whether to fade the screen in with the screen fader.
 * @param shaderMode The current shader mode.
 * @return The shader mode after drawing.
 */
agl::ShaderMode ViewRenderer::drawView(s32 viewIndex, s32 index, LiveActorKit* pKit,
                                       const SceneCameraInfo* pCameraInfo,
                                       const agl::RenderBuffer* pBuffer,
                                       const sead::Viewport& rViewport, bool isDrawHdrEffect,
                                       bool isFadeScreen, agl::ShaderMode shaderMode) const {
    if (mGraphicsSystemInfo == nullptr) {
        return shaderMode;
    }

    if (nn::oe::GetCurrentFocusState() == nn::oe::FocusState_Background) {
        return shaderMode;
    }

    if (viewIndex == 0) {
        alEffectSystemFunction::calcEffectCompute(pKit->getEffectSystem());
    }

    getDepthShadowDrawer(pKit)->getDepthShadow()->getShadowMap().setEnableHiZ(
        mDeferredRendering->isUsingDepthShadowMap());

    const sead::LookAtCamera* camera =
        viewIndex == 0 ? pCameraInfo->mLookAtCamera :
                         static_cast<const sead::LookAtCamera*>(pCameraInfo->_28);
    const sead::PerspectiveProjection* projection = static_cast<const sead::PerspectiveProjection*>(
        viewIndex == 0 ? pCameraInfo->mProjection : pCameraInfo->_38);
    mGraphicsSystemInfo->updateViewGpu(viewIndex, camera, projection);
    mSSR->setCam(camera);

    const ShaderCubeMapKeeper* cubeMapKeeper =
        pKit->mGraphicsSystemInfo->getShaderCubeMapKeeper();
    bool isGameOver = false;

    if (cubeMapKeeper->getForceCubeMapInfo() != nullptr) {
        isGameOver = isEqualString(cubeMapKeeper->getForceCubeMapInfo()->mName, "GameOver");
    }

    sead::Viewport viewport = rViewport;
    mDitherSampler.activate(GameFrameworkNx::getAglDrawContext(), getSamplerLocationDither(), -1,
                            false);

    if (viewIndex == 0) {
        shaderMode = drawSystem(pKit, shaderMode);
        mDitherSampler.activate(GameFrameworkNx::getAglDrawContext(), getSamplerLocationDither(),
                                -1, false);
    }

    sead::PrimitiveRenderer* primitiveRenderer = sead::PrimitiveRenderer::instance();
    primitiveRenderer->setCamera(*camera);
    primitiveRenderer->setProjection(*projection);

    s32 width = viewport.getSizeX();
    s32 height = viewport.getSizeY();
    RenderVariables variables(mGraphicsSystemInfo, pKit, mSimpleModelEnv, viewIndex, width,
                              height, false, true);
    variables._6b8 = const_cast<sead::PerspectiveProjection*>(projection);
    variables._6b0 = const_cast<sead::LookAtCamera*>(camera);

    FogDirector* fogDirector = mGraphicsSystemInfo->getFogDirector();

    // Both branches store the same flag; the target keeps them as two separate blocks.
    if (fogDirector->isUsingMulFog() || fogDirector->isUsingMulYFog()) {
        variables.mIsFast = mIsFastRendering;
    } else {
        variables.mIsFast = mIsFastRendering;
    }

    HdrCompose* hdrCompose = mGraphicsSystemInfo->mHdrCompose;
    f32 near = projection->getNear();
    f32 far = projection->getFar();
    f32 fovy = projection->getFovy();
    shaderMode = drawMirror(viewIndex, &variables, isGameOver, shaderMode);

    const sead::Matrix44f& projMtx = projection->getProjectionMatrix();
    const sead::Matrix34f& viewMtx = camera->getMatrix();
    mGraphicsSystemInfo->updateViewVolume(viewMtx, projMtx);
    pBuffer->bind(GameFrameworkNx::getDrawContext());
    viewport.apply(GameFrameworkNx::getDrawContext(), *pBuffer);

    {
        agl::DrawContext* drawContext = GameFrameworkNx::getAglDrawContext();
        sead::Viewport clearViewport(*pBuffer);
        pBuffer->fastClear(drawContext, 0, 6, sead::Color4f::cBlack, 1.0f, 0, clearViewport,
                           true);
    }

    GraphicsStressDirector* stressDirector = mGraphicsSystemInfo->getGraphicsStressDirector();
    s32 antiAliasingType = stressDirector->getCurrentParam().getAntiAliasingType();

    if (antiAliasingType == 2 ||
        (antiAliasingType == 0 && stressDirector->getFXAAAlphaOut() == 0.0f)) {
        sead::Vector2f offset;
        stressDirector->calcPseudoAAProjOffset(&offset, width, height);
        sead::PerspectiveProjection* mutableProjection =
            const_cast<sead::PerspectiveProjection*>(projection);
        mutableProjection->setOffset(projection->getOffsetDirect() + offset);
    }

    mGraphicsSystemInfo->getShadowDirector()->mDepthShadowDrawer->setupDepthShadow(viewIndex);

    GBufferArray gBufferArray(&variables.mDepthTarget, mGraphicsSystemInfo,
                              stressDirector->getCurrentParam().isUsingLppSpcMask(),
                              mGraphicsSystemInfo->getInitArg()._f);
    gBufferArray.allocGBuffer(viewIndex);
    variables._28 = &gBufferArray;

    agl::lght::LightPrePass* lightPrePass =
        mGraphicsSystemInfo->getPrePassLightKeeper()->getLightPrePass();

    if (variables.mIsFast) {
        gBufferArray.createLightBufferAndCalcContext(lightPrePass, viewIndex, variables.mWidth,
                                                     variables.mHeight, *camera, *projection,
                                                     false);
        variables.allocColorBuffer(lightPrePass,
                                   stressDirector->isForceStressOff() ||
                                       stressDirector->getCurrentParam().isClearLightBuffer());
    } else {
        variables.allocColorBuffer(nullptr, true);
        gBufferArray.createLightBufferAndCalcContext(lightPrePass, viewIndex, variables.mWidth,
                                                     variables.mHeight, *camera, *projection,
                                                     true);
    }

    if (mGBufferDrawer != nullptr) {
        agl::RenderBuffer renderBuffer;
        f32 bufferWidth = variables.mWidth;
        f32 bufferHeight = variables.mHeight;
        renderBuffer.setVirtualSize(sead::Vector2f(bufferWidth, bufferHeight));
        renderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, bufferWidth, bufferHeight));
        renderBuffer.setRenderTargetColorNullAll();
        renderBuffer.setRenderTargetColor(&const_cast<GBuffer*>(
            static_cast<GBufferArray*>(variables._28)->getGBufAlbedo())->mRenderTarget);
        mGBufferDrawer->draw(renderBuffer, variables.mViewport);
    }

    pushDebugGroup(GameFrameworkNx::getDrawContext()->getNvnCommandBuffer(), "Draw HDR");
    const agl::TextureData* hdrTexture =
        drawHdr(viewIndex, variables, isDrawHdrEffect, false, isGameOver, shaderMode);
    popDebugGroup(GameFrameworkNx::getDrawContext()->getNvnCommandBuffer());

    const agl::TextureData* colorTexture = variables.mColorTexture;
    variables.mDepthTarget.expandHiZBuffer(GameFrameworkNx::getAglDrawContext());

    if ((mReducedBufferRenderer->isEnable() || mReducedBufferRenderer->isEnableHdr()) &&
        mIsDrawEffect) {
        variables.getLiveActorKit()->getEffectSystem()->getShaderHolder()->setupTextureDepth(
            gBufferArray.getGBufDepthViewTex());
        mReducedBufferRenderer->draw(variables, gBufferArray.getGBufDepthViewTex(), projMtx,
                                     viewMtx, near, far, fovy, true);
    }

    sead::Viewport bufferViewport(*pBuffer);
    bufferViewport.apply(GameFrameworkNx::getDrawContext(), *pBuffer);

    const agl::TextureData* depthTexture = variables.mDepthTexture;
    SSIIKeeper* ssiiKeeper = mGraphicsSystemInfo->mSSIIKeeper;
    const SSIIParam& ssiiParam = ssiiKeeper->getCurrentParam();

    if (ssiiParam.isEnableDiffuse() || ssiiParam.isEnableReflection()) {
        GBufferArray* drawGBufferArray = static_cast<GBufferArray*>(variables._28);
        ssiiKeeper->getSSII()->drawAlbedoMode(
            GameFrameworkNx::getAglDrawContext(), viewIndex, variables.mRenderBuffer,
            bufferViewport, *colorTexture, *drawGBufferArray->getGBufAlbedoTex(),
            *drawGBufferArray->getGBufNrmViewTex(), *drawGBufferArray->getGBufDepthViewTex(),
            false);
    }

    if (hdrCompose->isUsingMyHdrCompose()) {
        hdrCompose->setupComposeBuffer(viewIndex, variables.mRenderBuffer, &mBloom);
    }

    if (mBloom.isEnable()) {
        if (hdrCompose->isUsingMyHdrCompose()) {
            const agl::RenderBuffer* bloomBuffer = hdrCompose->getRenderBufferBloom();
            sead::Viewport bloomViewport(*bloomBuffer);
            bloomViewport.apply(GameFrameworkNx::getDrawContext(), *bloomBuffer);

            agl::pfx::Bloom::DrawArg drawArg;
            drawArg.mpRenderBuffer = bloomBuffer;
            drawArg.mpViewport = &bloomViewport;
            drawArg.mpColor = colorTexture;
            drawArg.mpDepth = depthTexture;
            drawArg.mIsLinearDepth = false;
            drawArg.mpMask = nullptr;
            mBloom.draw(GameFrameworkNx::getAglDrawContext(), viewIndex, drawArg);
        } else {
            agl::pfx::Bloom::DrawArg drawArg;
            drawArg.mpRenderBuffer = &variables.mRenderBuffer;
            drawArg.mpViewport = &variables.mViewport;
            drawArg.mpColor = colorTexture;
            drawArg.mpDepth = depthTexture;
            drawArg.mIsLinearDepth = false;
            drawArg.mpMask = nullptr;
            mBloom.drawToBloomBuffer(GameFrameworkNx::getAglDrawContext(), viewIndex, drawArg);
        }
    }

    LightStreakDirector* lightStreakDirector = mGraphicsSystemInfo->mLightStreakDirector;

    if (lightStreakDirector != nullptr && lightStreakDirector->isEnable()) {
        if (hdrCompose->isUsingMyHdrCompose()) {
            const agl::RenderBuffer* streakBuffer = hdrCompose->getRenderBufferLightStreak();
            sead::Viewport streakViewport(*streakBuffer);
            streakViewport.apply(GameFrameworkNx::getDrawContext(), *streakBuffer);
            shaderMode = lightStreakDirector->drawToRenderBuffer(
                viewIndex, *streakBuffer, streakViewport, *colorTexture, shaderMode);
        } else {
            shaderMode = lightStreakDirector->drawToRenderBuffer(
                viewIndex, *pBuffer, bufferViewport, *colorTexture, shaderMode);
        }
    }

    DepthOfFieldDrawer* depthOfFieldDrawer = mGraphicsSystemInfo->mDepthOfFieldDrawer;

    if (depthOfFieldDrawer != nullptr) {
        shaderMode = depthOfFieldDrawer->draw(
            viewIndex, variables.mRenderBuffer, *depthTexture,
            *static_cast<const sead::Projection*>(variables._6b8), near, far, false, shaderMode);
        variables.mColorTarget.invalidateGPUCache(GameFrameworkNx::getAglDrawContext());
    }

    if (mIsDrawEffect) {
        variables.bindRenderBuffer();
        tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(), agl::cShaderMode_UniformBlock);
        pKit->getEffectSystem()->getShaderHolder()->setupTextureDepth(
            gBufferArray.getGBufDepthViewTex());
        alEffectSystemFunction::drawEffectAfterFog(pKit->getEffectSystem(), projMtx, viewMtx,
                                                   near, far, fovy);
    }

    GodRayDirector* godRayDirector = mGraphicsSystemInfo->mGodRayDirector;

    if (godRayDirector != nullptr && godRayDirector->isEnable()) {
        bool isUsingMyHdrCompose = hdrCompose->isUsingMyHdrCompose();
        shaderMode = godRayDirector->draw(viewIndex, variables.mDepthTarget, *colorTexture,
                                          viewMtx, projMtx, shaderMode, mSimpleModelEnv, pKit);

        if (isUsingMyHdrCompose) {
            const agl::RenderBuffer* godRayBuffer = hdrCompose->getRenderBufferGodRay();
            sead::Viewport godRayViewport(*godRayBuffer);
            godRayViewport.apply(GameFrameworkNx::getDrawContext(), *godRayBuffer);
            shaderMode = godRayDirector->composeToRenderBuffer(viewIndex, *godRayBuffer,
                                                               godRayViewport, shaderMode);
        } else {
            shaderMode = godRayDirector->composeToRenderBuffer(
                viewIndex, variables.mRenderBuffer, variables.mViewport, shaderMode);
        }

        godRayDirector->releaseBuffer();
    }

    FlareFilterDirector* flareFilterDirector = mGraphicsSystemInfo->mFlareFilterDirector;

    if (flareFilterDirector != nullptr && flareFilterDirector->getParam()->isEnable()) {
        if (hdrCompose->isUsingMyHdrCompose()) {
            const agl::RenderBuffer* flareBuffer = hdrCompose->getRenderBufferFlareFilter();
            sead::Viewport flareViewport(*flareBuffer);
            flareViewport.apply(GameFrameworkNx::getDrawContext(), *flareBuffer);
            shaderMode = flareFilterDirector->draw(viewIndex, *flareBuffer, flareViewport,
                                                   *colorTexture, shaderMode);
        } else {
            shaderMode = flareFilterDirector->draw(viewIndex, variables.mRenderBuffer,
                                                   variables.mViewport, *colorTexture, shaderMode);
        }
    }

    if (isGameOver || !isFadeScreen) {
        if (!mScreenFader->isEnd()) {
            mScreenFader->end(-1);
        }

        if (mIsDrawEffect) {
            variables.bindRenderBuffer();
            tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(),
                                agl::cShaderMode_UniformBlock);
            pKit->getEffectSystem()->getShaderHolder()->setupTextureDepth(
                gBufferArray.getGBufDepthViewTex());
            alEffectSystemFunction::drawEffectPostEffectBackground(
                pKit->getEffectSystem(), projMtx, viewMtx, near, far, fovy);
        }
    } else {
        tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(), agl::cShaderMode_UniformBlock);

        if (mScreenFader->isEnd()) {
            mScreenFader->start(mIsDrawEffect ? 0 : 30, mIsDrawEffect ? 12 : 46,
                                mIsDrawEffect ? 0.4f : 0.9f, cScreenFadeColor);
        }

        tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(), agl::cShaderMode_UniformBlock);
        mScreenFader->update();
        mScreenFader->tryDraw(GameFrameworkNx::getAglDrawContext(), variables.mViewport,
                              variables.mRenderBuffer);
        tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(), agl::cShaderMode_UniformBlock);
        variables.bindRenderBuffer();

        if (mIsDrawEffect) {
            tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(),
                                agl::cShaderMode_UniformBlock);
            pKit->getEffectSystem()->getShaderHolder()->setupTextureDepth(
                gBufferArray.getGBufDepthViewTex());
            alEffectSystemFunction::drawEffectPostEffectBackground(
                pKit->getEffectSystem(), projMtx, viewMtx, near, far, fovy);
        }

        mSimpleModelEnv->prepareModelDraw(variables.mViewIndex);
        variables.bindRenderBuffer();
        executeDraw(pKit, "３Ｄ（フォワードプレイヤー）");
        shaderMode = agl::cShaderMode_UniformBlock;
    }

    if (mIsDrawEffect) {
        variables.bindRenderBuffer();
        tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(), agl::cShaderMode_UniformBlock);
        pKit->getEffectSystem()->getShaderHolder()->setupTextureDepth(
            gBufferArray.getGBufDepthViewTex());
        alEffectSystemFunction::drawEffectPostEffect(pKit->getEffectSystem(), projMtx, viewMtx,
                                                     near, far, fovy);
    }

    tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(), agl::cShaderMode_UniformBlock);
    mSimpleModelEnv->prepareModelDraw(variables.mViewIndex);
    variables.bindRenderBuffer();

    sead::Viewport composeViewport = viewport;
    tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(), agl::cShaderMode_UniformBlock);

    if (hdrCompose->isUsingMyHdrCompose()) {
        tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(), agl::cShaderMode_UniformBlock);
        shaderMode = hdrCompose->draw(viewIndex, *pBuffer, composeViewport, *colorTexture,
                                      &mBloom, shaderMode);
    } else {
        mHDRCompose.draw(GameFrameworkNx::getAglDrawContext(), viewIndex, *pBuffer,
                         composeViewport, *colorTexture);
    }

    hdrCompose->releaseComposeBuffer();

    if (mGraphicsSystemInfo->mEdgeDrawer != nullptr) {
        tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(), agl::cShaderMode_UniformBlock);
        EdgeDrawer* edgeDrawer = mGraphicsSystemInfo->mEdgeDrawer;
        const agl::TextureData* nrmViewTexture = gBufferArray.getGBufNrmViewTex();
        const agl::TextureData* depthViewTexture = gBufferArray.getGBufDepthViewTex();
        shaderMode = edgeDrawer->draw(pBuffer, pBuffer, nrmViewTexture, depthViewTexture,
                                      shaderMode, viewIndex, pCameraInfo->mProjection->getNear(),
                                      pCameraInfo->mProjection->getFar());
    }

    if (mBloom.isEnable()) {
        mBloom.releaseBloomBuffer(variables.mViewIndex);
    }

    OccludedEffectDirector* occludedEffectDirector = mGraphicsSystemInfo->mOccludedEffectDirector;

    if (occludedEffectDirector != nullptr && !isGameOver) {
        shaderMode = occludedEffectDirector->draw(variables.mViewIndex, *pBuffer, composeViewport,
                                                  variables.mDepthTarget, shaderMode);
    }

    if (variables._6c0) {
        ShaderMirrorDirector* mirrorDirector = mGraphicsSystemInfo->mShaderMirrorDirector;

        if (mirrorDirector->isEnable()) {
            mirrorDirector->freeRenderBuffer();
        }
    }

    agl::pfx::FilterAA& filterAA = mGraphicsSystemInfo->mFilterAA;

    if (mIsForceFilterAA) {
        filterAA.setEnable(true);
        filterAA.setAlphaOut(1.0f);
    }

    if (filterAA.isEnable() && filterAA.getAlphaOut() > 0.0f) {
        agl::TextureSampler sampler;
        sampler.setFilterDirect(1, 1, 0);
        sampler.applyTextureData(*pBuffer->getRenderTargetColor());

        sead::Viewport filterViewport = viewport;
        filterAA.draw(GameFrameworkNx::getAglDrawContext(), viewIndex, *pBuffer, filterViewport,
                      &sampler, nullptr, false, nullptr);
    }

    pBuffer->bind(GameFrameworkNx::getDrawContext());

    RadialBlurDirector* radialBlurDirector = mGraphicsSystemInfo->mRadialBlurDirector;

    if (radialBlurDirector != nullptr) {
        shaderMode = radialBlurDirector->draw(viewIndex, *pBuffer, shaderMode);
    }

    PostProcessingFilter* postProcessingFilter = mGraphicsSystemInfo->mPostProcessingFilter;

    if (postProcessingFilter != nullptr) {
        postProcessingFilter->updateViewGpu(
            viewIndex, static_cast<const sead::Camera*>(variables._6b0),
            static_cast<const sead::PerspectiveProjection*>(variables._6b8));
        mSimpleModelEnv->prepareModelDraw(variables.mViewIndex);
        postProcessingFilter->drawFilter(
            GameFrameworkNx::getAglDrawContext(), viewIndex, mSimpleModelEnv, *pBuffer,
            *depthTexture, *gBufferArray.getGBufDepthViewTex(), hdrTexture,
            *gBufferArray.getGBufAlbedoTex(), *gBufferArray.getGBufNrmViewTex(),
            *static_cast<const sead::Camera*>(variables._6b0),
            *static_cast<const sead::PerspectiveProjection*>(variables._6b8), near, far, true);
    }

    if (hdrTexture != nullptr) {
        agl::utl::DynamicTextureAllocator::instance()->free(hdrTexture);
    }

    pBuffer->bind(GameFrameworkNx::getDrawContext());
    viewport.apply(GameFrameworkNx::getDrawContext(), *pBuffer);
    return shaderMode;
}

/**
 * Renders the mirror view into the mirror render buffer.
 * @param viewIndex The index of the view.
 * @param pVariables The render variables of the main view.
 * @param isPlayerEffect Whether to draw the player-only effect passes in the HDR pass.
 * @param shaderMode The current shader mode.
 * @return The shader mode after drawing.
 */
agl::ShaderMode ViewRenderer::drawMirror(s32 viewIndex, RenderVariables* pVariables,
                                         bool isPlayerEffect,
                                         agl::ShaderMode shaderMode) const {
    ShaderMirrorDirector* mirrorDirector = mGraphicsSystemInfo->mShaderMirrorDirector;

    if (!mirrorDirector->isEnable()) {
        return shaderMode;
    }

    const sead::LookAtCamera* camera = mirrorDirector->getRenderingCamera();

    if (camera == nullptr) {
        return shaderMode;
    }

    sead::Vector2i size;
    mirrorDirector->calcMirrorRenderBufferSize(&size);
    tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(), agl::cShaderMode_UniformBlock);

    f32 nearOffset = mirrorDirector->getRenderingCameraNearOffset();
    sead::PerspectiveProjection* projection =
        static_cast<sead::PerspectiveProjection*>(pVariables->_6b8);
    f32 near = projection->getNear();
    static_cast<sead::PerspectiveProjection*>(pVariables->_6b8)->getFar();
    projection->setNear(nearOffset + near);

    const sead::Matrix44f& projectionMtx = projection->getProjectionMatrix();
    LiveActorKit* kit = pVariables->getLiveActorKit();
    mGraphicsSystemInfo->updateViewVolume(camera->getMatrix(), projectionMtx);

    DepthShadowDrawer* depthShadowDrawer = getDepthShadowDrawer(kit);

    if (depthShadowDrawer->isEnable()) {
        depthShadowDrawer->setupDepthShadow(1);
    }

    GraphicsStressDirector* stressDirector = mGraphicsSystemInfo->getGraphicsStressDirector();
    agl::ShaderMode newShaderMode;

    {
        RenderVariables variables(mGraphicsSystemInfo, pVariables->getLiveActorKit(),
                                  mSimpleModelEnv, 1, size.x, size.y, false, false);
        variables.mIsFast = false;
        mirrorDirector->allocRenderBuffer();

        const agl::RenderBuffer& mirrorBuffer = mirrorDirector->getRenderBuffer();

        {
            agl::DrawContext* drawContext = GameFrameworkNx::getAglDrawContext();
            sead::Viewport viewport(mirrorBuffer);
            mirrorBuffer.fastClear(drawContext, 0, 1, sead::Color4f::cBlue, 1.0f, 0, viewport,
                                   true);
        }

        variables.mRenderBuffer = mirrorBuffer;
        variables.mDepthTarget = *mirrorBuffer.getRenderTargetDepth();
        variables.mDepthTexture = mirrorDirector->getDepthTexture();
        variables.mColorTarget = *mirrorBuffer.getRenderTargetColor();
        variables.mColorTexture = mirrorDirector->getColorTexture();

        GBufferArray gBufferArray(&variables.mDepthTarget, mGraphicsSystemInfo,
                                  stressDirector->getCurrentParam().isUsingLppSpcMask(),
                                  mGraphicsSystemInfo->getInitArg()._f);
        gBufferArray.allocGBuffer(1);
        variables._28 = &gBufferArray;
        variables._6b8 = projection;
        variables._6b0 = const_cast<sead::LookAtCamera*>(camera);
        gBufferArray.createLightBufferAndCalcContext(
            mGraphicsSystemInfo->getPrePassLightKeeper()->getLightPrePass(), 1, variables.mWidth,
            variables.mHeight, *camera, *projection,
            stressDirector->isForceStressOff() ||
                stressDirector->getCurrentParam().isClearLightBuffer());

        mirrorDirector->startRendering();
        mGraphicsSystemInfo->updateViewGpu(
            1, static_cast<const sead::Camera*>(variables._6b0),
            static_cast<const sead::PerspectiveProjection*>(variables._6b8));

        const agl::TextureData* hdrTexture = drawHdr(1, variables, true, true, isPlayerEffect,
                                                     agl::cShaderMode_UniformBlock);

        if (hdrTexture != nullptr) {
            agl::utl::DynamicTextureAllocator::instance()->free(hdrTexture);
        }

        newShaderMode = mirrorDirector->endRendering(agl::cShaderMode_UniformBlock);
    }

    projection->setNear(near);
    pVariables->_6c0 = true;
    return newShaderMode;
}

/**
 * Draws the system passes: color correction map, depth shadow and cube maps.
 * @param pKit The live actor kit of the scene.
 * @param shaderMode The current shader mode.
 * @return The shader mode after drawing.
 */
agl::ShaderMode ViewRenderer::drawSystem(LiveActorKit* pKit, agl::ShaderMode shaderMode) const {
    const ColorCorrectionParamKeeper* colorCorrectionKeeper =
        mGraphicsSystemInfo->mColorCorrectionParamKeeper;

    if (colorCorrectionKeeper != nullptr && colorCorrectionKeeper->getParam() != nullptr) {
        colorCorrectionKeeper->getParam()->drawMap(GameFrameworkNx::getAglDrawContext());

        sead::GraphicsContext context;
        context.setDepthEnable(false, false);
        context.setColorMask(0);
        context.setBlendEnableMask(0);
        context.apply(GameFrameworkNx::getDrawContext());
    }

    tryChangeShaderMode(GameFrameworkNx::getAglDrawContext(), agl::cShaderMode_UniformBlock);

    agl::ShaderMode newShaderMode = agl::cShaderMode_UniformBlock;
    DepthShadowDrawer* depthShadowDrawer =
        mGraphicsSystemInfo->getShadowDirector()->mDepthShadowDrawer;

    if (depthShadowDrawer->isEnable()) {
        depthShadowDrawer->setDrawFlags(true, true);
        EffectShaderHolder* effectShaderHolder = pKit->getEffectSystem()->getShaderHolder();

        if (effectShaderHolder != nullptr) {
            effectShaderHolder->setupTextureCubeMap(mGraphicsSystemInfo->getCubeMapDirector());
        }

        newShaderMode = depthShadowDrawer->drawToDepthShadow(agl::cShaderMode_UniformBlock);
    }

    return mGraphicsSystemInfo->getCubeMapDirector()->renderToCubeMap(newShaderMode);
}

/**
 * Enables screen space reflections.
 */
void ViewRenderer::enableSSR() {
    mSSR->setEnable(true);
}

}  // namespace al
