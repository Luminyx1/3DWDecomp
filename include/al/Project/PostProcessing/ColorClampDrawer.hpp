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
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer) const;
    void endInit();
    void clearRequest();
    void update();
    const ColorClampDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const ColorClampDrawParam& rParam);
    bool isEnable() const;

private:
    u8 _0[0x8];
    ParamRequestInterp* mRequestInterp;
};

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
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer) const;
    void endInit();
    void clearRequest();
    void update();
    const KaleidoscopeParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const KaleidoscopeParam& rParam);
    bool isEnable() const;

private:
    u8 _0[0x10];
    ParamRequestInterp* mRequestInterp;
};

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
    MetalReliefDrawer(ShaderHolder* pShaderHolder, UniformBlock* pUniformBlock);
    ~MetalReliefDrawer();
    void initProjectResource(nn::g3d::ResFile* pResFile);
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer, SimpleModelEnv* pEnv, const agl::TextureData& rTexture, const agl::TextureData& rDepth) const;
    void endInit();
    void clearRequest();
    void update();
    const MetalReliefDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const MetalReliefDrawParam& rParam);
    bool isEnable() const;

private:
    u8 _0[0x8];
    ParamRequestInterp* mRequestInterp;
};
}  // namespace al
