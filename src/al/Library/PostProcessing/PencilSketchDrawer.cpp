#include "Library/PostProcessing/PencilSketchDrawer.hpp"

namespace {
const sead::Color4f cDefaultOffsetColor(-0.5f, -0.5f, -0.5f, 0.0f);
}  // namespace

namespace al {

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool PencilSketchDrawParam::isEnable() const {
    return mIsEnable->getValue();
}


/**
 * Finishes initialization of the parameter interpolation.
 */
void PencilSketchDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void PencilSketchDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void PencilSketchDrawer::update() {
    mRequestInterp->updateInterp();
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const PencilSketchDrawParam* PencilSketchDrawer::getCurrentParam() const {
    return static_cast<const PencilSketchDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void PencilSketchDrawer::requestParam(s32 priority, s32 step, const PencilSketchDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool PencilSketchDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
}

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool MosaicPictureDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Gets the LightAngle parameter.
 * @return LightAngle.
 */
const sead::Vector2f& MosaicPictureDrawParam::getLightAngle() const {
    return mLightAngle->getValue();
}

/**
 * Gets the LightAngle2 parameter.
 * @return LightAngle2.
 */
const sead::Vector2f& MosaicPictureDrawParam::getLightAngle2() const {
    return mLightAngle2->getValue();
}

/**
 * Gets the LightColor parameter.
 * @return LightColor.
 */
const sead::Color4f& MosaicPictureDrawParam::getLightColor() const {
    return mLightColor->getValue();
}

/**
 * Gets the LightColor2 parameter.
 * @return LightColor2.
 */
const sead::Color4f& MosaicPictureDrawParam::getLightColor2() const {
    return mLightColor2->getValue();
}

/**
 * Gets the GrooveColor parameter.
 * @return GrooveColor.
 */
const sead::Color4f& MosaicPictureDrawParam::getGrooveColor() const {
    return mGrooveColor->getValue();
}

/**
 * Gets the NoiseTextureId parameter.
 * @return NoiseTextureId.
 */
s32 MosaicPictureDrawParam::getNoiseTextureId() const {
    return mNoiseTextureId->getValue();
}

/**
 * Gets the NoiseMixRate parameter.
 * @return NoiseMixRate.
 */
f32 MosaicPictureDrawParam::getNoiseMixRate() const {
    return mNoiseMixRate->getValue();
}

/**
 * Gets the TileSizeX parameter.
 * @return TileSizeX.
 */
s32 MosaicPictureDrawParam::getTileSizeX() const {
    return mTileSizeX->getValue();
}

/**
 * Gets the TileSizeY parameter.
 * @return TileSizeY.
 */
s32 MosaicPictureDrawParam::getTileSizeY() const {
    return mTileSizeY->getValue();
}

/**
 * Gets the Exposure parameter.
 * @return Exposure.
 */
f32 MosaicPictureDrawParam::getExposure() const {
    return mExposure->getValue();
}

/**
 * Gets the BlackPoint parameter.
 * @return BlackPoint.
 */
f32 MosaicPictureDrawParam::getBlackPoint() const {
    return mBlackPoint->getValue();
}

/**
 * Gets the CrossOver parameter.
 * @return CrossOver.
 */
f32 MosaicPictureDrawParam::getCrossOver() const {
    return mCrossOver->getValue();
}

/**
 * Gets the WhitePoint parameter.
 * @return WhitePoint.
 */
f32 MosaicPictureDrawParam::getWhitePoint() const {
    return mWhitePoint->getValue();
}

/**
 * Gets the Toe parameter.
 * @return Toe.
 */
f32 MosaicPictureDrawParam::getToe() const {
    return mToe->getValue();
}

/**
 * Gets the Sholuder parameter.
 * @return Sholuder.
 */
f32 MosaicPictureDrawParam::getSholuder() const {
    return mSholuder->getValue();
}


/**
 * Finishes initialization of the parameter interpolation.
 */
void MosaicPictureDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void MosaicPictureDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void MosaicPictureDrawer::update() {
    mRequestInterp->updateInterp();
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const MosaicPictureDrawParam* MosaicPictureDrawer::getCurrentParam() const {
    return static_cast<const MosaicPictureDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void MosaicPictureDrawer::requestParam(s32 priority, s32 step, const MosaicPictureDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool MosaicPictureDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
}

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool EdgeDrawPostEffectParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Checks the ConstColor flag.
 * @return Whether ConstColor is set.
 */
bool EdgeDrawPostEffectParam::isConstColor() const {
    return mIsConstColor->getValue();
}

/**
 * Checks the Bold flag.
 * @return Whether Bold is set.
 */
bool EdgeDrawPostEffectParam::isBold() const {
    return mIsBold->getValue();
}

/**
 * Gets the BoundDepth parameter.
 * @return BoundDepth.
 */
f32 EdgeDrawPostEffectParam::getBoundDepth() const {
    return mBoundDepth->getValue();
}

/**
 * Gets the EdgeEndDepth parameter.
 * @return EdgeEndDepth.
 */
f32 EdgeDrawPostEffectParam::getEdgeEndDepth() const {
    return mEdgeEndDepth->getValue();
}

/**
 * Gets the EdgePowerMin parameter.
 * @return EdgePowerMin.
 */
f32 EdgeDrawPostEffectParam::getEdgePowerMin() const {
    return mEdgePowerMin->getValue();
}

/**
 * Gets the EdgeNormalEdgeBound parameter.
 * @return EdgeNormalEdgeBound.
 */
f32 EdgeDrawPostEffectParam::getEdgeNormalEdgeBound() const {
    return mEdgeNormalEdgeBound->getValue();
}

/**
 * Gets the OffsetColor parameter.
 * @return OffsetColor.
 */
const sead::Color4f& EdgeDrawPostEffectParam::getOffsetColor() const {
    return mOffsetColor->getValue();
}


/**
 * Finishes initialization of the parameter interpolation.
 */
void EdgeDrawerPostEffect::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void EdgeDrawerPostEffect::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void EdgeDrawerPostEffect::update() {
    mRequestInterp->updateInterp();
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const EdgeDrawPostEffectParam* EdgeDrawerPostEffect::getCurrentParam() const {
    return static_cast<const EdgeDrawPostEffectParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void EdgeDrawerPostEffect::requestParam(s32 priority, s32 step, const EdgeDrawPostEffectParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool EdgeDrawerPostEffect::isEnable() const {
    return getCurrentParam()->isEnable();
}

/**
 * Constructs the mosaic picture drawing parameters.
 */
MosaicPictureDrawParam::MosaicPictureDrawParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "有効", "", true);
    mLightAngle = new ParameterV2f({0.0f, 0.0f}, mParamObj, "LightAngle", "ライト方向", "Min=-180.f, Max=180.f", true);
    mLightAngle2 = new ParameterV2f({0.0f, 0.0f}, mParamObj, "LightAngle2", "ライト方向2", "Min=-180.f, Max=180.f", true);
    mLightColor = new ParameterC4f(sead::Color4f::cWhite, mParamObj, "LightColor", "ライトカラー", "Min=0.f, Max=1024.f", true);
    mLightColor2 = new ParameterC4f(sead::Color4f::cWhite, mParamObj, "LightColor2", "ライトカラー2", "Min=0.f, Max=1024.f", true);
    mGrooveColor = new ParameterC4f(sead::Color4f::cGray, mParamObj, "GrooveColor", "溝カラー", "Min=0.f, Max=1.f", true);
    mNoiseTextureId = new ParameterS32(-1, mParamObj, "NoiseTetxureId", "ノイズテクスチャ", "Min=0, Max=32", true);
    mNoiseMixRate = new ParameterF32(0.0f, mParamObj, "NoiseMixRate", "ノイズ混ぜ合わせ量", "Min=0.f, Max=1.f", true);
    mTileSizeX = new ParameterS32(16, mParamObj, "TileSizeX", "タイルサイズX", "Min=1, Max=160", true);
    mTileSizeY = new ParameterS32(16, mParamObj, "TileSizeY", "タイルサイズY", "Min=1, Max=90", true);
    mExposure = new ParameterF32(1.0f, mParamObj, "Exposure", "露出", "Min=0.f, Max=1.f", true);
    mBlackPoint = new ParameterF32(0.5f, mParamObj, "BlackPoint", "BlackPoint", "Min=0.0f, Max=1.0f", true);
    mCrossOver = new ParameterF32(2.0f, mParamObj, "CrossOver", "CrossOver", "Min=0.0f, Max=10.0f", true);
    mWhitePoint = new ParameterF32(10.0f, mParamObj, "WhitePoint", "WhitePoint", "Min=0.0f, Max=20.0f", true);
    mToe = new ParameterF32(0.0f, mParamObj, "Toe", "Toe", "Min=0.0f, Max=1.0f", true);
    mSholuder = new ParameterF32(0.0f, mParamObj, "Sholuder", "Sholuder", "Min=0.0f, Max=1.0f", true);
}

/**
 * Constructs the post effect edge drawing parameters.
 */
EdgeDrawPostEffectParam::EdgeDrawPostEffectParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "有効", "", true);
    mIsConstColor = new ParameterBool(false, mParamObj, "IsConstColor", "輪郭色を固定色にする", "", true);
    mIsBold = new ParameterBool(false, mParamObj, "IsBold", "太線", "", true);
    mBoundDepth = new ParameterF32(0.00015f, mParamObj, "BoundDepth", "深度エッジ境界値", "Min=0.f, Max=0.001f", true);
    mEdgeEndDepth = new ParameterF32(0.07f, mParamObj, "EdgeEndDepth", "エッジ有効深度", "Min=0.f, Max=1.f", true);
    mEdgePowerMin = new ParameterF32(0.1f, mParamObj, "EdgePowerMin", "エッジ濃さ", "Min=0.f, Max=1.f", true);
    mEdgeNormalEdgeBound = new ParameterF32(0.01f, mParamObj, "EdgeNormalEdgeBound", "法線エッジ境界値", "Min=0.f, Max=10.f", true);
    mOffsetColor = new ParameterC4f(cDefaultOffsetColor, mParamObj, "OffsetColor", "輪郭色スケーリング(固定色有効の場合は色)", "Min=-1.f, Max=1.f", true);
}

}  // namespace al
