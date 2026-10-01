#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <gfx/seadColor.h>
#include <gfx/seadGraphicsContext.h>
#include <hostio/seadHostIONode.h>
#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>
#include <prim/seadRuntimeTypeInfo.h>
#include <prim/seadSafeString.h>
#include <prim/seadSafeString.hpp>

#include "common/aglGPUMemAddr.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
#include "common/aglUniformBlock.h"
#include "common/aglVertexAttribute.h"
#include "cull/aglViewFrustumCulling.h"
#include "utility/aglDebugTexturePage.h"
#include "utility/aglParameter.h"

namespace sead {
class Camera;
class Heap;
class Projection;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
class ShaderProgram;
}  // namespace agl

namespace agl::lght {

class LightPrePass;

template <typename Context, typename CallbackArg>
class LightPrePassLightMgrBase : public sead::hostio::Node {
    SEAD_RTTI_BASE(LightPrePassLightMgrBase)

public:
    virtual void initialize(LightPrePass* pLightPrePass, s32 lightNum, s32 viewNum,
                            sead::Heap* pHeap) = 0;
    virtual void destroy() = 0;
    virtual void calc() = 0;
    virtual s32 calcView(s32 view, const Context& rContext, s32 bufferIndex) = 0;
    virtual void updateGPU() const = 0;
    virtual void updateViewGPU(s32 view, const Context& rContext) const = 0;
    virtual void draw(DrawContext* pDrawContext, s32 view, const Context& rContext,
                      CallbackArg& rArg) const = 0;
    virtual void drawDebug(DrawContext* pDrawContext, s32 view, const Context& rContext) const = 0;
    virtual void drawDebugTest(DrawContext* pDrawContext, s32 view,
                               const Context& rContext) const = 0;
    virtual s32 getLightType() const = 0;
    virtual sead::SafeString getLabel() const = 0;
    virtual void setValidNum(s32 num) = 0;
    virtual s32 getValidNum() const = 0;
    virtual s32 getLightMax() const = 0;
    virtual s32 getDrawLightNum(s32 view) const = 0;
    virtual void enableLight(s32 index, bool enable) = 0;
    virtual bool isLightEnabled(s32 index) const = 0;
    virtual void setVisible(s32 index, s32 view, bool visible) = 0;
    virtual void setVisibleDirect(s32 index, u32 mask) = 0;
    virtual void setVisibleAll(s32 index, bool visible) = 0;
    virtual bool isVisible(s32 index, s32 view) const = 0;
    virtual void genMessage(sead::hostio::Context* pContext) = 0;
    virtual void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) = 0;
};

template <typename Light, typename Context, typename CallbackArg>
class LightPrePassLightMgr : public LightPrePassLightMgrBase<Context, CallbackArg> {
    using Base = LightPrePassLightMgrBase<Context, CallbackArg>;
    SEAD_RTTI_OVERRIDE(LightPrePassLightMgr, Base)

public:
    void initialize(LightPrePass* pLightPrePass, s32 lightNum, s32 viewNum,
                    sead::Heap* pHeap) override;
    void destroy() override;
    void calc() override;
    s32 calcView(s32 view, const Context& rContext, s32 bufferIndex) override;
    void updateGPU() const override;
    void updateViewGPU(s32 view, const Context& rContext) const override;
    void draw(DrawContext* pDrawContext, s32 view, const Context& rContext,
              CallbackArg& rArg) const override;
    void drawDebug(DrawContext* pDrawContext, s32 view, const Context& rContext) const override;
    void drawDebugTest(DrawContext* pDrawContext, s32 view,
                       const Context& rContext) const override;
    void setValidNum(s32 num) override { mValidNum = num; }
    s32 getValidNum() const override { return mValidNum; }
    s32 getLightMax() const override { return mLights.size(); }
    s32 getDrawLightNum(s32 view) const override { return mDrawLightNum[view].mNum; }
    void enableLight(s32 index, bool enable) override { mLights[index].setEnable(enable); }
    bool isLightEnabled(s32 index) const override { return mLights[index].isEnable(); }
    void setVisible(s32 index, s32 view, bool visible) override
    {
        mLights[index].setVisible(view, visible);
    }
    void setVisibleDirect(s32 index, u32 mask) override { mLights[index].setVisibleMask(mask); }
    void setVisibleAll(s32 index, bool visible) override { mLights[index].setVisibleAll(visible); }
    bool isVisible(s32 index, s32 view) const override { return mLights[index].isVisible(view); }
    void genMessage(sead::hostio::Context* pContext) override;
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) override;

    const Light& getLight(s32 index) const { return mLights[index]; }
    Light& getLight(s32 index) { return mLights[index]; }

protected:
    u32 getActiveNum_() const
    {
        u32 valid = mValidNum;
        u32 size = mLights.size();
        return size < valid ? size : valid;
    }

    virtual void initPrepareImpl_(sead::Heap* pHeap) {}
    virtual void initImpl_(Light& rLight, sead::Heap* pHeap) {}
    virtual void initViewUboImpl_(UniformBlock* pUbo, sead::Heap* pHeap) {}
    virtual void destroyImpl_(Light& rLight) {}
    virtual void destroyFinishImpl_() {}
    virtual void calcPrepareImpl_() {}
    virtual void calcImpl_(Light& rLight) {}
    virtual bool calcViewImpl_(Light& rLight, s32 view, const Context& rContext) { return true; }
    virtual void updateGpuPrepareImpl_() const {}
    virtual void updateGpuImpl_(const Light& rLight) const {}
    virtual void updateViewGpuPrepareImpl_(s32 view, const Context& rContext) const {}
    virtual void updateViewGpuImpl_(const Light& rLight, s32 view, const Context& rContext) const
    {
    }
    virtual void updateUBO_(const Light& rLight, s32 view, const Context& rContext) const {}
    virtual void drawPrepareImpl_(s32 view, const Context& rContext, CallbackArg& rArg) const {}
    virtual void drawImpl_(DrawContext* pDrawContext, const Light& rLight, s32 view,
                           const Context& rContext, CallbackArg& rArg) const
    {
    }
    virtual void drawDebugImpl_(DrawContext* pDrawContext, const Light& rLight, s32 view,
                                const Context& rContext) const
    {
    }
    virtual void drawDebugTestImpl_(DrawContext* pDrawContext, const Light& rLight, s32 view,
                                    const Context& rContext) const
    {
    }
    virtual void genMessagePrepareImpl_(sead::hostio::Context* pContext) {}
    virtual void genMessageImpl_(sead::hostio::Context* pContext, Light& rLight, s32 index) {}
    virtual void listenPropertyEventPrepareImpl_(const sead::hostio::PropertyEvent* pEvent) {}
    virtual void listenPropertyEventImpl_(const sead::hostio::PropertyEvent* pEvent,
                                          Light& rLight, s32 index)
    {
    }

    LightPrePass* mLightPrePass;
    sead::Buffer<Light> mLights;
    s32 mValidNum;
    struct ViewInfo {
        s32 mNum;
    };

    sead::Buffer<ViewInfo> mDrawLightNum;
};

class LightPrePass : public sead::hostio::Node {
public:
    enum LightType {
        cLightType_Point = 0,
        cLightType_Spot = 1,
        cLightType_Proj = 2,
        cLightType_Num = 3,
    };

    enum ShadowType {
        cShadowType_None = 0,
        cShadowType_Normal = 1,
    };

    struct CreateArg {
        s32 mPointLightNum;
        s32 mSpotLightNum;
        s32 mProjLightNum;
        s32 mViewNum;
        s32 mUserLightMgrNum;
    };
    static_assert(sizeof(CreateArg) == 0x14);

    struct LightInfo {
        s32 mType;
        s32 mIsOrtho;
        sead::Vector3f mPos;
        sead::Vector3f mDir;
        sead::Vector3f mUp;
        f32 mParam[8];
    };
    static_assert(sizeof(LightInfo) == 0x4c);

    struct Context {
        Context() : mLightState(0), mBufferState(0) {}

        cull::ViewFrustumCulling mCulling;
        UniformBlock mViewUbo;
        TextureSampler mLightBufferSampler;
        TextureSampler mNormalSampler;
        TextureSampler mDepthSampler;
        TextureData* mpLightBufferTexture;
        RenderBuffer mRenderBuffer;
        RenderTargetColor mColorTarget[2];
        RenderTargetDepth mDepthTarget;
        sead::GraphicsContext mGraphicsContext;
        u32 mLightState;
        u32 mBufferState;
        u8 mCurrentDepthTest;
        u8 mCurrentStencil;
        sead::Matrix34f mViewMtx;
        sead::Matrix44f mViewProjMtx;
        f32 mScreenScaleX;
        f32 mScreenScaleY;
        f32 mDepthScale;
        f32 mFarNearDiff;
        f32 mFarNearDiffInv;
        f32 mProjScale;
        f32 mProjSign;
    };
    static_assert(sizeof(Context) == 0xce8);

    struct CallbackArg {
        const LightPrePass* mLightPrePass;
        const RenderBuffer* mRenderBuffer;
        const Context* mContext;
        const UniformBlock* mViewUbo;
        s32 mView;
        const TextureSampler* mDepthSampler;
        const TextureSampler* mNormalSampler;
        const TextureSampler* mSpecPowSampler;
        DrawContext* mDrawContext;
    };
    static_assert(sizeof(CallbackArg) == 0x48);

    class Callback {
    public:
        virtual ~Callback() = default;
        virtual void invoke(const CallbackArg& rArg) = 0;
    };

    using LightMgrBase = LightPrePassLightMgrBase<Context, CallbackArg>;

    template <typename Light>
    class DrawCallback {
    public:
        virtual bool invoke(CallbackArg& rArg, const Light& rLight) = 0;
    };

    struct PointLightView {
        PointLightView() : mFlags(0) {}

        u32 mFlags;
        UniformBlock mUbo;
    };
    static_assert(sizeof(PointLightView) == 0x80);

    struct ShadowLightView {
        ShadowLightView() : mFlags(0) {}

        u32 mFlags;
        const TextureSampler* mShadowMap;
        TextureSampler* mShadowSampler;
        sead::Matrix44f mShadowMtx;
        UniformBlock mUbo;
    };
    static_assert(sizeof(ShadowLightView) == 0xd0);

    struct PointLight {
        PointLight() : mFlags(0), mVisibleMask(0) {}

        void setEnable(bool enable) { mFlags.change(1, enable); }
        bool isEnable() const { return mFlags.isOn(1); }
        void setVisible(s32 view, bool visible)
        {
            if (visible)
            {
                mVisibleMask |= 1 << view;
            }
            else
            {
                mVisibleMask &= ~(1 << view);
            }
        }
        bool isVisible(s32 view) const { return (mVisibleMask & (1 << view)) != 0; }
        void setVisibleMask(u32 mask) { mVisibleMask = mask; }
        void setVisibleAll(bool visible) { mVisibleMask = visible ? 0xffffffff : 0; }

        sead::BitFlag16 mFlags;
        u32 mVisibleMask;
        u8 _8[8];
        sead::Vector3f mPos;
        f32 mRadius;
        sead::Color4f mColor;
        sead::Color4f mSpecColor;
        f32 mAttnPow;
        f32 mAttnStart;
        sead::Buffer<PointLightView> mView;
    };
    static_assert(sizeof(PointLight) == 0x58);

    struct SpotLight {
        SpotLight() : mFlags(0), mVisibleMask(0) {}

        void setEnable(bool enable) { mFlags.change(1, enable); }
        bool isEnable() const { return mFlags.isOn(1); }
        void setVisible(s32 view, bool visible)
        {
            if (visible)
            {
                mVisibleMask |= 1 << view;
            }
            else
            {
                mVisibleMask &= ~(1 << view);
            }
        }
        bool isVisible(s32 view) const { return (mVisibleMask & (1 << view)) != 0; }
        void setVisibleMask(u32 mask) { mVisibleMask = mask; }
        void setVisibleAll(bool visible) { mVisibleMask = visible ? 0xffffffff : 0; }

        sead::BitFlag8 mFlags;
        u32 mVisibleMask;
        u8 _8[8];
        sead::Vector3f mPos;
        sead::Vector3f mDir;
        sead::Color4f mColor;
        sead::Color4f mSpecColor;
        f32 mAngle;
        f32 mLength;
        f32 mAttnPow;
        f32 mAngleAttnPow;
        ShadowType mShadowType;
        f32 mShadowParam;
        sead::Buffer<ShadowLightView> mView;
    };
    static_assert(sizeof(SpotLight) == 0x70);

    struct ProjLight {
        ProjLight() : mFlags(0), mVisibleMask(0) {}

        void setEnable(bool enable) { mFlags.change(1, enable); }
        bool isEnable() const { return mFlags.isOn(1); }
        void setVisible(s32 view, bool visible)
        {
            if (visible)
            {
                mVisibleMask |= 1 << view;
            }
            else
            {
                mVisibleMask &= ~(1 << view);
            }
        }
        bool isVisible(s32 view) const { return (mVisibleMask & (1 << view)) != 0; }
        void setVisibleMask(u32 mask) { mVisibleMask = mask; }
        void setVisibleAll(bool visible) { mVisibleMask = visible ? 0xffffffff : 0; }

        void setNormVec()
        {
            mNormDir = mDir;
            mNormDir.normalize();
            mNormUp = mUp;
            mNormUp.normalize();
        }

        sead::BitFlag8 mFlags;
        u32 mVisibleMask;
        u8 _8[8];
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
        TextureSampler mTexture;
        sead::Vector2f mTexScale;
        sead::Vector2f mTexOffset;
        ShadowType mShadowType;
        f32 mShadowParam;
        sead::Buffer<ShadowLightView> mView;
        f32 _230;
        sead::Vector3f mDebugPos;
        f32 mDebugParam0;
        f32 mDebugParam1;
        sead::BoundBox3f mBoundBox;
        bool _260;
        u64 _268;
        TextureData mTempTexture;
        TextureSampler mTempTextureSampler;
    };
    static_assert(sizeof(ProjLight) == 0x508);

    class PointLightMgr : public LightPrePassLightMgr<PointLight, Context, CallbackArg> {
        using Base = LightPrePassLightMgr<PointLight, Context, CallbackArg>;
        SEAD_RTTI_OVERRIDE(PointLightMgr, Base)

    public:
        s32 getLightType() const override { return cLightType_Point; }
        sead::SafeString getLabel() const override { return "PointLight"; }

    protected:
        void initPrepareImpl_(sead::Heap* pHeap) override;
        void initImpl_(PointLight& rLight, sead::Heap* pHeap) override;
        void initViewUboImpl_(UniformBlock* pUbo, sead::Heap* pHeap) override;
        void destroyFinishImpl_() override;
        bool calcViewImpl_(PointLight& rLight, s32 view, const Context& rContext) override;
        void updateUBO_(const PointLight& rLight, s32 view, const Context& rContext) const override;
        void drawImpl_(DrawContext* pDrawContext, const PointLight& rLight, s32 view,
                       const Context& rContext, CallbackArg& rArg) const override;
        void drawDebugImpl_(DrawContext* pDrawContext, const PointLight& rLight, s32 view,
                            const Context& rContext) const override;
        void drawDebugTestImpl_(DrawContext* pDrawContext, const PointLight& rLight, s32 view,
                                const Context& rContext) const override;
        void genMessageImpl_(sead::hostio::Context* pContext, PointLight& rLight,
                             s32 index) override;

        VertexAttribute mVertexAttribute;
        DrawCallback<PointLight>* mDrawCallback = nullptr;
        u8 _220[0x20];
    };
    static_assert(sizeof(PointLightMgr) == 0x240);

    class SpotLightMgr : public LightPrePassLightMgr<SpotLight, Context, CallbackArg> {
        using Base = LightPrePassLightMgr<SpotLight, Context, CallbackArg>;
        SEAD_RTTI_OVERRIDE(SpotLightMgr, Base)

    public:
        s32 getLightType() const override { return cLightType_Spot; }
        sead::SafeString getLabel() const override { return "SpotLight"; }

    protected:
        void initPrepareImpl_(sead::Heap* pHeap) override;
        void initImpl_(SpotLight& rLight, sead::Heap* pHeap) override;
        void initViewUboImpl_(UniformBlock* pUbo, sead::Heap* pHeap) override;
        void destroyImpl_(SpotLight& rLight) override;
        void destroyFinishImpl_() override;
        bool calcViewImpl_(SpotLight& rLight, s32 view, const Context& rContext) override;
        void updateUBO_(const SpotLight& rLight, s32 view, const Context& rContext) const override;
        void drawImpl_(DrawContext* pDrawContext, const SpotLight& rLight, s32 view,
                       const Context& rContext, CallbackArg& rArg) const override;
        void drawDebugImpl_(DrawContext* pDrawContext, const SpotLight& rLight, s32 view,
                            const Context& rContext) const override;
        void genMessageImpl_(sead::hostio::Context* pContext, SpotLight& rLight,
                             s32 index) override;

        u8 _38[8];
        VertexAttribute mVertexAttribute;
        DrawCallback<SpotLight>* mDrawCallback = nullptr;
        u8 _228[0x20];
    };
    static_assert(sizeof(SpotLightMgr) == 0x248);

    class ProjLightMgr : public LightPrePassLightMgr<ProjLight, Context, CallbackArg> {
        using Base = LightPrePassLightMgr<ProjLight, Context, CallbackArg>;
        SEAD_RTTI_OVERRIDE(ProjLightMgr, Base)

    public:
        s32 getLightType() const override { return cLightType_Proj; }
        sead::SafeString getLabel() const override { return "ProjLight"; }

        void updateParameters_(ProjLight& rLight);
        bool loadTextureOR(ProjLight* pLight);

    protected:
        void initPrepareImpl_(sead::Heap* pHeap) override;
        void initImpl_(ProjLight& rLight, sead::Heap* pHeap) override;
        void initViewUboImpl_(UniformBlock* pUbo, sead::Heap* pHeap) override;
        void destroyImpl_(ProjLight& rLight) override;
        void destroyFinishImpl_() override;
        bool calcViewImpl_(ProjLight& rLight, s32 view, const Context& rContext) override;
        void updateUBO_(const ProjLight& rLight, s32 view, const Context& rContext) const override;
        void drawImpl_(DrawContext* pDrawContext, const ProjLight& rLight, s32 view,
                       const Context& rContext, CallbackArg& rArg) const override;
        void drawDebugImpl_(DrawContext* pDrawContext, const ProjLight& rLight, s32 view,
                            const Context& rContext) const override;
        void genMessageImpl_(sead::hostio::Context* pContext, ProjLight& rLight,
                             s32 index) override;
        void listenPropertyEventImpl_(const sead::hostio::PropertyEvent* pEvent, ProjLight& rLight,
                                      s32 index) override;

        VertexAttribute mVertexAttribute;
        DrawCallback<ProjLight>* mDrawCallback = nullptr;
        u8 _220[0x20];
    };
    static_assert(sizeof(ProjLightMgr) == 0x240);

    LightPrePass();
    virtual ~LightPrePass();

    void initialize(const CreateArg& rArg, sead::Heap* pHeap);
    void setUserLightMgr(s32 index, LightMgrBase* pMgr);
    void calc();
    void calcContext(s32 view, const sead::Camera& rCamera, const sead::Projection& rProjection);
    void calcContext(s32 view, const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                     f32 near, f32 far, f32 fovy, f32 aspect, const sead::Vector2f& rOffset);
    void updateGPU() const;
    void updateViewGPU(s32 view) const;
    void clearLightBuffer(DrawContext* pDrawContext, s32 view) const;
    void bindLightBuffer(DrawContext* pDrawContext, s32 view, bool bindDepth) const;
    void draw(DrawContext* pDrawContext, s32 view, const TextureData& rDepth,
              const RenderTargetDepth& rDepthTarget, const TextureData& rNormal) const;
    void applyGraphicsContext(DrawContext* pDrawContext, const CallbackArg& rArg, bool stencil,
                              bool force) const;
    void drawDebug(DrawContext* pDrawContext, s32 view) const;
    void release(s32 view) const;

    void setPointLight(s32 index, const sead::Vector3f& rPos, f32 radius,
                       const sead::Color4f& rColor, f32 attnPow, bool useSpecColor,
                       const sead::Color4f& rSpecColor, f32 attnStart);
    PointLight& getPointLightStruct(s32 index);
    const PointLight& getPointLightStruct(s32 index) const;

    void setSpotLight(s32 index, const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                      f32 angle, f32 length, const sead::Color4f& rColor, f32 attnPow,
                      f32 angleAttnPow, bool useSpecColor, const sead::Color4f& rSpecColor);
    void setSpotLightShadowMap(s32 index, s32 view, const TextureSampler* pShadowMap,
                               const sead::Matrix44f& rShadowMtx, bool blackBorder);
    void setSpotLightShadowType(s32 index, ShadowType type, f32 param);
    SpotLight& getSpotLightStruct(s32 index);
    const SpotLight& getSpotLightStruct(s32 index) const;

    void setProjLight(s32 index, const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                      const sead::Vector3f& rUp, const sead::Color4f& rColor, f32 fovy,
                      f32 aspect, f32 near, f32 far, f32 attnPow, bool useSpecColor,
                      const sead::Color4f& rSpecColor, TextureSampler* pTexture,
                      bool texWrap, const sead::Vector2f& rTexScale,
                      const sead::Vector2f& rTexOffset);
    void setProjLight_Ortho(s32 index, const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                            const sead::Vector3f& rUp, const sead::Color4f& rColor, f32 near,
                            f32 far, f32 top, f32 bottom, f32 left, f32 right, f32 attnPow,
                            bool useSpecColor, const sead::Color4f& rSpecColor,
                            TextureSampler* pTexture, bool texWrap,
                            const sead::Vector2f& rTexScale, const sead::Vector2f& rTexOffset);
    void setProjLightShadowMap(s32 index, s32 view, const TextureSampler* pShadowMap,
                               const sead::Matrix44f& rShadowMtx, bool blackBorder);
    void setProjLightShadowType(s32 index, ShadowType type, f32 param);
    ProjLight& getProjLightStruct(s32 index);
    const ProjLight& getProjLightStruct(s32 index) const;

    void getSpotLightInfo(s32 index, LightInfo* pInfo) const;
    void getProjLightInfo(s32 index, LightInfo* pInfo) const;
    void setSpecularCurve(const utl::ParameterCurve<2>& rCurve);

    static const ShaderProgram* getShader(LightType type, bool a, bool b, bool c, bool d,
                                          ShadowType shadowType, bool e, bool f);
    static void GetShader_(const ShaderProgram** ppProgram, LightType type, bool a, bool b,
                           bool c, bool d, ShadowType shadowType, bool e, bool f);
    static void createPointLightUBO(UniformBlock* pUbo, sead::Heap* pHeap);
    static void createSpotLightUBO(UniformBlock* pUbo, sead::Heap* pHeap);
    static void createProjLightUBO(UniformBlock* pUbo, sead::Heap* pHeap);

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    TextureData* createLightBuffer(DrawContext* pDrawContext, s32 view, u32 width, u32 height,
                                   bool useMultiTarget, bool clear) const
    {
        return createLightBuffer_(pDrawContext, view, width, height, useMultiTarget, clear);
    }
    const Context& getContext(s32 view) const { return mContext[view]; }
    Context& getContext(s32 view) { return mContext[view]; }
    u32 getBufferIndex() const { return mBufferIndex; }
    const sead::BitFlag32& getFlags() const { return mFlags; }
    sead::BitFlag32& getFlags() { return mFlags; }
    LightMgrBase* getPointLightMgr() const { return mPointLightMgr; }
    LightMgrBase* getSpotLightMgr() const { return mSpotLightMgr; }
    LightMgrBase* getProjLightMgr() const { return mProjLightMgr; }
    void setSpecPowScale(f32 scale)
    {
        mSpecPowScale = scale;
        updateSpecPowTex_();
    }
    s32 getQuality() const { return mQuality; }

private:
    void changeTextureFilter_();
    void updateSpecPowTex_();
    void calcContext_(s32 view);
    TextureData* createLightBuffer_(DrawContext* pDrawContext, s32 view, u32 width, u32 height,
                                    bool useMultiTarget, bool clear) const;
    bool isLightTypeEnabled_(s32 type) const
    {
        u32 bit = 1 << type;
        return mLightTypeMask & bit;
    }
    void setDirty_() const
    {
        if (mDirtyFlags & 1)
        {
            mDirtyFlags |= 2;
        }
    }

    static void InitializeShaderVariationTable_();

    sead::BitFlag32 mFlags;
    mutable u32 mDirtyFlags;
    u8 mLightTypeMask;
    sead::Buffer<Context> mContext;
    LightMgrBase* mPointLightMgr;
    LightMgrBase* mSpotLightMgr;
    LightMgrBase* mProjLightMgr;
    sead::Buffer<LightMgrBase*> mUserLightMgr;
    f32 mFogScale;
    s32 mQuality;
    sead::GraphicsContext mGraphicsContext[4];
    utl::DebugTexturePage mDebugTexturePage;
    u32 mSpecPowTexWidth;
    GPUMemVoidAddr mSpecPowTexBuffer;
    void* _488;
    TextureData mSpecPowTex;
    TextureSampler mSpecPowSampler;
    utl::ParameterCurve<2> mSpecularCurve;
    f32 mSpecPowScale;
    f32 mSpecPowOffset;
    f32 mSpecIntensityScale;
    f32 mSpecIntensityOffset;
    Callback* mPreDrawCallback;
    Callback* mPostDrawCallback;
    u8 _890[0x40];
    bool _8d0;
    u32 mBufferIndex;
    u32 mTileWidth;
    u32 mTileHeight;
};
static_assert(sizeof(LightPrePass) == 0x8e0);

}  // namespace agl::lght

namespace agl::lght {

template <typename Light, typename Context, typename CallbackArg>
void LightPrePassLightMgr<Light, Context, CallbackArg>::initialize(LightPrePass* pLightPrePass,
                                                                   s32 lightNum, s32 viewNum,
                                                                   sead::Heap* pHeap)
{
    mLightPrePass = pLightPrePass;
    mValidNum = 0;
    initPrepareImpl_(pHeap);
    mLights.tryAllocBuffer(lightNum, pHeap);
    mDrawLightNum.tryAllocBuffer(viewNum, pHeap);

    UniformBlock* pBaseUbo = nullptr;
    for (auto it = mLights.begin(), end = mLights.end(); it != end; ++it)
    {
        it->mView.tryAllocBuffer(viewNum, pHeap);
        setVisibleAll(it.getIndex(), true);
        initImpl_(*it, pHeap);
        for (auto& rView : it->mView)
        {
            if (pBaseUbo)
            {
                rView.mUbo.declare(*pBaseUbo);
            }
            else
            {
                initViewUboImpl_(&rView.mUbo, pHeap);
                pBaseUbo = &rView.mUbo;
            }
            rView.mUbo.create(pHeap, 2, 1);
        }
    }
}

/**
 * Destroys the per-light view uniform blocks and frees the light buffers.
 */
template <typename Light, typename Context, typename CallbackArg>
void LightPrePassLightMgr<Light, Context, CallbackArg>::destroy()
{
    for (auto& rLight : mLights)
    {
        for (auto& rView : rLight.mView)
        {
            rView.mUbo.destroy();
        }
        destroyImpl_(rLight);
        rLight.mView.freeBuffer();
    }
    mDrawLightNum.freeBuffer();
    mLights.freeBuffer();
}

/**
 * Runs the per-light calc hook for every enabled light.
 */
template <typename Light, typename Context, typename CallbackArg>
void LightPrePassLightMgr<Light, Context, CallbackArg>::calc()
{
    if (mValidNum == 0)
    {
        return;
    }

    calcPrepareImpl_();
    u32 num = getActiveNum_();
    Light* pLights = mLights.getBufferPtr();
    for (s32 i = 0; i != num; i++)
    {
        if (isLightEnabled(i))
        {
            calcImpl_(pLights[i]);
        }
    }
}

template <typename Light, typename Context, typename CallbackArg>
s32 LightPrePassLightMgr<Light, Context, CallbackArg>::calcView(s32 view, const Context& rContext,
                                                                s32 bufferIndex)
{
    s32& rDrawNum = mDrawLightNum[view].mNum;
    rDrawNum = 0;
    if (mValidNum == 0)
    {
        return 0;
    }

    u32 num = getActiveNum_();
    if (num == 0)
    {
        return 0;
    }

    auto* pLights = mLights.getBufferPtr();
    for (s32 i = 0; i != num; i++)
    {
        if (!isLightEnabled(i))
        {
            continue;
        }

        bool inside = calcViewImpl_(pLights[i], view, rContext);
        auto& rView = mLights[i].mView[view];
        rView.mFlags = inside ? rView.mFlags & ~1u : rView.mFlags | 1;

        if (inside)
        {
            pLights[i].mView[view].mUbo.setCurrentBufferIndex(bufferIndex);
            rDrawNum++;
        }
    }
    return rDrawNum;
}

/**
 * Runs the per-light GPU update hook for every enabled light.
 */
template <typename Light, typename Context, typename CallbackArg>
void LightPrePassLightMgr<Light, Context, CallbackArg>::updateGPU() const
{
    if (mValidNum == 0)
    {
        return;
    }

    updateGpuPrepareImpl_();
    u32 num = getActiveNum_();
    auto* pLights = mLights.getBufferPtr();
    for (s32 i = 0; i != num; i++)
    {
        if (isLightEnabled(i))
        {
            updateGpuImpl_(pLights[i]);
        }
    }
}

template <typename Light, typename Context, typename CallbackArg>
void LightPrePassLightMgr<Light, Context, CallbackArg>::updateViewGPU(s32 view,
                                                                     const Context& rContext) const
{
    if (mValidNum == 0 || mDrawLightNum[view].mNum == 0)
    {
        return;
    }

    updateViewGpuPrepareImpl_(view, rContext);
    u32 num = getActiveNum_();
    auto* pLights = mLights.getBufferPtr();
    for (s32 i = 0; i != num; i++)
    {
        if (!isLightEnabled(i) || !isVisible(i, view) || (mLights[i].mView[view].mFlags & 1))
        {
            continue;
        }
        updateViewGpuImpl_(pLights[i], view, rContext);
        updateUBO_(pLights[i], view, rContext);
        pLights[i].mView[view].mUbo.flushCurrentBuffer();
    }
}

/**
 * Draws every enabled, visible light that passed culling in a view.
 * @param pDrawContext draw context
 * @param view view index
 * @param rContext light pre-pass context of the view
 * @param rArg callback argument
 */
template <typename Light, typename Context, typename CallbackArg>
void LightPrePassLightMgr<Light, Context, CallbackArg>::draw(DrawContext* pDrawContext, s32 view,
                                                            const Context& rContext,
                                                            CallbackArg& rArg) const
{
    if (mValidNum == 0 || mDrawLightNum[view].mNum == 0)
    {
        return;
    }

    drawPrepareImpl_(view, rContext, rArg);
    u32 num = getActiveNum_();
    auto* pLights = mLights.getBufferPtr();
    for (s32 i = 0; i != num; i++)
    {
        if (!isLightEnabled(i) || !isVisible(i, view) || (mLights[i].mView[view].mFlags & 1))
        {
            continue;
        }
        drawImpl_(pDrawContext, pLights[i], view, rContext, rArg);
    }
}

/**
 * Draws the debug shapes of every enabled, visible light in a view.
 * @param pDrawContext draw context
 * @param view view index
 * @param rContext light pre-pass context of the view
 */
template <typename Light, typename Context, typename CallbackArg>
void LightPrePassLightMgr<Light, Context, CallbackArg>::drawDebug(DrawContext* pDrawContext,
                                                                 s32 view,
                                                                 const Context& rContext) const
{
    u32 num = getActiveNum_();
    auto* pLights = mLights.getBufferPtr();
    for (s32 i = 0; i != num; i++)
    {
        if (!isLightEnabled(i) || !isVisible(i, view) || (mLights[i].mView[view].mFlags & 1))
        {
            continue;
        }
        drawDebugImpl_(pDrawContext, pLights[i], view, rContext);
    }
}

/**
 * Draws the debug test shapes of every enabled, visible light in a view.
 * @param pDrawContext draw context
 * @param view view index
 * @param rContext light pre-pass context of the view
 */
template <typename Light, typename Context, typename CallbackArg>
void LightPrePassLightMgr<Light, Context, CallbackArg>::drawDebugTest(DrawContext* pDrawContext,
                                                                     s32 view,
                                                                     const Context& rContext) const
{
    u32 num = getActiveNum_();
    auto* pLights = mLights.getBufferPtr();
    for (s32 i = 0; i != num; i++)
    {
        if (!isLightEnabled(i) || !isVisible(i, view) || (mLights[i].mView[view].mFlags & 1))
        {
            continue;
        }
        drawDebugTestImpl_(pDrawContext, pLights[i], view, rContext);
    }
}

/**
 * Generates the host IO messages of every active light.
 * @param pContext host IO context
 */
template <typename Light, typename Context, typename CallbackArg>
void LightPrePassLightMgr<Light, Context, CallbackArg>::genMessage(sead::hostio::Context* pContext)
{
    {
        sead::FormatFixedSafeString<256> meta("Min=0, Max=%d", mLights.size() - 1);
    }
    genMessagePrepareImpl_(pContext);
    u32 num = getActiveNum_();
    auto* pLights = mLights.getBufferPtr();
    for (s32 i = 0; i != num; i++)
    {
        sead::FormatFixedSafeString<256> header("GroupHeader = %d", i);
        genMessageImpl_(pContext, pLights[i], i);
    }
}

/**
 * Forwards a property event to every active light.
 * @param pEvent property event
 */
template <typename Light, typename Context, typename CallbackArg>
void LightPrePassLightMgr<Light, Context, CallbackArg>::listenPropertyEvent(
    const sead::hostio::PropertyEvent* pEvent)
{
    listenPropertyEventPrepareImpl_(pEvent);
    u32 num = getActiveNum_();
    auto* pLights = mLights.getBufferPtr();
    for (s32 i = 0; i != num; i++)
    {
        listenPropertyEventImpl_(pEvent, pLights[i], i);
    }
}

}  // namespace agl::lght
