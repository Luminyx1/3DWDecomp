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

class VignettingParam : public IUseRequestParam {
public:
    VignettingParam();

    const char* getParamName() const override { return "Vignetting Draw"; }
    ParameterObj* getParamObj() override { return mParamObj; }
    const ParameterObj* getParamObj() const override { return mParamObj; }

    friend class VignettingDrawer;

private:
    ParameterObj* mParamObj;
    Parameter<bool>* mIsBlurEnable;
    Parameter<bool>* mIsBlurPlayerEnable;
    Parameter<s32>* mBlurType;
    Parameter<f32>* mBlurRange;
    Parameter<f32>* mBlurChangeRange;
    Parameter<f32>* mBlurPower;
    Parameter<f32>* mBlurPowerMax;
    Parameter<sead::Vector2f>* mBlurScale;
    Parameter<sead::Vector2f>* mBlurOffset;
    Parameter<s32>* mBlurQuality;
    Parameter<bool>* mIsColorEnable;
    Parameter<bool>* mIsColorPlayerEnable;
    Parameter<s32>* mColorType;
    Parameter<f32>* mColorRange;
    Parameter<f32>* mColorChangeRange;
    Parameter<sead::Vector2f>* mColorScale;
    Parameter<sead::Vector2f>* mColorOffset;
    Parameter<sead::Color4f>* mColor;
    Parameter<s32>* mColorBlendType;
};

class VignettingDrawer {
public:
    void endInit();
    void clearRequest();
    void update();
    void requestParam(s32 priority, s32 step, const VignettingParam& rParam);
    const VignettingParam* getCurrentParam() const;
    bool isEnableBlur() const;
    bool isEnableColor() const;

private:
    u8 _0[0x8];
    ParamRequestInterp* mRequestInterp;
};
}  // namespace al
