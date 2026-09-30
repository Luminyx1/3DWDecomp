#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Yaml/ParameterBase.hpp"
#include "Project/Base/ParamRequestInterp.hpp"

namespace agl {
class DrawContext;
class RenderBuffer;
class TextureData;
}  // namespace agl

namespace agl::fx {
class RadialBlur;
}

namespace nn::g3d {
class ResFile;
}

namespace al {
class NoiseTextureKeeper;
class ShaderHolder;
class SimpleModelEnv;
class UniformBlock;

class RetroColorDrawParam : public IUseRequestParam {
public:
    RetroColorDrawParam();
    const char* getParamName() const override { return "Retro Color Draw"; }
    ParameterObj* getParamObj() override { return mParamObj; }
    const ParameterObj* getParamObj() const override { return mParamObj; }
    bool isEnable() const;
    bool isCheckPaletteWithLuma() const;
    bool isPointSampling() const;
    bool isBaseColorMode() const;
    bool isDrawCRTDisplay() const;
    bool isDrawCRTPixelSelect() const;
    bool isDrawSTNDisplay() const;
    s32 getColorPaletteId() const;
    f32 getWidthScale() const;
    f32 getHeightScale() const;
    s32 getRedBit() const;
    s32 getGreenBit() const;
    s32 getBlueBit() const;
    f32 getCRTDistortion() const;
    f32 getCRTVignetRate() const;
    f32 getCRTScanLineScale() const;
    f32 getCRTScanLineColor() const;
    const sead::Vector3f& getCRTNoiseParam() const;
    f32 getSTNLineScale() const;
    f32 getSTNLineColor() const;

private:
    ParameterObj* mParamObj;
    Parameter<bool>* mIsEnable;
    Parameter<bool>* mIsCheckPaletteWithLuma;
    Parameter<bool>* mIsPointSampling;
    Parameter<bool>* mIsBaseColorMode;
    Parameter<bool>* mIsDrawCRTDisplay;
    Parameter<bool>* mIsDrawCRTPixelSelect;
    Parameter<bool>* mIsDrawSTNDisplay;
    Parameter<s32>* mColorPaletteId;
    Parameter<f32>* mWidthScale;
    Parameter<f32>* mHeightScale;
    Parameter<s32>* mRedBit;
    Parameter<s32>* mGreenBit;
    Parameter<s32>* mBlueBit;
    Parameter<f32>* mCRTDistortion;
    Parameter<f32>* mCRTVignetRate;
    Parameter<f32>* mCRTScanLineScale;
    Parameter<f32>* mCRTScanLineColor;
    Parameter<sead::Vector3f>* mCRTNoiseParam;
    Parameter<f32>* mSTNLineScale;
    Parameter<f32>* mSTNLineColor;
};


class RetroColorDrawer {
public:
    RetroColorDrawer(ShaderHolder* pShaderHolder);
    ~RetroColorDrawer();
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer, const agl::TextureData& rTexture) const;
    void endInit();
    void clearRequest();
    void update();
    const RetroColorDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const RetroColorDrawParam& rParam);
    bool isEnable() const;

private:
    u8 _0[0x8];
    ParamRequestInterp* mRequestInterp;
};
}  // namespace al
