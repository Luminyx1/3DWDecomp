#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Yaml/ParameterBase.hpp"
#include "Project/Base/ParamRequestInterp.hpp"

namespace sead {
class Camera;
}

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

class PencilSketchDrawParam : public IUseRequestParam {
public:
    PencilSketchDrawParam();
    const char* getParamName() const override { return "Pencil Sketch Draw"; }
    ParameterObj* getParamObj() override { return mParamObj; }
    const ParameterObj* getParamObj() const override { return mParamObj; }
    bool isEnable() const;

private:
    ParameterObj* mParamObj;
    Parameter<bool>* mIsEnable;
};


class PencilSketchDrawer {
public:
    PencilSketchDrawer(ShaderHolder* pShaderHolder);
    ~PencilSketchDrawer();
    void draw(agl::DrawContext* pContext, SimpleModelEnv* pEnv, const agl::RenderBuffer& rBuffer, const agl::TextureData* pTexture) const;
    void endInit();
    void clearRequest();
    void update();
    const PencilSketchDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const PencilSketchDrawParam& rParam);
    bool isEnable() const;

private:
    u8 _0[0x8];
    ParamRequestInterp* mRequestInterp;
};

class MosaicPictureDrawParam : public IUseRequestParam {
public:
    MosaicPictureDrawParam();
    const char* getParamName() const override { return "モザイク描画"; }
    ParameterObj* getParamObj() override { return mParamObj; }
    const ParameterObj* getParamObj() const override { return mParamObj; }
    bool isEnable() const;
    const sead::Vector2f& getLightAngle() const;
    const sead::Vector2f& getLightAngle2() const;
    const sead::Color4f& getLightColor() const;
    const sead::Color4f& getLightColor2() const;
    const sead::Color4f& getGrooveColor() const;
    s32 getNoiseTextureId() const;
    f32 getNoiseMixRate() const;
    s32 getTileSizeX() const;
    s32 getTileSizeY() const;
    f32 getExposure() const;
    f32 getBlackPoint() const;
    f32 getCrossOver() const;
    f32 getWhitePoint() const;
    f32 getToe() const;
    f32 getSholuder() const;

private:
    ParameterObj* mParamObj;
    Parameter<bool>* mIsEnable;
    Parameter<sead::Vector2f>* mLightAngle;
    Parameter<sead::Vector2f>* mLightAngle2;
    Parameter<sead::Color4f>* mLightColor;
    Parameter<sead::Color4f>* mLightColor2;
    Parameter<sead::Color4f>* mGrooveColor;
    Parameter<s32>* mNoiseTextureId;
    Parameter<f32>* mNoiseMixRate;
    Parameter<s32>* mTileSizeX;
    Parameter<s32>* mTileSizeY;
    Parameter<f32>* mExposure;
    Parameter<f32>* mBlackPoint;
    Parameter<f32>* mCrossOver;
    Parameter<f32>* mWhitePoint;
    Parameter<f32>* mToe;
    Parameter<f32>* mSholuder;
};


class MosaicPictureDrawer {
public:
    MosaicPictureDrawer(ShaderHolder* pShaderHolder, UniformBlock* pUniformBlock, NoiseTextureKeeper* pNoiseTextureKeeper);
    ~MosaicPictureDrawer();
    void initProjectResource(nn::g3d::ResFile* pResFile);
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer, SimpleModelEnv* pEnv) const;
    void endInit();
    void clearRequest();
    void update();
    const MosaicPictureDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const MosaicPictureDrawParam& rParam);
    bool isEnable() const;

private:
    u8 _0[0x8];
    ParamRequestInterp* mRequestInterp;
    u8 _10[0x28];
};

class EdgeDrawPostEffectParam : public IUseRequestParam {
public:
    EdgeDrawPostEffectParam();
    const char* getParamName() const override { return "エッジ描画"; }
    ParameterObj* getParamObj() override { return mParamObj; }
    const ParameterObj* getParamObj() const override { return mParamObj; }
    bool isEnable() const;
    bool isConstColor() const;
    bool isBold() const;
    f32 getBoundDepth() const;
    f32 getEdgeEndDepth() const;
    f32 getEdgePowerMin() const;
    f32 getEdgeNormalEdgeBound() const;
    const sead::Color4f& getOffsetColor() const;

private:
    ParameterObj* mParamObj;
    Parameter<bool>* mIsEnable;
    Parameter<bool>* mIsConstColor;
    Parameter<bool>* mIsBold;
    Parameter<f32>* mBoundDepth;
    Parameter<f32>* mEdgeEndDepth;
    Parameter<f32>* mEdgePowerMin;
    Parameter<f32>* mEdgeNormalEdgeBound;
    Parameter<sead::Color4f>* mOffsetColor;
};


class EdgeDrawerPostEffect {
public:
    EdgeDrawerPostEffect(ShaderHolder* pShaderHolder, s32 viewNum);
    virtual ~EdgeDrawerPostEffect();
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer* pRenderBuffer,
              const agl::RenderBuffer* pEdgeBuffer, const agl::TextureData* pColor,
              const agl::TextureData* pLinearDepth, const agl::TextureData* pNormal, s32 viewIndex,
              const sead::Camera& rCamera, f32 near, f32 far, bool isKeepBuffer) const;
    void endInit();
    void clearRequest();
    void update();
    const EdgeDrawPostEffectParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const EdgeDrawPostEffectParam& rParam);
    bool isEnable() const;

private:
    u8 _8[0x28];
    ParamRequestInterp* mRequestInterp;
};
}  // namespace al
