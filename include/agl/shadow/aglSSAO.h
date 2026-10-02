#pragma once

#include <container/seadBuffer.h>
#include <hostio/seadHostIONode.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "common/aglGPUMemAddr.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureData.h"
#include "common/aglTextureEnum.h"
#include "common/aglTextureSampler.h"
#include "utility/aglDebugTexturePage.h"
#include "utility/aglDevTools.h"
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

namespace sdw
{

class SSAO : public utl::IParameterIO, public sead::hostio::Node
{
public:
    class Context
    {
    public:
        Context();
        ~Context();

        void allocTexture(DrawContext* pDrawContext, TextureFormat format, s32 width, s32 height,
                          s32 mipLevelNum);

        const TextureData* mAOBuffer = nullptr;
        const TextureData* mSSAOTexture = nullptr;
        RenderBuffer mRenderBuffer;
        RenderTargetColor mRenderTarget;
        const TextureData* mReduceDepth = nullptr;
        TextureSampler mAOSampler;
        const TextureData* mBlurBuffer = nullptr;
        RenderBuffer mBlurRenderBuffer;
        RenderTargetColor mBlurRenderTarget;
        TextureSampler mBlurSampler;
        const TextureData* _6c0 = nullptr;
        TextureSampler mSSAOSampler;
        const TextureData* _838 = nullptr;
        TextureSampler mDepthSampler;
        const TextureData* _9b0 = nullptr;
    };
    static_assert(sizeof(Context) == 0x9b8);

    class Parameter : public utl::IParameterObj
    {
    public:
        explicit Parameter(SSAO* pSSAO) : mSSAO(pSSAO) {}

    protected:
        void postRead_() override { mSSAO->postRead_(); }

    private:
        SSAO* mSSAO;
    };

    static const f32 cSSAODefaultBaseRadius;
    static const f32 cSSAODefaultDistanceIntensityZero;
    static const f32 cSSAODefaultDensity;
    static const f32 cSSAODefaultVariableDistMin;
    static const s32 cSSAODefaultSamplePairNum;
    static const f32 cSSAODefaultDepthOffset;
    static const f32 cAlchemyAODefaultRadius;
    static const f32 cAlchemyAODefaultMaxRadius;
    static const f32 cAlchemyAODefaultBias;
    static const f32 cAlchemyAODefaultDetectionIntensity;
    static const f32 cAlchemyAODefaultDensity;
    static const s32 cAlchemyAODefaultSamplePairNum;

    SSAO();
    ~SSAO() override;

    void initialize(s32 contextNum, sead::Heap* pHeap);
    void drawToAOBuffer(DrawContext* pDrawContext, s32 index, s32 width, s32 height,
                        const TextureData& rDepth, const sead::Matrix34f& rViewMtx,
                        const sead::Matrix44f& rProjMtx) const;
    void drawToAOBuffer(DrawContext* pDrawContext, s32 index, s32 width, s32 height,
                        const TextureData& rDepth, f32 near, f32 far, f32 fovy, f32 aspect) const;
    void release(s32 index);

    void genMessage(sead::hostio::Context* pContext);
    void genMessageParameter(sead::hostio::Context* pContext, sead::hostio::Node* pNode);
    void genMessageDebugParameter(sead::hostio::Context* pContext, sead::hostio::Node* pNode);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);
    void listenPropertyEventParameter(const sead::hostio::PropertyEvent* pEvent);
    void listenPropertyEventDebugParameter(const sead::hostio::PropertyEvent* pEvent);

    bool isEnable() const { return *mIsEnable; }
    const TextureSampler& getAOSampler() const { return mContexts.front().mAOSampler; }

    void set_53c(f32 value) { _53c = value; }
    void setAOFar(f32 far) { *mAOFar = far; }

    void setSSAOParameter(f32 radius, f32 distAttn, f32 density, s32 samplePairNum,
                          f32 depthOffset)
    {
        *mRadius = radius;
        *mDistAttn = distAttn;
        *mDensity = density;
        *mSamplePairNum = samplePairNum;
        *mDepthOffset = depthOffset;
    }

protected:
    void postRead_() override;

private:
    void initRotateTexture_(bool force);
    void initSphereVolume_(s32 sampleNum, bool force);
    void drawToAOBuffer_(DrawContext* pDrawContext, s32 index, s32 width, s32 height,
                         const TextureData& rDepth, f32 near, f32 far, f32 fovy, f32 aspect) const;
    void drawReduceDepth_(DrawContext* pDrawContext, s32 index, const TextureData& rDepth) const;
    void drawSSAO_(DrawContext* pDrawContext, s32 index, s32 width, s32 height, f32 near, f32 far,
                   f32 fovy, bool reduce) const;
    void drawAlchemyAO_(DrawContext* pDrawContext, s32 index, s32 width, s32 height, f32 near,
                        f32 far, f32 fovy, f32 aspect, bool reduce) const;
    void drawBlur_(DrawContext* pDrawContext, s32 index, s32 width, s32 height) const;

    sead::Buffer<Context> mContexts;
    f32 mCenterWeight = 0.0f;
    sead::Vector4f mSphereVolume[9];
    f32 mSphereHeight[9];
    utl::DebugTexturePage mDebugTexturePage;
    s32 _530 = 0;
    s32 _534 = 3;
    bool _538 = false;
    f32 _53c = 1.0f;
    TextureData mRotateTexture;
    TextureSampler mRotateSampler;
    GPUMemVoidAddr mRotateTextureBuffer;
    Parameter mParameter;
    utl::Parameter<bool> mIsEnable;
    utl::Parameter<s32> mSSAOType;
    utl::Parameter<f32> mAOFar;
    utl::Parameter<f32> mRadius;
    utl::Parameter<f32> mDistAttn;
    utl::Parameter<f32> mDensity;
    utl::Parameter<f32> mVariableDistMin;
    utl::Parameter<f32> mDepthOffset;
    utl::Parameter<s32> mSamplePairNum;
    utl::Parameter<f32> mAlchemyRadius;
    utl::Parameter<f32> mAlchemyMaxRadius;
    utl::Parameter<f32> mAlchemyBias;
    utl::Parameter<f32> mAlchemyDetectionIntensity;
    utl::Parameter<f32> mAlchemyDensity;
    utl::Parameter<s32> mAlchemySamplePairNum;
    utl::Parameter<s32> mBlurNum;
    utl::Parameter<s32> mMipBlurNum;
};
static_assert(sizeof(SSAO) == 0xa48);

}  // namespace sdw
}  // namespace agl
