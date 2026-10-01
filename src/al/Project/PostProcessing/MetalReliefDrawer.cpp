#include "Project/PostProcessing/MetalReliefDrawer.hpp"

#include <cmath>
#include <gfx/seadGraphicsContext.h>

#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureSampler.h"
#include "g3d/aglNW4FToNN.h"
#include "g3d/aglTextureDataInitializerG3D.h"

#include "Library/Debug/Render/RenderBufferAttacher.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Model/SimpleModelEnv.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/PostProcessing/PostProcessingDrawerUtil.hpp"

namespace al {

/**
 * Constructs the metal relief drawing parameters.
 */
MetalReliefDrawParam::MetalReliefDrawParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "有効", "", true);
    mLightAngle = new ParameterV2f({0.0f, 0.0f}, mParamObj, "LightAngle", "ライト方向", "Min=-180.f, Max=180.f", true);
    mLightAngle2 = new ParameterV2f({0.0f, 0.0f}, mParamObj, "LightAngle2", "ライト方向2", "Min=-180.f, Max=180.f", true);
    mLightColor = new ParameterC4f(sead::Color4f::cWhite, mParamObj, "LightColor", "ライトカラー", "Min=0.f, Max=1024.f", true);
    mLightColor2 = new ParameterC4f(sead::Color4f::cWhite, mParamObj, "LightColor2", "ライトカラー2", "Min=0.f, Max=1024.f", true);
    mLightSpcColor = new ParameterC4f(sead::Color4f::cWhite, mParamObj, "LightSpcColor", "ライトスペキュラカラー", "Min=0.f, Max=1024.f", true);
    mLightSpcColor2 = new ParameterC4f(sead::Color4f::cWhite, mParamObj, "LightSpcColor2", "ライトスペキュラカラー2", "Min=0.f, Max=1024.f", true);
    mMetalColor = new ParameterC4f(sead::Color4f::cWhite, mParamObj, "MetalColor", "メタルカラー", "Min=0.f, Max=1.f", true);
    mMetalness = new ParameterF32(1.0f, mParamObj, "Metalness", "メタルネス", "Min=0.f, Max=1.f", true);
    mRoughness = new ParameterF32(0.5f, mParamObj, "Roughness", "ラフネス", "Min=0.f, Max=1.f", true);
    mCoinRange = new ParameterF32(0.5f, mParamObj, "CoinRange", "コイン半径", "Min=0.f, Max=1.f", true);
    mAlbedoRange = new ParameterF32(0.5f, mParamObj, "AlbedoRange", "背景半径", "Min=0.f, Max=1.f", true);
    mCoinYOffset = new ParameterF32(0.05f, mParamObj, "CoinYOffset", "コインYオフセット", "Min=0.f, Max=1.f", true);
    mExposure = new ParameterF32(1.0f, mParamObj, "Exposure", "露出", "Min=0.f, Max=1.f", true);
    mBlackPoint = new ParameterF32(0.5f, mParamObj, "BlackPoint", "BlackPoint", "Min=0.0f, Max=1.0f", true);
    mCrossOver = new ParameterF32(2.0f, mParamObj, "CrossOver", "CrossOver", "Min=0.0f, Max=10.0f", true);
    mWhitePoint = new ParameterF32(4.0f, mParamObj, "WhitePoint", "WhitePoint", "Min=0.0f, Max=20.0f", true);
    mToe = new ParameterF32(0.0f, mParamObj, "Toe", "Toe", "Min=0.0f, Max=1.0f", true);
    mSholuder = new ParameterF32(0.0f, mParamObj, "Sholuder", "Sholuder", "Min=0.0f, Max=1.0f", true);
    mTargetDepth = new ParameterF32(1000.0f, mParamObj, "TargetDepth", "注視位置", "Min=0.0f, Max=10000.0f", true);
    mCoinRoughness = new ParameterF32(0.2f, mParamObj, "CoinRoughness", "コイン外周のラフネス", "Min=0.0f, Max=1.0f", true);
    mCoinNormalBase = new ParameterF32(0.7f, mParamObj, "CoinNormalBase", "コインのノーマルブレンド率", "Min=0.0f, Max=1.0f", true);
    mRoughnessScale = new ParameterF32(0.0003f, mParamObj, "RoughnessScale", "ラフネススケール", "Min=0.0f, Max=1.0f", true);
    mNormalScale = new ParameterF32(0.0001f, mParamObj, "NormalScale", "ノーマルスケール", "Min=0.0f, Max=1.0f", true);
    mCoinNormalCurve = new ParameterF32(4.0f, mParamObj, "CoinNormalCurve", "コイン法線曲率", "Min=0.0f, Max=5.0f", true);
    mCoinNormalScale = new ParameterF32(0.05f, mParamObj, "CoinNormalScale", "コイン法線スケール", "Min=0.0f, Max=1.0f", true);
}

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool MetalReliefDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Gets the LightAngle parameter.
 * @return LightAngle.
 */
const sead::Vector2f& MetalReliefDrawParam::getLightAngle() const {
    return mLightAngle->getValue();
}

/**
 * Gets the LightAngle2 parameter.
 * @return LightAngle2.
 */
const sead::Vector2f& MetalReliefDrawParam::getLightAngle2() const {
    return mLightAngle2->getValue();
}

/**
 * Gets the LightColor parameter.
 * @return LightColor.
 */
const sead::Color4f& MetalReliefDrawParam::getLightColor() const {
    return mLightColor->getValue();
}

/**
 * Gets the LightColor2 parameter.
 * @return LightColor2.
 */
const sead::Color4f& MetalReliefDrawParam::getLightColor2() const {
    return mLightColor2->getValue();
}

/**
 * Gets the LightSpcColor parameter.
 * @return LightSpcColor.
 */
const sead::Color4f& MetalReliefDrawParam::getLightSpcColor() const {
    return mLightSpcColor->getValue();
}

/**
 * Gets the LightSpcColor2 parameter.
 * @return LightSpcColor2.
 */
const sead::Color4f& MetalReliefDrawParam::getLightSpcColor2() const {
    return mLightSpcColor2->getValue();
}

/**
 * Gets the MetalColor parameter.
 * @return MetalColor.
 */
const sead::Color4f& MetalReliefDrawParam::getMetalColor() const {
    return mMetalColor->getValue();
}

/**
 * Gets the Metalness parameter.
 * @return Metalness.
 */
f32 MetalReliefDrawParam::getMetalness() const {
    return mMetalness->getValue();
}

/**
 * Gets the Roughness parameter.
 * @return Roughness.
 */
f32 MetalReliefDrawParam::getRoughness() const {
    return mRoughness->getValue();
}

/**
 * Gets the CoinRange parameter.
 * @return CoinRange.
 */
f32 MetalReliefDrawParam::getCoinRange() const {
    return mCoinRange->getValue();
}

/**
 * Gets the AlbedoRange parameter.
 * @return AlbedoRange.
 */
f32 MetalReliefDrawParam::getAlbedoRange() const {
    return mAlbedoRange->getValue();
}

/**
 * Gets the CoinYOffset parameter.
 * @return CoinYOffset.
 */
f32 MetalReliefDrawParam::getCoinYOffset() const {
    return mCoinYOffset->getValue();
}

/**
 * Gets the Exposure parameter.
 * @return Exposure.
 */
f32 MetalReliefDrawParam::getExposure() const {
    return mExposure->getValue();
}

/**
 * Gets the BlackPoint parameter.
 * @return BlackPoint.
 */
f32 MetalReliefDrawParam::getBlackPoint() const {
    return mBlackPoint->getValue();
}

/**
 * Gets the CrossOver parameter.
 * @return CrossOver.
 */
f32 MetalReliefDrawParam::getCrossOver() const {
    return mCrossOver->getValue();
}

/**
 * Gets the WhitePoint parameter.
 * @return WhitePoint.
 */
f32 MetalReliefDrawParam::getWhitePoint() const {
    return mWhitePoint->getValue();
}

/**
 * Gets the Toe parameter.
 * @return Toe.
 */
f32 MetalReliefDrawParam::getToe() const {
    return mToe->getValue();
}

/**
 * Gets the Sholuder parameter.
 * @return Sholuder.
 */
f32 MetalReliefDrawParam::getSholuder() const {
    return mSholuder->getValue();
}

/**
 * Gets the TargetDepth parameter.
 * @return TargetDepth.
 */
f32 MetalReliefDrawParam::getTargetDepth() const {
    return mTargetDepth->getValue();
}

/**
 * Gets the CoinRoughness parameter.
 * @return CoinRoughness.
 */
f32 MetalReliefDrawParam::getCoinRoughness() const {
    return mCoinRoughness->getValue();
}

/**
 * Gets the CoinNormalBase parameter.
 * @return CoinNormalBase.
 */
f32 MetalReliefDrawParam::getCoinNormalBase() const {
    return mCoinNormalBase->getValue();
}

/**
 * Gets the RoughnessScale parameter.
 * @return RoughnessScale.
 */
f32 MetalReliefDrawParam::getRoughnessScale() const {
    return mRoughnessScale->getValue();
}

/**
 * Gets the NormalScale parameter.
 * @return NormalScale.
 */
f32 MetalReliefDrawParam::getNormalScale() const {
    return mNormalScale->getValue();
}

/**
 * Gets the CoinNormalCurve parameter.
 * @return CoinNormalCurve.
 */
f32 MetalReliefDrawParam::getCoinNormalCurve() const {
    return mCoinNormalCurve->getValue();
}

/**
 * Gets the CoinNormalScale parameter.
 * @return CoinNormalScale.
 */
f32 MetalReliefDrawParam::getCoinNormalScale() const {
    return mCoinNormalScale->getValue();
}

/**
 * Gets the metal relief shader and creates the parameter interpolation.
 * @param pShaderHolder Shader holder to get the shader from.
 * @param pLightEnvBlock Light environment uniform block.
 */
MetalReliefDrawer::MetalReliefDrawer(ShaderHolder* pShaderHolder, UniformBlock* pLightEnvBlock)
    : mShaderProgram(nullptr), mRequestInterp(nullptr), mLightEnvBlock(pLightEnvBlock),
      mCoinTexture(nullptr) {
    mShaderProgram = pShaderHolder->getShaderProgram("alRenderMetalRelief");
    ParamRequestInterp* interp = new ParamRequestInterp();
    mRequestInterp = interp;
    interp->mCurrentParam = new MetalReliefDrawParam();
    interp->mStartParam = new MetalReliefDrawParam();
    interp->mEndParam = new MetalReliefDrawParam();
    interp->mRequestParam = new MetalReliefDrawParam();
}

/**
 * Destroys the coin texture.
 */
MetalReliefDrawer::~MetalReliefDrawer() {
    if (mCoinTexture != nullptr) {
        delete mCoinTexture;
        mCoinTexture = nullptr;
    }
}

/**
 * Creates the coin texture from the project resource.
 * @param pResFile Project resource file.
 */
void MetalReliefDrawer::initProjectResource(nn::g3d::ResFile* pResFile) {
    nn::gfx::ResTexture* texture = agl::g3d::ResFile::GetTexture(pResFile, "TextureCoinAlb");

    if (texture != nullptr) {
        mCoinTexture = new agl::TextureData();
        agl::g3d::TextureDataInitializerG3D::initialize(mCoinTexture, *texture);
        mCoinTexture->flushCPUCache();
    }
}

/**
 * Finishes initialization of the parameter interpolation.
 */
void MetalReliefDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void MetalReliefDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void MetalReliefDrawer::update() {
    mRequestInterp->updateInterp();
}

void MetalReliefDrawer::draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer,
                             SimpleModelEnv* pEnv, const agl::TextureData& rNormal,
                             const agl::TextureData& rDepth) const {
    const MetalReliefDrawParam* param = getCurrentParam();

    if (!param->isEnable()) {
        return;
    }

    const agl::TextureData* color = rBuffer.getRenderTargetColor();
    mShaderProgram->activate(pContext, true);
    pEnv->prepareModelDraw(0);

    agl::UniformBlockLocation lightEnvLocation =
        searchShaderLocation<agl::UniformBlockLocation>(*mShaderProgram, "LightEnv");
    mLightEnvBlock->activate(pContext, lightEnvLocation);

    agl::SamplerLocation normalLocation =
        searchShaderLocation<agl::SamplerLocation>(*mShaderProgram, "cWorldNormal");
    agl::TextureSampler normalSampler;
    normalSampler.setFilter(0, 0, 0);
    normalSampler.applyTextureData(rNormal);
    normalSampler.activate(pContext, normalLocation, -1, false);

    agl::SamplerLocation depthLocation =
        searchShaderLocation<agl::SamplerLocation>(*mShaderProgram, "cViewDepth");
    agl::TextureSampler depthSampler;
    depthSampler.applyTextureData(rDepth);
    depthSampler.activate(pContext, depthLocation, -1, false);
    depthSampler.setFilter(0, 0, 0);

    agl::SamplerLocation coinLocation =
        searchShaderLocation<agl::SamplerLocation>(*mShaderProgram, "cCoinTextureAlb");
    agl::TextureSampler coinSampler;
    coinSampler.applyTextureData(*mCoinTexture);
    coinSampler.activate(pContext, coinLocation, -1, false);

    const sead::Color4f& metalColor = param->getMetalColor();
    sead::Vector4f params1(metalColor.r, metalColor.g, metalColor.b, param->getMetalness());
    setPostProcessingUniform(pContext, mShaderProgram, "uMetalParams1", params1);

    const sead::Color4f& lightColor = param->getLightColor();
    sead::Vector4f params2(lightColor.r, lightColor.g, lightColor.b, param->getRoughness());
    setPostProcessingUniform(pContext, mShaderProgram, "uMetalParams2", params2);

    sead::Vector3f lightDir(1.0f, 1.0f, 1.0f);
    const sead::Vector2f& lightAngle = param->getLightAngle();
    calcDirFromLongitudeLatitude(&lightDir, lightAngle.x, lightAngle.y);
    sead::Vector4f params3(lightDir.x, lightDir.y, lightDir.z, param->getExposure());
    setPostProcessingUniform(pContext, mShaderProgram, "uMetalParams3", params3);

    sead::Vector3f lightDir2(1.0f, 1.0f, 1.0f);
    const sead::Vector2f& lightAngle2 = param->getLightAngle2();
    calcDirFromLongitudeLatitude(&lightDir2, lightAngle2.x, lightAngle2.y);
    sead::Vector4f params4(lightDir2.x, lightDir2.y, lightDir2.z, param->getTargetDepth());
    setPostProcessingUniform(pContext, mShaderProgram, "uMetalParams4", params4);

    sead::Vector4f params5(param->getCoinRoughness(), param->getRoughnessScale(),
                           param->getNormalScale(), param->getCoinNormalBase());
    setPostProcessingUniform(pContext, mShaderProgram, "uMetalParams5", params5);

    const sead::Color4f& lightColor2 = param->getLightColor2();
    sead::Vector4f params6(lightColor2.r, lightColor2.g, lightColor2.b, param->getCoinYOffset());
    setPostProcessingUniform(pContext, mShaderProgram, "uMetalParams6", params6);

    setPostProcessingUniform(pContext, mShaderProgram, "uCoinRange", param->getCoinRange());
    setPostProcessingUniform(pContext, mShaderProgram, "uAlbedoRange", param->getAlbedoRange());

    const sead::Color4f& spcColor = param->getLightSpcColor();
    sead::Vector4f params7(spcColor.r, spcColor.g, spcColor.b, param->getCoinNormalScale());
    setPostProcessingUniform(pContext, mShaderProgram, "uMetalParams7", params7);

    const sead::Color4f& spcColor2 = param->getLightSpcColor2();
    sead::Vector4f params8(spcColor2.r, spcColor2.g, spcColor2.b, param->getCoinNormalCurve());
    setPostProcessingUniform(pContext, mShaderProgram, "uMetalParams8", params8);

    {
        f32 blackPoint = param->getBlackPoint();
        f32 crossOver = param->getCrossOver();
        f32 whitePoint = param->getWhitePoint();
        f32 toe = param->getToe();
        f32 shoulder = param->getSholuder();
        f32 k = ((1.0f - toe) * (crossOver - blackPoint)) /
                std::fmax((1.0f - shoulder) * (whitePoint - crossOver) +
                              (1.0f - toe) * (crossOver - blackPoint),
                          0.0001f);
        f32 shoulderOffset = whitePoint * (1.0f - shoulder) - crossOver;
        sead::Vector4f toeCoeff((1.0f - toe) * k, -toe, -(1.0f - toe) * (blackPoint * k),
                                crossOver - blackPoint * (1.0f - toe));
        sead::Vector4f shoulderCoeff((1.0f - k) + shoulder * k, shoulder,
                                     shoulderOffset * k - crossOver * (1.0f - k), shoulderOffset);

        setPostProcessingUniform(pContext, mShaderProgram, "uCrossOver", crossOver);
        setPostProcessingUniform(pContext, mShaderProgram, "uToeCoeff", toeCoeff);
        setPostProcessingUniform(pContext, mShaderProgram, "uSholuderCoeff", shoulderCoeff);
    }

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(pContext);

    {
        agl::RenderBuffer renderBuffer;
        RenderBufferAttacher attacher(&renderBuffer, color, nullptr, nullptr, nullptr, nullptr);
        drawPostProcessingQuad(pContext);
    }
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const MetalReliefDrawParam* MetalReliefDrawer::getCurrentParam() const {
    return static_cast<const MetalReliefDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void MetalReliefDrawer::requestParam(s32 priority, s32 step, const MetalReliefDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool MetalReliefDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
}

}  // namespace al
