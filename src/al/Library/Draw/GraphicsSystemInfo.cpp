#include "Library/Draw/GraphicsSystemInfo.hpp"

#include <common/aglRenderBuffer.h>
#include <common/aglShaderLocation.h>
#include <common/aglShaderProgram.h>
#include <cull/aglViewFrustumCulling.h>
#include <gfx/seadCamera.h>
#include <gfx/seadGraphics.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadProjection.h>
#include <postfx/aglColorCorrection.h>
#include <postfx/aglFlareFilter.h>
#include <shadow/aglPrimitiveOcclusion.h>
#include <shadow/aglSSAO.h>
#include <utility/aglResParameter.h>

#include "Library/Camera/CameraDirector.hpp"
#include "Library/Debug/Render/RenderBufferAttacher.hpp"
#include "Library/Draw/GraphicsFunction.hpp"
#include "Library/Draw/ViewRenderer.hpp"
#include "Library/Fog/FogDirector.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Light/DirectionalLightKeeper.hpp"
#include "Library/Light/LightIntensityDirector.hpp"
#include "Library/Light/PrePassLightKeeper.hpp"
#include "Library/Light/SSIIKeeper.hpp"
#include "Library/LiveActor/Common/LiveActorKit.hpp"
#include "Library/Memory/Util.hpp"
#include "Library/Model/SimpleModelEnv.hpp"
#include "Library/Play/Draw/PartsGraphics.hpp"
#include "Library/Play/Graphics/NoiseTextureKeeper.hpp"
#include "Library/PostProcessing/DepthOfFieldDrawer.hpp"
#include "Library/PostProcessing/FlareFilterDirector.hpp"
#include "Library/PostProcessing/GodRayDirector.hpp"
#include "Library/PostProcessing/HdrCompose.hpp"
#include "Library/PostProcessing/LightStreakDirector.hpp"
#include "Library/PostProcessing/OccludedEffectDirector.hpp"
#include "Library/PostProcessing/PostProcessingFilter.hpp"
#include "Library/PostProcessing/RadialBlurDirector.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/AtmosScatter.hpp"
#include "Library/Shader/DeferredRendering/AtmosScatterDrawer.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDirector.hpp"
#include "Library/Shader/DeferredRendering/FullScreenQuadModel.hpp"
#include "Library/Shader/DeferredRendering/FullScreenTriangle.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Library/Shader/DeferredRendering/ModelLightParam.hpp"
#include "Library/Shader/ForwardRendering/CubeMapKeeper.hpp"
#include "Library/Shader/ForwardRendering/ShaderEnvTextureKeeper.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Library/Shader/ForwardRendering/ShaderMirrorDirector.hpp"
#include "Library/Shader/ForwardRendering/SkyboxDirector.hpp"
#include "Library/Shadow/ShadowDirector.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GpuMemAllocator.hpp"
#include "Project/Draw/GraphicsAreaDirector.hpp"
#include "Project/Draw/GraphicsParamKeeper.hpp"
#include "Project/Draw/GraphicsStressDirector.hpp"
#include "Project/PostProcessing/EdgeDrawer.hpp"

namespace {
using namespace al;

const UniformBlockLayout cFarClearUboLayout[] = {
    {0, agl::UniformBlock::cType_Vec4, 1},
};

const UniformBlockLayout cLightEnvUboLayout[] = {
    {0, agl::UniformBlock::cType_Float, 1},
    {1, agl::UniformBlock::cType_Float, 1},
};

const UniformBlockLayout cLightEnvExUboLayout[] = {
    {0, agl::UniformBlock::cType_Float, 1}, {1, agl::UniformBlock::cType_Float, 1},
    {2, agl::UniformBlock::cType_Float, 1}, {3, agl::UniformBlock::cType_Float, 1},
    {4, agl::UniformBlock::cType_Float, 1},
};

/**
 * Deletes an owned object and clears the pointer to it.
 * @param rPtr pointer to the object, set to nullptr afterwards
 */
template <typename T>
void deleteAndClear(T*& rPtr) {
    if (rPtr != nullptr) {
        delete rPtr;
        rPtr = nullptr;
    }
}

/**
 * Creates the default init arg, which owns a default view renderer creator.
 * @return the default init arg
 */
GraphicsInitArg createDefaultInitArg() {
    GraphicsInitArg arg;
    arg.mViewRendererCreator = new ViewRendererCreator();
    arg.setViewNum(1);
    return arg;
}

}  // namespace

namespace al {

/**
 * Checks whether the atmos scatter is also rendered into a cube map.
 * @return true if atmos scatter is enabled and uses a cube map
 */
bool GraphicsInitArg::isUsingCubeMapAtmosScatter() const {
    if (mAtmosScatterType == 0) {
        return false;
    }

    return mIsUsingCubeMapAtmosScatter;
}

/**
 * Gets the number of views the atmos scatter renders; a cube map adds its six faces.
 * @return the atmos scatter view count
 */
s32 GraphicsInitArg::getAtmosScatterViewNum() const {
    if (isUsingCubeMapAtmosScatter()) {
        return _20 + 6;
    }

    return _20;
}

/**
 * Constructs the graphics system info; the directors themselves are created in init().
 * @param pStageName name of the stage the graphics belong to
 */
GraphicsSystemInfo::GraphicsSystemInfo(const char* pStageName)
    : mInitArg(createDefaultInitArg()), mCubeMapDirector(nullptr),
      mDirectionalLightKeeper(nullptr), mSkyboxDirector(nullptr), mGraphicsAreaDirector(nullptr), mLightIntensityDirector(nullptr),
      mRadialBlurDirector(nullptr), mPrePassLightKeeper(nullptr), mShaderEnvTextureKeeper(nullptr),
      mModelLightDirector(nullptr), mShadowDirector(nullptr), mEdgeDrawer(nullptr),
      mDepthOfFieldDrawer(nullptr), mGraphicsStressDirector(nullptr),
      mShaderMirrorDirector(nullptr), mSSAOParamKeeper(nullptr),
      mColorCorrectionParamKeeper(nullptr), mFlareFilterDirector(nullptr),
      mGodRayDirector(nullptr), mFogDirector(nullptr), mOccludedEffectDirector(nullptr),
      mLightStreakDirector(nullptr), mHdrCompose(nullptr), mSSIIKeeper(nullptr),
      mPrimitiveOcclusion(nullptr), mPostProcessingFilter(nullptr), mNoiseTextureKeeper(nullptr),
      mShaderHolder(nullptr), mAreaTarget(0), mViewRenderer(nullptr), mSimpleModelEnv(nullptr),
      mDrawGBufferArray(nullptr), mDrawCamera(nullptr), mDrawProjection(nullptr),
      mDrawViewIndex(-1), mDrawEnvUpdateCount(0), mDrawCameraPos(0.0f, 0.0f, 0.0f),
      mAtmosScatter(nullptr), mAtmosScatterDrawer(nullptr),
      mFullScreenQuadModel(new FullScreenQuadModel()), mFarClearShader(nullptr),
      mFarClearUbo(createUniformBlock(cFarClearUboLayout, 1, nullptr, 2)),
      mParamFilePath(new GraphicsParamFilePath("GraphicsSystem", "aglgfx")),
      mAtmosScatterType(0, "AtmosScatterType", "大気散乱使うかどうか", &mParamObj),
      mIsUsingUpdateAtmosCubeMap(false, "IsUsingUpdateAtmosCubeMap",
                                 "大気散乱の毎フレーム更新キューブマップを使う", &mParamObj),
      mLightEnvUbo(createUniformBlock(cLightEnvUboLayout, 2, nullptr, 2)),
      mLightEnvExUbo(createUniformBlock(cLightEnvExUboLayout, 5, nullptr, 2)),
      mLightEnvParams{12.0f, 0.16f, 0.5f, 1.0f}, mApplicationMessageReceiver(nullptr),
      mGpuMemAllocator(nullptr), mFullScreenTriangle(nullptr),
      mIsEnableForceCameraAreaFind(false) {
    mShaderHolder = ShaderHolder::sInstance;
    mLodSettingName = pStageName;
    mParamIO.addObj(&mParamObj, "GraphicsSystemParam");
}

/**
 * Destroys every director created by init().
 */
GraphicsSystemInfo::~GraphicsSystemInfo() {
    deleteAndClear(mShaderEnvTextureKeeper);
    deleteAndClear(mModelLightDirector);
    deleteAndClear(mCubeMapDirector);
    deleteAndClear(mShaderMirrorDirector);
    deleteAndClear(mShadowDirector);
    deleteAndClear(mLightIntensityDirector);
    deleteAndClear(mPrePassLightKeeper);

    if (mColorCorrectionParamKeeper != nullptr) {
        delete mColorCorrectionParamKeeper->getParam();
        deleteAndClear(mColorCorrectionParamKeeper);
    }

    if (mSSAOParamKeeper != nullptr) {
        delete mSSAOParamKeeper->getParam();
    }

    deleteAndClear(mFlareFilterDirector);
    deleteAndClear(mRadialBlurDirector);
    deleteAndClear(mSSIIKeeper);
    deleteAndClear(mPrimitiveOcclusion);
    deleteAndClear(mNoiseTextureKeeper);
    deleteAndClear(mPostProcessingFilter);
    deleteAndClear(mOccludedEffectDirector);
    deleteAndClear(mFogDirector);
    deleteAndClear(mLightStreakDirector);
    deleteAndClear(mDepthOfFieldDrawer);
    deleteAndClear(mEdgeDrawer);
    deleteAndClear(mAtmosScatter);
    deleteAndClear(mAtmosScatterDrawer);
    deleteAndClear(mFullScreenQuadModel);
    deleteAndClear(mFarClearUbo);
    deleteAndClear(mParamFilePath);
    deleteAndClear(mDirectionalLightKeeper);
    deleteAndClear(mSkyboxDirector);

    if (mViewRenderer != nullptr) {
        mInitArg.mViewRendererCreator->deleteViewRenderer(mViewRenderer);
        mViewRenderer = nullptr;
    }

    deleteAndClear(mFullScreenTriangle);
    deleteAndClear(mHdrCompose);
    deleteAndClear(mGpuMemAllocator);
    deleteAndClear(mSSAOParamKeeper);
    deleteAndClear(mGraphicsStressDirector);
    deleteAndClear(mGodRayDirector);
    deleteAndClear(mLightEnvUbo);
    deleteAndClear(mLightEnvExUbo);
}

/**
 * Gets the shader cube map keeper of the cube map director.
 * @return the shader cube map keeper
 */
ShaderCubeMapKeeper* GraphicsSystemInfo::getShaderCubeMapKeeper() const {
    return mCubeMapDirector->getShaderCubeMapKeeper();
}

/**
 * Looks up the per-view uniform block array registered under a name.
 * @param pName name of the uniform block array
 * @return the registered array, or nullptr if none is registered
 */
const GraphicsSystemInfo::UniformBlockArray*
GraphicsSystemInfo::getViewIndexedUboArray(const char* pName) const {
    ViewIndexedUboArrayTree::Node* node = mViewIndexedUboArrayTree.find(pName);

    if (node == nullptr) {
        return nullptr;
    }

    return node->value();
}

/**
 * Registers a per-view uniform block array under a name.
 * @param pName name of the uniform block array
 * @param pArray the array to register
 */
void GraphicsSystemInfo::setViewIndexedUboArray(const char* pName,
                                                const UniformBlockArray* pArray) {
    mViewIndexedUboArrayTree.insert(pName, pArray);
}

/**
 * Creates the atmos scatter and its drawer if they do not exist yet.
 * @param pKit live actor kit providing the execute director
 */
void GraphicsSystemInfo::initAtmosScatter(LiveActorKit* pKit) {
    if (mAtmosScatter == nullptr) {
        mAtmosScatter =
            new AtmosScatter(this, mInitArg.getAtmosScatterViewNum(), mInitArg.mFar);
    }

    if (mAtmosScatterDrawer == nullptr) {
        mAtmosScatterDrawer = new AtmosScatterDrawer(pKit->getExecuteDirector(), this);
    }
}

/**
 * Creates and initializes every graphics director.
 * @param rArg graphics init arg
 * @param pKit live actor kit the graphics belong to
 */
void GraphicsSystemInfo::init(const GraphicsInitArg& rArg, LiveActorKit* pKit) {
    mIsEnableForceCameraAreaFind = pKit->_a0;
    mInitArg = rArg;
    mViewIndexedUboArrayTree.allocBuffer(4, nullptr);

    mGpuMemAllocator = new GpuMemAllocator(0x10000, 0x10000, 0x10080);

    s32 displayListSize = !rArg._10 ? 0x1400000 : 0xf00000;
    s32 shaderOptionUboSize = !rArg._10 ? 0xd00000 : 0x800000;
    s32 modelUboSize = !rArg._10 ? 0x3a00000 : 0x2600000;
    mGpuMemAllocator->createMemory("ModelUBO", modelUboSize, getCurrentHeap(), 0x100,
                                   agl::MemoryAttribute::_01);
    mGpuMemAllocator->createMemoryWithTmp("DisplayList", displayListSize, 0x1000,
                                          getCurrentHeap(), 4,
                                          agl::MemoryAttribute::_00);
    mGpuMemAllocator->createMemory("ShaderOptionUBO", shaderOptionUboSize, getCurrentHeap(),
                                   0x100, agl::MemoryAttribute::_01);

    mFullScreenTriangle = new FullScreenTriangle();

    SceneCameraInfo* sceneCameraInfo = pKit->getCameraDirector()->getSceneCameraInfo();
    AreaObjDirector* areaObjDirector = pKit->getAreaObjDirector();
    PlayerHolder* playerHolder = pKit->getPlayerHolder();
    ShaderHolder* shaderHolder = ShaderHolder::sInstance;

    mGraphicsAreaDirector = new GraphicsAreaDirector(this);
    mGraphicsAreaDirector->init(areaObjDirector, sceneCameraInfo, playerHolder);
    mSkyboxDirector = new SkyboxDirector(this);
    mDirectionalLightKeeper = new DirectionalLightKeeper(this);
    mShaderEnvTextureKeeper = new ShaderEnvTextureKeeper(this, playerHolder);
    mShaderEnvTextureKeeper->initTexture(shaderHolder);

    if (rArg.mAtmosScatterType == 1) {
        initAtmosScatter(pKit);
    }

    mCubeMapDirector = new CubeMapDirector(this);

    if (rArg.isUsingCubeMapAtmosScatter()) {
        mCubeMapDirector->initByAtmosScatter();
    }

    mCubeMapDirector->initByCapturePoint(playerHolder);
    mModelLightDirector = new ModelLightDirector(this);
    mLightIntensityDirector = new LightIntensityDirector(areaObjDirector, playerHolder);
    mPrePassLightKeeper = new PrePassLightKeeper(this, rArg.getViewNumWithStereo());
    mPrePassLightKeeper->initShader(shaderHolder);
    mDepthOfFieldDrawer = new DepthOfFieldDrawer(this, rArg._20);
    mDepthOfFieldDrawer->init(areaObjDirector, sceneCameraInfo, playerHolder);
    mEdgeDrawer = new EdgeDrawer(this, rArg._20);
    mEdgeDrawer->init(sceneCameraInfo, shaderHolder, areaObjDirector, playerHolder);
    mShadowDirector = new ShadowDirector(rArg.getViewNumWithStereo(), this, pKit);
    mShadowDirector->initShader(shaderHolder);

    mSSAOParamKeeper =
        new SSAOParamKeeper(this, new agl::sdw::SSAO(), "SSAO", "aglssao", 8);
    mSSAOParamKeeper->getParam()->initialize(rArg._20, nullptr);
    mSSAOParamKeeper->getParam()->set_53c(200.0f);
    f32 density = agl::sdw::SSAO::cSSAODefaultDensity;
    s32 samplePairNum = agl::sdw::SSAO::cSSAODefaultSamplePairNum;
    f32 depthOffset = agl::sdw::SSAO::cSSAODefaultDepthOffset;
    mSSAOParamKeeper->getParam()->setSSAOParameter(30000.0f, 200.0f, density, samplePairNum,
                                                   depthOffset);
    mSSAOParamKeeper->getParam()->setAOFar(30000.0f);

    mGraphicsStressDirector = new GraphicsStressDirector(this);
    mRadialBlurDirector = new RadialBlurDirector();
    mRadialBlurDirector->initialize(rArg.getViewNumWithStereo(), 3, nullptr);
    mShaderMirrorDirector = new ShaderMirrorDirector(this, pKit);
    mGodRayDirector = new GodRayDirector(this);
    mFlareFilterDirector = new FlareFilterDirector(rArg._20, this);
    mFogDirector = new FogDirector(this);
    mOccludedEffectDirector = new OccludedEffectDirector(this, rArg.getViewNumWithStereo());
    mLightStreakDirector = new LightStreakDirector(this);
    mHdrCompose = new HdrCompose(rArg.getViewNumWithStereo(), this);
    mSSIIKeeper = new SSIIKeeper(rArg._20, this);
    mPrimitiveOcclusion = new agl::sdw::PrimitiveOcclusion();

    sead::Graphics::instance()->lockDrawContext();
    mPrimitiveOcclusion->init(rArg.getPrimitiveOcclusionArg());
    sead::Graphics::instance()->unlockDrawContext();

    mPrimitiveOcclusion->setShaderSphereAo(shaderHolder->getShaderProgram("OppSphereAo"));
    mPrimitiveOcclusion->setShaderMakeTableSphereDo(
        shaderHolder->getShaderProgram("MakeTableSphereDo"));
    mPrimitiveOcclusion->setShaderSphereDo(shaderHolder->getShaderProgram("OppSphereDo"));

    NoiseTextureKeeper* noiseTextureKeeper = new NoiseTextureKeeper(this, mShaderHolder);
    mNoiseTextureKeeper = noiseTextureKeeper;
    mPostProcessingFilter = new PostProcessingFilter(shaderHolder, noiseTextureKeeper,
                                                     mLightEnvExUbo, mFullScreenTriangle);

    agl::pfx::FilterAA::InitializeArg filterArg = {rArg._20, 0};
    mFilterAA.setType(agl::pfx::FilterAA::cType_FXAA);
    mFilterAA.initialize(filterArg, getCurrentHeap());
    mFilterAA.setEnable(true);

    mFarClearShader = shaderHolder->getShaderProgram("FarClearGBuffer");

    mColorCorrectionParamKeeper = new ColorCorrectionParamKeeper(
        this, new agl::pfx::ColorCorrection(), "ColorCorrection", "aglcc", 9);
    mColorCorrectionParamKeeper->getParam()->initialize(1, getCurrentHeap(), false);
    mColorCorrectionParamKeeper->getParam()->setEnable(false);

    if (rArg.mIsUsingViewRenderer) {
        mViewRenderer = mInitArg.mViewRendererCreator->createViewRenderer(this);
        mSimpleModelEnv = mViewRenderer->getSimpleModelEnv();
    } else {
        mSimpleModelEnv = new SimpleModelEnv();
        mSimpleModelEnv->initialize(rArg.getViewNumWithStereo() + 6, this, nullptr);
    }

    initProjectResource();
}

/**
 * Initializes the project resources of the post processing filter.
 */
void GraphicsSystemInfo::initProjectResource() {
    if (mPostProcessingFilter != nullptr) {
        mPostProcessingFilter->initProjectResource();
    }
}

/**
 * Loads the stage graphics parameters into every director.
 * @param pResource stage resource
 * @param pStageName name of the stage
 * @param pKit live actor kit of the stage
 * @param isSkipAreaParam whether only the cube map and light resources are loaded
 * @param scenarioNo scenario number of the stage
 */
void GraphicsSystemInfo::initStageResource(const Resource* pResource, const char* pStageName,
                                           LiveActorKit* pKit, bool isSkipAreaParam,
                                           s32 scenarioNo) {
    StringTmp<256> path;
    mParamFilePath->makeBinaryPath(&path);

    if (pResource != nullptr && pResource->isExistFile(path)) {
        const void* file = pResource->getOtherFile(path, nullptr);
        mParamIO.applyResParameterArchive(agl::utl::ResParameterArchive(file));

        if (*mAtmosScatterType == 1) {
            mInitArg.mAtmosScatterType = *mAtmosScatterType;
            mInitArg.mIsUsingCubeMapAtmosScatter = *mIsUsingUpdateAtmosCubeMap;
            initAtmosScatter(pKit);

            if (mInitArg.isUsingCubeMapAtmosScatter()) {
                mCubeMapDirector->initByAtmosScatter();
            }
        }
    }

    if (!isSkipAreaParam) {
        if (mGraphicsAreaDirector != nullptr) {
            mGraphicsAreaDirector->setStageName(pStageName, scenarioNo);
        }

        if (mDepthOfFieldDrawer != nullptr) {
            mDepthOfFieldDrawer->setStageName(pStageName);
        }
    }

    if (mCubeMapDirector != nullptr) {
        mCubeMapDirector->initStageResource(pResource, pStageName, pKit);
    }

    if (mDirectionalLightKeeper != nullptr) {
        mDirectionalLightKeeper->initStageResource(pResource, pStageName);
    }

    if (isSkipAreaParam) {
        return;
    }

    if (mSkyboxDirector != nullptr) {
        mSkyboxDirector->initStageResource(pResource, pStageName);
    }

    if (mShadowDirector != nullptr) {
        mShadowDirector->initStageResource(pResource, pStageName);
    }

    if (mGraphicsStressDirector != nullptr) {
        mGraphicsStressDirector->initStageResource(pResource, pStageName);
    }

    if (mShaderMirrorDirector != nullptr) {
        mShaderMirrorDirector->initStageResource(pResource, pStageName);
    }

    if (mSSAOParamKeeper != nullptr) {
        mSSAOParamKeeper->initStageResource(pResource, pStageName);
    }

    if (mColorCorrectionParamKeeper != nullptr) {
        mColorCorrectionParamKeeper->initStageResource(pResource, pStageName);
    }

    if (mGodRayDirector != nullptr) {
        mGodRayDirector->initStageResource(pResource, pStageName);
    }

    if (mLightStreakDirector != nullptr) {
        mLightStreakDirector->initStageResource(pResource, pStageName);
    }

    if (mFlareFilterDirector != nullptr) {
        mFlareFilterDirector->initStageResource(pResource, pStageName);
    }

    if (mFogDirector != nullptr) {
        mFogDirector->initStageResource(pResource, pStageName);
    }

    if (mHdrCompose != nullptr) {
        mHdrCompose->initStageResource(pResource, pStageName);
    }

    if (mSSIIKeeper != nullptr) {
        mSSIIKeeper->initStageResource(pResource, pStageName);
    }

    if (mAtmosScatter != nullptr) {
        mAtmosScatter->initStageResource(pResource, pStageName);
    }
}

/**
 * Finishes the initialization of every director once the stage is placed.
 */
void GraphicsSystemInfo::endInit() {
    if (mGraphicsAreaDirector != nullptr) {
        mGraphicsAreaDirector->initAfterPlacement();

        if (mLodSettingName != nullptr) {
            if (mLightIntensityDirector != nullptr) {
                mLightIntensityDirector->initGraphicsAreaParam(mGraphicsAreaDirector,
                                                               mLodSettingName);
            }

            if (mShaderEnvTextureKeeper != nullptr) {
                mShaderEnvTextureKeeper->initGraphicsAreaParam(mGraphicsAreaDirector,
                                                               mLodSettingName);
            }
        }
    }

    if (mDepthOfFieldDrawer != nullptr) {
        mDepthOfFieldDrawer->initAfterPlacement();
    }

    if (mDirectionalLightKeeper != nullptr) {
        mDirectionalLightKeeper->endInit();
    }

    if (mPrePassLightKeeper != nullptr) {
        mPrePassLightKeeper->endInit();
    }

    if (mLightIntensityDirector != nullptr) {
        mLightIntensityDirector->endInit();
    }

    if (mShadowDirector != nullptr) {
        mShadowDirector->endInit();
    }

    if (mShaderEnvTextureKeeper != nullptr) {
        mShaderEnvTextureKeeper->endInit();
    }

    if (mCubeMapDirector != nullptr) {
        mCubeMapDirector->endInit();
    }

    if (mGraphicsAreaDirector != nullptr) {
        mGraphicsAreaDirector->endInit();
    }

    if (mGraphicsStressDirector != nullptr) {
        mGraphicsStressDirector->endInit();
    }

    if (mShaderMirrorDirector != nullptr) {
        mShaderMirrorDirector->endInit();
    }

    if (mGodRayDirector != nullptr) {
        mGodRayDirector->endInit();
    }

    if (mLightStreakDirector != nullptr) {
        mLightStreakDirector->endInit();
    }

    if (mFogDirector != nullptr) {
        mFogDirector->endInit();
    }

    if (mHdrCompose != nullptr) {
        mHdrCompose->endInit();
    }

    if (mSSIIKeeper != nullptr) {
        mSSIIKeeper->endInit();
    }

    if (mModelLightDirector != nullptr) {
        mModelLightDirector->endInit();
    }

    if (mPostProcessingFilter != nullptr) {
        mPostProcessingFilter->endInit();
    }

    if (mNoiseTextureKeeper != nullptr) {
        mNoiseTextureKeeper->endInit();
    }
}

/**
 * Sets the view currently being drawn.
 * @param viewIndex index of the view
 * @param pGBufferArray g-buffer of the view
 * @param pCamera camera of the view
 * @param pProjection projection of the view
 */
void GraphicsSystemInfo::setDrawEnv(s32 viewIndex, GBufferArray* pGBufferArray,
                                    const sead::Camera* pCamera,
                                    const sead::PerspectiveProjection* pProjection) {
    mDrawViewIndex = viewIndex;
    mDrawGBufferArray = pGBufferArray;
    mDrawCamera = static_cast<sead::LookAtCamera*>(const_cast<sead::Camera*>(pCamera));
    mDrawProjection = const_cast<sead::PerspectiveProjection*>(pProjection);
    mDrawEnvUpdateCount++;
    pCamera->getWorldPosByMatrix(&mDrawCameraPos);
}

/**
 * Clears the graphics parameter requests made this frame.
 */
void GraphicsSystemInfo::clearGraphicsRequest() {
    if (mDirectionalLightKeeper != nullptr) {
        mDirectionalLightKeeper->clearRequest();
    }

    if (mPrePassLightKeeper != nullptr) {
        mPrePassLightKeeper->clear();
    }

    if (mGodRayDirector != nullptr) {
        mGodRayDirector->clearRequest();
    }

    if (mLightStreakDirector != nullptr) {
        mLightStreakDirector->clearRequest();
    }

    if (mFogDirector != nullptr) {
        mFogDirector->clear();
    }

    if (mOccludedEffectDirector != nullptr) {
        mOccludedEffectDirector->clear();
    }

    if (mGraphicsStressDirector != nullptr) {
        mGraphicsStressDirector->clearRequest();
    }

    if (mSSIIKeeper != nullptr) {
        mSSIIKeeper->clearRequest();
    }

    if (mHdrCompose != nullptr) {
        mHdrCompose->clearRequest();
    }

    if (mShadowDirector != nullptr) {
        mShadowDirector->clear();
    }

    if (mShaderMirrorDirector != nullptr) {
        mShaderMirrorDirector->clearRequest();
    }

    if (mPrimitiveOcclusion != nullptr) {
        mPrimitiveOcclusion->clearRequest();
    }

    if (mPostProcessingFilter != nullptr) {
        mPostProcessingFilter->clearRequest();
    }
}

/**
 * Cancels the running interpolations of the area parameters.
 */
void GraphicsSystemInfo::cancelLerp() {
    if (mGraphicsAreaDirector != nullptr) {
        mGraphicsAreaDirector->cancelLerp();
    }

    if (mDepthOfFieldDrawer != nullptr) {
        mDepthOfFieldDrawer->cancelLerp();
    }
}

/**
 * Updates every director once per frame.
 * @param isPaused whether the scene is paused
 */
void GraphicsSystemInfo::updateGraphics(bool isPaused) {
    if (mGraphicsAreaDirector != nullptr) {
        mGraphicsAreaDirector->update();
    }

    if (mShaderEnvTextureKeeper != nullptr) {
        mShaderEnvTextureKeeper->execute();
    }

    if (mDirectionalLightKeeper != nullptr) {
        mDirectionalLightKeeper->execute();
    }

    if (mPrePassLightKeeper != nullptr) {
        mPrePassLightKeeper->execute(mShadowDirector, isPaused);
    }

    if (mLightIntensityDirector != nullptr) {
        mLightIntensityDirector->execute();
    }

    if (mDepthOfFieldDrawer != nullptr) {
        mDepthOfFieldDrawer->update(isPaused);
    }

    if (mEdgeDrawer != nullptr) {
        mEdgeDrawer->update();
    }

    if (mShadowDirector != nullptr) {
        mShadowDirector->update();
    }

    if (mSSAOParamKeeper != nullptr) {
        mSSAOParamKeeper->updateRequest();
    }

    if (mColorCorrectionParamKeeper != nullptr) {
        mColorCorrectionParamKeeper->updateRequest();
    }

    if (mGodRayDirector != nullptr) {
        mGodRayDirector->updateRequest();
    }

    if (mLightStreakDirector != nullptr) {
        mLightStreakDirector->updateRequest();
    }

    if (mFogDirector != nullptr) {
        mFogDirector->updateRequest();
    }

    if (mGraphicsStressDirector != nullptr) {
        mGraphicsStressDirector->movement();
    }

    if (mFlareFilterDirector != nullptr) {
        mFlareFilterDirector->movement();
    }

    if (mSSIIKeeper != nullptr) {
        mSSIIKeeper->movement();
    }

    if (mHdrCompose != nullptr) {
        mHdrCompose->movement(isPaused);
    }

    if (mShadowDirector != nullptr) {
        mShadowDirector->movement();
    }

    if (mShaderMirrorDirector != nullptr) {
        mShaderMirrorDirector->movement();
    }

    if (mOccludedEffectDirector != nullptr) {
        mOccludedEffectDirector->movement();
    }

    ShaderCubeMapKeeper* cubeMapKeeper = getShaderCubeMapKeeper();

    if (cubeMapKeeper != nullptr) {
        cubeMapKeeper->updateCubeMapKeeper();
    }

    if (mAtmosScatterDrawer != nullptr) {
        mAtmosScatterDrawer->execute();
    }

    if (mAtmosScatter != nullptr) {
        mAtmosScatter->updateAtmosScatter();
    }

    if (mRadialBlurDirector != nullptr) {
        mRadialBlurDirector->control();
    }

    if (mPostProcessingFilter != nullptr) {
        mPostProcessingFilter->update();
    }
}

/**
 * Prepares every director for drawing the frame.
 * @param pCameraInfo scene camera info
 */
void GraphicsSystemInfo::preDrawGraphics(const SceneCameraInfo* pCameraInfo) {
    if (!mInitArg.mIsUsingViewRenderer) {
        mSimpleModelEnv->swapBuffer();
    }

    if (mCubeMapDirector != nullptr) {
        mCubeMapDirector->preDrawGraphics();
    }

    if (mShaderEnvTextureKeeper != nullptr) {
        mShaderEnvTextureKeeper->updateEnvTexture();
    }

    if (mShadowDirector != nullptr) {
        mShadowDirector->preDrawGraphics();
    }

    if (mHdrCompose != nullptr) {
        mHdrCompose->preDrawGraphics();
    }

    if (mPrePassLightKeeper != nullptr) {
        mPrePassLightKeeper->preDrawGraphics();
    }

    if (mAtmosScatter != nullptr) {
        mAtmosScatter->preDrawGraphics();
    }

    mDrawGBufferArray = nullptr;
    mDrawCamera = nullptr;
    mDrawProjection = nullptr;
    mDrawViewIndex = -1;
    mViewVolume.ClearPlanes();

    if (mGraphicsStressDirector != nullptr) {
        mFilterAA.setEnable(
            mGraphicsStressDirector->getCurrentParam().getAntiAliasingType() == 0 &&
            mGraphicsStressDirector->getFXAAAlphaOut() != 0.0f);
        f32 alphaOut = mGraphicsStressDirector->getFXAAAlphaOut();
        sead::Color4f lumaCoeff = mFilterAA.getLumaCoeff();
        mFilterAA.setFXAAParam(
            alphaOut,
            mGraphicsStressDirector->getCurrentParam().getAntiAliasDetectEdgeQuality(),
            lumaCoeff, 5.0f);
    }

    f32 near = mInitArg.mNear;
    mLightEnvUbo->setValue(0, 1.0f / (near * near));
    mLightEnvUbo->setValue(1, mLightEnvParams[0]);
    near = mInitArg.mNear;
    mLightEnvExUbo->setValue(0, 1.0f / (near * near));
    mLightEnvExUbo->setValue(1, mLightEnvParams[0]);
    mLightEnvExUbo->setValue(2, mLightEnvParams[1]);
    mLightEnvExUbo->setValue(3, mLightEnvParams[2]);
    mLightEnvExUbo->setValue(4, mLightEnvParams[3]);

    if (mViewRenderer != nullptr) {
        mViewRenderer->preDrawGraphics(pCameraInfo);
    }
}

/**
 * Updates the GPU side view data of every director.
 * @param viewIndex index of the view
 * @param pCamera camera of the view
 * @param pProjection projection of the view
 */
void GraphicsSystemInfo::updateViewGpu(s32 viewIndex, const sead::Camera* pCamera,
                                       const sead::PerspectiveProjection* pProjection) {
    const sead::Matrix34f& viewMtx = pCamera->getMatrix();
    const sead::Matrix44f& projMtx = pProjection->getProjectionMatrix();
    agl::cull::ViewFrustumCulling culling;
    culling.update(viewMtx, *pProjection);

    if (mFlareFilterDirector != nullptr) {
        mFlareFilterDirector->getParam()->calcView(viewIndex, culling);
    }

    if (mFogDirector != nullptr) {
        mFogDirector->updateCamera(pCamera);
    }

    if (mOccludedEffectDirector != nullptr) {
        mOccludedEffectDirector->calcView(viewIndex, pCamera, pProjection);
    }

    if (mSSIIKeeper != nullptr) {
        mSSIIKeeper->updateViewGPU(viewIndex, pCamera, pProjection);
    }

    if (mHdrCompose != nullptr) {
        mHdrCompose->calcGPU();
    }

    if (mPrimitiveOcclusion != nullptr) {
        mPrimitiveOcclusion->calcView(viewIndex, pCamera, pProjection);
    }

    if (mRadialBlurDirector != nullptr) {
        mRadialBlurDirector->calcView(viewIndex, viewMtx, projMtx);
    }

    if (mShadowDirector != nullptr) {
        mShadowDirector->updateViewGpu(viewIndex, pCamera, pProjection);
    }

    if (mPostProcessingFilter != nullptr) {
        mPostProcessingFilter->updateViewGpu(viewIndex, pCamera, pProjection);
    }
}

/**
 * Recalculates the view volume used for culling.
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 */
void GraphicsSystemInfo::updateViewVolume(const sead::Matrix34f& rViewMtx,
                                          const sead::Matrix44f& rProjMtx) {
    calcViewVolume(&mViewVolume, rViewMtx, rProjMtx);
}

/**
 * Gets the light direction of the atmos scatter.
 * @param pDir receives the light direction
 * @return true if an atmos scatter exists
 */
bool GraphicsSystemInfo::tryGetAtmosLightDir(sead::Vector3f* pDir) const {
    if (mAtmosScatter == nullptr) {
        return false;
    }

    mAtmosScatter->calcInfo(nullptr, nullptr, pDir);
    return true;
}

/**
 * Registers a parts graphics so it is updated and drawn with the system.
 * @param pPartsGraphics parts graphics to register
 * @return always true
 */
bool GraphicsSystemInfo::registPartsGraphics(PartsGraphics* pPartsGraphics) {
    mPartsGraphicsList.pushBack(pPartsGraphics);
    return true;
}

/**
 * Gets the direction (and intensity) of a directional light; the atmos scatter overrides the
 * direction if it exists.
 * @param pDir receives the light direction
 * @param pName name of the light, or nullptr for the current light
 * @param pIntensity receives the light intensity, can be nullptr
 * @return true if a light was found
 */
bool GraphicsSystemInfo::tryDirectionalLightInfo(sead::Vector3f* pDir, const char* pName,
                                                 f32* pIntensity) const {
    bool isFound = false;

    if (mDirectionalLightKeeper != nullptr) {
        const DirLightParam* param =
            pName != nullptr ? mDirectionalLightKeeper->tryGetNamedOrDefaultDirLight(pName) :
                               &mDirectionalLightKeeper->getCurrentParam();
        const sead::Vector3f& dir = param->getDirectionFrom();
        pDir->set(-dir.x, -dir.y, -dir.z);

        if (pIntensity != nullptr) {
            *pIntensity = param->getSpcPower();
        }

        isFound = true;
    }

    return tryGetAtmosLightDir(pDir) | isFound;
}

/**
 * Clears the far area of the current g-buffer.
 * @param shaderMode current shader mode
 * @return the shader mode after drawing
 */
agl::ShaderMode GraphicsSystemInfo::drawFarClearGBuffer(agl::ShaderMode shaderMode) const {
    GBufferArray* gBufferArray = mDrawGBufferArray;
    agl::RenderBuffer renderBuffer;
    RenderBufferAttacher attacher(&renderBuffer, gBufferArray->getGBufAlbedoTex(),
                                  gBufferArray->getGBufNrmViewTex(),
                                  gBufferArray->getGBufDepthViewTex(), nullptr, nullptr);
    return drawFarClear(shaderMode, true, sead::Color4f::cBlack);
}

/**
 * Clears the far area of the bound render buffer.
 * @param shaderMode current shader mode
 * @param isGBuffer whether the g-buffer variation is used
 * @param rColor clear color used when not clearing a g-buffer
 * @return the shader mode after drawing
 */
agl::ShaderMode GraphicsSystemInfo::drawFarClear(agl::ShaderMode shaderMode, bool isGBuffer,
                                                 const sead::Color4f& rColor) const {
    const char* macros[] = {"CLEAR_TYPE"};
    const char* values[] = {"0"};

    if (!isGBuffer) {
        values[0] = "1";
    }

    const agl::ShaderProgram* program = mFarClearShader->searchVariation(1, macros, values);
    program->activate(GameFrameworkNx::getAglDrawContext(), true);

    if (!isGBuffer) {
        mFarClearUbo->setValue(0, rColor);
        agl::UniformBlockLocation location("FarClearParam");
        location.search(*program);
        mFarClearUbo->activate(GameFrameworkNx::getAglDrawContext(), location);
        mFarClearUbo->flushCurrentBuffer();
    }

    sead::GraphicsContext context;
    context.setDepthEnable(true, false);
    context.setDepthFunc(3);
    context.apply(GameFrameworkNx::getAglDrawContext());
    mFullScreenQuadModel->drawQuad();
    return shaderMode;
}

/**
 * Activates the directional light color texture (unused in this game).
 */
void GraphicsSystemInfo::activateDirLitColorTex() const {}

}  // namespace al

namespace alGfxUtil {

/**
 * Warns about light preset settings (empty in release builds).
 * @param pInfo graphics system info
 */
void tryWarningLightPresetSetting(const al::GraphicsSystemInfo* pInfo) {}

}  // namespace alGfxUtil
