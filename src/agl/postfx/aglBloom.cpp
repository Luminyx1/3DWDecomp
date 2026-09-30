#include "postfx/aglBloom.h"

#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <prim/seadSafeString.h>
#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "detail/aglRootNode.h"
#include "detail/aglShaderHolder.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglDevTools.h"
#include "utility/aglPrimitiveTexture.h"
#include "utility/aglResParameter.h"

namespace agl::pfx {

namespace {

bool isEventTarget(const sead::hostio::PropertyEvent* pEvent, const void* pStart,
                   const void* pEnd)
{
    uintptr_t id = pEvent->getIdValue();
    return id >= reinterpret_cast<uintptr_t>(pStart) && id < reinterpret_cast<uintptr_t>(pEnd);
}

inline s32 getMipHeight(const TextureData& rTexture, s32 mipLevel)
{
    s32 min = rTexture.getMinHeight_();
    s32 height = rTexture.getHeight() >> mipLevel;
    return min > height ? min : height;
}

}  // namespace

BloomParameter::BloomParameter() = default;

void BloomParameter::initialize(utl::IParameterObj* pObj, sead::Heap* pHeap)
{
    mEditType.init(0, "edit_type", "Edit Type", pObj);
    mAspect.init(1.0f, "aspect", "Aspect", pObj);
    mThresholdBalance.init(sead::Color4f::cWhite, "threshhold_balance", "Threshold Balance",
                           pObj);
    mFinalBlend.init(2, "finalblend", "Final Blend", pObj);
    mEnableDepthClamp.init(false, "enable_depth_clamp", "Enable Depth Clamp", pObj);
    mEnableLuminanceOffset.init(false, "enable_luminance_offset", "Enable Luminance Offset",
                                pObj);
    mEnableClampedLuminance.init(true, "enable_clamped_luminance", "Enable Clamped Luminance",
                                 pObj);
    mLuminanceIntensity.init(1.0f, "luminance_intensity", "Luminance Intensity", pObj);
    mClampedLuminance.init(2.0f, "clamped_luminance", "Clamped Luminance", pObj);
    mExType.init(0, "ex_type", "Ex Type", pObj);
    mExIteration.init(5, "ex_iteration", "Ex Iteration", pObj);
    mMain.mThreshold.init(0.75f, sead::FormatFixedSafeString<128>("%sthreshhold", ""),
                          "Threshold", pObj);
    mMain.mThresholdRange.init(0.1f, sead::FormatFixedSafeString<128>("%sthreshold_range", ""),
                               "Threshold Range", pObj);
    mMain.mIntensity.init(1.0f, sead::FormatFixedSafeString<128>("%sintensity", ""),
                          "Intensity", pObj);
    mMain.mFinalGather.init(sead::Color4f::cWhite,
                            sead::FormatFixedSafeString<128>("%sfinalgather", ""),
                            "Final Gather", pObj);
    mMain.mExpand.init(1.0f, sead::FormatFixedSafeString<128>("%sexpand", ""), "Expand", pObj);
    mShaft.mThreshold.init(0.75f, sead::FormatFixedSafeString<128>("%sthreshhold", "shaft_"),
                          "Threshold", pObj);
    mShaft.mThresholdRange.init(0.1f, sead::FormatFixedSafeString<128>("%sthreshold_range", "shaft_"),
                               "Threshold Range", pObj);
    mShaft.mIntensity.init(1.0f, sead::FormatFixedSafeString<128>("%sintensity", "shaft_"),
                          "Intensity", pObj);
    mShaft.mFinalGather.init(sead::Color4f::cWhite,
                            sead::FormatFixedSafeString<128>("%sfinalgather", "shaft_"),
                            "Final Gather", pObj);
    mShaft.mExpand.init(1.0f, sead::FormatFixedSafeString<128>("%sexpand", "shaft_"), "Expand", pObj);
    for (s32 i = 0; i < cColorNum; i++)
    {
        mColors[i].init(sead::Color4f::cWhite, sead::FormatFixedSafeString<128>("color%d", i + 1),
                        "Color", pObj);
    }

    mColors[cColorNum - 1]->a = 0.0f;
    {
        Depth& rDepth = mDepths[cDepth_Gain];
        rDepth.mEnable.init(false, sead::FormatFixedSafeString<128>("enable_%s", "depth_gain"), "Enable",
                            pObj);
        rDepth.mStart.init(utl::DevTools::calcScale(0.0f),
                           sead::FormatFixedSafeString<128>("%s_start", "depth_gain"), "Start", pObj);
        rDepth.mEnd.init(utl::DevTools::calcScale(100.0f),
                         sead::FormatFixedSafeString<128>("%s_end", "depth_gain"), "End", pObj);
        rDepth.mValue.init(1.0f, sead::FormatFixedSafeString<128>("%s_value", "depth_gain"), "End Scale", pObj);
        rDepth.mValueStart.init(1.0f, sead::FormatFixedSafeString<128>("%s_value_start", "depth_gain"),
                                "Start Scale", pObj);
    }

    {
        Depth& rDepth = mDepths[cDepth_Offset];
        rDepth.mEnable.init(false, sead::FormatFixedSafeString<128>("enable_%s", "depth_offset"), "Enable",
                            pObj);
        rDepth.mStart.init(utl::DevTools::calcScale(0.0f),
                           sead::FormatFixedSafeString<128>("%s_start", "depth_offset"), "Start", pObj);
        rDepth.mEnd.init(utl::DevTools::calcScale(100.0f),
                         sead::FormatFixedSafeString<128>("%s_end", "depth_offset"), "End", pObj);
        rDepth.mValue.init(1.0f, sead::FormatFixedSafeString<128>("%s_value", "depth_offset"), "Offset", pObj);
        rDepth.mValueStart.init(1.0f, sead::FormatFixedSafeString<128>("%s_value_start", "depth_offset"),
                                "Start Offset", pObj);
    }

    {
        Depth& rDepth = mDepths[cDepth_Shaft];
        rDepth.mEnable.init(false, sead::FormatFixedSafeString<128>("enable_%s", "shaft_d"), "Enable",
                            pObj);
        rDepth.mStart.init(utl::DevTools::calcScale(0.0f),
                           sead::FormatFixedSafeString<128>("%s_start", "shaft_d"), "Start", pObj);
        rDepth.mEnd.init(utl::DevTools::calcScale(100.0f),
                         sead::FormatFixedSafeString<128>("%s_end", "shaft_d"), "End", pObj);
        rDepth.mValue.init(1.0f, sead::FormatFixedSafeString<128>("%s_value", "shaft_d"), "End Scale", pObj);
        rDepth.mValueStart.init(1.0f, sead::FormatFixedSafeString<128>("%s_value_start", "shaft_d"),
                                "Start Scale", pObj);
    }

    updateBalance_();
}

void BloomParameter::updateBalance_()
{
    if (mBalanceType == 1)
    {
        mBalance.x = mThresholdBalance->r;
        mBalance.y = mThresholdBalance->g;
        mBalance.z = mThresholdBalance->b;
    }
    else
    {
        mBalance.set(0.298912f, 0.586611f, 0.114478f);
    }

    f32 sum = mBalance.x + mBalance.y + mBalance.z;
    if (sum > 0.0f)
    {
        f32 inv = 1.0f / sum;
        mBalance *= inv;
    }
}

void BloomParameter::genMessageBloomParameter(sead::hostio::Context* pContext)
{
    {
        sead::SafeString label = mFinalBlend.getLabel();
    }

    s32 editType = *mEditType;
    mMain.genMessage(pContext, editType == 1);
    if (*mEditType == 0)
    {
        for (auto& rColor : mColors)
        {
            rColor.genMessageParameter(pContext, rColor.getMeta());
        }
    }

    Depth& rGain = mDepths[cDepth_Gain];
    rGain.mEnable.genMessageParameter(pContext, rGain.mEnable.getMeta());
    if (*rGain.mEnable)
    {
        rGain.mStart.genMessageParameter(pContext, utl::DevTools::getStringMinMax(0.0f, 1000.0f));
        rGain.mEnd.genMessageParameter(pContext, utl::DevTools::getStringMinMax(0.0f, 1000.0f));
        rGain.mValueStart.genMessageParameter(pContext, "Min=0,Max=8");
        rGain.mValue.genMessageParameter(pContext, "Min=0,Max=8");
    }

    Depth& rOffset = mDepths[cDepth_Offset];
    rOffset.mEnable.genMessageParameter(pContext, rOffset.mEnable.getMeta());
    if (*rOffset.mEnable)
    {
        rOffset.mStart.genMessageParameter(pContext,
                                           utl::DevTools::getStringMinMax(0.0f, 1000.0f));
        rOffset.mEnd.genMessageParameter(pContext, utl::DevTools::getStringMinMax(0.0f, 1000.0f));
        rOffset.mValue.genMessageParameter(pContext, "Min=0,Max=8");
    }

    mEnableDepthClamp.genMessageParameter(pContext, mEnableDepthClamp.getMeta());
    mMain.mFinalGather.genMessageParameter(pContext, mMain.mFinalGather.getMeta());

    if (*mExType != 0)
    {
        mShaft.genMessage(pContext, false);
        Depth& rShaft = mDepths[cDepth_Shaft];
        rShaft.mEnable.genMessageParameter(pContext, rShaft.mEnable.getMeta());
        if (*rShaft.mEnable)
        {
            rShaft.mStart.genMessageParameter(pContext, "Min=0,Max=10000");
            rShaft.mEnd.genMessageParameter(pContext, "Min=0,Max=10000");
            rShaft.mValueStart.genMessageParameter(pContext, "Min=0,Max=8");
            rShaft.mValue.genMessageParameter(pContext, "Min=0,Max=8");
        }

        mShaft.mFinalGather.genMessageParameter(pContext, mShaft.mFinalGather.getMeta());
        mExIteration.genMessageParameter(pContext, "Min=0, Max=10");
    }
}

void BloomParameter::Unit::genMessage(sead::hostio::Context* pContext, bool isEditExpand)
{
    mThreshold.genMessageParameter(pContext, "Min=0, Max=2");
    mThresholdRange.genMessageParameter(pContext, "Min=0.001, Max=0.5");
    mIntensity.genMessageParameter(pContext, "Min=0, Max=8");
    if (isEditExpand)
    {
        mExpand.genMessageParameter(pContext, "Min=0, Max=2");
    }
}

/**
 * Requests a message regeneration or updates the balance when a relevant property changes.
 * @param pReflexible host IO reflexible that owns the parameters
 * @param pEvent property event
 */
void BloomParameter::listenPropertyEventBloomParameter(sead::hostio::Reflexible* pReflexible,
                                                       const sead::hostio::PropertyEvent* pEvent)
{
    if (pEvent->getType() & 2)
    {
        return;
    }

    if (isEventTarget(pEvent, &*mDepths[cDepth_Gain].mEnable, &*mDepths[cDepth_Gain].mEnable + 1) ||
        isEventTarget(pEvent, &*mDepths[cDepth_Offset].mEnable,
                      &*mDepths[cDepth_Offset].mEnable + 1) ||
        isEventTarget(pEvent, &*mDepths[cDepth_Shaft].mEnable,
                      &*mDepths[cDepth_Shaft].mEnable + 1) ||
        isEventTarget(pEvent, &*mExType, &*mExType + 1) ||
        isEventTarget(pEvent, &*mEditType, &*mEditType + 1) ||
        isEventTarget(pEvent, &*mEnableLuminanceOffset, &*mEnableLuminanceOffset + 1) ||
        isEventTarget(pEvent, &*mEnableClampedLuminance, &*mEnableClampedLuminance + 1))
    {
        requestGenMessage_(pReflexible);
        return;
    }

    if (isEventTarget(pEvent, &mBalanceType, &mBalanceType + 1) ||
        isEventTarget(pEvent, &mThresholdBalance, &mThresholdBalance + 1))
    {
        updateBalance_();
    }
}

void BloomParameter::Unit::getUniformThreshold(sead::Vector4f* pThreshold,
                                               sead::Vector4f* pBalance, f32 scale,
                                               const sead::Vector3f& rBalance) const
{
    f32 threshold = *mThreshold * scale;
    f32 range = *mThresholdRange * scale;
    f32 inv = range > 0.0f ? 1.0f / range : 0.0f;
    pThreshold->x = inv;
    pThreshold->y = 0.0f;
    pThreshold->z = *mIntensity;
    pThreshold->w = 10000.0f;
    pBalance->x = inv * rBalance.x;
    pBalance->y = inv * rBalance.y;
    pBalance->z = inv * rBalance.z;
    pBalance->w = -(threshold * inv);
}

Bloom::Bloom() : IParameterIO("aglblm", 1)
{
    agl::detail::RootNode::setNodeMeta(this, "Icon=EFFECT");
    mFlags.set(cFlag_Reduce);
}

Bloom::~Bloom() = default;

void Bloom::initialize(s32 contextNum, sead::Heap* pHeap)
{
    initializeContextParameterBuffer(contextNum, false, pHeap);
    mEnable.init(true, "enable", "Enable", &mParameterObjs[0]);
    addObj(&mParameterObjs[0], "bloom");
    copyParameterToAllContext(0);
    updateBalance_();
    mDebugTexturePage.setUp(contextNum, "Bloom", pHeap);
}

void Bloom::initializeContext(Context* pContext, sead::Heap* pHeap)
{
    pContext->mColorSampler.setUseTextureView(true);
    pContext->mColorSampler.setCompSel(cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B,
                                       cTextureCompSel_1);
    pContext->mNear = utl::DevTools::calcScale(0.1f);
    pContext->mFar = utl::DevTools::calcScale(10000.0f);
    pContext->mThresholdScale = 1.0f;
    pContext->mResolution = 0.25f;
    pContext->mScale = sead::Vector2f::ones;
    pContext->mTextureCache.initialize(0x80, pHeap);
    pContext->mpAddSampler = nullptr;
}

void Bloom::update() {}

void Bloom::calcGPU() const {}

void Bloom::calcGPU(s32 context) const {}

void Bloom::setNearFar(s32 context, f32 near, f32 far)
{
    getContext_(context).mNear = near;
    getContext_(context).mFar = far;
}

void Bloom::draw(DrawContext* pDrawContext, s32 context, const DrawArg& rArg) const
{
    draw_(pDrawContext, context, rArg, false);
}

void Bloom::draw_(DrawContext* pDrawContext, s32 context, const DrawArg& rArg,
                  bool isBuffer) const
{
    const BloomParameter& rParam = getParameter(context);
    if (!*mEnable || !isEnableContext(context))
    {
        return;
    }

    Context& rContext = getContext_(context);
    utl::DynamicTextureCache& rCache = rContext.mTextureCache;
    if (!rCache.begin())
    {
        return;
    }

    sead::Vector2f size;
    rArg.mpViewport->getOnFrameBufferSize(&size, *rArg.mpRenderBuffer);
    if (size.x < 64.0f || size.y < 64.0f)
    {
        rContext.mResultSampler.applyTextureData(
            utl::PrimitiveTexture::instance()
                ->getTextureSampler(utl::PrimitiveTexture::cType_Black2D)
                ->getTextureData());
        return;
    }

    f32 resolution = rContext.mResolution;
    f32 scale = rContext.mScale.x;
    rContext.mColorSampler.applyTextureData(
        rArg.mpColor ? *rArg.mpColor :
                       *reinterpret_cast<const TextureData*>(
                           rArg.mpRenderBuffer->getRenderTargetColor()));

    bool isLinearDepth = rArg.mIsLinearDepth;
    if (rArg.mpDepth && !mFlags.isOn(cFlag_IgnoreDepth))
    {
        rContext.mDepthSampler.applyTextureData(*rArg.mpDepth);
    }
    else if (const auto* pDepth = rArg.mpRenderBuffer->getRenderTargetDepth())
    {
        rContext.mDepthSampler.applyTextureData(*reinterpret_cast<const TextureData*>(pDepth));
        isLinearDepth = false;
    }

    resolution *= scale;
    if (rArg.mpMask)
    {
        rContext.mMaskSampler.applyTextureData(*rArg.mpMask);
    }

    u32 width = resolution * size.x;
    u32 height = resolution * size.y;

    utl::VertexAttributeHolder::instance()
        ->getVertexAttribute(utl::VertexAttributeHolder::cAttribute_QuadTriangleTexCoord)
        .activate(pDrawContext);

    TextureData* pReduce = nullptr;
    if (mFlags.isOn(cFlag_Reduce) && !rArg.mpColor)
    {
        f32 reduceWidth = f32(width) + f32(width);
        f32 reduceHeight = f32(height) + f32(height);
        pReduce = rCache.alloc(pDrawContext, "reduce", TextureFormat::cTextureFormat_R11_G11_B10_float,
                               reduceWidth, reduceHeight, 1, nullptr,
                               utl::DynamicTextureCache::cAllocateType_0, true);
        MRT& rMRT = rContext.mMRTs[0];
        rContext.mRenderBuffer.setVirtualSize(sead::Vector2f(reduceWidth, reduceHeight));
        rContext.mRenderBuffer.setPhysicalArea(
            sead::BoundBox2f(0.0f, 0.0f, reduceWidth, reduceHeight));
        rMRT.mTarget.setMipLevel(0);
        rMRT.mTarget.applyTextureData(*pReduce);
        rContext.mRenderBuffer.setRenderTargetColor(&rMRT.mTarget);

        sead::GraphicsContext graphicsContext;
        graphicsContext.setDepthEnable(false, false);
        graphicsContext.setBlendEnable(false);
        graphicsContext.apply(pDrawContext);

        sead::Viewport viewport(rContext.mRenderBuffer);
        viewport.apply(pDrawContext, rContext.mRenderBuffer);
        rContext.mRenderBuffer.bind(pDrawContext);

        const ShaderProgram* pProgram = agl::detail::ShaderHolder::instance()->getShaderProgram(
            agl::detail::ShaderHolder::cBloomReduce);
        pProgram->activate(pDrawContext, true);
        rContext.mColorSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
        detail::drawIndexStream(pDrawContext,
                                utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
        rMRT.mTarget.invalidateGPUCache(pDrawContext);
        rContext.mColorSampler.applyTextureData(*pReduce);
    }

    u32 detectWidth = (width + 3) & ~3u;
    u32 detectHeight = (height + 3) & ~3u;
    for (s32 i = 0; i < 2; i++)
    {
        MRT& rMRT = rContext.mMRTs[i];
        const char* pName = i == 0 ? "detect0" : "detect1";
        if (!mFlags.isOn(cFlag_UseMipLevel))
        {
            rMRT.mTextures[0] = rCache.alloc(pDrawContext, pName,
                                             TextureFormat::cTextureFormat_R11_G11_B10_float,
                                             detectWidth, detectHeight, 1, nullptr,
                                             utl::DynamicTextureCache::cAllocateType_0, true);
            rMRT.mTextures[1] = rCache.alloc(pDrawContext, pName,
                                             TextureFormat::cTextureFormat_R11_G11_B10_float,
                                             detectWidth >> 1, detectHeight >> 1, 1, nullptr,
                                             utl::DynamicTextureCache::cAllocateType_0, true);
            rMRT.mTextures[2] = rCache.alloc(pDrawContext, pName,
                                             TextureFormat::cTextureFormat_R11_G11_B10_float,
                                             (width + 3) >> 2, (height + 3) >> 2, 1, nullptr,
                                             utl::DynamicTextureCache::cAllocateType_0, true);
            rMRT.mTextures[3] = rCache.alloc(pDrawContext, pName,
                                             TextureFormat::cTextureFormat_R11_G11_B10_float,
                                             (width + 3) >> 3, (height + 3) >> 3, 1, nullptr,
                                             utl::DynamicTextureCache::cAllocateType_0, true);
            rMRT.mTextures[4] = rCache.alloc(pDrawContext, pName,
                                             TextureFormat::cTextureFormat_R11_G11_B10_float,
                                             (width + 3) >> 4, (height + 3) >> 4, 1, nullptr,
                                             utl::DynamicTextureCache::cAllocateType_0, true);
        }
        else
        {
            rMRT.mTextures[0] = rCache.alloc(pDrawContext, pName,
                                             TextureFormat::cTextureFormat_R11_G11_B10_float,
                                             detectWidth, detectHeight, i == 0 ? 5 : 1, nullptr,
                                             utl::DynamicTextureCache::cAllocateType_0, true);
            rMRT.mTarget.setMipLevel(0);
            rMRT.mSampler.setLod(0.0f, 0.0f, 0.0f);
        }

        rMRT.mTarget.applyTextureData(*rMRT.mTextures[0]);
        rMRT.mSampler.applyTextureData(rMRT.mTarget);
        rContext.mRenderBuffer.setRenderTargetColor(&rMRT.mTarget, i);
    }

    drawDetect_(pDrawContext, context, isLinearDepth);
    rContext.mRenderBuffer.setRenderTargetColor(nullptr, 1);
    if (pReduce)
    {
        rCache.free(pReduce);
    }

    if (!mFlags.isOn(cFlag_NoGaussian))
    {
        drawGaussian_(pDrawContext, context, 0, 1.0f);
        drawGaussian_(pDrawContext, context, 1, 1.0f);
        drawGaussian_(pDrawContext, context, 2, 1.0f);
        drawGaussian_(pDrawContext, context, 3, 1.0f);
    }

    if (!mFlags.isOn(cFlag_NoGather))
    {
        sead::Color4f colors[4];
        switch (*mEditType)
        {
        case 0:
            colors[0] = *rParam.mColors[0];
            colors[1] = *rParam.mColors[1];
            colors[2] = *rParam.mColors[2];
            colors[3] = *rParam.mColors[3];
            break;
        case 1:
        {
            f32 expand = *rParam.mMain.mExpand;
            f32 weight1;
            f32 weight2;
            if (expand < 1.0f / 3.0f)
            {
                weight1 = expand * 3.0f;
                weight2 = 0.0f;
            }
            else
            {
                weight1 = 1.0f;
                weight2 = expand * 3.0f - 1.0f;
            }

            f32 inv = 1.0f / (weight1 + 1.0f + weight2);
            weight1 *= inv;
            weight2 *= inv;
            colors[0] = sead::Color4f(inv, inv, inv, 1.0f);
            colors[1] = sead::Color4f(weight1, weight1, weight1, 1.0f);
            colors[2] = sead::Color4f(weight2, weight2, weight2, 1.0f);
            colors[3] = sead::Color4f(0.0f, 0.0f, 0.0f, 1.0f);
            break;
        }
        default:
            break;
        }

        MRT& rMRT = rContext.mMRTs[0];
        for (s32 i = 3; i > 0; i--)
        {
            const TextureData* pTexture = rMRT.mTextures[0];
            f32 w = pTexture->getWidth(0) >> i;
            f32 h = pTexture->getHeight(0) >> i;
            if (!mFlags.isOn(cFlag_UseMipLevel))
            {
                rMRT.mTarget.applyTextureData(*rMRT.mTextures[i]);
                rMRT.mSampler.applyTextureData(*rMRT.mTextures[i + 1]);
            }
            else
            {
                rMRT.mTarget.setMipLevel(i);
                f32 lod = f32(i) + 1.0f;
                rMRT.mSampler.setLod(lod, lod, 0.0f);
            }

            rContext.mRenderBuffer.setVirtualSize(sead::Vector2f(w, h));
            rContext.mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, w, h));
            rContext.mRenderBuffer.bind(pDrawContext);
            sead::Viewport viewport(rContext.mRenderBuffer);
            viewport.apply(pDrawContext, rContext.mRenderBuffer);
            if (i != 1)
            {
                drawGather_(pDrawContext, rMRT.mSampler, colors[i], colors[i - 1]);
            }
            else if (!mFlags.isOn(cFlag_NoFinalGather))
            {
                drawGather_(pDrawContext, rMRT.mSampler, *rParam.mMain.mFinalGather,
                            *rParam.mMain.mFinalGather * colors[i - 1]);
            }

            rMRT.mTarget.invalidateGPUCache(pDrawContext);
        }
    }

    for (auto& rMRT : rContext.mMRTs)
    {
        if (!mFlags.isOn(cFlag_UseMipLevel))
        {
            rMRT.mSampler.applyTextureData(*rMRT.mTextures[0]);
            rMRT.mTarget.applyTextureData(*rMRT.mTextures[0]);
        }
        else
        {
            rMRT.mSampler.setLod(0.0f, 0.0f, 0.0f);
            rMRT.mTarget.setMipLevel(0);
        }
    }

    drawShaft_(pDrawContext, context);
    rContext.mMRTs[1].free(&rCache);

    if (!mFlags.isOn(cFlag_UseMipLevel))
    {
        rContext.mResultSampler.applyTextureData(*rContext.mMRTs[0].mTextures[1]);
    }
    else
    {
        rContext.mResultSampler.applyTextureData(*rContext.mMRTs[0].mTextures[0]);
        rContext.mResultSampler.setLod(1.0f, 1.0f, 0.0f);
    }

    if (const TextureSampler* pAdd = rContext.mpAddSampler)
    {
        MRT& rMRT = rContext.mMRTs[0];
        u32 w = pAdd->getTextureData().getWidth(0);
        u32 h = pAdd->getTextureData().getHeight(0);
        rMRT.mTarget.applyTextureData(pAdd->getTextureData());
        rContext.mRenderBuffer.setVirtualSize(sead::Vector2f(w, h));
        rContext.mRenderBuffer.setRenderTargetColor(&rMRT.mTarget);
        rContext.mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, w, h));
        rContext.mRenderBuffer.bind(pDrawContext);
        sead::Viewport viewport(rContext.mRenderBuffer);
        viewport.apply(pDrawContext, rContext.mRenderBuffer);

        sead::GraphicsContext graphicsContext;
        graphicsContext.setDepthEnable(false, false);
        graphicsContext.setBlendEnable(true);
        graphicsContext.setBlendFactor(0, 2, 2);
        graphicsContext.apply(pDrawContext);

        const ShaderProgram* pProgram =
            agl::detail::ShaderHolder::instance()
                ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cBloomCompose)
                ->getVariation(1);
        pProgram->activate(pDrawContext, true);
        rContext.mResultSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1,
                                         false);
        detail::drawIndexStream(pDrawContext,
                                utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
        rContext.mResultSampler.applyTextureData(rContext.mpAddSampler->getTextureData());
        rContext.mResultSampler.setLod(0.0f, 0.0f, 0.0f);
    }

    rArg.mpRenderBuffer->bind(pDrawContext);
    rArg.mpViewport->apply(pDrawContext, *rArg.mpRenderBuffer);

    if (!isBuffer && !mFlags.isOn(cFlag_NoComposite))
    {
        const ShaderProgram* pProgram =
            agl::detail::ShaderHolder::instance()
                ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cBloomCompose)
                ->getVariation(1);
        sead::GraphicsContext graphicsContext;
        graphicsContext.setDepthEnable(false, false);
        graphicsContext.setColorMask(true, true, true, false);
        if (mFlags.isOn(cFlag_NoBlend))
        {
            graphicsContext.setBlendEnable(false);
        }
        else
        {
            switch (*rParam.mFinalBlend)
            {
            case 0:
                graphicsContext.setBlendEnable(false);
                break;
            case 1:
                graphicsContext.setBlendEnable(true);
                graphicsContext.setBlendFactor(0, 2, 2);
                break;
            case 2:
                graphicsContext.setBlendEnable(true);
                graphicsContext.setBlendFactor(0, 10, 2);
                break;
            default:
                break;
            }
        }

        graphicsContext.apply(pDrawContext);
        pProgram->activate(pDrawContext, true);
        rContext.mResultSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1,
                                         false);
        detail::drawIndexStream(pDrawContext,
                                utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
    }

    if (mDebugFlags & 0x10)
    {
        drawDepthDepth_(pDrawContext, context, cDepth_Gain, *rArg.mpRenderBuffer);
    }

    if (mDebugFlags & 0x20)
    {
        drawDepthDepth_(pDrawContext, context, cDepth_Offset, *rArg.mpRenderBuffer);
    }

    if (mDebugFlags & 0x40)
    {
        drawDepthDepth_(pDrawContext, context, cDepth_Shaft, *rArg.mpRenderBuffer);
    }

    rContext.mMRTs[0].entry(pDrawContext, context, mDebugTexturePage);
    if (!isBuffer)
    {
        releaseBloomBuffer(context);
    }

    rCache.end();
}

void Bloom::drawToBloomBuffer(DrawContext* pDrawContext, s32 context, const DrawArg& rArg) const
{
    draw_(pDrawContext, context, rArg, true);
}

void Bloom::drawTexture_(DrawContext* pDrawContext, const ShaderProgram& rProgram,
                         const TextureSampler& rSampler) const
{
    rSampler.activate(pDrawContext, rProgram.getSamplerLocation(0), -1, false);
    detail::drawIndexStream(pDrawContext,
                            utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
}

void Bloom::drawDetect_(DrawContext* pDrawContext, s32 context, bool isLinearDepth) const
{
    const BloomParameter& rParam = getParameter(context);
    Context& rContext = getContext_(context);
    const RenderTargetColor& rTarget = rContext.mMRTs[0].mTarget;
    u32 width = rContext.mColorSampler.getTextureData().getWidth(0);
    u32 height = rContext.mColorSampler.getTextureData().getHeight(0);
    f32 texelWidth = 1.0f / f32(width);
    f32 texelHeight = 1.0f / f32(height);

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnableMask(0);
    graphicsContext.setColorMask(*rParam.mExType == 0 ? 0xf : 0xff);
    graphicsContext.apply(pDrawContext);

    const ShaderProgram* pProgram = agl::detail::ShaderHolder::instance()->getShaderProgram(
        agl::detail::ShaderHolder::cBloomMask);
    s32 variation = (*rParam.mEnableDepthClamp ? pProgram->getVariationMacroStride(0) : 0) +
                    (isLinearDepth ? pProgram->getVariationMacroStride(3) : 0) +
                    (*rParam.mExType != 0 ? pProgram->getVariationMacroStride(4) : 0) +
                    (*rParam.mDepths[cDepth_Shaft].mEnable ? pProgram->getVariationMacroStride(5) :
                                                             0) +
                    (*mEnableLuminanceOffset ? pProgram->getVariationMacroStride(6) : 0) +
                    (*mEnableClampedLuminance ? pProgram->getVariationMacroStride(7) : 0) +
                    (*rParam.mDepths[cDepth_Gain].mEnable &&
                             *rParam.mDepths[cDepth_Gain].mValue > 0.0f ?
                         pProgram->getVariationMacroStride(1) :
                         0) +
                    (*rParam.mDepths[cDepth_Offset].mEnable &&
                             *rParam.mDepths[cDepth_Offset].mValue > 0.0f ?
                         pProgram->getVariationMacroStride(2) :
                         0);
    pProgram = pProgram->getVariation(variation);
    pProgram->activate(pDrawContext, true);

    {
        sead::Vector4f param0;
        sead::Vector4f param1;
        rParam.mMain.getUniformThreshold(&param0, &param1, rContext.mThresholdScale, mBalance);
        param0.x *= *mClampedLuminance;
        pProgram->getUniformLocation(1).setUniform(pDrawContext, 4, &param0);
        pProgram->getUniformLocation(0).setUniform(pDrawContext, 4, &param1);
        rParam.mShaft.getUniformThreshold(&param0, &param1, rContext.mThresholdScale, mBalance);
        pProgram->getUniformLocation(8).setUniform(pDrawContext, 4, &param0);

        rContext.mMaskSampler.activate(pDrawContext, pProgram->getSamplerLocation(2), -1, false);
        param0.set(*mLuminanceIntensity + *mLuminanceIntensity, 0.0f, 0.0f, 0.0f);
        pProgram->getUniformLocation(7).setUniform(pDrawContext, 4, &param0);
        rContext.mDepthSampler.activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);

        const BloomParameter::Depth& rGain = rParam.mDepths[cDepth_Gain];
        const BloomParameter::Depth& rOffset = rParam.mDepths[cDepth_Offset];
        const BloomParameter::Depth& rShaft = rParam.mDepths[cDepth_Shaft];
        f32 far = rContext.mFar;
        f32 near = rContext.mNear;
        f32 range = far - near;
        f32 invRange = 1.0f / range;
        f32 gainScale = range / (*rGain.mEnd - *rGain.mStart);
        f32 offsetScale = range / (*rOffset.mEnd - *rOffset.mStart);
        f32 shaftScale = range / (*rShaft.mEnd - *rShaft.mStart);
        param0.set(gainScale, -(invRange * (*rGain.mStart * gainScale)), offsetScale,
                   -(invRange * (*rOffset.mStart * offsetScale)));
        param1.set(*rGain.mValueStart, *rGain.mValue, *rOffset.mValue, 0.0f);
        sead::Vector4f param2(1.0f - near / far, near * invRange, texelWidth, texelHeight);
        sead::Vector4f param3(shaftScale, -(invRange * (*rShaft.mStart * shaftScale)),
                              *rShaft.mValueStart, *rShaft.mValue);
        pProgram->getUniformLocation(2).setUniform(pDrawContext, 4, &param0);
        pProgram->getUniformLocation(3).setUniform(pDrawContext, 4, &param1);
        pProgram->getUniformLocation(4).setUniform(pDrawContext, 4, &param2);
        pProgram->getUniformLocation(9).setUniform(pDrawContext, 4, &param3);
    }

    f32 w = rTarget.getWidth(0);
    f32 h = rTarget.getHeight(0);
    rContext.mRenderBuffer.setVirtualSize(sead::Vector2f(w, h));
    rContext.mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, w, h));
    rContext.mRenderBuffer.bind(pDrawContext);
    sead::Viewport viewport(rContext.mRenderBuffer);
    viewport.apply(pDrawContext, rContext.mRenderBuffer);

    rContext.mColorSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    detail::drawIndexStream(pDrawContext,
                            utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
    rContext.mMRTs[0].mTarget.invalidateGPUCache(pDrawContext);
    rContext.mMRTs[1].mTarget.invalidateGPUCache(pDrawContext);
}

/**
 * Copies one bloom level to the next and applies a separable gaussian blur to it.
 * @param pDrawContext draw context
 * @param context context index
 * @param level source level
 * @param scale blur offset scale
 */
void Bloom::drawGaussian_(DrawContext* pDrawContext, s32 context, s32 level, f32 scale) const
{
    Context& rContext = getContext_(context);
    MRT& rMRT = rContext.mMRTs[0];
    TextureSampler& rSampler = rMRT.mSampler;
    RenderTargetColor& rTarget = rMRT.mTarget;

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(pDrawContext);

    s32 nextLevel = level + 1;
    if (!mFlags.isOn(cFlag_UseMipLevel))
    {
        rSampler.applyTextureData(*rMRT.mTextures[level]);
        rTarget.applyTextureData(*rMRT.mTextures[nextLevel]);
    }
    else
    {
        rSampler.setLod(level, level, 0.0f);
        rTarget.setMipLevel(nextLevel);
    }

    u32 mipLevel = rTarget.getMipLevel();
    f32 w = u32(rTarget.getMipWidth(mipLevel));
    f32 h = u32(getMipHeight(rTarget, mipLevel));
    rContext.mRenderBuffer.setVirtualSize(sead::Vector2f(w, h));
    rContext.mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, w, h));
    rContext.mRenderBuffer.bind(pDrawContext);
    sead::Viewport viewport(rContext.mRenderBuffer);
    viewport.apply(pDrawContext, rContext.mRenderBuffer);

    const ShaderProgram* pCopy = agl::detail::ShaderHolder::instance()
                                     ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cBloomCompose)
                                     ->getVariation(1);
    pCopy->activate(pDrawContext, true);
    rSampler.activate(pDrawContext, pCopy->getSamplerLocation(0), -1, false);
    detail::drawIndexStream(pDrawContext,
                            utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
    rTarget.invalidateGPUCache(pDrawContext);

    const ShaderProgram* pBlur = agl::detail::ShaderHolder::instance()->getShaderProgram(
        agl::detail::ShaderHolder::cBloomGaussian);
    sead::Vector2f texel(1.0f / w, 1.0f / h);
    TextureData* pTemp =
        rContext.mTextureCache.alloc(pDrawContext, "blur",
                                     TextureFormat::cTextureFormat_R11_G11_B10_float, w, h, 1,
                                     nullptr, utl::DynamicTextureCache::cAllocateType_0, true);
    rMRT.mSampler.applyTextureData(rMRT.mTarget);
    if (mFlags.isOn(cFlag_UseMipLevel))
    {
        rSampler.setLod(nextLevel, nextLevel, 0.0f);
        rTarget.setMipLevel(0);
    }

    rMRT.mTarget.applyTextureData(*pTemp);
    rContext.mRenderBuffer.bind(pDrawContext);

    const ShaderProgram* pBlurH = pBlur->getVariation(0);
    pBlurH->activate(pDrawContext, true);
    {
        sead::Vector4f offset(texel.x * scale, 0.0f, 0.0f, 0.0f);
        pBlurH->getUniformLocation(6).setUniform(pDrawContext, 4, &offset);
    }

    rMRT.mSampler.activate(pDrawContext, pBlurH->getSamplerLocation(0), -1, false);
    detail::drawIndexStream(pDrawContext,
                            utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
    rMRT.mTarget.invalidateGPUCache(pDrawContext);

    if (!mFlags.isOn(cFlag_UseMipLevel))
    {
        rMRT.mTarget.applyTextureData(rMRT.mSampler.getTextureData());
    }
    else
    {
        rTarget.applyTextureData(rSampler.getTextureData(), nextLevel, 0);
        rMRT.mSampler.setLod(0.0f, 0.0f, 0.0f);
    }

    rMRT.mSampler.applyTextureData(*pTemp);
    rContext.mRenderBuffer.bind(pDrawContext);

    const ShaderProgram* pBlurV = pBlurH->getVariation(1);
    pBlurV->activate(pDrawContext, true);
    {
        sead::Vector4f offset(0.0f, texel.y * scale, 0.0f, 0.0f);
        pBlurV->getUniformLocation(6).setUniform(pDrawContext, 4, &offset);
    }

    rMRT.mSampler.activate(pDrawContext, pBlurV->getSamplerLocation(0), -1, false);
    detail::drawIndexStream(pDrawContext,
                            utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
    rMRT.mTarget.invalidateGPUCache(pDrawContext);
    rMRT.mSampler.applyTextureData(rMRT.mTarget);
    rContext.mTextureCache.free(pTemp);
}

void Bloom::drawGather_(DrawContext* pDrawContext, const TextureSampler& rSampler,
                        const sead::Color4f& rColor, const sead::Color4f& rConstantColor) const
{
    const ShaderProgram* pProgram =
        agl::detail::ShaderHolder::instance()
            ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cBloomCompose)
            ->getVariation(0);
    pProgram->activate(pDrawContext, true);
    {
        sead::Color4f color(rColor.r * rColor.a, rColor.g * rColor.a, rColor.b * rColor.a,
                            rColor.a);
        pProgram->getUniformLocation(5).setUniform(pDrawContext, 4, &color);
    }

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendFactor(0, 2, 0x61);
    graphicsContext.setBlendConstantColor(sead::Color4f(
        rConstantColor.r * rConstantColor.a, rConstantColor.g * rConstantColor.a,
        rConstantColor.b * rConstantColor.a, rConstantColor.a));
    graphicsContext.apply(pDrawContext);

    rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    detail::drawIndexStream(pDrawContext,
                            utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
}

/**
 * Draws the light shaft pass and gathers it into the bloom result.
 * @param pDrawContext draw context
 * @param context context index
 */
void Bloom::drawShaft_(DrawContext* pDrawContext, s32 context) const
{
    const BloomParameter& rParam = getParameter(context);
    if (*rParam.mExType == 0)
    {
        return;
    }

    Context& rContext = getContext_(context);
    s32 iteration = *rParam.mExIteration;
    MRT& rMRT = rContext.mMRTs[0];
    MRT& rShaftMRT = rContext.mMRTs[1];
    u32 width = rShaftMRT.mTarget.getWidth(0);
    u32 height = rShaftMRT.mTarget.getHeight(0);

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(pDrawContext);

    TextureData* pPrev = nullptr;
    f32 w = width;
    f32 h = height;
    for (s32 i = 0; i < iteration; i++)
    {
        f32 offsetX;
        f32 offsetY;
        switch (*rParam.mExType)
        {
        case 1:
            w *= 0.5f;
            offsetY = 0.0f;
            offsetX = 1.0f / w;
            break;
        case 2:
            h *= 0.5f;
            offsetY = 1.0f / h;
            offsetX = 0.0f;
            break;
        default:
            offsetY = 0.0f;
            offsetX = 0.0f;
            break;
        }

        if (h < 1.0f || w < 1.0f)
        {
            continue;
        }

        TextureData* pTarget =
            rContext.mTextureCache.alloc(pDrawContext, "bloom_ex_tgt",
                                         TextureFormat::cTextureFormat_R11_G11_B10_float, w, h, 1,
                                         nullptr, utl::DynamicTextureCache::cAllocateType_0, true);
        rMRT.mSampler.applyTextureData(*pTarget);
        rMRT.mTarget.applyTextureData(*pTarget);
        rContext.mRenderBuffer.setVirtualSize(sead::Vector2f(w, h));
        rContext.mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, w, h));
        sead::Viewport viewport(rContext.mRenderBuffer);
        viewport.apply(pDrawContext, rContext.mRenderBuffer);

        const ShaderProgram* pCopy =
            agl::detail::ShaderHolder::instance()
                ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cBloomCompose)
                ->getVariation(1);
        pCopy->activate(pDrawContext, true);
        {
            sead::Vector4f zero(0.0f, 0.0f, 0.0f, 0.0f);
            pCopy->getUniformLocation(5).setUniform(pDrawContext, 4, &zero);
        }

        rContext.mRenderBuffer.setRenderTargetColor(&rMRT.mTarget);
        rContext.mRenderBuffer.bind(pDrawContext);
        rShaftMRT.mSampler.activate(pDrawContext, pCopy->getSamplerLocation(0), -1, false);
        detail::drawIndexStream(pDrawContext,
                                utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
        rMRT.mTarget.invalidateGPUCache(pDrawContext);

        if (pPrev)
        {
            rContext.mTextureCache.free(pPrev);
        }

        pPrev = rContext.mTextureCache.alloc(pDrawContext, "bloom_ex",
                                             TextureFormat::cTextureFormat_R11_G11_B10_float, w,
                                             h, 1, nullptr,
                                             utl::DynamicTextureCache::cAllocateType_0, true);
        rShaftMRT.mTarget.applyTextureData(*pPrev);
        rShaftMRT.mSampler.applyTextureData(*pPrev);

        const ShaderProgram* pBlur =
            agl::detail::ShaderHolder::instance()
                ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cBloomGaussian)
                ->getVariation(*rParam.mExType == 1 ? 0 : 1);
        pBlur->activate(pDrawContext, true);
        {
            sead::Vector4f offset(offsetX, offsetY, 0.0f, 0.0f);
            pBlur->getUniformLocation(6).setUniform(pDrawContext, 4, &offset);
        }

        rContext.mRenderBuffer.setRenderTargetColor(&rShaftMRT.mTarget);
        rContext.mRenderBuffer.bind(pDrawContext);
        rMRT.mSampler.activate(pDrawContext, pBlur->getSamplerLocation(0), -1, false);
        detail::drawIndexStream(pDrawContext,
                                utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
        rShaftMRT.mTarget.invalidateGPUCache(pDrawContext);
        rContext.mTextureCache.free(pTarget);
    }

    const TextureData& rResult = rContext.mResultSampler.getTextureData();
    RenderTargetColor& rTarget = rMRT.mTarget;
    u32 mipLevel = sead::Mathu::min(u32(rContext.mResultSampler.getMinLod()),
                                    rResult.getMipLevelNum() - 1);
    f32 resultWidth = u32(rResult.getMipWidth(mipLevel));
    f32 resultHeight = u32(getMipHeight(rResult, mipLevel));
    rContext.mRenderBuffer.setVirtualSize(sead::Vector2f(resultWidth, resultHeight));
    rContext.mRenderBuffer.setPhysicalArea(
        sead::BoundBox2f(0.0f, 0.0f, resultWidth, resultHeight));
    rTarget.applyTextureData(rResult, mipLevel, 0);
    rContext.mRenderBuffer.setRenderTargetColor(&rTarget);
    rContext.mRenderBuffer.bind(pDrawContext);
    sead::Viewport viewport(rContext.mRenderBuffer);
    viewport.apply(pDrawContext, rContext.mRenderBuffer);
    drawGather_(pDrawContext, rShaftMRT.mSampler, *rParam.mShaft.mFinalGather,
                sead::Color4f::cWhite);
    if (pPrev)
    {
        rContext.mTextureCache.free(pPrev);
    }
}

void Bloom::MRT::free(utl::DynamicTextureCache* pCache)
{
    for (auto& pTexture : mTextures)
    {
        if (pTexture)
        {
            pCache->free(pTexture);
            pTexture = nullptr;
        }
    }
}

void Bloom::drawDepthDepth_(DrawContext* pDrawContext, s32 context, s32 index,
                            const RenderBuffer& rRenderBuffer) const
{
    Context& rContext = getContext_(context);
    const BloomParameter::Depth& rDepth = getParameter(context).mDepths[index];
    if (!*rDepth.mEnable)
    {
        return;
    }

    f32 depth[2] = {*rDepth.mStart, *rDepth.mEnd};
    sead::Color4f colors[2] = {sead::Color4f::cBlack, sead::Color4f::cWhite};
    utl::DevTools::drawDepthGradation(pDrawContext, rRenderBuffer, 2, depth, colors,
                                      rContext.mNear, rContext.mFar);
}

void Bloom::MRT::entry(DrawContext* pDrawContext, s32 context,
                       const utl::DebugTexturePage& rPage) const
{
    for (s32 i = 0; i < 5; i++)
    {
        if (mTextures[i])
        {
            sead::FormatFixedSafeString<1024> name("bloom:%d", i);
        }
    }
}

void Bloom::releaseBloomBuffer(s32 context) const
{
    Context& rContext = getContext_(context);
    rContext.mMRTs[0].free(&rContext.mTextureCache);
    rContext.mMRTs[1].free(&rContext.mTextureCache);
}

void Bloom::postRead_()
{
    copyParameterToAllContext(0);
    updateBalance_();
}

void Bloom::callbackNotAppliable_(utl::IParameterObj* pObj, utl::ParameterBase* pParam,
                                  utl::ResParameterObj obj)
{
    if (!obj || pObj != &mParameterObjs[0])
    {
        return;
    }

    if (pParam == &mMain.mIntensity)
    {
        s32 index = obj.searchIndex(utl::ParameterBase::calcHash("gain"));
        if (index == -1)
        {
            return;
        }

        utl::ResParameter res = obj.getResParameter(index);
        if (!res.ptr())
        {
            return;
        }

        pParam->applyResource(res);
    }
    else if (pParam == &mDepths[cDepth_Gain].mValue)
    {
        s32 index = obj.searchIndex(utl::ParameterBase::calcHash("depth_gain"));
        if (index == -1)
        {
            return;
        }

        utl::ResParameter res = obj.getResParameter(index);
        if (!res.ptr())
        {
            return;
        }

        pParam->applyResource(res);
        *mDepths[cDepth_Gain].mValue += 1.0f;
    }
    else if (pParam == &mDepths[cDepth_Offset].mValue)
    {
        s32 index = obj.searchIndex(utl::ParameterBase::calcHash("depth_offset"));
        if (index == -1)
        {
            return;
        }

        utl::ResParameter res = obj.getResParameter(index);
        if (!res.ptr())
        {
            return;
        }

        pParam->applyResource(res);
    }
}

void Bloom::callbackInvalidVersion_(utl::ResParameterArchive archive) {}

void Bloom::genMessage(sead::hostio::Context* pContext)
{
    genMessageIO(pContext, 0xf);
    mDebugTexturePage.genMessagePage(pContext, this);
    mEnable.genMessageParameter(pContext, mEnable.getMeta());
    if (mBalanceType == 1)
    {
        mThresholdBalance.genMessageParameter(pContext, mThresholdBalance.getMeta());
    }

    genMessageBloomParameter(pContext);
    u32 contextNum = getContextBuffer_().size();
    for (u32 i = 0; i < contextNum; i++)
    {
        {
            sead::FormatFixedSafeString<1024> header("GroupHeader= viewpoint: %d", i);
        }

        {
            sead::FixedSafeString<256> nearMeta = utl::DevTools::getStringMinMax(0.0f, 10000.0f);
        }

        {
            sead::FixedSafeString<256> farMeta = utl::DevTools::getStringMinMax(0.0f, 10000.0f);
        }
    }
}

void Bloom::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    listenPropertyEventIO(this, pEvent);
    listenPropertyEventBloomParameter(this, pEvent);
    copyParameterToAllContext(0);
}

Bloom::MRT::MRT()
{
    for (auto& pTexture : mTextures)
    {
        pTexture = nullptr;
    }
}

}  // namespace agl::pfx
