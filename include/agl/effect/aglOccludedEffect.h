#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>
#include <prim/seadRuntimeTypeInfo.h>
#include <prim/seadSafeString.h>

#include "common/aglUniformBlock.h"
#include "common/aglVertexAttribute.h"
#include "cull/aglViewFrustumCulling.h"
#include "effect/aglOcclusionRenderer.h"
#include "environment/aglEnvObj.h"
#include "utility/aglParameter.h"

namespace sead {
class Heap;
class Viewport;
class XmlElement;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
class RenderBuffer;
class RenderTargetDepth;
}  // namespace agl

namespace agl::fx {

class OccludedEffectMgr;

class OfxBase : public env::EnvObj {
    SEAD_RTTI_BASE(OfxBase)

    friend class OccludedEffectMgr;

public:
    struct CreateArg {
        s32 mViewNum;
    };

    struct UpdateViewGPUArg {
        sead::Vector2f mScreenPos;
        f32 mEdgeRateX;
        f32 mEdgeRateY;
        f32 mAngle;
        sead::Matrix34f mWorldMtx;
        sead::Matrix34f mWorldViewMtx;
        sead::Matrix44f mWorldViewProjMtx;
        bool mIsVisible;
    };
    static_assert(sizeof(UpdateViewGPUArg) == 0xb8);

    struct Context : UpdateViewGPUArg {
        cull::ViewFrustumCulling mViewFrustumCulling;
    };
    static_assert(sizeof(Context) == 0x2f0);

    class PresetBase : public env::EnvObj {
        SEAD_RTTI_BASE(PresetBase)

    public:
        struct CreateArg {
            s32 mViewNum;
        };

        PresetBase();
        ~PresetBase() override = default;

        void initialize(s32 viewNum, sead::Heap* pHeap) override {}

        void initializeOfx(const CreateArg& rArg, OccludedEffectMgr* pMgr, s32 index, s32 unused,
                           sead::Heap* pHeap);
        s32 getOfxTypeID() const;
        void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);
        void copy(const PresetBase& rOther);
        void genMessage(sead::hostio::Context* pContext);
        void writeToXML(s32 index, sead::XmlElement* pElement, sead::Heap* pHeap) const;

        const sead::SafeString& getPresetName() const { return *mPresetName; }

    protected:
        virtual void initializeOfxImpl_(const CreateArg& rArg, sead::Heap* pHeap) = 0;

    public:
        utl::Parameter<sead::Vector3f> mPosition;
        utl::Parameter<f32> mRadius;
        utl::Parameter<f32> mCoreRadius;
        utl::Parameter<f32> mVerticesBias;
        utl::Parameter<f32> mDepthOffset;
        utl::Parameter<sead::Vector2f> mScrEdgeSize;
        utl::Parameter<sead::Vector3f> mDirection;
        utl::Parameter<bool> mPseudoOccl;
        utl::Parameter<sead::FixedSafeString<32>> mPresetName;
        utl::Parameter<bool> mIsFixPosX;
        utl::Parameter<bool> mIsFixPosY;
        utl::Parameter<bool> mIsFixPosZ;
        OccludedEffectMgr* mMgr = nullptr;
        bool mIsSyncInstance = true;
        s32 mCopySrcIndex = -1;
        bool mIsDebugDraw = false;
        s32 mDebugColorType = 0;
    };
    static_assert(sizeof(PresetBase) == 0x2e8);

    OfxBase();
    ~OfxBase() override;

    void initialize(s32 viewNum, sead::Heap* pHeap) override;

    void initializeOfx(const CreateArg& rArg, s32 ofxType, const PresetBase* pPreset,
                       sead::Heap* pHeap, OccludedEffectMgr* pMgr);
    s32 getOfxTypeID() const;
    void calc();
    bool loadPresetByName(const sead::SafeString& rName, bool force);
    bool loadPresetByIndex(s32 index, bool force);
    void calcContext(s32 viewIndex, const sead::Matrix34f& rViewMtx,
                     const sead::Matrix44f& rProjMtx, f32 near, f32 far, f32 fovy, f32 aspect,
                     const sead::Vector2f& rOffset);
    void updateGPU();
    void updateViewGPU(s32 viewIndex, const RenderBuffer& rRenderBuffer);
    void draw(DrawContext* pDrawContext, s32 viewIndex, const RenderBuffer& rRenderBuffer,
              const sead::Viewport& rViewport, const RenderTargetDepth& rDepth) const;
    void setEnable(bool enable);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);
    void genMessage(sead::hostio::Context* pContext);
    void genMessageSimple(sead::hostio::Context* pContext);
    void listenPropertyEventSimple(const sead::hostio::PropertyEvent* pEvent);

    virtual void drawDebugOfx(DrawContext* pDrawContext, s32 viewIndex) const;
    virtual f32 getOcclusionRate(s32 viewIndex) const;
    virtual const sead::SafeString& getOfxName() const { return mOfxName; }
    virtual const sead::SafeString& getOfxLabel() const { return mOfxLabel; }
    virtual void initializePreset(PresetBase* pPreset, const PresetBase::CreateArg& rArg,
                                  s32 index, sead::Heap* pHeap);

protected:
    virtual void initializeImpl_(const CreateArg& rArg, sead::Heap* pHeap) = 0;
    virtual void updateImpl_() = 0;
    virtual void calcContextImpl_(s32 viewIndex, const Context& rContext,
                                  const UpdateViewGPUArg& rArg) = 0;
    virtual void updateGPUImpl_() = 0;
    virtual void updateViewGPUImpl_(s32 viewIndex, const Context& rContext,
                                    const UpdateViewGPUArg& rArg) = 0;
    virtual void drawImpl_(DrawContext* pDrawContext, s32 viewIndex,
                           const RenderBuffer& rRenderBuffer, const sead::Viewport& rViewport,
                           const Context& rContext,
                           const OcclusionRenderer& rOcclusionRenderer) const = 0;
    virtual void drawDebugImpl_(DrawContext* pDrawContext, s32 viewIndex,
                                const Context& rContext) const = 0;
    virtual void genMessageImpl_(sead::hostio::Context* pContext) {}
    virtual void listenPropertyEventImpl_(const sead::hostio::PropertyEvent* pEvent) {}

    template <typename T>
    T* getPreset_() const
    {
        return sead::DynamicCast<T>(mOwnPreset);
    }

    sead::Buffer<Context> mContext;
    const PresetBase* mPreset = nullptr;
    PresetBase* mOwnPreset = nullptr;
    sead::FixedSafeString<32> mPresetName;
    s32 mPresetIndex = -1;
    OcclusionRenderer mOcclusionRenderer;
    sead::Color4f mDebugColor0;
    sead::Color4f mDebugColor1;
    sead::Color4f mDebugColor2;
    OccludedEffectMgr* mMgr = nullptr;
    mutable sead::BitFlag16 mFlag;
    sead::FixedSafeString<32> mOfxName{""};
    sead::FixedSafeString<64> mOfxLabel;
};
static_assert(sizeof(OfxBase) == 0xd60);

class OfxLensFlare : public OfxBase {
    SEAD_RTTI_OVERRIDE(OfxLensFlare, OfxBase)

public:
    struct ContextLensFlare {
        struct Instance {
            f32 mIntensity;
            sead::Vector2f mScale;
            sead::Vector2f mOffset;
            sead::Color4f mColor;
            sead::Vector2f mHalfSize;
            sead::Vector2f mPos;
            sead::Vector2f mDir;
            sead::Vector2f mTexMtx[2];
            sead::Vector2f mTexScale;
        };
        static_assert(sizeof(Instance) == 0x54);

        explicit ContextLensFlare(s32 index);

        sead::Buffer<UniformBlock> mUniformBlock;
        sead::Buffer<Instance> mInstance;
        s32 mIndex;
    };
    static_assert(sizeof(ContextLensFlare) == 0x28);

    struct Element {
        explicit Element(s32 index);

        const TextureSampler* mSampler;
        const TextureSampler* mSampler2;
        s32 mIndex;
    };
    static_assert(sizeof(Element) == 0x18);

    class Preset : public PresetBase {
        SEAD_RTTI_OVERRIDE(Preset, PresetBase)

    public:
        class PresetElement {
        public:
            PresetElement(s32 index, utl::IParameterObj* pObj);

            void copy(const PresetElement& rOther);

            utl::Parameter<s32> mTextureIdx;
            utl::Parameter<s32> mTexture2Idx;
            utl::Parameter<f32> mPosition;
            utl::Parameter<f32> mRotate;
            utl::Parameter<f32> mIntensity;
            utl::Parameter<bool> mIsSizeZoom;
            utl::Parameter<sead::Vector2f> mSize;
            utl::Parameter<sead::Color4f> mColor;
            utl::Parameter<s32> mBlendMode;
            utl::Parameter<bool> mIsEnableDraw;
            utl::Parameter<bool> mIsEnableRotate;
            utl::Parameter<bool> mIsEnableRotateInv;
            utl::Parameter<bool> mIsEnableOccludedScaling;
            utl::Parameter<bool> mIsEnableOccludedDec;
            utl::Parameter<bool> mIsEnableOccludedAlpha;
            utl::Parameter<bool> mIsEnableRotatePos;
            utl::Parameter<f32> mRotatePosRate;
            utl::Parameter<bool> mIsEnableEdgeScaling;
            utl::Parameter<sead::Vector2f> mEdgeScaleRate;
            utl::Parameter<f32> mCenterPosScalingRate;
            utl::Parameter<f32> mCenterPosScalingPow;
            utl::Parameter<f32> mCenterPosAlphaRate;
            utl::Parameter<f32> mCenterPosAlphaPow;
            utl::Parameter<bool> mIsEnableOctagon;
            utl::Parameter<bool> mIsEnableAngleOcclusion;
            utl::Parameter<f32> mAngleCenter;
            utl::Parameter<f32> mAngleWidth;
            utl::Parameter<f32> mAnglePower;
            s32 mIndex;
            s32 mOrder = 0;
        };
        static_assert(sizeof(PresetElement) == 0x390);

        class PresetContext {
        public:
            PresetContext(s32 index, utl::IParameterObj* pObj);

            utl::Parameter<sead::Vector2f> mCenterPos;
            utl::Parameter<s32> mBaseAxis;
            s32 mIndex;
        };
        static_assert(sizeof(PresetContext) == 0x48);

        Preset();
        ~Preset() override;

        s32 getTypeID() const override { return sTypeInfo->id; }
        virtual s32 getMaxElementNum() const { return 20; }

        void listenPropertyEventImpl_(const sead::hostio::PropertyEvent* pEvent);

        PresetElement* searchElement_(s32 index) const
        {
            for (s32 i = 0; i < mElement.size(); i++)
            {
                if (mElement.unsafeAt(i)->mIndex == index)
                {
                    return mElement.unsafeAt(i);
                }
            }
            return nullptr;
        }

        void genMessageImpl_(sead::hostio::Context* pContext);

    protected:
        void initializeOfxImpl_(const CreateArg& rArg, sead::Heap* pHeap) override;

    public:
        sead::PtrArray<PresetElement> mElement;
        sead::PtrArray<PresetContext> mPresetContext;
        utl::Parameter<f32> mSizeBaseScale;
        utl::Parameter<s32> mCoreOcclusionType;
        utl::Parameter<s32> mScrEdgeType;
        utl::Parameter<f32> mScrEdgePow;
        utl::Parameter<f32> mScrEdgeFlash;

        static const env::TypeInfo* sTypeInfo;
    };
    static_assert(sizeof(Preset) == 0x3a8);

    OfxLensFlare();
    ~OfxLensFlare() override;

    s32 getTypeID() const override { return sTypeInfo->id; }
    f32 getOcclusionRate(s32 viewIndex) const override;
    virtual s32 getMaxElementNum() const { return 20; }

protected:
    void initializeImpl_(const CreateArg& rArg, sead::Heap* pHeap) override;
    void updateImpl_() override;
    void calcContextImpl_(s32 viewIndex, const Context& rContext,
                          const UpdateViewGPUArg& rArg) override;
    void updateGPUImpl_() override;
    void updateViewGPUImpl_(s32 viewIndex, const Context& rContext,
                            const UpdateViewGPUArg& rArg) override;
    void drawImpl_(DrawContext* pDrawContext, s32 viewIndex, const RenderBuffer& rRenderBuffer,
                   const sead::Viewport& rViewport, const Context& rContext,
                   const OcclusionRenderer& rOcclusionRenderer) const override;
    void drawDebugImpl_(DrawContext* pDrawContext, s32 viewIndex,
                        const Context& rContext) const override
    {
    }
    void genMessageImpl_(sead::hostio::Context* pContext) override;
    void listenPropertyEventImpl_(const sead::hostio::PropertyEvent* pEvent) override;

public:
    sead::PtrArray<ContextLensFlare> mContextLensFlare;
    sead::PtrArray<Element> mElement;
    s32 mBufferIndex = 0;
    sead::Color4f mColor{1.0f, 1.0f, 1.0f, 1.0f};
    VertexAttribute mVertexAttrQuad;
    VertexAttribute mVertexAttrQuadDouble;
    VertexAttribute mVertexAttrOctagon;
    VertexAttribute mVertexAttrOctagonDouble;

    static const env::TypeInfo* sTypeInfo;
};
static_assert(sizeof(OfxLensFlare) == 0x1518);

class OfxLensFlareDynamic : public OfxLensFlare {
    SEAD_RTTI_OVERRIDE(OfxLensFlareDynamic, OfxLensFlare)

public:
    class Preset : public OfxLensFlare::Preset {
        SEAD_RTTI_OVERRIDE(Preset, OfxLensFlare::Preset)

    public:
        s32 getTypeID() const override { return sTypeInfo->id; }

        static const env::TypeInfo* sTypeInfo;
    };
    static_assert(sizeof(Preset) == 0x3a8);

    OfxLensFlareDynamic();
    ~OfxLensFlareDynamic() override;

    s32 getTypeID() const override { return sTypeInfo->id; }

    void setParam(s32 presetIndex, f32 scale, const sead::Vector3f& rPos,
                  const sead::Vector2f& rSize, const sead::Color4f& rColor, bool enable);
    bool loadPresetByParentIndex(s32 index);

    s32 mParentIndex = -1;

    static const env::TypeInfo* sTypeInfo;
};
static_assert(sizeof(OfxLensFlareDynamic) == 0x1520);

}  // namespace agl::fx
