#pragma once

#include <container/seadBuffer.h>
#include <hostio/seadHostIONode.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>

#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureEnum.h"
#include "common/aglTextureSampler.h"
#include "common/aglUniformBlock.h"
#include "utility/aglDebugTexturePage.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterIO.h"
#include "utility/aglParameterObj.h"

namespace sead
{
class Heap;
namespace hostio
{
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl
{

class DrawContext;
class ShaderProgram;
class TextureData;

namespace cull
{
class ViewFrustumCulling;
}

namespace sdw
{

class ShadowPrePass : public sead::hostio::Node, public utl::IParameterIO
{
public:
    struct Context
    {
        UniformBlock mUniformBlock;
        TextureSampler mDepthSampler;
        TextureSampler mShadowSampler;
        TextureSampler mStaticShadowSampler;
        TextureSampler mLightBufferSampler;
        const TextureData* mLightBuffer;
        RenderBuffer mRenderBuffer;
        RenderTargetColor mRenderTarget;
        TextureSampler mTempSampler;
        const TextureData* mTempBuffer;
        RenderBuffer mTempRenderBuffer;
        RenderTargetColor mTempRenderTarget;
        RenderTargetDepth mTempRenderTargetDepth;
        TextureSampler mHalfSamplers[2];
        const TextureData* mHalfBuffers[2];
        RenderBuffer mHalfRenderBuffers[2];
        RenderTargetColor mHalfRenderTargets[2];
        RenderBuffer mMipRenderBuffer;
        RenderTargetColor mMipRenderTarget;
        RenderTargetDepth mMipRenderTargetDepth;
        TextureSampler mMipSampler;
        bool mIsCreated;
        bool mIsReleased;
        u32 mBufferState;
        f32 mInvWidth;
        f32 mInvHeight;
        const TextureData* mStaticDepth;
        const TextureData* _1880;
        u8 _1888[8];
        f32 mResolutionScale;
    };
    static_assert(sizeof(Context) == 0x1898);

    ShadowPrePass();
    ~ShadowPrePass() override;

    void initialize(s32 contextNum, sead::Heap* pHeap);
    void calc();
    void calcGPU() const;
    void calcGPU(s32 index, const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                 const sead::Matrix44f* pShadowMtx, f32* pNearFar, f32 param0, f32 param1,
                 f32 param2, f32 param3, const cull::ViewFrustumCulling& rCulling) const;
    void calcGPU_StaticDepthShadow(s32 index, const sead::Matrix44f* pProjMtx,
                                   const sead::Matrix34f& rViewMtx);
    void release(s32 index) const;
    void clearShadowBuffer(DrawContext* pDrawContext, s32 index) const;
    s32 getPassType() const;
    const TextureSampler* getPrevSampler(DrawContext* pDrawContext, s32 index, s32 pass) const;
    RenderBuffer* getRenderTarget(DrawContext* pDrawContext, s32 index, s32 pass) const;
    RenderBuffer* bindBuffer(DrawContext* pDrawContext, s32 index, s32 pass) const;
    u32 getBufferState(s32 index) const;
    RenderBuffer* createShadowBuffer(DrawContext* pDrawContext, s32 index, u32 width,
                                     u32 height) const;
    s32 getPcfShaderNo() const;
    s32 getShaderIndex(const ShaderProgram& rProgram, u32 value0, u32 value1, u32 value2,
                       u32 value3, u32 value4) const;

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    bool isEnable() const { return *mIsEnable; }

private:
    void getTextureFormat_(s32 index, TextureFormat& rFormat, TextureFormat& rTempFormat) const;
    void createShadowBuffer_core_(DrawContext* pDrawContext, s32 index, u32 width, u32 height,
                                  bool force) const;
    void createTempBuffer_(DrawContext* pDrawContext, s32 index, u32 width, u32 height) const;

    mutable sead::Buffer<Context> mContexts;
    sead::BitFlag32 mFlags = 0;
    utl::DebugTexturePage mDebugTexturePage;
    utl::ParameterObj mParamObj;
    utl::Parameter<bool> mIsEnable;
    utl::Parameter<s32> mResolutionMode;
    utl::Parameter<s32> mScreenSpaceBlurType;
    utl::Parameter<s32> mScreenSpaceBlurWidth;
    utl::Parameter<s32> mScreenSpaceBlurRepNum;
    utl::Parameter<f32> mPcfWidth;
    utl::Parameter<bool> mUseStaticDepthShadow;
    utl::Parameter<bool> mUseDecalAo;
    utl::Parameter<bool> mUseDecalTrailSigned;
    utl::Parameter<bool> mUseFarFade;
    utl::Parameter<f32> mDynamicShadowFarFadeStart;
    utl::Parameter<f32> mDynamicShadowFarFadeEnd;
    utl::Parameter<f32> mStaticShadowFarFadeStart;
    utl::Parameter<f32> mStaticShadowFarFadeEnd;
    utl::Parameter<bool> mUseDepth2Normal;
    utl::Parameter<bool> mUseDepth2NormalBlur;
    utl::Parameter<f32> mNormal2ShadowRatio;
    utl::Parameter<f32> mNormal2ShadowMul;
    utl::Parameter<f32> mFaceNormalBias;
    utl::Parameter<bool> mUseMipLevelBlur;
    utl::Parameter<bool> mUseMipLevelBlurReduce;
    utl::Parameter<s32> mMipBlurWidth;
    utl::Parameter<s32> mMipBlurRepNum;
    utl::Parameter<bool> mUsePreCombSsao;
    utl::Parameter<bool> mUseFarDepthTest;
    utl::Parameter<f32> mFarDepthTestDist;
    utl::Parameter<s32> mPcfShaderType;
    utl::Parameter<s32> mPcfSampleNum;
    s32 _830;
    sead::Vector4f _834;
    u8 mBufferIndex;
};
static_assert(sizeof(ShadowPrePass) == 0x848);

}  // namespace sdw
}  // namespace agl
