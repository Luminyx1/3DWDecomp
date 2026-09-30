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

class ViewDepthDrawParam : public IUseRequestParam {
public:
    ViewDepthDrawParam();
    const char* getParamName() const override { return "View Depth Draw"; }
    ParameterObj* getParamObj() override { return mParamObj; }
    const ParameterObj* getParamObj() const override { return mParamObj; }
    bool isEnable() const;
    f32 getFar() const;
    const sead::Color4f& getFarColor() const;
    const sead::Color4f& getNearColor() const;

private:
    ParameterObj* mParamObj;
    Parameter<bool>* mIsEnable;
    Parameter<f32>* mFar;
    Parameter<sead::Color4f>* mFarColor;
    Parameter<sead::Color4f>* mNearColor;
};


class ViewDepthDrawer {
public:
    ViewDepthDrawer(ShaderHolder* pShaderHolder);
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer, f32 near, f32 far, const agl::TextureData& rDepth) const;
    void endInit();
    void clearRequest();
    void update();
    const ViewDepthDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const ViewDepthDrawParam& rParam);
    bool isEnable() const;

private:
    u8 _0[0x8];
    ParamRequestInterp* mRequestInterp;
};
}  // namespace al
