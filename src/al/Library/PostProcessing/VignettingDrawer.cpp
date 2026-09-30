#include "Library/PostProcessing/VignettingDrawer.hpp"

namespace al {

/**
 * Constructs the vignetting drawing parameters.
 */
VignettingParam::VignettingParam() {
    mParamObj = new ParameterObj();
    mIsBlurEnable = new ParameterBool(false, mParamObj, "IsBlurEnable", "IsBlurEnable", "", true);
    mIsBlurPlayerEnable = new ParameterBool(false, mParamObj, "IsBlurPlayerEnable", "IsBlurPlayerEnable", "", true);
    mBlurType = new ParameterS32(0, mParamObj, "BlurType", "BlurType", "Min=0, Max=1", true);
    mBlurRange = new ParameterF32(0.25f, mParamObj, "BlurRange", "BlurRange", "Min=0.f, Max=1.f", true);
    mBlurChangeRange = new ParameterF32(2.0f, mParamObj, "BlurChangeRange", "BlurChangeRange", "Min=0.f, Max=1.f", true);
    mBlurPower = new ParameterF32(1.0f, mParamObj, "BlurPower", "BlurPower", "Min=0.f, Max=1.f", true);
    mBlurPowerMax = new ParameterF32(2.0f, mParamObj, "BlurPowerMax", "BlurPowerMax", "Min=0.f, Max=6.f", true);
    mBlurScale = new ParameterV2f({1.0f, 1.0f}, mParamObj, "BlurScale", "BlurScale", "Min=0.f, Max=2.f", true);
    mBlurOffset = new ParameterV2f({0.0f, 0.0f}, mParamObj, "BlurOffset", "BlurOffset", "Min=-1.f, Max=1.f", true);
    mBlurQuality = new ParameterS32(1, mParamObj, "BlurQuality", "BlurQuality", "Min=0, Max=1", true);
    mIsColorEnable = new ParameterBool(false, mParamObj, "IsColorEnable", "IsColorEnable", "", true);
    mIsColorPlayerEnable = new ParameterBool(false, mParamObj, "IsColorPlayerEnable", "IsColorPlayerEnable", "", true);
    mColorType = new ParameterS32(0, mParamObj, "ColorType", "ColorType", "Min=0, Max=1", true);
    mColorRange = new ParameterF32(0.25f, mParamObj, "ColorRange", "ColorRange", "Min=0.f, Max=1.f", true);
    mColorChangeRange = new ParameterF32(2.0f, mParamObj, "ColorChangeRange", "ColorChangeRange", "Min=0.f, Max=1.f", true);
    mColorScale = new ParameterV2f({1.0f, 1.0f}, mParamObj, "ColorScale", "ColorScale", "Min=0.f, Max=2.f", true);
    mColorOffset = new ParameterV2f({0.0f, 0.0f}, mParamObj, "ColorOffset", "ColorOffset", "Min=-1.f, Max=1.f", true);
    mColor = new ParameterC4f(sead::Color4f(0.0f, 0.0f, 0.0f, 0.75f), mParamObj, "Color", "Color", "Min=0.f, Max=1.f", true);
    mColorBlendType = new ParameterS32(0, mParamObj, "ColorBlendType", "ColorBlendType", "Min=0, Max=3", true);
}

/**
 * Finishes initialization of the parameter interpolation.
 */
void VignettingDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void VignettingDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void VignettingDrawer::update() {
    mRequestInterp->updateInterp();
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void VignettingDrawer::requestParam(s32 priority, s32 step, const VignettingParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const VignettingParam* VignettingDrawer::getCurrentParam() const {
    return static_cast<const VignettingParam*>(mRequestInterp->getCurrentParam());
}

}  // namespace al
