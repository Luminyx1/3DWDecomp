#include "Library/PostProcessing/RetroColorDrawer.hpp"

#include "Library/PostProcessing/ScreenBlurDrawer.hpp"

namespace al {

/**
 * Constructs the retro color drawing parameters.
 */
RetroColorDrawParam::RetroColorDrawParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "IsEnable", "", true);
    mIsCheckPaletteWithLuma = new ParameterBool(false, mParamObj, "IsCheckPaletteWithLuma", "IsCheckPaletteWithLuma", "", true);
    mIsPointSampling = new ParameterBool(true, mParamObj, "IsPointSampling", "IsPointSampling", "", true);
    mIsBaseColorMode = new ParameterBool(false, mParamObj, "IsBaseColorMode", "IsBaseColorMode", "", true);
    mIsDrawCRTDisplay = new ParameterBool(false, mParamObj, "IsDrawCRTDisplay", "IsDrawCRTDisplay", "", true);
    mIsDrawCRTPixelSelect = new ParameterBool(false, mParamObj, "IsDrawCRTPixelSelect", "IsDrawCRTPixelSelect", "", true);
    mIsDrawSTNDisplay = new ParameterBool(false, mParamObj, "IsDrawSTNDisplay", "IsDrawSTNDisplay", "", true);
    mColorPaletteId = new ParameterS32(0, mParamObj, "ColorPaletteId", "ColorPaletteId", "", true);
    mWidthScale = new ParameterF32(0.5f, mParamObj, "WidthScale", "WidthScale", "Min=0.01f, Max=1.f", true);
    mHeightScale = new ParameterF32(0.5f, mParamObj, "HeightScale", "HeightScale", "Min=0.01f, Max=1.f", true);
    mRedBit = new ParameterS32(3, mParamObj, "RedBit", "RedBit", "Min=1, Max=8", true);
    mGreenBit = new ParameterS32(3, mParamObj, "GreenBit", "GreenBit", "Min=1, Max=8", true);
    mBlueBit = new ParameterS32(2, mParamObj, "BlueBit", "BlueBit", "Min=1, Max=8", true);
    mCRTDistortion = new ParameterF32(0.1f, mParamObj, "CRTDistortion", "CRTDistortion", "Min=0.f, Max=1.f", true);
    mCRTVignetRate = new ParameterF32(1.3f, mParamObj, "CRTVignetRate", "CRTVignetRate", "Min=0.f, Max=2.f", true);
    mCRTScanLineScale = new ParameterF32(4.0f, mParamObj, "CRTScanLineScale", "CRTScanLineScale", "Min=0.f, Max=4.f", true);
    mCRTScanLineColor = new ParameterF32(0.125f, mParamObj, "CRTScanLineColor", "CRTScanLineColor", "Min=0.f, Max=2.f", true);
    mCRTNoiseParam = new ParameterV3f({1.0f, 1.0f, 1.0f}, mParamObj, "CRTNoiseParam", "CRTNoiseParam", "Min=0.f, Max=2.f", true);
    mSTNLineScale = new ParameterF32(4.0f, mParamObj, "STNLineScale", "STNLineScale", "Min=0.f, Max=4.f", true);
    mSTNLineColor = new ParameterF32(0.125f, mParamObj, "STNLineColor", "STNLineColor", "Min=0.f, Max=2.f", true);
}

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool RetroColorDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Checks the CheckPaletteWithLuma flag.
 * @return Whether CheckPaletteWithLuma is set.
 */
bool RetroColorDrawParam::isCheckPaletteWithLuma() const {
    return mIsCheckPaletteWithLuma->getValue();
}

/**
 * Checks the PointSampling flag.
 * @return Whether PointSampling is set.
 */
bool RetroColorDrawParam::isPointSampling() const {
    return mIsPointSampling->getValue();
}

/**
 * Checks the BaseColorMode flag.
 * @return Whether BaseColorMode is set.
 */
bool RetroColorDrawParam::isBaseColorMode() const {
    return mIsBaseColorMode->getValue();
}

/**
 * Checks the DrawCRTDisplay flag.
 * @return Whether DrawCRTDisplay is set.
 */
bool RetroColorDrawParam::isDrawCRTDisplay() const {
    return mIsDrawCRTDisplay->getValue();
}

/**
 * Checks the DrawCRTPixelSelect flag.
 * @return Whether DrawCRTPixelSelect is set.
 */
bool RetroColorDrawParam::isDrawCRTPixelSelect() const {
    return mIsDrawCRTPixelSelect->getValue();
}

/**
 * Checks the DrawSTNDisplay flag.
 * @return Whether DrawSTNDisplay is set.
 */
bool RetroColorDrawParam::isDrawSTNDisplay() const {
    return mIsDrawSTNDisplay->getValue();
}

/**
 * Gets the ColorPaletteId parameter.
 * @return ColorPaletteId.
 */
s32 RetroColorDrawParam::getColorPaletteId() const {
    return mColorPaletteId->getValue();
}

/**
 * Gets the WidthScale parameter.
 * @return WidthScale.
 */
f32 RetroColorDrawParam::getWidthScale() const {
    return mWidthScale->getValue();
}

/**
 * Gets the HeightScale parameter.
 * @return HeightScale.
 */
f32 RetroColorDrawParam::getHeightScale() const {
    return mHeightScale->getValue();
}

/**
 * Gets the RedBit parameter.
 * @return RedBit.
 */
s32 RetroColorDrawParam::getRedBit() const {
    return mRedBit->getValue();
}

/**
 * Gets the GreenBit parameter.
 * @return GreenBit.
 */
s32 RetroColorDrawParam::getGreenBit() const {
    return mGreenBit->getValue();
}

/**
 * Gets the BlueBit parameter.
 * @return BlueBit.
 */
s32 RetroColorDrawParam::getBlueBit() const {
    return mBlueBit->getValue();
}

/**
 * Gets the CRTDistortion parameter.
 * @return CRTDistortion.
 */
f32 RetroColorDrawParam::getCRTDistortion() const {
    return mCRTDistortion->getValue();
}

/**
 * Gets the CRTVignetRate parameter.
 * @return CRTVignetRate.
 */
f32 RetroColorDrawParam::getCRTVignetRate() const {
    return mCRTVignetRate->getValue();
}

/**
 * Gets the CRTScanLineScale parameter.
 * @return CRTScanLineScale.
 */
f32 RetroColorDrawParam::getCRTScanLineScale() const {
    return mCRTScanLineScale->getValue();
}

/**
 * Gets the CRTScanLineColor parameter.
 * @return CRTScanLineColor.
 */
f32 RetroColorDrawParam::getCRTScanLineColor() const {
    return mCRTScanLineColor->getValue();
}

/**
 * Gets the CRTNoiseParam parameter.
 * @return CRTNoiseParam.
 */
const sead::Vector3f& RetroColorDrawParam::getCRTNoiseParam() const {
    return mCRTNoiseParam->getValue();
}

/**
 * Gets the STNLineScale parameter.
 * @return STNLineScale.
 */
f32 RetroColorDrawParam::getSTNLineScale() const {
    return mSTNLineScale->getValue();
}

/**
 * Gets the STNLineColor parameter.
 * @return STNLineColor.
 */
f32 RetroColorDrawParam::getSTNLineColor() const {
    return mSTNLineColor->getValue();
}


/**
 * Clears the parameter request.
 */
void RetroColorDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void RetroColorDrawer::update() {
    mRequestInterp->updateInterp();
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const RetroColorDrawParam* RetroColorDrawer::getCurrentParam() const {
    return static_cast<const RetroColorDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void RetroColorDrawer::requestParam(s32 priority, s32 step, const RetroColorDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Constructs the screen blur drawing parameters.
 */
ScreenBlurDrawParam::ScreenBlurDrawParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "IsEnable", "", true);
    mBlurNum = new ParameterS32(1, mParamObj, "BlurNum", "BlurNum", "Min=0, Max=3", true);
    mSampleNum = new ParameterS32(16, mParamObj, "SampleNum", "SampleNum", "Min=0, Max=32", true);
    mPosition = new ParameterV2f({0.0f, 0.0f}, mParamObj, "Position", "Position", "", true);
    mStrength =
        new ParameterF32(0.0f, mParamObj, "Strength", "Strength", "Min=0.f, Max=1.f", true);
    mRadius = new ParameterF32(0.0f, mParamObj, "Radius", "Radius", "Min=0.f, Max=2000.f", true);
    mAlpha = new ParameterF32(1.0f, mParamObj, "Alpha", "Alpha", "Min=0.f, Max=1.f", true);
}

}  // namespace al
