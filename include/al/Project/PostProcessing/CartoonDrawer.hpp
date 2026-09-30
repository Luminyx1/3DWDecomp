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

class CartoonDrawParam : public IUseRequestParam {
public:
    CartoonDrawParam();
    const char* getParamName() const override { return "トゥーン描画"; }
    ParameterObj* getParamObj() override { return mParamObj; }
    const ParameterObj* getParamObj() const override { return mParamObj; }
    bool isEnable() const;
    bool isEnableFishEye() const;
    f32 getToonShadeRate() const;
    const sead::Vector2f& getToonStep() const;
    const sead::Vector2f& getToonWidth() const;
    s32 getNoiseTextureId() const;
    f32 getNoiseMixRate() const;
    f32 getNoiseScale() const;
    const sead::Vector2f& getNoiseOffset() const;
    s32 getCanvasTextureId() const;
    f32 getCanvasRepeat() const;
    f32 getCanvasMix() const;
    s32 getIndirectTextureId() const;
    f32 getIndirectScale() const;
    const sead::Vector2f& getIndirectTexScale() const;
    const sead::Vector2f& getIndirectTexOffset() const;
    f32 getFishEyeParam() const;

private:
    ParameterObj* mParamObj;
    Parameter<bool>* mIsEnable;
    Parameter<bool>* mIsEnableFishEye;
    Parameter<f32>* mToonShadeRate;
    Parameter<sead::Vector2f>* mToonStep;
    Parameter<sead::Vector2f>* mToonWidth;
    Parameter<s32>* mNoiseTextureId;
    Parameter<f32>* mNoiseMixRate;
    Parameter<f32>* mNoiseScale;
    Parameter<sead::Vector2f>* mNoiseOffset;
    Parameter<s32>* mCanvasTextureId;
    Parameter<f32>* mCanvasRepeat;
    Parameter<f32>* mCanvasMix;
    Parameter<s32>* mIndirectTextureId;
    Parameter<f32>* mIndirectScale;
    Parameter<sead::Vector2f>* mIndirectTexScale;
    Parameter<sead::Vector2f>* mIndirectTexOffset;
    Parameter<f32>* mFishEyeParam;
};


class CartoonDrawer {
public:
    CartoonDrawer(ShaderHolder* pShaderHolder, NoiseTextureKeeper* pNoiseTextureKeeper);
    ~CartoonDrawer();
    void initProjectResource(nn::g3d::ResFile* pResFile);
    void draw(agl::DrawContext* pContext, SimpleModelEnv* pEnv, const agl::RenderBuffer& rBuffer, const agl::TextureData* pTexture, f32 rate) const;
    void endInit();
    void clearRequest();
    void update();
    const CartoonDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const CartoonDrawParam& rParam);
    bool isEnable() const;

private:
    u8 _0[0x8];
    ParamRequestInterp* mRequestInterp;
};

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
    Parameter<bool>* mIsEnable;
    Parameter<bool>* mIsNeon;
    Parameter<bool>* mIsUseDepth;
    Parameter<bool>* mIsUseLinearDepth;
    Parameter<s32>* mKernelSize;
    Parameter<s32>* mGaussianType;
    Parameter<s32>* mGaussianTypeForEdge;
    Parameter<f32>* mThreshold;
    Parameter<f32>* mThresholdDepth;
    Parameter<f32>* mBrightnessOffset;
};


class ContoursDrawer {
public:
    ContoursDrawer(ShaderHolder* pShaderHolder);
    ~ContoursDrawer();
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer, const agl::TextureData* pTexture, const agl::TextureData* pDepth) const;
    void endInit();
    void clearRequest();
    void update();
    const ContoursDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const ContoursDrawParam& rParam);
    bool isEnable() const;

private:
    u8 _0[0x10];
    ParamRequestInterp* mRequestInterp;
};
}  // namespace al
