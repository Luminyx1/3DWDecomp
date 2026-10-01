#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjList.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadVector.h>

#include "Library/Effect/EffectShaderHolder.hpp"
#include "Project/Effect/EffectUtil.hpp"

namespace nn::util::neon {
struct Vector3fType;
}  // namespace nn::util::neon

namespace nn::vfx {
class Emitter;
struct EmitterFinalizeArg;
struct EmitterInitializeArg;
struct ParticleCalculateArgImpl;

namespace detail {
struct ParticleProperty;
struct ResFieldCustom;
}  // namespace detail
}  // namespace nn::vfx

namespace al {
class PrePassLightKeeper;

/**
 * @brief A point light requested by an effect emitter or particle.
 */
class EffectLight {
public:
    void set(const sead::Vector3f& rPos, f32 radius, const sead::Color4f& rColor,
             bool isEnableSpecular);

    const sead::Vector3f& getPos() const { return mPos; }

    f32 getRadius() const { return mRadius; }

    const sead::Color4f& getColor() const { return mColor; }

    bool isEnableSpecular() const { return mIsEnableSpecular; }

private:
    sead::Vector3f mPos;
    f32 mRadius;
    sead::Color4f mColor;
    bool mIsEnableSpecular;
};

static_assert(sizeof(EffectLight) == 0x24);

/**
 * @brief Pool of the point lights requested by effects.
 */
class EffectLightDirector {
public:
    EffectLightDirector();

    EffectLight* tryCreateLight();
    void removeLight(EffectLight* pLight);
    void update(PrePassLightKeeper* pKeeper);

private:
    using LightList = sead::ObjList<EffectLight>;

    LightList mLightList;
};

static_assert(sizeof(EffectLightDirector) == 0x30);

/**
 * Sets an effect light from the point light custom action data.
 * @param pLight the light
 * @param rPos the light position
 * @param pData the point light custom action data
 * @param scale the scale of the light color
 * @param frame the current frame
 * @param life the life of the emitter or particle, or 0 if it is endless
 */
inline void setEffectLight(EffectLight* pLight, const sead::Vector3f& rPos,
                           const CustomActionDataPointLight* pData, f32 scale, f32 frame,
                           f32 life) {
    s32 fadeInFrame = pData->fadeInFrame;
    s32 fadeOutFrame = pData->fadeOutFrame;
    sead::Color4f color;
    calcAnim8Key(&color,
                 *reinterpret_cast<const nn::vfx::detail::ResAnim8KeyParamSet*>(pData->alphaAnim),
                 life, frame);
    f32 alpha = color.r;
    calcAnim8Key(&color,
                 *reinterpret_cast<const nn::vfx::detail::ResAnim8KeyParamSet*>(pData->colorAnim),
                 life, frame);
    color *= alpha;
    color.a = 1.0f;
    sead::Color4f lightColor;

    if (fadeInFrame >= 1) {
        lightColor.setLerp(sead::Color4f::cBlack, color,
                           sead::Mathf::clamp(frame / fadeInFrame, 0.0f, 1.0f));
    } else {
        lightColor = color;
    }

    if (life > 0.0f && fadeOutFrame >= 1) {
        f32 rate = sead::Mathf::clamp((frame - (life - fadeOutFrame)) / fadeOutFrame, 0.0f, 1.0f);
        lightColor.setLerp(sead::Color4f::cBlack, lightColor, 1.0f - rate);
    }

    if (scale != 1.0f) {
        lightColor *= scale;
    }

    pLight->set(rPos, pData->radius, lightColor, pData->flags & 1);
}

class EffectShaderHolderNew : public EffectShaderHolder {
public:
    EffectShaderHolderNew(PtclSystem* pPtclSystem, agl::DrawContext* pDrawContext,
                          EffectEnvParam* pEnvParam);

    void updateShaderParam(const sead::Matrix34f& rViewMtx,
                           const sead::Vector2f& rScreenSize) override;
    void swapUbo() override;
    void bindCustomShaderUbo(agl::DrawContext* pDrawContext) override;
    void updateLight() override;
    void renderDeferred(nn::vfx::RenderStateSetArg& rArg) const override;

    void updateShaderParamForCompute();
    void bindCustomShaderUboForCompute(agl::DrawContext* pDrawContext);
    void setupTextureProg0(const agl::TextureData* pTexture);
    const agl::TextureSampler* getTextureMaterialLight(s32 index) const;
    const agl::TextureSampler* getTextureNoise(s32 index) const;

    static bool customShaderCommon(nn::vfx::RenderStateSetArg& rArg);
    static void renderStateReduceBuffer(nn::vfx::RenderStateSetArg& rArg);
    static void customActionEmitterPostCalc(nn::vfx::EmitterPostCalculateArg& rArg);
    static bool customActionEmitterEmit(nn::vfx::EmitterInitializeArg& rArg);
    static void customActionEmitterRemove(nn::vfx::EmitterFinalizeArg& rArg);
    static void customActionPerticleCalc(nn::vfx::ParticleCalculateArgImpl& rArg);
    static bool customActionPerticleEmit(nn::vfx::ParticleCalculateArgImpl& rArg);
    static bool customActionPerticleRemove(nn::vfx::ParticleCalculateArgImpl& rArg);
    static void customActionRipplePerticle(nn::vfx::ParticleCalculateArgImpl& rArg);
    static bool customActionGenerateUv(nn::vfx::ParticleCalculateArgImpl& rArg);
    static bool customFieldCpu(nn::util::neon::Vector3fType* pPos,
                               nn::util::neon::Vector3fType* pVel, f32* pTime, f32* pLife,
                               nn::vfx::Emitter* pEmitter,
                               const nn::vfx::detail::ParticleProperty* pProperty,
                               const nn::vfx::detail::ResFieldCustom* pField, s32 particleIndex);

private:
    UniformBlock* mDitherUbo = nullptr;
    bool mIsDrawReduceBuffer = false;
    EffectEnvParam* mEnvParam;
    EffectLightDirector* mLightDirector = nullptr;
    sead::Vector2f* mLensFlareRect = nullptr;
    sead::Matrix34f mViewMtx;
    agl::TextureSampler mProg0Sampler;
};

static_assert(sizeof(EffectShaderHolderNew) == 0x1060);

}  // namespace al
