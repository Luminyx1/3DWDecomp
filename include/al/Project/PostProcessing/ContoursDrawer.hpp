#pragma once

#include <basis/seadTypes.h>

#include "Library/Yaml/ParameterBase.hpp"
#include "Project/Base/ParamRequestInterp.hpp"

namespace agl {
class DrawContext;
class RenderBuffer;
class ShaderProgram;
class TextureData;
}  // namespace agl

namespace al {
class ShaderHolder;

class ContoursDrawParam : public IUseRequestParam {
public:
    ContoursDrawParam();

    const char* getParamName() const override { return "輪郭線描画"; }

    ParameterObj* getParamObj() override { return mParamObj; }

    const ParameterObj* getParamObj() const override { return mParamObj; }

    bool isEnable() const;
    bool isNeon() const;
    bool isUseDepth() const;
    bool isUseLinearDepth() const;
    s32 getKernelSize() const;
    s32 getGaussianType() const;
    s32 getGaussianTypeForEdge() const;
    f32 getThreshold() const;
    f32 getThresholdDepth() const;
    f32 getBrightnessOffset() const;

private:
    ParameterObj* mParamObj;
    ParameterBool* mIsEnable;
    ParameterBool* mIsNeon;
    ParameterBool* mIsUseDepth;
    ParameterBool* mIsUseLinearDepth;
    ParameterS32* mKernelSize;
    ParameterS32* mGaussianType;
    ParameterS32* mGaussianTypeForEdge;
    ParameterF32* mThreshold;
    ParameterF32* mThresholdDepth;
    ParameterF32* mBrightnessOffset;
};

class ContoursDrawer {
public:
    ContoursDrawer(ShaderHolder* pShaderHolder);
    ~ContoursDrawer();

    void endInit();
    void clearRequest();
    void update();
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer,
              const agl::TextureData* pLinearDepth, const agl::TextureData* pDepth) const;
    const ContoursDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const ContoursDrawParam& rParam);
    bool isEnable() const;

private:
    ShaderHolder* mShaderHolder;
    agl::ShaderProgram* mShaderProgram;
    ParamRequestInterp* mRequestInterp;
};

}  // namespace al
