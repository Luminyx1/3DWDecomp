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

class ScreenBlurDrawParam : public IUseRequestParam {
public:
    ScreenBlurDrawParam();
    const char* getParamName() const override { return "Screen Blur Draw"; }
    ParameterObj* getParamObj() override { return mParamObj; }
    const ParameterObj* getParamObj() const override { return mParamObj; }
    bool isEnable() const;
    s32 getBlurNum() const;
    s32 getSampleNum() const;
    const sead::Vector2f& getPosition() const;
    f32 getStrength() const;
    f32 getRadius() const;
    f32 getAlpha() const;

private:
    ParameterObj* mParamObj;
    Parameter<bool>* mIsEnable;
    Parameter<s32>* mBlurNum;
    Parameter<s32>* mSampleNum;
    Parameter<sead::Vector2f>* mPosition;
    Parameter<f32>* mStrength;
    Parameter<f32>* mRadius;
    Parameter<f32>* mAlpha;
};


class ScreenBlurDrawer {
public:
    ScreenBlurDrawer(s32 contextNum);
    ~ScreenBlurDrawer();
    void updateViewGpu(s32 index, const sead::Matrix34f& rView, const sead::Matrix44f& rProj);
    void draw(agl::DrawContext* pContext, s32 index, const agl::RenderBuffer& rBuffer) const;
    void endInit();
    void clearRequest();
    void update();
    const ScreenBlurDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const ScreenBlurDrawParam& rParam);
    bool isEnable() const;

private:
    ParamRequestInterp* mRequestInterp;
    agl::fx::RadialBlur* mRadialBlur;
};
}  // namespace al
