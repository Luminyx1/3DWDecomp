#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>

#include "Library/Yaml/ParameterBase.hpp"
#include "Project/Base/ParamRequestInterp.hpp"

namespace agl {
class DrawContext;
class RenderBuffer;
class ShaderProgram;
}  // namespace agl

namespace al {
class ShaderHolder;

class ColorClampDrawParam : public IUseRequestParam {
public:
    ColorClampDrawParam();
    const char* getParamName() const override { return "Color Clamp Draw"; }
    ParameterObj* getParamObj() override { return mParamObj; }
    const ParameterObj* getParamObj() const override { return mParamObj; }
    bool isEnable() const;
    const sead::Color4f& getClampColor() const;
    const s32& getModifyStyle() const;

private:
    ParameterObj* mParamObj;
    Parameter<bool>* mIsEnable;
    Parameter<sead::Color4f>* mClampColor;
    Parameter<s32>* mModifyStyle;
};

class ColorClampDrawer {
public:
    ColorClampDrawer(ShaderHolder* pShaderHolder);

    void endInit();
    void clearRequest();
    void update();
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer) const;
    const ColorClampDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const ColorClampDrawParam& rParam);
    bool isEnable() const;

private:
    agl::ShaderProgram* mShaderProgram;
    ParamRequestInterp* mRequestInterp;
};
}  // namespace al
