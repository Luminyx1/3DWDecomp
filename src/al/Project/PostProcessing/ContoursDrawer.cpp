#include "Project/PostProcessing/CartoonDrawer.hpp"

#include "Project/PostProcessing/ColorClampDrawer.hpp"

namespace al {

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool ContoursDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Checks the Neon flag.
 * @return Whether Neon is set.
 */
bool ContoursDrawParam::isNeon() const {
    return mIsNeon->getValue();
}

/**
 * Checks the UseDepth flag.
 * @return Whether UseDepth is set.
 */
bool ContoursDrawParam::isUseDepth() const {
    return mIsUseDepth->getValue();
}

/**
 * Checks the UseLinearDepth flag.
 * @return Whether UseLinearDepth is set.
 */
bool ContoursDrawParam::isUseLinearDepth() const {
    return mIsUseLinearDepth->getValue();
}

/**
 * Gets the KernelSize parameter.
 * @return KernelSize.
 */
s32 ContoursDrawParam::getKernelSize() const {
    return mKernelSize->getValue();
}

/**
 * Gets the GaussianType parameter.
 * @return GaussianType.
 */
s32 ContoursDrawParam::getGaussianType() const {
    return mGaussianType->getValue();
}

/**
 * Gets the GaussianTypeForEdge parameter.
 * @return GaussianTypeForEdge.
 */
s32 ContoursDrawParam::getGaussianTypeForEdge() const {
    return mGaussianTypeForEdge->getValue();
}

/**
 * Gets the Threshold parameter.
 * @return Threshold.
 */
f32 ContoursDrawParam::getThreshold() const {
    return mThreshold->getValue();
}

/**
 * Gets the ThresholdDepth parameter.
 * @return ThresholdDepth.
 */
f32 ContoursDrawParam::getThresholdDepth() const {
    return mThresholdDepth->getValue();
}

/**
 * Gets the BrightnessOffset parameter.
 * @return BrightnessOffset.
 */
f32 ContoursDrawParam::getBrightnessOffset() const {
    return mBrightnessOffset->getValue();
}


/**
 * Finishes initialization of the parameter interpolation.
 */
void ContoursDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void ContoursDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void ContoursDrawer::update() {
    mRequestInterp->updateInterp();
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const ContoursDrawParam* ContoursDrawer::getCurrentParam() const {
    return static_cast<const ContoursDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void ContoursDrawer::requestParam(s32 priority, s32 step, const ContoursDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool ContoursDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
}

/**
 * Constructs the contours drawing parameters.
 */
ContoursDrawParam::ContoursDrawParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "有効", "", true);
    mIsNeon = new ParameterBool(false, mParamObj, "IsNeon", "ネオン", "", true);
    mIsUseDepth = new ParameterBool(false, mParamObj, "IsUseDepth", "デプスも使う", "", true);
    mIsUseLinearDepth = new ParameterBool(false, mParamObj, "IsUseLinearDepth", "リニアデプスで判定", "", true);
    mKernelSize = new ParameterS32(0, mParamObj, "KernelSize", "カーネルサイズ", "Min=0, Max=2", true);
    mGaussianType = new ParameterS32(1, mParamObj, "GaussianType", "ガウシアンタイプ", "Min=-1, Max=5", true);
    mGaussianTypeForEdge = new ParameterS32(1, mParamObj, "GaussianTypeForEdge", "ガウシアンタイプ(エッジ用)", "Min=-1, Max=5", true);
    mThreshold = new ParameterF32(0.3f, mParamObj, "Threshold", "閾値", "Min=0, Max=1", true);
    mThresholdDepth = new ParameterF32(5.0f, mParamObj, "ThresholdDepth", "デプス閾値", "Min=0, Max=1", true);
    mBrightnessOffset = new ParameterF32(5.0f, mParamObj, "BrightnessOffset", "明度オフセット", "Min=0, Max=1", true);
}

}  // namespace al
