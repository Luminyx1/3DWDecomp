#include "Library/PostProcessing/PostProcessingFilter.hpp"

#include <driver/aglGraphicsDriverMgr.h>
#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <nn/g3d/g3d_Resources.h>
#include <nvn/nvn_FuncPtrInline.h>
#include <postfx/aglColorCorrection.h>
#include <utility/aglDynamicTextureAllocator.h>
#include <utility/aglParameterIO.h>
#include <utility/aglParameterObj.h>
#include <utility/aglResParameter.h>

#include "Library/Draw/ViewRendererFunction.hpp"
#include "Library/File/FileUtil.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Memory/Util.hpp"
#include "Library/PostProcessing/DepthOfFieldDrawer.hpp"
#include "Library/PostProcessing/PencilSketchDrawer.hpp"
#include "Library/PostProcessing/RetroColorDrawer.hpp"
#include "Library/PostProcessing/ScreenBlurDrawer.hpp"
#include "Library/PostProcessing/ViewDepthDrawer.hpp"
#include "Library/PostProcessing/VignettingDrawer.hpp"
#include "Library/Projection/Projection.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ParameterBase.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GraphicsParamKeeper.hpp"
#include "Project/PostProcessing/CartoonDrawer.hpp"
#include "Project/PostProcessing/ColorClampDrawer.hpp"
#include "Project/PostProcessing/ContoursDrawer.hpp"
#include "Project/PostProcessing/KaleidoscopeDrawer.hpp"
#include "Project/PostProcessing/MangaDrawer.hpp"
#include "Project/PostProcessing/MetalReliefDrawer.hpp"

namespace {
NVNcommandBuffer* getCommandBuffer() {
    return al::GameFrameworkNx::getDrawContext()->getNvnCommandBuffer();
}

void pushDebugGroup(NVNcommandBuffer* pCmdBuf, const char* pName) {
    reinterpret_cast<void (*)(NVNcommandBuffer*, const char*)>(
        pfnc_nvnCommandBufferPushDebugGroup)(pCmdBuf, pName);
}
}  // namespace

namespace al {
/**
 * Creates the preset parameters and loads them from the preset archive.
 * @param pName Preset name.
 * @param pResource Preset archive.
 * @param pFilter Owning filter.
 */
PostProcessingFilterPreset::PostProcessingFilterPreset(const char* pName,
                                                       const Resource* pResource,
                                                       PostProcessingFilter* pFilter)
    : mFilter(pFilter), mParamObj(new ParameterObj()),
      mNameParam(new ParameterString64(sead::FixedSafeString<64>(pName), mParamObj, "Name",
                                       "プリセット名", "", true)),
      mParamList(new ParameterList()), mAglParamObj(new agl::utl::ParameterObj),
      mParamIo(new agl::utl::IParameterIO()) {
    mName.init(sead::FixedSafeString<64>(pName), "Name", "プリセット名", "", mAglParamObj);

    mDepthOfFieldParam = new DepthOfFieldParam();
    mViewDepthDrawParam = new ViewDepthDrawParam();
    mVignettingParam = new VignettingParam();
    mEdgeDrawParam = new EdgeDrawPostEffectParam();
    mCartoonDrawParam = new CartoonDrawParam();
    mRetroColorDrawParam = new RetroColorDrawParam();
    mScreenBlurDrawParam = new ScreenBlurDrawParam();
    mPencilSketchDrawParam = new PencilSketchDrawParam();
    mColorClampDrawParam = new ColorClampDrawParam();
    mMetalReliefDrawParam = new MetalReliefDrawParam();
    mMosaicPictureDrawParam = new MosaicPictureDrawParam();
    mContoursDrawParam = new ContoursDrawParam();
    mKaleidoscopeParam = new KaleidoscopeParam();
    mMangaDrawParam = new MangaDrawParam();

    mParamList->addObj(mParamObj, "Name");
    mParamList->addObj(mViewDepthDrawParam->getParamObj(), "ViewDepthDraw");
    mParamList->addObj(mVignettingParam->getParamObj(), "VignettingParam");
    mParamList->addObj(mEdgeDrawParam->getParamObj(), "EdgeDraw");
    mParamList->addObj(mCartoonDrawParam->getParamObj(), "CartoonDraw");
    mParamList->addObj(mRetroColorDrawParam->getParamObj(), "RetroColorDraw");
    mParamList->addObj(mScreenBlurDrawParam->getParamObj(), "ScreenBlurDraw");
    mParamList->addObj(mPencilSketchDrawParam->getParamObj(), "PencilSketchDraw");
    mParamList->addObj(mColorClampDrawParam->getParamObj(), "ColorClampDraw");
    mParamList->addObj(mMetalReliefDrawParam->getParamObj(), "MetalReliefDraw");
    mParamList->addObj(mMosaicPictureDrawParam->getParamObj(), "MosaicPictureDraw");
    mParamList->addObj(mContoursDrawParam->getParamObj(), "ContoursDraw");
    mParamList->addObj(mKaleidoscopeParam->getParamObj(), "KaleidoscopeDraw");
    mParamList->addObj(mMangaDrawParam->getParamObj(), "MangaDraw");
    mParamIo->addObj(mAglParamObj, "Name");
    mParamIo->addObj(mDepthOfFieldParam->getParamObj(), "DepthOfField");

    if (isEqualString(pName, "")) {
        return;
    }

    const u8* byml = tryGetByml(pResource, pName);

    if (byml != nullptr) {
        ByamlIter iter(byml);

        if (iter.isValid()) {
            mParamList->tryGetParam(iter);
        }
    }

    StringTmp<256> presetPath("%s.b%s", pName, "aglpreset");

    if (pResource->isExistFile(presetPath)) {
        const void* file = pResource->getOtherFile(presetPath, nullptr);
        mParamIo->applyResParameterArchive(agl::utl::ResParameterArchive(file));
    }

    StringTmp<256> colorCorrectionPath("%s.b%s", pName, "aglcc");

    if (pResource->isExistFile(colorCorrectionPath)) {
        mColorCorrectionData = pResource->getOtherFile(colorCorrectionPath, nullptr);
    }
}

/**
 * Gets the preset name.
 * @return Preset name.
 */
const char* PostProcessingFilterPreset::getName() const {
    return mName->cstr();
}

/**
 * Creates the filter drawers.
 * @param pShaderHolder Shader holder.
 * @param pNoiseTextureKeeper Noise texture keeper.
 * @param pUniformBlock View uniform block.
 * @param pTriangle Screen covering triangle.
 */
PostProcessingFilter::PostProcessingFilter(ShaderHolder* pShaderHolder,
                                           NoiseTextureKeeper* pNoiseTextureKeeper,
                                           UniformBlock* pUniformBlock,
                                           const FullScreenTriangle* pTriangle)
    : mFullScreenTriangle(pTriangle) {
    mColorCorrectionRequester =
        new ColorCorrectionRequester(new agl::pfx::ColorCorrection(), "ColorCorrection");
    mColorCorrectionRequester->getParam()->initialize(1, getCurrentHeap(), false);
    mColorCorrectionRequester->getParam()->setEnable(false);

    mViewDepthDrawer = new ViewDepthDrawer(pShaderHolder);
    mVignettingDrawer = new VignettingDrawer(pShaderHolder, nullptr, nullptr);
    mCartoonDrawer = new CartoonDrawer(pShaderHolder, pNoiseTextureKeeper);
    mEdgeDrawer = new EdgeDrawerPostEffect(pShaderHolder, 1);
    mRetroColorDrawer = new RetroColorDrawer(pShaderHolder);
    mScreenBlurDrawer = new ScreenBlurDrawer(1);
    mPencilSketchDrawer = new PencilSketchDrawer(pShaderHolder);
    mColorClampDrawer = new ColorClampDrawer(pShaderHolder);
    mMetalReliefDrawer = new MetalReliefDrawer(pShaderHolder, pUniformBlock);
    mMosaicPictureDrawer =
        new MosaicPictureDrawer(pShaderHolder, pUniformBlock, pNoiseTextureKeeper);
    mContoursDrawer = new ContoursDrawer(pShaderHolder);
    mKaleidoscopeDrawer = new KaleidoscopeDrawer(pShaderHolder);
    mMangaDrawer = new MangaDrawer(pShaderHolder);
    mPresets.allocBuffer(40, nullptr);
}

/**
 * Destroys the filter drawers and the project texture resource.
 */
PostProcessingFilter::~PostProcessingFilter() {
    if (mDepthOfFieldDrawer != nullptr) {
        delete mDepthOfFieldDrawer;
        mDepthOfFieldDrawer = nullptr;
    }

    if (mColorCorrectionRequester != nullptr) {
        delete mColorCorrectionRequester;
        mColorCorrectionRequester = nullptr;
    }

    if (mViewDepthDrawer != nullptr) {
        delete mViewDepthDrawer;
        mViewDepthDrawer = nullptr;
    }

    if (mCartoonDrawer != nullptr) {
        delete mCartoonDrawer;
        mCartoonDrawer = nullptr;
    }

    if (mEdgeDrawer != nullptr) {
        delete mEdgeDrawer;
        mEdgeDrawer = nullptr;
    }

    if (mRetroColorDrawer != nullptr) {
        delete mRetroColorDrawer;
        mRetroColorDrawer = nullptr;
    }

    if (mScreenBlurDrawer != nullptr) {
        delete mScreenBlurDrawer;
        mScreenBlurDrawer = nullptr;
    }

    if (mPencilSketchDrawer != nullptr) {
        delete mPencilSketchDrawer;
        mPencilSketchDrawer = nullptr;
    }

    if (mVignettingDrawer != nullptr) {
        delete mVignettingDrawer;
        mVignettingDrawer = nullptr;
    }

    if (mColorClampDrawer != nullptr) {
        delete mColorClampDrawer;
        mColorClampDrawer = nullptr;
    }

    if (mMetalReliefDrawer != nullptr) {
        delete mMetalReliefDrawer;
        mMetalReliefDrawer = nullptr;
    }

    if (mMosaicPictureDrawer != nullptr) {
        delete mMosaicPictureDrawer;
        mMosaicPictureDrawer = nullptr;
    }

    if (mContoursDrawer != nullptr) {
        delete mContoursDrawer;
        mContoursDrawer = nullptr;
    }

    if (mKaleidoscopeDrawer != nullptr) {
        delete mKaleidoscopeDrawer;
        mKaleidoscopeDrawer = nullptr;
    }

    if (mMangaDrawer != nullptr) {
        delete mMangaDrawer;
        mMangaDrawer = nullptr;
    }

    if (mResFile != nullptr) {
        mResFile->Cleanup(static_cast<nn::gfx::Device*>(
            agl::driver::GraphicsDriverMgr::instance()->getGfxDevice()));
    }
}

/**
 * Selects the next preset with a name.
 */
void PostProcessingFilter::incrementPreset() {
    s32 nextId = mPresetId + 1;

    for (s32 i = 0; i < mPresets.size(); i++) {
        s32 presetId = (nextId + i) % mPresets.size();

        if (!isEqualString(mPresets.at(presetId)->getName(), "")) {
            mPresetId = presetId;
            return;
        }
    }
}

/**
 * Selects the previous preset with a name.
 */
void PostProcessingFilter::decrementPreset() {
    s32 prevId = mPresetId - 1;

    for (s32 i = 0; i < mPresets.size(); i++) {
        s32 presetId = (mPresets.size() + (prevId - i)) % mPresets.size();

        if (!isEqualString(mPresets.at(presetId)->getName(), "")) {
            mPresetId = presetId;
            return;
        }
    }
}

/**
 * Selects the first preset.
 */
void PostProcessingFilter::resetPreset() {
    mPresetId = 0;
}

/**
 * Selects the preset with the given name.
 * @param pName Preset name.
 */
void PostProcessingFilter::setPresetByName(const char* pName) {
    for (s32 i = 0; i < mPresets.size(); i++) {
        if (isEqualString(pName, mPresets.at(i)->getName())) {
            mPresetId = i;
            return;
        }
    }
}

/**
 * Loads the presets and the project textures of the drawers.
 */
void PostProcessingFilter::initProjectResource() {
    StringTmp<64> presetArchivePath("SystemData/%s", "PostProcessingFilterPreset");

    if (isExistArchive(presetArchivePath.cstr())) {
        Resource* resource = findOrCreateResourceSystemData("PostProcessingFilterPreset", nullptr);
        const u8* byml = tryGetByml(resource, "PostProcessingFilterPresetList");
        ByamlIter iter(byml);

        if (iter.isValid()) {
            s32 presetNum = iter.getSize();

            for (s32 i = 0; i < presetNum; i++) {
                const char* name = nullptr;
                iter.tryGetStringByIndex(&name, i);
                mPresets.pushBack(new PostProcessingFilterPreset(name, resource, this));
            }
        }
    }

    StringTmp<256> textureArchivePath("SystemData/TexturePostProcessFilter");

    if (!isExistArchive(textureArchivePath)) {
        return;
    }

    Resource* resource = findOrCreateResource(textureArchivePath, nullptr);

    if (resource == nullptr) {
        return;
    }

    StringTmp<256> textureFileName("TexturePostProcessFilter.bfres");

    if (!resource->isExistFile(textureFileName)) {
        return;
    }

    mResFile = resource->getResFile();
    mCartoonDrawer->initProjectResource(mResFile);
    mMetalReliefDrawer->initProjectResource(mResFile);
    mMosaicPictureDrawer->initProjectResource(mResFile);
    mMangaDrawer->initProjectResource(mResFile);
}

/**
 * Ends initialization of all parameter interpolations.
 */
void PostProcessingFilter::endInit() {
    mColorCorrectionRequester->endInit();
    mViewDepthDrawer->endInit();
    mVignettingDrawer->endInit();
    mEdgeDrawer->endInit();
    mCartoonDrawer->endInit();
    mRetroColorDrawer->endInit();
    mScreenBlurDrawer->endInit();
    mPencilSketchDrawer->endInit();
    mColorClampDrawer->endInit();
    mMetalReliefDrawer->endInit();
    mMosaicPictureDrawer->endInit();
    mContoursDrawer->endInit();
    mKaleidoscopeDrawer->endInit();
    mMangaDrawer->endInit();
}

/**
 * Clears the parameter requests of the current frame.
 */
void PostProcessingFilter::clearRequest() {
    if (!mIsValid) {
        return;
    }

    mColorCorrectionRequester->clearRequest();
    mViewDepthDrawer->clearRequest();
    mVignettingDrawer->clearRequest();
    mEdgeDrawer->clearRequest();
    mCartoonDrawer->clearRequest();
    mRetroColorDrawer->clearRequest();
    mScreenBlurDrawer->clearRequest();
    mPencilSketchDrawer->clearRequest();
    mColorClampDrawer->clearRequest();
    mMetalReliefDrawer->clearRequest();
    mMosaicPictureDrawer->clearRequest();
    mContoursDrawer->clearRequest();
    mKaleidoscopeDrawer->clearRequest();
    mMangaDrawer->clearRequest();
}

/**
 * Requests the parameters of the selected preset and updates the interpolations.
 */
void PostProcessingFilter::update() {
    if (!mIsValid) {
        return;
    }

    const PostProcessingFilterPreset* preset = findPreset(mPresetId);

    if (preset != nullptr) {
        mViewDepthDrawer->requestParam(-1, 0, preset->getViewDepthDrawParam());
        mVignettingDrawer->requestParam(-1, 0, preset->getVignettingParam());
        mEdgeDrawer->requestParam(-1, 0, preset->getEdgeDrawParam());
        mCartoonDrawer->requestParam(-1, 0, preset->getCartoonDrawParam());
        mRetroColorDrawer->requestParam(-1, 0, preset->getRetroColorDrawParam());
        mScreenBlurDrawer->requestParam(-1, 0, preset->getScreenBlurDrawParam());
        mPencilSketchDrawer->requestParam(-1, 0, preset->getPencilSketchDrawParam());
        mColorClampDrawer->requestParam(-1, 0, preset->getColorClampDrawParam());
        mMetalReliefDrawer->requestParam(-1, 0, preset->getMetalReliefDrawParam());
        mMosaicPictureDrawer->requestParam(-1, 0, preset->getMosaicPictureDrawParam());
        mContoursDrawer->requestParam(-1, 0, preset->getContoursDrawParam());
        mKaleidoscopeDrawer->requestParam(-1, 0, preset->getKaleidoscopeParam());
        mMangaDrawer->requestParam(-1, 0, preset->getMangaDrawParam());

        if (mLastPreset == nullptr || !isEqualString(mLastPreset->getName(), preset->getName())) {
            mColorCorrectionRequester->requestParam(-1, 0, preset->getColorCorrectionData());
        }
    }

    mLastPreset = preset;
    mColorCorrectionRequester->updateRequest();
    mViewDepthDrawer->update();
    mVignettingDrawer->update();
    mEdgeDrawer->update();
    mCartoonDrawer->update();
    mRetroColorDrawer->update();
    mScreenBlurDrawer->update();
    mPencilSketchDrawer->update();
    mColorClampDrawer->update();
    mMetalReliefDrawer->update();
    mMosaicPictureDrawer->update();
    mContoursDrawer->update();
    mKaleidoscopeDrawer->update();
    mMangaDrawer->update();
}

/**
 * Finds a preset with a name by index.
 * @param index Preset index.
 * @return The preset, or nullptr if it has no name.
 */
PostProcessingFilterPreset* PostProcessingFilter::findPreset(s32 index) const {
    if (mPresets.size() == 0) {
        return nullptr;
    }

    if (isEqualString(mPresets.at(index)->getName(), "")) {
        return nullptr;
    }

    return mPresets.at(index);
}

/**
 * Updates the view dependent GPU data of the screen blur.
 * @param index View index.
 * @param pCamera Camera.
 * @param pProjection Projection.
 */
void PostProcessingFilter::updateViewGpu(s32 index, const sead::Camera* pCamera,
                                         const Projection* pProjection) {
    if (!mIsValid) {
        return;
    }

    const sead::Matrix34f& viewMtx = pCamera->getMatrix();
    const sead::Matrix44f& projMtx = pProjection->getProjMtx();
    mScreenBlurDrawer->updateViewGpu(0, viewMtx, projMtx);
}

/**
 * Updates the view dependent GPU data of the screen blur.
 * @param index View index.
 * @param pCamera Camera.
 * @param pProjection Perspective projection.
 */
void PostProcessingFilter::updateViewGpu(s32 index, const sead::Camera* pCamera,
                                         const sead::PerspectiveProjection* pProjection) {
    if (!mIsValid) {
        return;
    }

    const sead::Matrix34f& viewMtx = pCamera->getMatrix();
    const sead::Matrix44f& projMtx = pProjection->getProjectionMatrix();
    mScreenBlurDrawer->updateViewGpu(0, viewMtx, projMtx);
}

/**
 * Draws all enabled filters.
 * @param pDrawContext Draw context.
 * @param index View index.
 * @param pModelEnv Model environment.
 * @param rRenderBuffer Render buffer to draw to.
 * @param rDepth Depth buffer.
 * @param rLinearDepth Linear depth texture.
 * @param pNormal Normal texture.
 * @param rBaseColor Base color texture.
 * @param rColor Color texture.
 * @param rCamera Camera.
 * @param rProjection Projection.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param isLinearDepthReady Whether rLinearDepth is already filled.
 */
void PostProcessingFilter::drawFilter(agl::DrawContext* pDrawContext, s32 index,
                                      SimpleModelEnv* pModelEnv,
                                      const agl::RenderBuffer& rRenderBuffer,
                                      const agl::TextureData& rDepth,
                                      const agl::TextureData& rLinearDepth,
                                      const agl::TextureData* pNormal,
                                      const agl::TextureData& rBaseColor,
                                      const agl::TextureData& rColor, const sead::Camera& rCamera,
                                      const Projection& rProjection, f32 near, f32 far,
                                      bool isLinearDepthReady) const {
    if (!mIsValid) {
        return;
    }

    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();
    const agl::TextureData* linearDepth = nullptr;

    if (mEdgeDrawer->isEnable() || mCartoonDrawer->isEnable() ||
        mPencilSketchDrawer->isEnable() || mMetalReliefDrawer->isEnable() ||
        mContoursDrawer->isEnable() || mMangaDrawer->isEnable()) {
        linearDepth = &rLinearDepth;

        if (!isLinearDepthReady) {
            linearDepth = allocator->alloc(
                pDrawContext, "temp_linear_depth", agl::TextureFormat(9), rDepth.getWidth(0),
                rDepth.getHeight(0), 1, nullptr, agl::utl::DynamicTextureAllocator::AllocateType(0),
                true, false);
            alViewRendererFunction::createLinearDepthFromDepthBuffer(
                pDrawContext, ShaderHolder::sInstance, linearDepth, &rDepth, mFullScreenTriangle);
        }
    }

    mVignettingDrawer->draw(pDrawContext, rRenderBuffer);
    mScreenBlurDrawer->draw(pDrawContext, index, rRenderBuffer);
    mEdgeDrawer->draw(pDrawContext, &rRenderBuffer, &rRenderBuffer, &rColor, linearDepth, pNormal,
                      index, rCamera, near, far, isLinearDepthReady);

    if (linearDepth != nullptr) {
        pushDebugGroup(getCommandBuffer(), "MEtal Releif Drawer");
        mMetalReliefDrawer->draw(pDrawContext, rRenderBuffer, pModelEnv, rColor, *linearDepth);
        nvnCommandBufferPopDebugGroup(getCommandBuffer());
    }

    mMosaicPictureDrawer->draw(pDrawContext, rRenderBuffer, pModelEnv);
    mKaleidoscopeDrawer->draw(pDrawContext, rRenderBuffer);
    mVignettingDrawer->draw(pDrawContext, rRenderBuffer);
    mScreenBlurDrawer->draw(pDrawContext, index, rRenderBuffer);

    pushDebugGroup(getCommandBuffer(), "Cartoon Drawer");
    mCartoonDrawer->draw(pDrawContext, pModelEnv, rRenderBuffer, linearDepth,
                         rProjection.getAspect());
    nvnCommandBufferPopDebugGroup(getCommandBuffer());

    mViewDepthDrawer->draw(pDrawContext, rRenderBuffer, near, far, rDepth);
    mPencilSketchDrawer->draw(pDrawContext, pModelEnv, rRenderBuffer, linearDepth);

    agl::pfx::ColorCorrection* colorCorrection = mColorCorrectionRequester->getParam();
    colorCorrection->drawMap(pDrawContext);

    if (colorCorrection->isEnable() && colorCorrection->getVariationIndex() != 0) {
        const agl::RenderTargetColor* target = rRenderBuffer.getRenderTargetColor();
        agl::utl::DynamicTextureAllocator* copyAllocator =
            agl::utl::DynamicTextureAllocator::instance();
        agl::TextureData* copy = copyAllocator->alloc(
            pDrawContext, "post_process_filter_texture",
            agl::TextureFormat(target->getTextureFormat()), target->getWidth(0),
            target->getHeight(0), 1, nullptr, agl::utl::DynamicTextureAllocator::AllocateType(0),
            true, false);
        target->copyToAll(pDrawContext, copy);
        colorCorrection->draw(pDrawContext, index, rRenderBuffer, *copy);
        copyAllocator->free(copy);
    }

    mMangaDrawer->draw(pDrawContext, rRenderBuffer, linearDepth, pNormal, &rDepth, &rColor, near,
                       far, rProjection.getFovy());
    mContoursDrawer->draw(pDrawContext, rRenderBuffer, linearDepth, &rDepth);
    mRetroColorDrawer->draw(pDrawContext, rRenderBuffer, rBaseColor);
    mColorClampDrawer->draw(pDrawContext, rRenderBuffer);

    if (linearDepth != nullptr && !isLinearDepthReady) {
        allocator->free(linearDepth);
    }
}

/**
 * Draws all enabled filters with a perspective projection.
 * @param pDrawContext Draw context.
 * @param index View index.
 * @param pModelEnv Model environment.
 * @param rRenderBuffer Render buffer to draw to.
 * @param rDepth Depth buffer.
 * @param rLinearDepth Linear depth texture.
 * @param pNormal Normal texture.
 * @param rBaseColor Base color texture.
 * @param rColor Color texture.
 * @param rCamera Camera.
 * @param rProjection Perspective projection.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param isLinearDepthReady Whether rLinearDepth is already filled.
 */
void PostProcessingFilter::drawFilter(
    agl::DrawContext* pDrawContext, s32 index, SimpleModelEnv* pModelEnv,
    const agl::RenderBuffer& rRenderBuffer, const agl::TextureData& rDepth,
    const agl::TextureData& rLinearDepth, const agl::TextureData* pNormal,
    const agl::TextureData& rBaseColor, const agl::TextureData& rColor,
    const sead::Camera& rCamera, const sead::PerspectiveProjection& rProjection, f32 near,
    f32 far, bool isLinearDepthReady) const {
    Projection projection(rProjection.getNear(), rProjection.getFar(), rProjection.getFovy(),
                          rProjection.getAspect());
    drawFilter(pDrawContext, index, pModelEnv, rRenderBuffer, rDepth, rLinearDepth, pNormal,
               rBaseColor, rColor, rCamera, projection, near, far, isLinearDepthReady);
}
}  // namespace al
