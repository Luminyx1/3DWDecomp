#pragma once

#include <container/seadTList.h>
#include <hostio/seadHostIONode.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureEnum.h"
#include "common/aglTextureSampler.h"
#include "utility/aglDynamicTextureCache.h"
#include "utility/aglImageFilter2D.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterList.h"
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
class TextureData;

namespace pfx {
class ColorCorrection;
}  // namespace pfx
}  // namespace agl

namespace agl::utl {

class MultiFilter;

struct MultiFilterResultInfo {
    TextureFormat mFormat;
    s32 mWidth;
    s32 mHeight;
    s32 mMaxWidth;
    s32 mMaxHeight;
};
static_assert(sizeof(MultiFilterResultInfo) == 0x14);

struct MultiFilterDrawContext {
    const TextureData* mSrcTexture;
    TextureData* mResultTexture;
    TextureFormat mFormat;
    TextureSampler mSampler;
    RenderBuffer mRenderBuffer;
    RenderTargetColor mRenderTarget;
    u32 mAlphaCompSel;
    bool mIsChangeCompSel;
    DynamicTextureCache mTextureCache;
};
static_assert(sizeof(MultiFilterDrawContext) == 0x3a0);

class MultiFilterUnit : public sead::TListNode<MultiFilterUnit*>,
                        public sead::hostio::Node,
                        public IParameterList {
public:
    enum FilterType {
        cFilterType_Reduce = 0,
        cFilterType_Expand = 1,
        cFilterType_Blur = 2,
        cFilterType_ColorCorrection = 3,
        cFilterType_ChangeFormat = 4,
        cFilterType_ColorDrift = 5,
        cFilterType_Trimming = 6,
        cFilterType_Num = 7,
    };

    enum BlurType {
        cBlurType_0 = 0,
    };

    enum FilterScale {
        cFilterScale_Half = 1,
        cFilterScale_Quarter = 2,
    };

    MultiFilterUnit(FilterType type, s32 index);
    ~MultiFilterUnit() override;

    static MultiFilterUnit* create(FilterType type, s32 index, sead::Heap* pHeap);
    static sead::SafeString getFilterName(FilterType type);
    static sead::SafeString getFilterLabel(FilterType type);

    void initialize(MultiFilter* pOwner, sead::TList<MultiFilterUnit*>* pFreeList,
                    sead::Heap* pHeap);
    void destroy();
    void activate();
    void inactivate();
    void resetParameters();

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    FilterType getType() const { return mType; }
    u32 getId() const { return mId; }
    bool isEnable() const { return *mEnable; }
    bool isActive() const { return *mActive; }
    s32 getSaveIndex() const { return *mSaveIndex; }
    void setSaveIndex(s32 index) { *mSaveIndex = index; }

protected:
    friend class MultiFilter;

    virtual void doInitialize_(sead::Heap* pHeap);
    virtual void doDestroy_();
    virtual void doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const = 0;
    virtual void doCalcResultInfo_(MultiFilterResultInfo* pInfo) const;
    virtual void doResetParameters_() = 0;
    virtual void doGenMessage_(sead::hostio::Context* pContext) = 0;
    virtual void doListenPropertyEvent_(const sead::hostio::PropertyEvent* pEvent);

    IParameterObj mParamObj;
    MultiFilter* mOwner = nullptr;
    sead::TList<MultiFilterUnit*>* mFreeList = nullptr;
    FilterType mType;
    u32 mId;
    Parameter<bool> mEnable{false, "enable", "有効", &mParamObj};
    Parameter<bool> mActive{false, "active", "active", &mParamObj};
    Parameter<s32> mSaveIndex{-1, "save_index", "save_index", &mParamObj};
};
static_assert(sizeof(MultiFilterUnit) == 0x168);

class ReduceFilter : public MultiFilterUnit {
public:
    explicit ReduceFilter(s32 index);
    ~ReduceFilter() override;

protected:
    void doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const override;
    void doCalcResultInfo_(MultiFilterResultInfo* pInfo) const override;
    void doResetParameters_() override;
    void doGenMessage_(sead::hostio::Context* pContext) override;

private:
    void drawReduce_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext,
                     FilterScale scale) const;

    Parameter<f32> mOffsetAdjust{0.0f, "offset_adjust", "フェッチ位置調整", &mParamObj};
    Parameter<s32> mScale{2, "reduce_scale", "縮小スケール", &mParamObj};
};
static_assert(sizeof(ReduceFilter) == 0x1a8);

class ExpandFilter : public MultiFilterUnit {
public:
    explicit ExpandFilter(s32 index);
    ~ExpandFilter() override;

protected:
    void doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const override;
    void doCalcResultInfo_(MultiFilterResultInfo* pInfo) const override;
    void doResetParameters_() override;
    void doGenMessage_(sead::hostio::Context* pContext) override;

private:
    void drawExpand_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext,
                     FilterScale scale) const;

    Parameter<f32> mOffsetAdjust{0.0f, "offset_adjust", "フェッチ位置調整", &mParamObj};
    Parameter<s32> mScale{2, "reduce_scale", "拡大スケール", &mParamObj};
};
static_assert(sizeof(ExpandFilter) == 0x1a8);

class BlurFilter : public MultiFilterUnit {
public:
    explicit BlurFilter(s32 index);
    ~BlurFilter() override;

protected:
    void doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const override;
    void doResetParameters_() override;
    void doGenMessage_(sead::hostio::Context* pContext) override;

private:
    void drawBlur_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext,
                   BlurType blurType, ImageFilter2D::GaussianKernel kernel) const;

    Parameter<s32> mBlurType{4, "blur_type", "ブラータイプ", &mParamObj};
    Parameter<s32> mBlurNum{1, "blur_num", "ブラー反復回数", &mParamObj};
    Parameter<s32> mGaussianKernel{9, "gaussian_kernel", "ガウシアンカーネル", &mParamObj};
};
static_assert(sizeof(BlurFilter) == 0x1c8);

class ColorCorrectionFilter : public MultiFilterUnit {
public:
    explicit ColorCorrectionFilter(s32 index);
    ~ColorCorrectionFilter() override;

protected:
    void doInitialize_(sead::Heap* pHeap) override;
    void doDestroy_() override;
    void doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const override;
    void doResetParameters_() override;
    void doGenMessage_(sead::hostio::Context* pContext) override;
    void doListenPropertyEvent_(const sead::hostio::PropertyEvent* pEvent) override;

private:
    pfx::ColorCorrection* mColorCorrection = nullptr;
};
static_assert(sizeof(ColorCorrectionFilter) == 0x170);

class ChangeFormat : public MultiFilterUnit {
public:
    explicit ChangeFormat(s32 index);
    ~ChangeFormat() override;

protected:
    void doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const override;
    void doCalcResultInfo_(MultiFilterResultInfo* pInfo) const override;
    void doResetParameters_() override;
    void doGenMessage_(sead::hostio::Context* pContext) override;

private:
    Parameter<s32> mFormat{4, "format", "フォーマット", &mParamObj};
    Parameter<s32> mCompSelR{0, "comp_sel_r", "R", &mParamObj};
    Parameter<s32> mCompSelG{1, "comp_sel_g", "G", &mParamObj};
    Parameter<s32> mCompSelB{2, "comp_sel_b", "B", &mParamObj};
    Parameter<s32> mCompSelA{5, "comp_sel_a", "A", &mParamObj};
};
static_assert(sizeof(ChangeFormat) == 0x208);

class ColorDrift : public MultiFilterUnit {
public:
    explicit ColorDrift(s32 index);
    ~ColorDrift() override;

protected:
    void doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const override;
    void doResetParameters_() override;
    void doGenMessage_(sead::hostio::Context* pContext) override;

private:
    Parameter<sead::Vector2f> mDriftR{sead::Vector2f::zero, "drift_r", "ずれR", &mParamObj};
    Parameter<sead::Vector2f> mDriftG{sead::Vector2f::zero, "drift_g", "ずれG", &mParamObj};
    Parameter<sead::Vector2f> mDriftB{sead::Vector2f::zero, "drift_b", "ずれB", &mParamObj};
};
static_assert(sizeof(ColorDrift) == 0x1c8);

class Trimming : public MultiFilterUnit {
public:
    explicit Trimming(s32 index);
    ~Trimming() override;

protected:
    void doDraw_(DrawContext* pDrawContext, MultiFilterDrawContext* pContext) const override;
    void doCalcResultInfo_(MultiFilterResultInfo* pInfo) const override;
    void doResetParameters_() override;
    void doGenMessage_(sead::hostio::Context* pContext) override;

private:
    friend class MultiFilter;

    Parameter<sead::Vector2f> mCenter{sead::Vector2f::zero, "trim_center", "中心位置",
                                      &mParamObj};
    Parameter<sead::Vector2f> mScale{{0.5f, 0.5f}, "trim_scale", "スケール", &mParamObj};
};
static_assert(sizeof(Trimming) == 0x1a8);

}  // namespace agl::utl
