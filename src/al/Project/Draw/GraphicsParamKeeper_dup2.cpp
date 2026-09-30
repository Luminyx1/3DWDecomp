#include "Library/PostProcessing/RetroColorDrawer.hpp"

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

}  // namespace al
