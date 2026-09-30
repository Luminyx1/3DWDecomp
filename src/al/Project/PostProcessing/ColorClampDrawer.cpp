#include "Project/PostProcessing/ColorClampDrawer.hpp"

#include "Library/PostProcessing/PencilSketchDrawer.hpp"

namespace al {

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool ColorClampDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Gets the ClampColor parameter.
 * @return ClampColor.
 */
const sead::Color4f& ColorClampDrawParam::getClampColor() const {
    return mClampColor->getValue();
}

/**
 * Gets the ModifyStyle parameter.
 * @return ModifyStyle.
 */
const s32& ColorClampDrawParam::getModifyStyle() const {
    return mModifyStyle->getValue();
}


/**
 * Finishes initialization of the parameter interpolation.
 */
void ColorClampDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void ColorClampDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void ColorClampDrawer::update() {
    mRequestInterp->updateInterp();
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const ColorClampDrawParam* ColorClampDrawer::getCurrentParam() const {
    return static_cast<const ColorClampDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void ColorClampDrawer::requestParam(s32 priority, s32 step, const ColorClampDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool ColorClampDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
}

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool KaleidoscopeParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Gets the MirrorType parameter.
 * @return MirrorType.
 */
s32 KaleidoscopeParam::getMirrorType() const {
    return mMirrorType->getValue();
}

/**
 * Gets the DivideNum parameter.
 * @return DivideNum.
 */
s32 KaleidoscopeParam::getDivideNum() const {
    return mDivideNum->getValue();
}

/**
 * Checks the Distort flag.
 * @return Whether Distort is set.
 */
bool KaleidoscopeParam::isDistort() const {
    return mIsDistort->getValue();
}


/**
 * Finishes initialization of the parameter interpolation.
 */
void KaleidoscopeDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void KaleidoscopeDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const KaleidoscopeParam* KaleidoscopeDrawer::getCurrentParam() const {
    return static_cast<const KaleidoscopeParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void KaleidoscopeDrawer::requestParam(s32 priority, s32 step, const KaleidoscopeParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool KaleidoscopeDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
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

/**
 * Constructs the kaleidoscope parameters.
 */
KaleidoscopeParam::KaleidoscopeParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "有効", "", true);
    mMirrorType = new ParameterS32(3, mParamObj, "MirrorType", "タイプ", "Min=0, Max=1", true);
    mDivideNum = new ParameterS32(4, mParamObj, "RadialDivNum", "分割数", "Min=1, Max=32", true);
    mIsDistort = new ParameterBool(true, mParamObj, "IsDistort", "中心圧縮する", "", true);
}

/**
 * Constructs the pencil sketch drawing parameters.
 */
PencilSketchDrawParam::PencilSketchDrawParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "IsEnable", "", true);
}

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

}  // namespace al
