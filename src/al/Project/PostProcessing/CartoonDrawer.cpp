#include "Project/PostProcessing/CartoonDrawer.hpp"

#include <gfx/seadGraphicsContext.h>
#include <math/seadVector.h>
#include <prim/seadEnum.h>

#include "common/aglDrawContext.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglShaderLocation.h"
#include "common/aglShaderProgram.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
#include "g3d/aglNW4FToNN.h"
#include "g3d/aglTextureDataInitializerG3D.h"
#include "utility/aglDynamicTextureAllocator.h"

#include "Library/Debug/Render/RenderBufferAttacher.hpp"
#include "Library/Model/SimpleModelEnv.hpp"
#include "Library/Play/Graphics/NoiseTextureKeeper.hpp"
#include "Library/Shader/DeferredRendering/FullScreenTriangle.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/PostProcessing/PostProcessingDrawerUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {

// clang-format off
SEAD_ENUM(IndirectTexture, None , PaintWater , PaintRough , ConcentrationLine)
// clang-format on

}  // namespace

namespace al {

/**
 * Constructs the cartoon drawing parameters.
 */
CartoonDrawParam::CartoonDrawParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "有効", "", true);
    mIsEnableFishEye = new ParameterBool(false, mParamObj, "IsEnableFishEye", "魚眼有効", "", true);
    mToonShadeRate = new ParameterF32(0.0f, mParamObj, "ToonShadeRate", "トゥーン率",
                                      "Min=0.0f, Max=1.0f", true);
    mToonStep = new ParameterV3f(sead::Vector3f(0.1f, 0.4f, 0.7f), mParamObj, "ToonStep",
                                 "トゥーンステップ", "Min=0.0f, Max=1.0f", true);
    mToonWidth = new ParameterV3f(sead::Vector3f(0.3f, 0.3f, 0.3f), mParamObj, "ToonWidth",
                                  "トゥーン幅", "Min=0.0f, Max=1.0f", true);
    mNoiseTextureId = new ParameterS32(-1, mParamObj, "NoiseTetxureId", "ノイズテクスチャ",
                                       "Min=0, Max=32", true);
    mNoiseMixRate = new ParameterF32(0.0f, mParamObj, "NoiseMixRate", "ノイズ混ぜ合わせ率",
                                     "Min=0.f, Max=1.f", true);
    mNoiseScale = new ParameterF32(0.0005f, mParamObj, "NoiseScale", "ノイズテクスチャスケール",
                                   "Min=0.f, Max=1.f", true);
    mNoiseOffset = new ParameterV3f(sead::Vector3f(0.0f, 0.0f, 0.0f), mParamObj, "NoiseOffset",
                                    "ノイズテクスチャオフセット", "Min=0.f, Max=1.f", true);
    mCanvasTextureId = new ParameterS32(-1, mParamObj, "CanvasTetxureId", "キャンバステクスチャ",
                                        "Min=0, Max=32", true);
    mCanvasRepeat = new ParameterF32(200.0f, mParamObj, "CanvasRepeat", "キャンバス地のリピート",
                                     "Min=0.f, Max=300.f", true);
    mCanvasMix = new ParameterF32(0.3f, mParamObj, "CanvasMix", "キャンバス地の割合",
                                  "Min=0.f, Max=1.f", true);
    mIndirectTextureId =
        new ParameterS32(-1, mParamObj, "IndirectTexId", "インダイレクトテクスチャ", "", true);
    mIndirectScale = new ParameterF32(0.01f, mParamObj, "IndirectScale",
                                      "インダイレクトのスケール", "Min=0.f, Max=1.f", true);
    mIndirectTexScale =
        new ParameterV2f(sead::Vector2f(1.0f, 1.0f), mParamObj, "IndirectTexScale",
                         "インダイレクトテクスチャスケール", "Min=0.f, Max=1.f", true);
    mIndirectTexOffset =
        new ParameterV2f(sead::Vector2f(0.0f, 0.0f), mParamObj, "IndirectTexOffset",
                         "インダイレクトテクスチャオフセット", "Min=0.f, Max=1.f", true);
    mFishEyeParam = new ParameterF32(1.0f, mParamObj, "FishEyeParam", "魚眼パラメータ",
                                     "Min=0.f, Max=1.f", true);
}

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool CartoonDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Checks the EnableFishEye flag.
 * @return Whether EnableFishEye is set.
 */
bool CartoonDrawParam::isEnableFishEye() const {
    return mIsEnableFishEye->getValue();
}

/**
 * Gets the ToonShadeRate parameter.
 * @return ToonShadeRate.
 */
f32 CartoonDrawParam::getToonShadeRate() const {
    return mToonShadeRate->getValue();
}

/**
 * Gets the ToonStep parameter.
 * @return ToonStep.
 */
const sead::Vector3f& CartoonDrawParam::getToonStep() const {
    return mToonStep->getValue();
}

/**
 * Gets the ToonWidth parameter.
 * @return ToonWidth.
 */
const sead::Vector3f& CartoonDrawParam::getToonWidth() const {
    return mToonWidth->getValue();
}

/**
 * Gets the NoiseTextureId parameter.
 * @return NoiseTextureId.
 */
s32 CartoonDrawParam::getNoiseTextureId() const {
    return mNoiseTextureId->getValue();
}

/**
 * Gets the NoiseMixRate parameter.
 * @return NoiseMixRate.
 */
f32 CartoonDrawParam::getNoiseMixRate() const {
    return mNoiseMixRate->getValue();
}

/**
 * Gets the NoiseScale parameter.
 * @return NoiseScale.
 */
f32 CartoonDrawParam::getNoiseScale() const {
    return mNoiseScale->getValue();
}

/**
 * Gets the NoiseOffset parameter.
 * @return NoiseOffset.
 */
const sead::Vector3f& CartoonDrawParam::getNoiseOffset() const {
    return mNoiseOffset->getValue();
}

/**
 * Gets the CanvasTextureId parameter.
 * @return CanvasTextureId.
 */
s32 CartoonDrawParam::getCanvasTextureId() const {
    return mCanvasTextureId->getValue();
}

/**
 * Gets the CanvasRepeat parameter.
 * @return CanvasRepeat.
 */
f32 CartoonDrawParam::getCanvasRepeat() const {
    return mCanvasRepeat->getValue();
}

/**
 * Gets the CanvasMix parameter.
 * @return CanvasMix.
 */
f32 CartoonDrawParam::getCanvasMix() const {
    return mCanvasMix->getValue();
}

/**
 * Gets the IndirectTextureId parameter.
 * @return IndirectTextureId.
 */
s32 CartoonDrawParam::getIndirectTextureId() const {
    return mIndirectTextureId->getValue();
}

/**
 * Gets the IndirectScale parameter.
 * @return IndirectScale.
 */
f32 CartoonDrawParam::getIndirectScale() const {
    return mIndirectScale->getValue();
}

/**
 * Gets the IndirectTexScale parameter.
 * @return IndirectTexScale.
 */
const sead::Vector2f& CartoonDrawParam::getIndirectTexScale() const {
    return mIndirectTexScale->getValue();
}

/**
 * Gets the IndirectTexOffset parameter.
 * @return IndirectTexOffset.
 */
const sead::Vector2f& CartoonDrawParam::getIndirectTexOffset() const {
    return mIndirectTexOffset->getValue();
}

/**
 * Gets the FishEyeParam parameter.
 * @return FishEyeParam.
 */
f32 CartoonDrawParam::getFishEyeParam() const {
    return mFishEyeParam->getValue();
}

/**
 * Gets the cartoon shader and creates the parameter interpolation.
 * @param pShaderHolder Shader holder to get the shader from.
 * @param pNoiseTextureKeeper Keeper of the noise and canvas textures.
 */
CartoonDrawer::CartoonDrawer(ShaderHolder* pShaderHolder, NoiseTextureKeeper* pNoiseTextureKeeper)
    : mShaderProgram(nullptr), mRequestInterp(nullptr), mNoiseTextureKeeper(pNoiseTextureKeeper),
      mIndirectTextures(nullptr) {
    mShaderProgram = pShaderHolder->getShaderProgram("alRenderCartoon");
    ParamRequestInterp* interp = new ParamRequestInterp();
    mRequestInterp = interp;
    interp->mCurrentParam = new CartoonDrawParam();
    interp->mStartParam = new CartoonDrawParam();
    interp->mEndParam = new CartoonDrawParam();
    interp->mRequestParam = new CartoonDrawParam();
}

/**
 * Destroys the indirect textures.
 */
CartoonDrawer::~CartoonDrawer() {
    if (mIndirectTextures != nullptr) {
        for (s32 i = 0; i < IndirectTexture::size() - 1; i++) {
            delete mIndirectTextures[i];
        }
    }
}

/**
 * Creates the indirect textures from the project resource.
 * @param pResFile Project resource file.
 */
void CartoonDrawer::initProjectResource(nn::g3d::ResFile* pResFile) {
    mIndirectTextures = new agl::TextureData*[IndirectTexture::size() - 1];

    for (s32 i = 0; i < IndirectTexture::size() - 1; i++) {
        mIndirectTextures[i] = nullptr;
        StringTmp<32> name("Texture%s", IndirectTexture::text(i + 1));
        nn::gfx::ResTexture* texture = agl::g3d::ResFile::GetTexture(pResFile, name.cstr());

        if (texture != nullptr) {
            mIndirectTextures[i] = new agl::TextureData();
            agl::g3d::TextureDataInitializerG3D::initialize(mIndirectTextures[i], *texture);
            mIndirectTextures[i]->flushCPUCache();
        }
    }
}

/**
 * Finishes initialization of the parameter interpolation.
 */
void CartoonDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void CartoonDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void CartoonDrawer::update() {
    mRequestInterp->updateInterp();
}

/**
 * Draws the cartoon effect onto the color target of a render buffer.
 * @param pContext Draw context.
 * @param pEnv Model environment.
 * @param rBuffer Render buffer to draw onto.
 * @param pLinearDepth Linear depth texture.
 * @param fishEyeRate Fish eye rate.
 */
void CartoonDrawer::draw(agl::DrawContext* pContext, SimpleModelEnv* pEnv,
                         const agl::RenderBuffer& rBuffer, const agl::TextureData* pLinearDepth,
                         f32 fishEyeRate) const {
    const CartoonDrawParam* param = getCurrentParam();

    if (!param->isEnable()) {
        return;
    }

    const agl::TextureData* color = rBuffer.getRenderTargetColor();
    agl::TextureData* temp = agl::utl::DynamicTextureAllocator::instance()->alloc(
        pContext, "temp_cartoon_draw", agl::TextureFormat(color->getTextureFormat()),
        color->getWidth(0), color->getHeight(0), 1, nullptr,
        agl::utl::DynamicTextureAllocator::AllocateType(0), true, false);

    const char* macros[] = {"IS_USE_CANVAS_TEX", "IS_USE_NOISE_TEX", "IS_USE_INDIRECT_TEX",
                            "IS_USE_FISH_EYE"};
    const char* values[] = {"IS_USE_CANVAS_TEX", "IS_USE_NOISE_TEX", "IS_USE_INDIRECT_TEX",
                            "IS_USE_FISH_EYE"};
    s32 index = ShaderSearchImpl::searchMacroIndex(macros, "IS_USE_CANVAS_TEX");

    if (index != -1) {
        values[index] = param->getCanvasMix() > 0.0f ? "1" : "0";
    }

    index = ShaderSearchImpl::searchMacroIndex(macros, "IS_USE_NOISE_TEX");

    if (index != -1) {
        values[index] = param->getNoiseTextureId() != -1 ? "1" : "0";
    }

    index = ShaderSearchImpl::searchMacroIndex(macros, "IS_USE_INDIRECT_TEX");

    if (index != -1) {
        values[index] = param->getIndirectTextureId() != -1 ? "1" : "0";
    }

    index = ShaderSearchImpl::searchMacroIndex(macros, "IS_USE_FISH_EYE");

    if (index != -1) {
        values[index] = param->isEnableFishEye() ? "1" : "0";
    }

    const agl::ShaderProgram* program = mShaderProgram->searchVariation(4, macros, values);
    program->activate(pContext, true);
    pEnv->prepareModelDraw(0);

    const agl::ShaderProgram& rBaseProgram = *mShaderProgram;
    agl::SamplerLocation frameBufferLocation("uFrameBuffer");
    frameBufferLocation.search(rBaseProgram);
    agl::TextureSampler frameBufferSampler;
    frameBufferSampler.applyTextureData(*color);
    frameBufferSampler.activate(pContext, frameBufferLocation, -1, false);

    if (param->getNoiseTextureId() != -1) {
        agl::SamplerLocation location("uNoiseTexture");
        location.search(*program);
        mNoiseTextureKeeper->getTexture3DSampler(param->getNoiseTextureId())
            ->activate(pContext, location, -1, false);
    }

    if (param->getCanvasMix() > 0.0f) {
        agl::SamplerLocation location("uCanvasTexture");
        location.search(*program);
        mNoiseTextureKeeper->getTexture3DSampler(param->getCanvasTextureId())
            ->activate(pContext, location, -1, false);
    }

    agl::SamplerLocation linearDepthLocation("uLinearDepthTex");
    linearDepthLocation.search(*program);
    agl::TextureSampler linearDepthSampler;
    linearDepthSampler.applyTextureData(*pLinearDepth);
    linearDepthSampler.activate(pContext, linearDepthLocation, -1, false);

    if (param->getIndirectTextureId() != -1) {
        agl::SamplerLocation location("uIndirectTexture");
        location.search(*program);
        agl::TextureSampler sampler;
        sampler.setWrap(1, 1, 1);
        s32 index = sead::Mathi::clamp(param->getIndirectTextureId(), 0, 2);
        sampler.applyTextureData(*mIndirectTextures[index]);
        sampler.activate(pContext, location, -1, false);

        const sead::Vector2f& texScale = param->getIndirectTexScale();
        const sead::Vector2f& texOffset = param->getIndirectTexOffset();
        sead::Vector4f indirectParam1(texOffset.x, texOffset.y, texScale.x, texScale.y);
        setPostProcessingUniform(pContext, program, "uIndirectTexParam1", indirectParam1);
        sead::Vector4f indirectParam2(param->getIndirectScale(), 0.0f, 0.0f, 0.0f);
        setPostProcessingUniform(pContext, program, "uIndirectTexParam2", indirectParam2);
    }

    if (param->isEnableFishEye()) {
        sead::Vector4f fishEyeParam(param->getFishEyeParam(), fishEyeRate, 0.0f, 0.0f);
        setPostProcessingUniform(pContext, program, "uFishEyeParam", fishEyeParam);
    }

    const sead::Vector3f& toonStep = param->getToonStep();
    sead::Vector4f params(param->getToonShadeRate(), toonStep.x, toonStep.y, toonStep.z);
    setPostProcessingUniform(pContext, program, "uParams", params);

    const sead::Vector3f& toonWidth = param->getToonWidth();
    sead::Vector4f params2(toonWidth.x, toonWidth.y, toonWidth.z, param->getNoiseMixRate());
    setPostProcessingUniform(pContext, program, "uParams2", params2);

    const sead::Vector3f& noiseOffset = param->getNoiseOffset();
    sead::Vector4f noiseParam(noiseOffset.x, noiseOffset.y, noiseOffset.z, param->getNoiseScale());
    setPostProcessingUniform(pContext, program, "uNoiseParam", noiseParam);

    sead::Vector4f canvasParam(param->getCanvasMix(), param->getCanvasRepeat(), 0.0f, 0.0f);
    setPostProcessingUniform(pContext, program, "uCanvasParam", canvasParam);

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(pContext);

    {
        agl::RenderBuffer renderBuffer;
        RenderBufferAttacher attacher(&renderBuffer, temp, nullptr, nullptr, nullptr, nullptr);
        drawPostProcessingQuad(pContext);
    }

    temp->copyToAll(pContext, color);
    agl::utl::DynamicTextureAllocator::instance()->free(temp);
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const CartoonDrawParam* CartoonDrawer::getCurrentParam() const {
    return static_cast<const CartoonDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void CartoonDrawer::requestParam(s32 priority, s32 step, const CartoonDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool CartoonDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
}

}  // namespace al
