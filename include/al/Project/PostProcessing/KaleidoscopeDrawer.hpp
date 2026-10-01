#pragma once

#include <basis/seadTypes.h>

#include "Library/Yaml/ParameterBase.hpp"
#include "Project/Base/ParamRequestInterp.hpp"

namespace agl {
class DrawContext;
class RenderBuffer;
class ShaderProgram;
}  // namespace agl

namespace al {
class ShaderHolder;
class UniformBlock;

class KaleidoscopeParam : public IUseRequestParam {
public:
    KaleidoscopeParam();
    const char* getParamName() const override { return "万華鏡描画"; }
    ParameterObj* getParamObj() override { return mParamObj; }
    const ParameterObj* getParamObj() const override { return mParamObj; }
    bool isEnable() const;
    s32 getMirrorType() const;
    s32 getDivideNum() const;
    bool isDistort() const;

private:
    ParameterObj* mParamObj;
    Parameter<bool>* mIsEnable;
    Parameter<s32>* mMirrorType;
    Parameter<s32>* mDivideNum;
    Parameter<bool>* mIsDistort;
};

class KaleidoscopeDrawer {
public:
    KaleidoscopeDrawer(ShaderHolder* pShaderHolder);
    ~KaleidoscopeDrawer();

    void endInit();
    void clearRequest();
    void update();
    const KaleidoscopeParam* getCurrentParam() const;
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer) const;
    void requestParam(s32 priority, s32 step, const KaleidoscopeParam& rParam);
    bool isEnable() const;

private:
    ShaderHolder* mShaderHolder;
    agl::ShaderProgram* mShaderProgram;
    ParamRequestInterp* mRequestInterp;
    UniformBlock* mUniformBlock;
};
}  // namespace al
