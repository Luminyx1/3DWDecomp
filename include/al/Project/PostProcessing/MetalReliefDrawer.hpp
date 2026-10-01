#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadVector.h>

#include "Library/Yaml/ParameterBase.hpp"
#include "Project/Base/ParamRequestInterp.hpp"

namespace agl {
class DrawContext;
class RenderBuffer;
class ShaderProgram;
class TextureData;
}  // namespace agl

namespace nn::g3d {
class ResFile;
}

namespace al {
class ShaderHolder;
class SimpleModelEnv;
class UniformBlock;

class MetalReliefDrawParam : public IUseRequestParam {
public:
    MetalReliefDrawParam();
    const char* getParamName() const override { return "金属レリーフ"; }
    ParameterObj* getParamObj() override { return mParamObj; }
    const ParameterObj* getParamObj() const override { return mParamObj; }
    bool isEnable() const;
    const sead::Vector2f& getLightAngle() const;
    const sead::Vector2f& getLightAngle2() const;
    const sead::Color4f& getLightColor() const;
    const sead::Color4f& getLightColor2() const;
    const sead::Color4f& getLightSpcColor() const;
    const sead::Color4f& getLightSpcColor2() const;
    const sead::Color4f& getMetalColor() const;
    f32 getMetalness() const;
    f32 getRoughness() const;
    f32 getCoinRange() const;
    f32 getAlbedoRange() const;
    f32 getCoinYOffset() const;
    f32 getExposure() const;
    f32 getBlackPoint() const;
    f32 getCrossOver() const;
    f32 getWhitePoint() const;
    f32 getToe() const;
    f32 getSholuder() const;
    f32 getTargetDepth() const;
    f32 getCoinRoughness() const;
    f32 getCoinNormalBase() const;
    f32 getRoughnessScale() const;
    f32 getNormalScale() const;
    f32 getCoinNormalCurve() const;
    f32 getCoinNormalScale() const;

private:
    ParameterObj* mParamObj;
    Parameter<bool>* mIsEnable;
    Parameter<sead::Vector2f>* mLightAngle;
    Parameter<sead::Vector2f>* mLightAngle2;
    Parameter<sead::Color4f>* mLightColor;
    Parameter<sead::Color4f>* mLightColor2;
    Parameter<sead::Color4f>* mLightSpcColor;
    Parameter<sead::Color4f>* mLightSpcColor2;
    Parameter<sead::Color4f>* mMetalColor;
    Parameter<f32>* mMetalness;
    Parameter<f32>* mRoughness;
    Parameter<f32>* mCoinRange;
    Parameter<f32>* mAlbedoRange;
    Parameter<f32>* mCoinYOffset;
    Parameter<f32>* mExposure;
    Parameter<f32>* mBlackPoint;
    Parameter<f32>* mCrossOver;
    Parameter<f32>* mWhitePoint;
    Parameter<f32>* mToe;
    Parameter<f32>* mSholuder;
    Parameter<f32>* mTargetDepth;
    Parameter<f32>* mCoinRoughness;
    Parameter<f32>* mCoinNormalBase;
    Parameter<f32>* mRoughnessScale;
    Parameter<f32>* mNormalScale;
    Parameter<f32>* mCoinNormalCurve;
    Parameter<f32>* mCoinNormalScale;
};

class MetalReliefDrawer {
public:
    MetalReliefDrawer(ShaderHolder* pShaderHolder, UniformBlock* pLightEnvBlock);
    ~MetalReliefDrawer();

    void initProjectResource(nn::g3d::ResFile* pResFile);
    void endInit();
    void clearRequest();
    void update();
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer, SimpleModelEnv* pEnv,
              const agl::TextureData& rNormal, const agl::TextureData& rDepth) const;
    const MetalReliefDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const MetalReliefDrawParam& rParam);
    bool isEnable() const;

private:
    agl::ShaderProgram* mShaderProgram;
    ParamRequestInterp* mRequestInterp;
    UniformBlock* mLightEnvBlock;
    agl::TextureData* mCoinTexture;
};
}  // namespace al
