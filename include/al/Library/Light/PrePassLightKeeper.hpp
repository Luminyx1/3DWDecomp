#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadTList.h>
#include <gfx/seadColor.h>
#include <gfx/seadGraphicsContext.h>
#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>
#include <prim/seadSafeString.h>

#include "common/aglShaderEnum.h"
#include "common/aglShaderLocation.h"
#include "common/aglTextureSampler.h"
#include "common/aglUniformBlock.h"
#include "common/aglVertexAttribute.h"
#include "lighting/aglLightPrePass.h"

namespace agl {
class DrawContext;
class RenderTargetDepth;
class ShaderProgram;
}  // namespace agl

namespace al {
class GBufferArray;
class GraphicsSystemInfo;
class LiveActor;
class PrePassLightBase;
class PrePassLightKeeper;
class ShaderHolder;
class ShadowDirector;

/**
 * @brief Per-view state of a light pre-pass light (uniform block and shadow map).
 */
struct LppLightView {
    LppLightView() : mFlags(0) {}

    u32 mFlags;
    agl::UniformBlock mUbo;
    const agl::TextureSampler* mShadowMap;
    agl::TextureSampler* mShadowSampler;
    sead::Matrix44f mShadowMtx;
};

static_assert(sizeof(LppLightView) == 0xd0);

/**
 * @brief Light data stored by the albedo mode light managers: the light parameters plus the
 * enable/visibility state and per-view data required by agl::lght::LightPrePassLightMgr.
 */
template <typename Param>
struct LppLightDataBase : public Param {
    using ViewBuffer = sead::Buffer<LppLightView>;

    LppLightDataBase() : mFlags(0), mVisibleMask(0) {}

    void setEnable(bool isEnable) { mFlags.change(1, isEnable); }

    bool isEnable() const { return mFlags.isOn(1); }

    void setVisible(s32 view, bool isVisible) {
        if (isVisible) {
            mVisibleMask |= 1 << view;
        } else {
            mVisibleMask &= ~(1 << view);
        }
    }

    bool isVisible(s32 view) const { return (mVisibleMask & (1 << view)) != 0; }

    void setVisibleMask(u32 mask) { mVisibleMask = mask; }

    void setVisibleAll(bool isVisible) { mVisibleMask = isVisible ? 0xffffffff : 0; }

    void clearShadowMap() {
        for (auto& rView : mView) {
            rView.mShadowMap = nullptr;
        }
    }

    sead::BitFlag8 mFlags;
    u32 mVisibleMask;
    ViewBuffer mView;
};

/**
 * @brief Parameters of a point light.
 */
struct PointLightParam {
    sead::Vector3f mPos;
    f32 mAttnPow;
    f32 mRadius;
    f32 mAttnStart;
    sead::Color4f mColor;
    sead::Color4f mSpecColor;
};

static_assert(sizeof(PointLightParam) == 0x38);

/**
 * @brief Parameters of a spot light.
 */
struct SpotLightParam {
    void calcAuxiliary();

    sead::Vector3f mPos;
    sead::Vector3f mDir;
    f32 mLength;
    f32 mAngle;
    f32 mAngleAttnStart;
    f32 mAttnPow;
    f32 mAngleAttnPow;
    sead::Color4f mColor;
    sead::Color4f mSpecColor;
    f32 mCosAngle;
    agl::lght::LightPrePass::ShadowType mShadowType;
    f32 mShadowParam;
};

static_assert(sizeof(SpotLightParam) == 0x58);

/**
 * @brief Parameters of a line light.
 */
struct LineLightParam {
    sead::Vector3f mStart;
    sead::Vector3f mEnd;
    f32 mRadius;
    f32 mAttnPow;
    sead::Color4f mColor;
};

static_assert(sizeof(LineLightParam) == 0x30);

/**
 * @brief Parameters of a projection light.
 */
struct ProjLightParam {
    void setNormVec() {
        mNormDir = mDir;
        mNormDir.normalize();
        mNormUp = mUp;
        mNormUp.normalize();
    }

    sead::Vector3f mPos;
    sead::Vector3f mDir;
    sead::Vector3f mUp;
    sead::Color4f mColor;
    sead::Color4f mSpecColor;
    sead::Vector3f mNormDir;
    sead::Vector3f mNormUp;
    f32 mParam[8];
    f32 mAttnPow;
    bool mHasTexture;
    agl::TextureSampler mTexture;
    sead::Vector2f mTexScale;
    sead::Vector2f mTexOffset;
    agl::lght::LightPrePass::ShadowType mShadowType;
    f32 mShadowParam;
    f32 mTanHalfFovy;
    sead::Vector3f mDebugPos;
    f32 mDebugParam0;
    f32 mDebugParam1;
    sead::BoundBox3f mBoundBox;
};

static_assert(sizeof(ProjLightParam) == 0x240);

/**
 * @brief Shader program and uniform block locations used by an albedo mode light manager.
 */
struct LppShaderInfo {
    explicit LppShaderInfo(const char* pViewName)
        : mContextLocation("Context"), mViewLocation(pViewName), mLightEnvLocation("LightEnv") {}

    void setShader(const agl::ShaderProgram* pProgram) {
        mProgram = pProgram;
        mContextLocation.search(*mProgram);
        mViewLocation.search(*mProgram);
        mLightEnvLocation.search(*mProgram);
    }

    const agl::ShaderProgram* mProgram = nullptr;
    agl::UniformBlockLocation mContextLocation;
    agl::UniformBlockLocation mViewLocation;
    agl::UniformBlockLocation mLightEnvLocation;
};

static_assert(sizeof(LppShaderInfo) == 0x50);

using LppContext = agl::lght::LightPrePass::Context;
using LppCallbackArg = agl::lght::LightPrePass::CallbackArg;
using PointLightData = LppLightDataBase<PointLightParam>;
using SpotLightData = LppLightDataBase<SpotLightParam>;
using LineLightData = LppLightDataBase<LineLightParam>;
using ProjLightData = LppLightDataBase<ProjLightParam>;
using PointLightMgrBase = agl::lght::LightPrePassLightMgr<PointLightData, LppContext, LppCallbackArg>;
using SpotLightMgrBase = agl::lght::LightPrePassLightMgr<SpotLightData, LppContext, LppCallbackArg>;
using LineLightMgrBase = agl::lght::LightPrePassLightMgr<LineLightData, LppContext, LppCallbackArg>;
using ProjLightMgrBase = agl::lght::LightPrePassLightMgr<ProjLightData, LppContext, LppCallbackArg>;

/**
 * @brief Point light manager drawing into the albedo mode light buffer.
 */
class AlbedoModePointLightMgr : public PointLightMgrBase {
    SEAD_RTTI_OVERRIDE(AlbedoModePointLightMgr, PointLightMgrBase)

public:
    AlbedoModePointLightMgr(PrePassLightKeeper* pKeeper);

    void setShader(const agl::ShaderProgram* pProgram);

    s32 getLightType() const override { return 0; }

    sead::SafeString getLabel() const override { return "AlbedoModePointLight"; }

protected:
    void initPrepareImpl_(sead::Heap* pHeap) override;
    void initImpl_(PointLightData& rLight, sead::Heap* pHeap) override;
    void initViewUboImpl_(agl::UniformBlock* pUbo, sead::Heap* pHeap) override;
    bool calcViewImpl_(PointLightData& rLight, s32 view, const LppContext& rContext) override;
    void updateUBO_(const PointLightData& rLight, s32 view,
                    const LppContext& rContext) const override;
    void drawImpl_(agl::DrawContext* pDrawContext, const PointLightData& rLight, s32 view,
                   const LppContext& rContext, LppCallbackArg& rArg) const override;
    void drawDebugImpl_(agl::DrawContext* pDrawContext, const PointLightData& rLight, s32 view,
                        const LppContext& rContext) const override;

private:
    PrePassLightKeeper* mKeeper;
    LppShaderInfo* mShaderInfo;
};

static_assert(sizeof(AlbedoModePointLightMgr) == 0x48);

/**
 * @brief Spot light manager drawing into the albedo mode light buffer.
 */
class AlbedoModeSpotLightMgr : public SpotLightMgrBase {
    SEAD_RTTI_OVERRIDE(AlbedoModeSpotLightMgr, SpotLightMgrBase)

public:
    AlbedoModeSpotLightMgr(PrePassLightKeeper* pKeeper);

    void setShader(ShaderHolder* pHolder);
    void setSpotLightShadowMap(s32 index, s32 view, const agl::TextureSampler* pShadowMap,
                               const sead::Matrix44f& rShadowMtx,
                               agl::lght::LightPrePass::ShadowType shadowType, f32 shadowParam,
                               bool isBlackBorder);

    s32 getLightType() const override { return 0; }

    sead::SafeString getLabel() const override { return "AlbedoModeSpotLight"; }

protected:
    void initPrepareImpl_(sead::Heap* pHeap) override;
    void initImpl_(SpotLightData& rLight, sead::Heap* pHeap) override;
    void initViewUboImpl_(agl::UniformBlock* pUbo, sead::Heap* pHeap) override;
    void destroyImpl_(SpotLightData& rLight) override;
    bool calcViewImpl_(SpotLightData& rLight, s32 view, const LppContext& rContext) override;
    void updateUBO_(const SpotLightData& rLight, s32 view,
                    const LppContext& rContext) const override;
    void drawImpl_(agl::DrawContext* pDrawContext, const SpotLightData& rLight, s32 view,
                   const LppContext& rContext, LppCallbackArg& rArg) const override;
    void drawDebugImpl_(agl::DrawContext* pDrawContext, const SpotLightData& rLight, s32 view,
                        const LppContext& rContext) const override;

private:
    PrePassLightKeeper* mKeeper;
    LppShaderInfo* mShaderInfo;
};

static_assert(sizeof(AlbedoModeSpotLightMgr) == 0x48);

/**
 * @brief Line light manager drawing into the albedo mode light buffer.
 */
class AlbedoModeLineLightMgr : public LineLightMgrBase {
    SEAD_RTTI_OVERRIDE(AlbedoModeLineLightMgr, LineLightMgrBase)

public:
    AlbedoModeLineLightMgr(PrePassLightKeeper* pKeeper);

    void setShader(const agl::ShaderProgram* pProgram);

    s32 getLightType() const override { return 0; }

    sead::SafeString getLabel() const override { return "AlbedoModeLineLight"; }

protected:
    void initPrepareImpl_(sead::Heap* pHeap) override;
    void initImpl_(LineLightData& rLight, sead::Heap* pHeap) override;
    void initViewUboImpl_(agl::UniformBlock* pUbo, sead::Heap* pHeap) override;
    bool calcViewImpl_(LineLightData& rLight, s32 view, const LppContext& rContext) override;
    void updateUBO_(const LineLightData& rLight, s32 view,
                    const LppContext& rContext) const override;
    void drawImpl_(agl::DrawContext* pDrawContext, const LineLightData& rLight, s32 view,
                   const LppContext& rContext, LppCallbackArg& rArg) const override;
    void drawDebugImpl_(agl::DrawContext* pDrawContext, const LineLightData& rLight, s32 view,
                        const LppContext& rContext) const override;

private:
    PrePassLightKeeper* mKeeper;
    LppShaderInfo* mShaderInfo;
};

static_assert(sizeof(AlbedoModeLineLightMgr) == 0x48);

/**
 * @brief Projection light manager drawing into the albedo mode light buffer.
 */
class AlbedoModeProjLightMgr : public ProjLightMgrBase {
    SEAD_RTTI_OVERRIDE(AlbedoModeProjLightMgr, ProjLightMgrBase)

public:
    AlbedoModeProjLightMgr(PrePassLightKeeper* pKeeper);

    void setShader(const agl::ShaderProgram* pProgram);
    void setProjLightShadowMap(s32 index, s32 view, const agl::TextureSampler* pShadowMap,
                               const sead::Matrix44f& rShadowMtx,
                               agl::lght::LightPrePass::ShadowType shadowType, f32 shadowParam,
                               bool isBlackBorder);
    void updateParameters_(ProjLightData& rLight);

    s32 getLightType() const override { return 0; }

    sead::SafeString getLabel() const override { return "AlbedoModeProjLight"; }

protected:
    void initPrepareImpl_(sead::Heap* pHeap) override;
    void initImpl_(ProjLightData& rLight, sead::Heap* pHeap) override;
    void initViewUboImpl_(agl::UniformBlock* pUbo, sead::Heap* pHeap) override;
    void destroyImpl_(ProjLightData& rLight) override;
    bool calcViewImpl_(ProjLightData& rLight, s32 view, const LppContext& rContext) override;
    void updateUBO_(const ProjLightData& rLight, s32 view,
                    const LppContext& rContext) const override;
    void drawImpl_(agl::DrawContext* pDrawContext, const ProjLightData& rLight, s32 view,
                   const LppContext& rContext, LppCallbackArg& rArg) const override;
    void drawDebugImpl_(agl::DrawContext* pDrawContext, const ProjLightData& rLight, s32 view,
                        const LppContext& rContext) const override;

private:
    PrePassLightKeeper* mKeeper;
    LppShaderInfo* mShaderInfo;
};

static_assert(sizeof(AlbedoModeProjLightMgr) == 0x48);

/**
 * @brief Owns agl's light pre-pass and the albedo mode light managers; collects the light requests
 * of every PrePassLightBase each frame.
 */
class PrePassLightKeeper {
public:
    using LightList = sead::TList<PrePassLightBase*>;

    PrePassLightKeeper(GraphicsSystemInfo* pInfo, s32 viewNum);
    ~PrePassLightKeeper();

    f32 getSpecularPower() const;
    f32 getFlesnel() const;
    void initShader(ShaderHolder* pHolder);
    void endInit();
    void clear();
    void pushBackLight(PrePassLightBase* pLight);
    void eraseLight(PrePassLightBase* pLight);
    void setSpotLightShadowMap(s32 index, s32 view, const agl::TextureSampler* pShadowMap,
                               const sead::Matrix44f& rShadowMtx,
                               agl::lght::LightPrePass::ShadowType shadowType, f32 shadowParam,
                               bool isBlackBorder);
    void setProjLightShadowMap(s32 index, s32 view, const agl::TextureSampler* pShadowMap,
                               const sead::Matrix44f& rShadowMtx,
                               agl::lght::LightPrePass::ShadowType shadowType, f32 shadowParam,
                               bool isBlackBorder);
    void execute(ShadowDirector* pDirector, bool isPaused);
    void preDrawGraphics();
    void updateViewGPU(s32 view, ShadowDirector* pDirector);
    void requestPointLight(const sead::Vector3f& rPos, f32 radius, const sead::Color4f& rColor,
                           f32 attnPow, f32 attnStart, bool isEnableSpecular,
                           bool isUseSpecularColor, const sead::Color4f& rSpecularColor,
                           s32 lightShaderFunc);
    s32 requestSpotLight(const sead::Vector3f& rPos, const sead::Vector3f& rDir, f32 angle,
                         f32 length, const sead::Color4f& rColor, f32 attnPow, f32 angleAttnPow,
                         f32 angleAttnStart, bool isEnableSpecular, bool isUseSpecularColor,
                         const sead::Color4f& rSpecularColor);
    s32 requestProjLight(const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                         const sead::Vector3f& rUp, const sead::Color4f& rColor, f32 near, f32 far,
                         f32 fovy, f32 aspect, f32 attnPow, bool isEnableSpecular,
                         bool isUseSpecularColor, const sead::Color4f& rSpecularColor,
                         agl::TextureSampler* pTexture, bool isTextureWrap,
                         const sead::Vector2f& rTexScale, const sead::Vector2f& rTexOffset);
    s32 requestProjLightOrtho(const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                              const sead::Vector3f& rUp, const sead::Color4f& rColor, f32 near,
                              f32 far, f32 top, f32 bottom, f32 left, f32 right, f32 attnPow,
                              bool isEnableSpecular, bool isUseSpecularColor,
                              const sead::Color4f& rSpecularColor, agl::TextureSampler* pTexture,
                              bool isTextureWrap, const sead::Vector2f& rTexScale,
                              const sead::Vector2f& rTexOffset);
    void requestLineLight(const sead::Vector3f& rStart, const sead::Vector3f& rEnd, f32 radius,
                          const sead::Color4f& rColor, f32 attnPow, bool isEnableSpecular);
    agl::ShaderMode drawLpp(s32 view, const GBufferArray* pGBuffer,
                            const agl::RenderTargetDepth& rDepthTarget,
                            agl::ShaderMode shaderMode) const;

    bool isPaused() const { return mIsPaused; }

    agl::lght::LightPrePass* getLightPrePass() const { return mLightPrePass; }

    const sead::GraphicsContext& getGraphicsContext() const { return mGraphicsContext; }

    const agl::VertexAttribute& getQuadAttribute() const { return mQuadAttribute; }

    const agl::VertexAttribute& getSphereAttribute() const { return mSphereAttribute; }

    const agl::VertexAttribute& getConeAttribute() const { return mConeAttribute; }

    const agl::VertexAttribute& getCylinderAttribute() const { return mCylinderAttribute; }

    const agl::VertexAttribute& getCubeAttribute() const { return mCubeAttribute; }

    GraphicsSystemInfo* mGraphicsSystemInfo;
    bool mIsPaused;
    s32 mViewNum;
    s32 mPointLightNum;
    s32 mSpotLightNum;
    s32 mProjLightNum;
    s32 mLineLightNum;
    agl::lght::LightPrePass* mLightPrePass = nullptr;
    s32 mPointLightRequestNum = 0;
    s32 mSpotLightRequestNum = 0;
    s32 mProjLightRequestNum = 0;
    s32 mLineLightRequestNum = 0;
    f32 mSpecularPowerScale = 120.0f;
    sead::GraphicsContext mGraphicsContext;
    agl::VertexAttribute mQuadAttribute;
    agl::VertexAttribute mSphereAttribute;
    agl::VertexAttribute mConeAttribute;
    agl::VertexAttribute mCylinderAttribute;
    agl::VertexAttribute mCubeAttribute;
    u8 _a10[0xa90 - 0xa10];
    LightList mLightList;
    AlbedoModePointLightMgr* mPointLightMgr;
    AlbedoModeSpotLightMgr* mSpotLightMgr;
    AlbedoModeLineLightMgr* mLineLightMgr;
    AlbedoModeProjLightMgr* mProjLightMgr;
};

static_assert(sizeof(PrePassLightKeeper) == 0xac8);

}  // namespace al

namespace LightPrePassFunction {
void declareUsingPointLight(const al::LiveActor* pActor, s32 num);
void declareUsingSpotLight(const al::LiveActor* pActor, s32 num);
void requestPointLight(const al::LiveActor* pActor, const sead::Vector3f& rPos, f32 radius,
                       const sead::Color4f& rColor, f32 attnPow, f32 attnStart,
                       bool isEnableSpecular, bool isUseSpecularColor,
                       const sead::Color4f& rSpecularColor, s32 lightShaderFunc);
void requestSpotLight(const al::LiveActor* pActor, const sead::Vector3f& rPos,
                      const sead::Vector3f& rDir, f32 angle, f32 length,
                      const sead::Color4f& rColor, f32 attnPow, f32 angleAttnPow,
                      f32 angleAttnStart, bool isEnableSpecular, bool isUseSpecularColor,
                      const sead::Color4f& rSpecularColor);
}  // namespace LightPrePassFunction
