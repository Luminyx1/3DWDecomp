#pragma once

#include <container/seadSafeArray.h>
#include <gfx/seadColor.h>
#include <hostio/seadHostIONode.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>
#include "common/aglGPUMemBlock.h"
#include "common/aglIndexStream.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
#include "common/aglVertexAttribute.h"
#include "common/aglVertexBuffer.h"
#include "environment/aglEnvObj.h"
#include "utility/aglContextParameterBuffer.h"
#include "utility/aglDebugTexturePage.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterIO.h"
#include "utility/aglParameterObj.h"

namespace sead {
class Heap;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
class ShaderProgram;
}  // namespace agl

namespace agl::pfx {

// TODO
class DepthOfFieldObj : public env::EnvObj {
public:
    static const env::TypeInfo* sTypeInfo;
};

class DepthOfFieldParameter {
public:
    enum VignettingBlendType {
        cVignettingBlendType_Normal,
        cVignettingBlendType_Add,
        cVignettingBlendType_Mul,
        cVignettingBlendType_Screen,
    };

    class VignettingShapeParam {
    public:
        void initialize(const sead::SafeString& rName, utl::IParameterObj* pObj,
                        sead::Heap* pHeap);
        void genMessage(sead::hostio::Context* pContext);

        bool isDifferent(const VignettingShapeParam& rOther) const
        {
            return *mType != *rOther.mType || *mRange != *rOther.mRange ||
                   *mScale != *rOther.mScale || *mTrans != *rOther.mTrans;
        }

        void copy(const VignettingShapeParam& rOther)
        {
            *mType = *rOther.mType;
            *mRange = *rOther.mRange;
            *mScale = *rOther.mScale;
            *mTrans = *rOther.mTrans;
        }

        utl::Parameter<s32> mType;
        utl::Parameter<sead::Vector2f> mRange;
        utl::Parameter<sead::Vector2f> mScale;
        utl::Parameter<sead::Vector2f> mTrans;
    };
    static_assert(sizeof(VignettingShapeParam) == 0x80);

    DepthOfFieldParameter();

    void initialize(utl::IParameterObj* pObj, sead::Heap* pHeap);

    void setDepthParam(f32 start, f32 end);
    void setDepthNearParam(f32 start, f32 end);
    void setEnableDofNear(bool enable);
    void setEnableDofFar(bool enable);
    void setBlurParam(f32 level);
    void setDepthBlurParam(bool enable, f32 add);
    void setEnableVignettingBlur(bool enable);
    void setVignettingBlur(f32 blur);
    void setEnableVignettingColor(bool enable);
    void setVignettingColor(sead::Color4f color);
    void setVignettingBlendType(VignettingBlendType type);
    void setIndirectEnable(bool enable);
    void setIndirectTextureScale(const sead::Vector2f& rScale);
    void setIndirectTextureRotateRad(f32 rotate);
    void setIndirectTextureTrans(const sead::Vector2f& rTrans);
    void setIndirectTextureSRT(const sead::Vector2f& rScale, f32 rotate,
                               const sead::Vector2f& rTrans);
    void setEnableVignetting2Shape(bool enable);
    void setVignettingShapeParam0(const VignettingShapeParam& rParam);
    void setVignettingShapeParam1(const VignettingShapeParam& rParam);
    void setEnableDofFarCancel(bool enable);
    void setEnableIndirectDepthCancel(bool enable);
    void setColorChangeEnable(bool enable);
    void setColorChangeSaturateMin(f32 min);
    void setColorChangeMulColor(const sead::Color4f& rColor);

    void genMessageDepthOfFieldParameter(sead::hostio::Context* pContext);
    void listenPropertyEventDepthOfFieldParameter(sead::hostio::Node* pNode,
                                                  const sead::hostio::PropertyEvent* pEvent);

protected:
    void assignShaderProgram_();
    void updateIndirectMatrix_();
    bool enableMipFromZeroLevel_() const;
    bool enableDepthBlur_() const;
    bool enableBlurMipMapPass_() const;
    bool enableSeparateVignettingPass_() const;
    bool enableDifferntShape_() const;
    bool enableIndirect_() const;
    bool enableDepthOfField_() const;

    utl::Parameter<bool> mNearEnable;
    utl::Parameter<bool> mFarEnable;
    utl::Parameter<bool> mDepthBlur;
    utl::Parameter<bool> mEnableVignettingColor;
    utl::Parameter<bool> mEnableVignettingBlur;
    utl::Parameter<bool> mEnableVignetting2Shape;
    utl::Parameter<bool> mEnableColorControl;
    utl::Parameter<bool> mEnableColorReverse;
    utl::Parameter<bool> mIndirectEnable;
    utl::Parameter<bool> mIndirectDepthCancelEnable;
    utl::Parameter<bool> mEnableReduceDraw;
    utl::Parameter<bool> mEnableDofFarMax;
    utl::Parameter<bool> mEnableIndirectFromFull;
    utl::Parameter<f32> mStart;
    utl::Parameter<f32> mEnd;
    utl::Parameter<f32> mFarStart;
    utl::Parameter<f32> mFarEnd;
    utl::Parameter<f32> mLevel;
    utl::Parameter<f32> mDepthBlurAdd;
    utl::Parameter<f32> mDofFarMax;
    utl::Parameter<f32> mSaturateMin;
    utl::Parameter<sead::Vector4f> mColorCtrlDepth;
    utl::Parameter<sead::Vector2f> mIndirectTexTrans;
    utl::Parameter<sead::Vector2f> mIndirectTexScale;
    utl::Parameter<f32> mIndirectTexRotate;
    utl::Parameter<f32> mIndirectScale;
    utl::Parameter<f32> mVignettingBlur;
    utl::Parameter<s32> mVignettingBlend;
    utl::Parameter<sead::Color4f> mVignettingColor;
    VignettingShapeParam mVignettingShape0;
    VignettingShapeParam mVignettingShape1;
    utl::Parameter<sead::Color4f> mFarMulColor;
    u8 _4d8[0x10];
    const ShaderProgram* mpMipMapProgram;
    const ShaderProgram* mpDepthMipMapProgram;
    const ShaderProgram* mpDepthMaskPrograms[2];
    sead::UnsafeArray<const ShaderProgram*, 4> mpFinalPrograms;
    const ShaderProgram* mpVignettingProgram = nullptr;
    const ShaderProgram* mpExpandReduceProgram = nullptr;
    f32 mReduceScale = 1.0f;
    const TextureData* mpIndirectTexture = nullptr;
    mutable sead::Vector4f mIndirectParam = sead::Vector4f::zero;
    f32 mIndirectMatrix[2][3];

    friend class DepthOfField;
};
static_assert(sizeof(DepthOfFieldParameter) == 0x570);

class DepthOfField : public utl::ContextParameterBuffer<DepthOfField, DepthOfFieldParameter>,
                     public utl::IParameterIO,
                     public sead::hostio::Node {
public:
    struct Vertex {
        sead::Vector2f mPos;
        sead::Vector2f mParam;
    };
    static_assert(sizeof(Vertex) == 0x10);

    struct Context {
        TextureSampler mColorSampler;
        TextureSampler mDepthSampler;
        RenderBuffer mRenderBuffer;
        RenderTargetColor mColorTarget;
        const TextureData* mpBlurTexture;
        TextureSampler mBlurSampler;
        const TextureData* mpDepthBlurTexture;
        TextureSampler mDepthBlurSampler;
        TextureSampler mComposeSampler;
    };
    static_assert(sizeof(Context) == 0x920);

    struct DrawArg {
        DrawArg(s32 context, Context& rContext, const DepthOfFieldParameter& rParam,
                const RenderBuffer& rRenderBuffer, const TextureData& rDepth, bool isLinearDepth,
                f32 near, f32 far);

        s32 mContext;
        s32 mPass;
        Context* mpContext;
        const DepthOfFieldParameter* mpParam;
        const RenderBuffer* mpRenderBuffer;
        f32 mNear;
        f32 mFar;
        s32 mWidth;
        s32 mHeight;
        bool mIsLinearDepth;
    };
    static_assert(sizeof(DrawArg) == 0x38);

    class TempVignetting : public utl::IParameterObj {
    public:
        TempVignetting(DepthOfField* pOwner, s32 index, const sead::SafeString& rName);

    protected:
        bool preWrite_() const override;
        void postRead_() override;

    private:
        friend class DepthOfField;

        utl::Parameter<s32> mType;
        utl::Parameter<sead::Vector2f> mRange;
        utl::Parameter<sead::Vector2f> mScale;
        utl::Parameter<sead::Vector2f> mTrans;
        DepthOfField* mpOwner;
        s32 mIndex;
    };
    static_assert(sizeof(TempVignetting) == 0xc0);

    DepthOfField();
    ~DepthOfField() override;

    void initialize(s32 contextNum, sead::Heap* pHeap);
    void initializeContext(Context* pContext, sead::Heap* pHeap);
    void allocBuffer(DrawContext* pDrawContext, s32 context,
                     const RenderBuffer& rRenderBuffer) const;
    void allocBuffer(DrawContext* pDrawContext, s32 context, TextureFormat format, s32 width,
                     s32 height) const;
    void freeBuffer(s32 context) const;
    void draw(DrawContext* pDrawContext, s32 context, const RenderBuffer& rRenderBuffer, f32 near,
              f32 far) const;
    void draw(DrawContext* pDrawContext, s32 context, const RenderBuffer& rRenderBuffer,
              const TextureData& rDepth, bool isLinearDepth, f32 near, f32 far) const;
    void setIndirectTextureData(const TextureData* pTexture);
    void postCopyParameter(s32 index, const utl::IParameterObj* pObjA,
                           const utl::IParameterObj* pObjB, f32 t) override;
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

protected:
    void postRead_() override;

private:
    struct Shape {
        GPUMemBlock<Vertex> mVertex;
        GPUMemBlock<u16> mIndex;
        VertexBuffer mVertexBuffer;
        VertexAttribute mVertexAttribute;
        IndexStream mIndexStream;
    };
    static_assert(sizeof(Shape) == 0x3d0);

    void initVertex_(sead::Heap* pHeap);
    void initIndex_(sead::Heap* pHeap);
    void assignShaderProgramAll_();
    void drawColorMipMap_(DrawContext* pDrawContext, const DrawArg& rArg) const;
    void drawDepthMipMap_(DrawContext* pDrawContext, const DrawArg& rArg) const;
    void drawDebugBlur_(DrawContext* pDrawContext, const DrawArg& rArg) const;
    void drawCompose_(DrawContext* pDrawContext, const DrawArg& rArg) const;
    void drawVignetting_(DrawContext* pDrawContext, const DrawArg& rArg) const;
    void bindRenderBuffer_(DrawContext* pDrawContext, RenderBuffer& rRenderBuffer, s32 mipLevel,
                           s32 depthMipOffset) const;
    void drawKick_(DrawContext* pDrawContext, const DrawArg& rArg) const;
    void uniformComposeParam_(DrawContext* pDrawContext, const DrawArg& rArg,
                              const ShaderProgram* pProgram) const;
    void uniformVignettingParam_(DrawContext* pDrawContext, const DrawArg& rArg,
                                 const ShaderProgram* pProgram) const;
    void uniformExpandReduceParam_(DrawContext* pDrawContext, const DrawArg& rArg,
                                   const ShaderProgram* pProgram) const;
    void tempVignettingPostRead_(s32 index, const TempVignetting& rVignetting);

    const TextureData* mpIndirectTexture = nullptr;
    TextureSampler mIndirectSampler;
    s32 mDebugMode = 0;
    f32 mMipBlurScale = 1.0f;
    f32 mComposeBlurScale = 1.0f;
    utl::DebugTexturePage mDebugTexturePage;
    sead::SafeArray<Shape, 2> mShapes;
    bool _1360 = true;
    bool _1361 = true;
    utl::Parameter<bool> mEnable;
    TempVignetting mTempVignetting0{this, 0, "vignetting_shape_0"};
    TempVignetting mTempVignetting1{this, 1, "vignetting_shape_1"};
};
static_assert(sizeof(DepthOfField) == 0x1508);

}  // namespace agl::pfx
