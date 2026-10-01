#pragma once

#include <basis/seadTypes.h>
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
class NoiseTextureKeeper;
class ShaderHolder;
class SimpleModelEnv;

class CartoonDrawParam : public IUseRequestParam {
public:
    CartoonDrawParam();

    const char* getParamName() const override { return "トゥーン描画"; }

    ParameterObj* getParamObj() override { return mParamObj; }

    const ParameterObj* getParamObj() const override { return mParamObj; }

    bool isEnable() const;
    bool isEnableFishEye() const;
    f32 getToonShadeRate() const;
    const sead::Vector3f& getToonStep() const;
    const sead::Vector3f& getToonWidth() const;
    s32 getNoiseTextureId() const;
    f32 getNoiseMixRate() const;
    f32 getNoiseScale() const;
    const sead::Vector3f& getNoiseOffset() const;
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
    ParameterBool* mIsEnable;
    ParameterBool* mIsEnableFishEye;
    ParameterF32* mToonShadeRate;
    ParameterV3f* mToonStep;
    ParameterV3f* mToonWidth;
    ParameterS32* mNoiseTextureId;
    ParameterF32* mNoiseMixRate;
    ParameterF32* mNoiseScale;
    ParameterV3f* mNoiseOffset;
    ParameterS32* mCanvasTextureId;
    ParameterF32* mCanvasRepeat;
    ParameterF32* mCanvasMix;
    ParameterS32* mIndirectTextureId;
    ParameterF32* mIndirectScale;
    ParameterV2f* mIndirectTexScale;
    ParameterV2f* mIndirectTexOffset;
    ParameterF32* mFishEyeParam;
};

class CartoonDrawer {
public:
    CartoonDrawer(ShaderHolder* pShaderHolder, NoiseTextureKeeper* pNoiseTextureKeeper);
    ~CartoonDrawer();

    void initProjectResource(nn::g3d::ResFile* pResFile);
    void endInit();
    void clearRequest();
    void update();
    void draw(agl::DrawContext* pContext, SimpleModelEnv* pEnv, const agl::RenderBuffer& rBuffer,
              const agl::TextureData* pLinearDepth, f32 fishEyeRate) const;
    const CartoonDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const CartoonDrawParam& rParam);
    bool isEnable() const;

private:
    agl::ShaderProgram* mShaderProgram;
    ParamRequestInterp* mRequestInterp;
    NoiseTextureKeeper* mNoiseTextureKeeper;
    agl::TextureData** mIndirectTextures;
};

}  // namespace al
