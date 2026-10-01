#include "lighting/aglLightPrePass.h"

#include <gfx/seadCamera.h>
#include <gfx/seadViewport.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <math/seadMathCalcCommon.h>

#include "common/aglDrawContext.h"
#include "common/aglGPUMemBlock.h"
#include "common/aglTextureDataInitializer.h"
#include "detail/aglRootNode.h"
#include "driver/aglNVNMgr.h"
#include "utility/aglDynamicTextureAllocator.h"

namespace agl::lght {

namespace {

u16 convertF32ToF16(f32 value)
{
    u32 bits = *reinterpret_cast<u32*>(&value);
    u32 upper = bits >> 16;
    u32 sign = upper & 0x8000;
    u32 exponent = bits & 0x7f800000;

    if (exponent == 0x7f800000)
    {
        u32 mantissa = bits & 0x7fffff;

        if (mantissa == 0)
        {
            return sign | 0x7c00;
        }

        return mantissa < 0x400000 ? (sign | 0x7dff) : (upper | 0x7fff);
    }

    s32 e = (exponent >> 23) - 0x70;
    u32 half = e < 1 ? sign : (((bits >> 13) & 0x3ff) | sign | ((e & 0x1f) << 10));
    return e < 0x1f ? half : (sign | 0x7c00);
}

}  // namespace

LightPrePass::LightPrePass()
    : mFlags(0x220), mDirtyFlags(0), mLightTypeMask(0), mFogScale(1.0f), mQuality(0),
      mSpecPowTexWidth(0), _488(nullptr),
      mSpecularCurve("SpecularCurve",
                     "\xe3\x83\xa9\xe3\x83\x95\xe3\x83\x8d\xe3\x82\xb9\xe5\xaf\xbe"
                     "\xe3\x82\xb9\xe3\x83\x9a\xe3\x82\xad\xe3\x83\xa5\xe3\x83\xa9"
                     "\xe4\xb9\x97\xe6\x95\xb0/\xe5\xbc\xb7\xe5\xba\xa6\xe3\x81\xae"
                     "\xe3\x82\xab\xe3\x83\xbc\xe3\x83\x96",
                     nullptr),
      mSpecPowScale(0.0f), mSpecPowOffset(0.0f), mSpecIntensityScale(0.0f),
      mSpecIntensityOffset(0.0f), mPreDrawCallback(nullptr), mPostDrawCallback(nullptr),
      _8d0(true), mBufferIndex(0), mTileWidth(0x40), mTileHeight(0x40)
{
    detail::RootNode::setNodeMeta(this, "Icon=LIGHT");

    mGraphicsContext[0].setColorMask(true, true, true, true);
    mGraphicsContext[0].setBlendEnable(0, true);
    mGraphicsContext[0].setDepthEnable(false, false);
    mGraphicsContext[0].setBlendFactor(0, 2, 2);
    mGraphicsContext[0].setBlendEquation(0, 1);
    mGraphicsContext[0].setCullingMode(1);

    mGraphicsContext[1] = mGraphicsContext[0];
    mGraphicsContext[1].setDepthEnable(true, false);
    mGraphicsContext[1].setDepthFunc(5);

    mGraphicsContext[2].setBlendEnable(0, true);
    mGraphicsContext[2].setBlendEnable(1, true);
    mGraphicsContext[2].setDepthEnable(false, false);
    mGraphicsContext[2].setCullingMode(1);
    mGraphicsContext[2].setBlendFactor(0, 2, 2);
    mGraphicsContext[2].setBlendEquation(0, 1);
    mGraphicsContext[2].setBlendFactor(1, 2, 2);
    mGraphicsContext[2].setBlendEquation(1, 1);
    mGraphicsContext[2].setColorMask(0xff);

    mGraphicsContext[3] = mGraphicsContext[2];
    mGraphicsContext[3].setDepthEnable(true, false);
    mGraphicsContext[3].setDepthFunc(5);

    InitializeShaderVariationTable_();
}

LightPrePass::~LightPrePass()
{
    for (auto& rContext : mContext)
    {
        rContext.mViewUbo.destroy();
    }

    mContext.freeBuffer();

    mPointLightMgr->destroy();
    delete mPointLightMgr;
    mSpotLightMgr->destroy();
    delete mSpotLightMgr;
    mProjLightMgr->destroy();
    delete mProjLightMgr;

    mUserLightMgr.freeBuffer();
    mSpecPowTexBuffer.deleteGPUMemBlock();
    mDebugTexturePage.cleanUp();
}

/**
 * Allocates the per-view contexts, the built-in light managers and the specular power texture.
 * @param rArg creation parameters
 * @param pHeap heap to allocate from
 */
void LightPrePass::initialize(const CreateArg& rArg, sead::Heap* pHeap)
{
    mContext.tryAllocBuffer(rArg.mViewNum, pHeap);

    for (auto it = mContext.begin(), end = mContext.end(); it != end; ++it)
    {
        Context& rContext = *it;
        rContext.mpLightBufferTexture = nullptr;
        rContext.mCurrentDepthTest = 0;
        rContext.mCurrentStencil = 0;
        rContext.mLightState = 0;
        rContext.mBufferState = 0;

        if (it.getIndex() == 0)
        {
            rContext.mViewUbo.startDeclare(4, pHeap);
            rContext.mViewUbo.declare(UniformBlock::cType_Vec4, 4);
            rContext.mViewUbo.declare(UniformBlock::cType_Vec4, 3);
            rContext.mViewUbo.declare(UniformBlock::cType_Vec4, 3);
            rContext.mViewUbo.declare(UniformBlock::cType_Vec4, 3);
        }
        else
        {
            rContext.mViewUbo.declare(mContext.front().mViewUbo);
        }

        rContext.mViewUbo.create(pHeap, 2, 1);
        rContext.mGraphicsContext.setDepthEnable(false, false);
        rContext.mGraphicsContext.setBlendEnable(0, false);
        rContext.mGraphicsContext.setBlendEnable(1, false);
        rContext.mGraphicsContext.setColorMask(0xff);
        rContext.mGraphicsContext.setDepthFunc(8);
    }

    mPointLightMgr = new (pHeap) PointLightMgr();
    mSpotLightMgr = new (pHeap) SpotLightMgr();
    mProjLightMgr = new (pHeap) ProjLightMgr();
    mPointLightMgr->initialize(this, rArg.mPointLightNum, rArg.mViewNum, pHeap);
    mSpotLightMgr->initialize(this, rArg.mSpotLightNum, rArg.mViewNum, pHeap);
    mProjLightMgr->initialize(this, rArg.mProjLightNum, rArg.mViewNum, pHeap);

    mUserLightMgr.tryAllocBuffer(rArg.mUserLightMgrNum, pHeap);

    for (auto& rpMgr : mUserLightMgr)
    {
        rpMgr = nullptr;
    }

    mLightTypeMask = 0xff;
    changeTextureFilter_();

    mSpecularCurve.getCurve(0).setCurveType(sead::hostio::CurveType::Hermit2D);
    mSpecularCurve.getCurve(1).setCurveType(sead::hostio::CurveType::Hermit2D);
    mSpecPowScale = 120.0f;
    mSpecPowOffset = 0.0f;
    mSpecIntensityScale = 1.0f;
    mSpecIntensityOffset = 0.0f;
    _8d0 = true;

    for (s32 i = 0; i < 2; i++)
    {
        sead::hostio::CurveData& rData = mSpecularCurve.getCurveData(i);
        rData.numUse = 6;
        rData.curveType = 7;
        rData.f[0] = 0.0f;
        rData.f[1] = 1.0f;
        rData.f[2] = -3.083333f;
        rData.f[3] = 1.0f;
        rData.f[4] = 0.0f;
        rData.f[5] = -0.1578947f;
        mSpecularCurve.getCurve(i).setNumUse(6);
    }

    mSpecPowTexWidth = 0x100;
    mSpecPowTex.initialize_(TextureType::cTextureType_2D, TextureFormat::cTextureFormat_R16_G16_float,
                            0x100, 1, 1, 1, TextureAttribute(), MultiSampleType(), true);
    u32 size = mSpecPowTex.getSurface().mStorageSize;
    auto* pBlock = new (pHeap) GPUMemBlockU8;
    pBlock->allocBuffer_(size, pHeap, 0x2000, MemoryAttribute::Default);
    mSpecPowTexBuffer = GPUMemVoidAddr(*pBlock, 0);
    updateSpecPowTex_();

    mDebugTexturePage.setUp(rArg.mViewNum, "LightPrePass", pHeap);
}

void LightPrePass::changeTextureFilter_()
{
    u8 flags = mFlags.getDirect();
    u8 normalFilter = (flags >> 3) & 1;
    u8 depthFilter = (flags >> 4) & 1;

    for (auto& rContext : mContext)
    {
        rContext.mNormalSampler.setFilter(normalFilter, normalFilter, 0);
        rContext.mDepthSampler.setFilter(depthFilter, depthFilter, 0);
    }
}

void LightPrePass::updateSpecPowTex_()
{
    u16* pTexel = static_cast<u16*>(mSpecPowTexBuffer.getPtr());

    for (u32 i = 0; i < mSpecPowTexWidth; i++)
    {
        f32 t = f32(i) / f32(mSpecPowTexWidth);
        f32 power = mSpecularCurve.interpolateToF32(0, t) * mSpecPowScale + mSpecPowOffset;
        f32 intensity =
            mSpecularCurve.interpolateToF32(1, t) * mSpecIntensityScale +
            mSpecIntensityOffset;
        pTexel[i * 2] = convertF32ToF16(power);
        pTexel[i * 2 + 1] = convertF32ToF16(intensity);
    }

    mSpecPowTex.initialize_(TextureType::cTextureType_2D, TextureFormat::cTextureFormat_R16_G16_float,
                            mSpecPowTexWidth, 1, 1, 1, TextureAttribute(), MultiSampleType(), true);
    mSpecPowTex.setDebugLabel("LightPrePass_SpecPow");
    mSpecPowTex.setImagePtr(mSpecPowTexBuffer);
    TextureDataInitializerRAW::copyTileImage(&mSpecPowTex, mSpecPowTexBuffer, 0);
    mSpecPowSampler.applyTextureData(mSpecPowTex);
    mSpecPowSampler.setWrap(6, 6, 6);
    mSpecPowSampler.setFilter(0, 0, 0);
}

/**
 * Stores a user light manager in the given slot.
 * @param index slot index
 * @param pMgr light manager
 */
void LightPrePass::setUserLightMgr(s32 index, LightMgrBase* pMgr)
{
    mUserLightMgr[index] = pMgr;
}

/**
 * Flips the buffer index and runs the per-frame calculation of every user light manager.
 */
void LightPrePass::calc()
{
    mBufferIndex = mBufferIndex == 0;

    for (LightMgrBase* pMgr : mUserLightMgr)
    {
        if (pMgr)
        {
            pMgr->calc();
        }
    }

    if ((mDirtyFlags & 3) == 3)
    {
        mDirtyFlags &= ~3u;
    }
}

/**
 * Updates the culling data of a view from a camera and projection, then the view context.
 * @param view view index
 * @param rCamera camera of the view
 * @param rProjection projection of the view
 */
void LightPrePass::calcContext(s32 view, const sead::Camera& rCamera,
                               const sead::Projection& rProjection)
{
    mContext[view].mCulling.update(rCamera.getMatrix(), rProjection);
    calcContext_(view);
}

void LightPrePass::calcContext_(s32 view)
{
    Context& rContext = mContext[view];
    const cull::ViewFrustumCulling& rCulling = rContext.mCulling;
    rContext.mBufferState &= ~7u;
    rContext.mDepthScale = 1.0f - rCulling.mNear / rCulling.mFar;
    rContext.mViewMtx = rCulling.mViewMtx;
    rContext.mViewProjMtx.setMul(rCulling.mProjMtx, rCulling.mViewMtx);
    rContext.mFarNearDiff = rCulling.mFar - rCulling.mNear;
    rContext.mFarNearDiffInv = 1.0f / rContext.mFarNearDiff;
    rContext.mProjScale = rCulling.mAspect * rCulling.mTanHalfFovy;
    rContext.mProjSign = -1.0f;
    rContext.mScreenScaleX = rContext.mProjScale * (rCulling.mOffset.x + rCulling.mOffset.x);
    rContext.mScreenScaleY = rCulling.mTanHalfFovy * (rCulling.mOffset.y + rCulling.mOffset.y);
    rContext.mViewUbo.setCurrentBufferIndex(mBufferIndex);

    s32 num = mPointLightMgr->calcView(view, rContext, mBufferIndex) +
              mSpotLightMgr->calcView(view, rContext, mBufferIndex) +
              mProjLightMgr->calcView(view, rContext, mBufferIndex);
    for (LightMgrBase* pMgr : mUserLightMgr)
    {
        if (pMgr)
        {
            num += pMgr->calcView(view, rContext, mBufferIndex);
        }
    }

    if (num == 0)
    {
        rContext.mLightState |= 1;
    }
    else
    {
        rContext.mLightState &= ~1u;
    }
}

/**
 * Updates the culling data of a view from explicit matrices, then the view context.
 * @param view view index
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 * @param near near clip distance
 * @param far far clip distance
 * @param fovy vertical field of view
 * @param aspect aspect ratio
 * @param rOffset projection offset
 */
void LightPrePass::calcContext(s32 view, const sead::Matrix34f& rViewMtx,
                               const sead::Matrix44f& rProjMtx, f32 near, f32 far, f32 fovy,
                               f32 aspect, const sead::Vector2f& rOffset)
{
    mContext[view].mCulling.update(rViewMtx, rProjMtx, near, far, fovy, aspect, rOffset);
    calcContext_(view);
}

/**
 * Runs the GPU update of the built-in and user light managers.
 */
void LightPrePass::updateGPU() const
{
    mPointLightMgr->updateGPU();
    mSpotLightMgr->updateGPU();
    mProjLightMgr->updateGPU();

    for (const LightMgrBase* pMgr : mUserLightMgr)
    {
        if (pMgr)
        {
            pMgr->updateGPU();
        }
    }
}

/**
 * Writes the view uniform block of a view and runs the view GPU update of every light manager.
 * @param view view index
 */
void LightPrePass::updateViewGPU(s32 view) const
{
    const Context& rContext = mContext[view];
    const UniformBlock& rUbo = rContext.mViewUbo;
    rUbo.dcbz(0);
    rUbo.setData(0, &rContext.mViewProjMtx, 0, 4);
    rUbo.setData(1, &rContext.mCulling.mViewMtx, 0, 3);
    rUbo.setData(2, &rContext.mCulling.mViewInvMtx, 0, 3);

    {
        sead::Vector4f param(rContext.mCulling.mNear, rContext.mFarNearDiff,
                             rContext.mFarNearDiffInv, rContext.mDepthScale);
        rUbo.setData(3, &param, 0, 1);
    }

    {
        sead::Vector4f param(rContext.mProjScale,
                             rContext.mProjSign * rContext.mCulling.mTanHalfFovy,
                             rContext.mScreenScaleX, rContext.mScreenScaleY);
        rUbo.setData(3, &param, 1, 1);
    }

    {
        sead::Vector4f param(mFogScale > 0.0f ? 1.0f / mFogScale : 0.0f, 0.0f, 0.0f, 0.0f);
        rUbo.setData(3, &param, 2, 1);
    }

    rUbo.flushCurrentBuffer();

    mPointLightMgr->updateViewGPU(view, rContext);
    mSpotLightMgr->updateViewGPU(view, rContext);
    mProjLightMgr->updateViewGPU(view, rContext);

    for (const LightMgrBase* pMgr : mUserLightMgr)
    {
        if (pMgr)
        {
            pMgr->updateViewGPU(view, rContext);
        }
    }
}

TextureData* LightPrePass::createLightBuffer_(DrawContext* pDrawContext, s32 view, u32 width,
                                              u32 height, bool clear, bool temporary) const
{
    Context& rContext = const_cast<Context&>(mContext[view]);
    rContext.mBufferState |= 1;
    rContext.mRenderBuffer.setVirtualSize(sead::Vector2f(width, height));
    rContext.mRenderBuffer.setPhysicalArea(0.0f, 0.0f, width, height);

    auto type = utl::DynamicTextureAllocator::AllocateType(!temporary);

    if (!mFlags.isOn(1 << 8))
    {
        rContext.mpLightBufferTexture = utl::DynamicTextureAllocator::instance()->allocArray(
            pDrawContext, "LightPrePass", TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, width,
            height, 1, 1, nullptr, type, true, false);
        rContext.mLightBufferSampler.applyTextureData(*rContext.mpLightBufferTexture);
        rContext.mColorTarget[0].applyTextureData(*rContext.mpLightBufferTexture);
        rContext.mRenderBuffer.setRenderTargetColor(&rContext.mColorTarget[0], 0);
        rContext.mRenderBuffer.setRenderTargetColor(nullptr, 1);
    }
    else if (!mFlags.isOn(1 << 9))
    {
        rContext.mpLightBufferTexture = utl::DynamicTextureAllocator::instance()->allocArray(
            pDrawContext, "LightPrePass", TextureFormat::cTextureFormat_R11_G11_B10_float, width,
            height, 1, 1, nullptr, type, true, false);
        rContext.mLightBufferSampler.applyTextureData(*rContext.mpLightBufferTexture);
        rContext.mColorTarget[0].applyTextureData(*rContext.mpLightBufferTexture, 0, 0);
        rContext.mRenderBuffer.setRenderTargetColor(&rContext.mColorTarget[0], 0);
        rContext.mRenderBuffer.setRenderTargetColor(nullptr, 1);
    }
    else
    {
        rContext.mpLightBufferTexture = utl::DynamicTextureAllocator::instance()->allocArray(
            pDrawContext, "LightPrePass", TextureFormat::cTextureFormat_R11_G11_B10_float, width,
            height, 2, 1, nullptr, type, true, false);
        rContext.mLightBufferSampler.applyTextureData(*rContext.mpLightBufferTexture);
        rContext.mColorTarget[0].applyTextureData(*rContext.mpLightBufferTexture, 0, 0);
        rContext.mColorTarget[1].applyTextureData(*rContext.mpLightBufferTexture, 0, 1);
        rContext.mRenderBuffer.setRenderTargetColor(&rContext.mColorTarget[0], 0);
        rContext.mRenderBuffer.setRenderTargetColor(&rContext.mColorTarget[1], 1);
    }

    setDirty_();

    if (clear)
    {
        clearLightBuffer(pDrawContext, view);
    }

    return rContext.mpLightBufferTexture;
}

/**
 * Binds the light buffer of a view and clears it.
 * @param pDrawContext draw context
 * @param view view index
 */
void LightPrePass::clearLightBuffer(DrawContext* pDrawContext, s32 view) const
{
    Context& rContext = const_cast<Context&>(mContext[view]);
    rContext.mRenderBuffer.setRenderTargetDepth(nullptr);
    rContext.mRenderBuffer.bind(pDrawContext);
    sead::Viewport(rContext.mRenderBuffer).apply(pDrawContext, rContext.mRenderBuffer);

    if (mFlags.isOnAll(0x300))
    {
        rContext.mRenderBuffer.clear(pDrawContext, 0, 1, sead::Color4f(0.0f, 0.0f, 0.0f, 0.0f),
                                     1.0f, 0);
        rContext.mRenderBuffer.clear(pDrawContext, 1, 1, sead::Color4f(0.0f, 0.0f, 0.0f, 0.0f),
                                     1.0f, 0);
    }
    else
    {
        rContext.mRenderBuffer.clear(static_cast<sead::DrawContext*>(pDrawContext), 1,
                                     sead::Color4f(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);
    }

    setDirty_();
}

/**
 * Binds the light buffer of a view as the render target.
 * @param pDrawContext draw context
 * @param view view index
 * @param bindDepth whether to bind the depth target as well
 */
void LightPrePass::bindLightBuffer(DrawContext* pDrawContext, s32 view, bool bindDepth) const
{
    Context& rContext = const_cast<Context&>(mContext[view]);
    rContext.mRenderBuffer.setRenderTargetDepth(bindDepth ? &rContext.mDepthTarget : nullptr);
    rContext.mRenderBuffer.bind(pDrawContext);
    sead::Viewport viewport(rContext.mRenderBuffer);
    viewport.apply(pDrawContext, rContext.mRenderBuffer);
}

/**
 * Accumulates every visible light into the light buffer of a view.
 * @param pDrawContext draw context
 * @param view view index
 * @param rDepth depth texture of the scene
 * @param rDepthTarget depth target of the scene
 * @param rNormal normal texture of the scene
 */
void LightPrePass::draw(DrawContext* pDrawContext, s32 view, const TextureData& rDepth,
                        const RenderTargetDepth& rDepthTarget, const TextureData& rNormal) const
{
    setDirty_();
    Context& rContext = const_cast<Context&>(mContext[view]);
    rContext.mBufferState |= 4;
    rContext.mNormalSampler.applyTextureData(rNormal);
    rContext.mDepthSampler.applyTextureData(rDepth);

    if (!mFlags.isOn(1 << 20))
    {
        rContext.mDepthTarget = rDepthTarget;
    }

    if (!mFlags.isOn(1 << 20))
    {
        if (!(rContext.mBufferState & 1))
        {
            createLightBuffer_(pDrawContext, view, rDepthTarget.getMipWidth(0),
                               rDepthTarget.getMipHeight(0), true, true);
        }
        else
        {
            setDirty_();
            bindLightBuffer(pDrawContext, view, mFlags.isOn(1 << 5));
        }
    }

    if (mFlags.isOn(1 << 21))
    {
        driver::NVNMgr::instance()->enableTiledCaching(pDrawContext, mTileWidth, mTileHeight);
    }

    CallbackArg arg;
    arg.mLightPrePass = this;
    arg.mRenderBuffer = &rContext.mRenderBuffer;
    arg.mContext = &rContext;
    arg.mViewUbo = &rContext.mViewUbo;
    arg.mView = view;
    arg.mDepthSampler = &rContext.mDepthSampler;
    arg.mNormalSampler = &rContext.mNormalSampler;
    arg.mSpecPowSampler = &mSpecPowSampler;
    arg.mDrawContext = pDrawContext;

    if (!mFlags.isOn(1 << 20))
    {
        applyGraphicsContext(pDrawContext, arg, true, true);
    }

    if (mPreDrawCallback != nullptr)
    {
        mPreDrawCallback->invoke(arg);
    }

    if (!mFlags.isOn(1 << 20))
    {
        applyGraphicsContext(pDrawContext, arg, true, true);
    }

    if (isLightTypeEnabled_(mPointLightMgr->getLightType()))
    {
        mPointLightMgr->draw(pDrawContext, view, rContext, arg);
    }

    if (isLightTypeEnabled_(mSpotLightMgr->getLightType()))
    {
        mSpotLightMgr->draw(pDrawContext, view, rContext, arg);
    }

    if (isLightTypeEnabled_(mProjLightMgr->getLightType()))
    {
        mProjLightMgr->draw(pDrawContext, view, rContext, arg);
    }

    for (const LightMgrBase* pMgr : mUserLightMgr)
    {
        if (pMgr)
        {
            pMgr->draw(pDrawContext, view, rContext, arg);
        }
    }

    if (mPostDrawCallback != nullptr)
    {
        mPostDrawCallback->invoke(arg);
    }

    if (mFlags.isOn(1 << 21))
    {
        driver::NVNMgr::instance()->disableTiledCaching(pDrawContext);
    }

    if (!mFlags.isOn(1 << 20))
    {
        rContext.mColorTarget[0].invalidateGPUCache(pDrawContext);

        if (mFlags.isOn(1 << 8))
        {
            rContext.mColorTarget[1].invalidateGPUCache(pDrawContext);
        }
    }

    rContext.mRenderBuffer.setRenderTargetDepth(nullptr);
    setDirty_();

    if (rContext.mBufferState & 2)
    {
        rContext.mBufferState &= ~7u;
    }
}

/**
 * Chooses and applies the graphics context matching the depth and stencil state of a draw.
 * @param pDrawContext draw context
 * @param rArg callback argument of the current draw
 * @param stencil whether stencil testing is requested
 * @param force whether to apply even when the state did not change
 */
void LightPrePass::applyGraphicsContext(DrawContext* pDrawContext, const CallbackArg& rArg,
                                        bool stencil, bool force) const
{
    Context& rContext = const_cast<Context&>(mContext[rArg.mView]);
    bool hasDepth = rArg.mRenderBuffer->getRenderTargetDepth() != nullptr;
    bool useStencil = stencil & mFlags.isOn(1 << 9);

    if (!force && rContext.mCurrentDepthTest == hasDepth && rContext.mCurrentStencil == useStencil)
    {
        return;
    }

    rContext.mCurrentDepthTest = hasDepth;
    rContext.mCurrentStencil = useStencil;

    if (mFlags.isOn(1 << 8) && useStencil)
    {
        if (hasDepth)
        {
            mGraphicsContext[3].apply(pDrawContext);
        }
        else
        {
            mGraphicsContext[2].apply(pDrawContext);
        }
    }
    else if (hasDepth)
    {
        mGraphicsContext[1].apply(pDrawContext);
    }
    else
    {
        mGraphicsContext[0].apply(pDrawContext);
    }
}

/**
 * Draws the debug visualization of every light of a view.
 * @param pDrawContext draw context
 * @param view view index
 */
void LightPrePass::drawDebug(DrawContext* pDrawContext, s32 view) const
{
    if (!mFlags.isOn(0x480))
    {
        return;
    }

    const Context& rContext = mContext[view];
    sead::GraphicsContext graphicsContext;
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(pDrawContext);

    if (mFlags.isOn(1 << 7))
    {
        if (isLightTypeEnabled_(mPointLightMgr->getLightType()))
        {
            mPointLightMgr->drawDebug(pDrawContext, view, rContext);
        }

        if (isLightTypeEnabled_(mSpotLightMgr->getLightType()))
        {
            mSpotLightMgr->drawDebug(pDrawContext, view, rContext);
        }

        if (isLightTypeEnabled_(mProjLightMgr->getLightType()))
        {
            mProjLightMgr->drawDebug(pDrawContext, view, rContext);
        }

        for (const LightMgrBase* pMgr : mUserLightMgr)
        {
            if (pMgr)
            {
                pMgr->drawDebug(pDrawContext, view, rContext);
            }
        }
    }

    if (mFlags.isOn(1 << 10))
    {
        if (isLightTypeEnabled_(mPointLightMgr->getLightType()))
        {
            mPointLightMgr->drawDebugTest(pDrawContext, view, rContext);
        }

        if (isLightTypeEnabled_(mSpotLightMgr->getLightType()))
        {
            mSpotLightMgr->drawDebugTest(pDrawContext, view, rContext);
        }

        if (isLightTypeEnabled_(mProjLightMgr->getLightType()))
        {
            mProjLightMgr->drawDebugTest(pDrawContext, view, rContext);
        }

        for (const LightMgrBase* pMgr : mUserLightMgr)
        {
            if (pMgr)
            {
                pMgr->drawDebugTest(pDrawContext, view, rContext);
            }
        }
    }
}

/**
 * Releases the light buffer texture of a view.
 * @param view view index
 */
void LightPrePass::release(s32 view) const
{
    Context& rContext = const_cast<Context&>(mContext[view]);

    if (rContext.mpLightBufferTexture != nullptr)
    {
        utl::DynamicTextureAllocator::instance()->free(rContext.mpLightBufferTexture);
        rContext.mpLightBufferTexture = nullptr;
        setDirty_();
    }

    if (rContext.mBufferState & 4)
    {
        rContext.mBufferState &= ~7u;
    }
    else
    {
        rContext.mBufferState |= 2;
    }
}

/**
 * Sets the parameters of a point light.
 * @param index light index
 * @param rPos position
 * @param radius radius
 * @param rColor diffuse color
 * @param attnPow attenuation power
 * @param useSpecColor whether rSpecColor is used
 * @param rSpecColor specular color
 * @param attnStart attenuation start
 */
void LightPrePass::setPointLight(s32 index, const sead::Vector3f& rPos, f32 radius,
                                 const sead::Color4f& rColor, f32 attnPow, bool useSpecColor,
                                 const sead::Color4f& rSpecColor, f32 attnStart)
{
    PointLight& rLight = sead::DynamicCast<PointLightMgr>(mPointLightMgr)->getLight(index);
    rLight.mPos = rPos;
    rLight.mRadius = radius;
    rLight.mColor = rColor;
    rLight.mAttnPow = attnPow;
    rLight.mAttnStart = attnStart;

    if (useSpecColor)
    {
        rLight.mFlags.set(2);
        rLight.mVisibleMask = 0xffffffff;
        rLight.mSpecColor = rSpecColor;
    }
    else
    {
        rLight.mFlags.reset(2);
        rLight.mVisibleMask = 0xffffffff;
    }
}

/**
 * Gets a point light.
 * @param index light index
 * @return the point light
 */
LightPrePass::PointLight& LightPrePass::getPointLightStruct(s32 index)
{
    return sead::DynamicCast<PointLightMgr>(mPointLightMgr)->getLight(index);
}

/**
 * Gets a point light.
 * @param index light index
 * @return the point light
 */
const LightPrePass::PointLight& LightPrePass::getPointLightStruct(s32 index) const
{
    return sead::DynamicCast<PointLightMgr>(mPointLightMgr)->getLight(index);
}

/**
 * Sets the parameters of a spot light.
 * @param index light index
 * @param rPos position
 * @param rDir direction
 * @param angle cone angle
 * @param length cone length
 * @param rColor diffuse color
 * @param attnPow attenuation power
 * @param angleAttnPow angular attenuation power
 * @param useSpecColor whether rSpecColor is used
 * @param rSpecColor specular color
 */
void LightPrePass::setSpotLight(s32 index, const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                                f32 angle, f32 length, const sead::Color4f& rColor, f32 attnPow,
                                f32 angleAttnPow, bool useSpecColor,
                                const sead::Color4f& rSpecColor)
{
    SpotLight& rLight = sead::DynamicCast<SpotLightMgr>(mSpotLightMgr)->getLight(index);
    rLight.mPos = rPos;
    rLight.mDir = rDir;
    rLight.mAngle = angle;
    rLight.mLength = length;
    rLight.mColor = rColor;
    rLight.mVisibleMask = 0xffffffff;
    rLight.mAttnPow = attnPow;
    rLight.mAngleAttnPow = angleAttnPow;
    rLight.mShadowType = cShadowType_Normal;
    rLight.mShadowParam = 0.5f;
    rLight.mFlags.change(2, useSpecColor);
    rLight.mSpecColor = rSpecColor;

    for (auto& rView : rLight.mView)
    {
        rView.mShadowMap = nullptr;
    }

    rLight.mDir.normalize();
}

/**
 * Sets the shadow map of a spot light for a view.
 * @param index light index
 * @param view view index
 * @param pShadowMap shadow map sampler
 * @param rShadowMtx shadow matrix
 * @param blackBorder whether the border color is black instead of white
 */
void LightPrePass::setSpotLightShadowMap(s32 index, s32 view, const TextureSampler* pShadowMap,
                                         const sead::Matrix44f& rShadowMtx, bool blackBorder)
{
    SpotLight& rLight = sead::DynamicCast<SpotLightMgr>(mSpotLightMgr)->getLight(index);
    rLight.mView[view].mShadowMap = pShadowMap;
    rLight.mView[view].mShadowMtx = rShadowMtx;
    rLight.mView[view].mShadowSampler->setBorderColorDirect(blackBorder ? sead::Color4f::cBlack :
                                                                          sead::Color4f::cWhite);
}

/**
 * Sets the shadow type of a spot light.
 * @param index light index
 * @param type shadow type
 * @param param shadow parameter
 */
void LightPrePass::setSpotLightShadowType(s32 index, ShadowType type, f32 param)
{
    SpotLight& rLight = sead::DynamicCast<SpotLightMgr>(mSpotLightMgr)->getLight(index);
    rLight.mShadowType = type;
    rLight.mShadowParam = param;
}

/**
 * Gets a spot light.
 * @param index light index
 * @return the spot light
 */
LightPrePass::SpotLight& LightPrePass::getSpotLightStruct(s32 index)
{
    return sead::DynamicCast<SpotLightMgr>(mSpotLightMgr)->getLight(index);
}

/**
 * Gets a spot light.
 * @param index light index
 * @return the spot light
 */
const LightPrePass::SpotLight& LightPrePass::getSpotLightStruct(s32 index) const
{
    return sead::DynamicCast<SpotLightMgr>(mSpotLightMgr)->getLight(index);
}

/**
 * Sets the parameters of a perspective projection light.
 * @param index light index
 * @param rPos position
 * @param rDir direction
 * @param rUp up vector
 * @param rColor diffuse color
 * @param fovy vertical field of view
 * @param aspect aspect ratio
 * @param near near distance
 * @param far far distance
 * @param attnPow attenuation power
 * @param useSpecColor whether rSpecColor is used
 * @param rSpecColor specular color
 * @param pTexture projected texture, or nullptr
 * @param texWrap whether the projected texture wraps
 * @param rTexScale projected texture scale
 * @param rTexOffset projected texture offset
 */
void LightPrePass::setProjLight(s32 index, const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                                const sead::Vector3f& rUp, const sead::Color4f& rColor, f32 fovy,
                                f32 aspect, f32 near, f32 far, f32 attnPow, bool useSpecColor,
                                const sead::Color4f& rSpecColor, TextureSampler* pTexture,
                                bool texWrap, const sead::Vector2f& rTexScale,
                                const sead::Vector2f& rTexOffset)
{
    ProjLightMgr* pMgr = sead::DynamicCast<ProjLightMgr>(mProjLightMgr);
    ProjLight& rLight = pMgr->getLight(index);
    rLight.mPos = rPos;
    rLight.mDir = rDir;
    rLight.mUp = rUp;
    rLight.mColor = rColor;
    rLight.mSpecColor = rSpecColor;
    rLight.mParam[2] = near;
    rLight.mParam[3] = far;
    rLight.mParam[0] = fovy;
    rLight.mParam[1] = aspect;
    rLight.mAttnPow = attnPow;
    rLight.mTexScale = rTexScale;
    rLight.mTexOffset = rTexOffset;
    rLight.mFlags.change(2, useSpecColor);
    rLight.mFlags.reset(4);
    rLight.mVisibleMask = 0xffffffff;

    if (pTexture != nullptr)
    {
        rLight.mHasTexture = true;
        rLight.mTexture = *pTexture;

        if (texWrap)
        {
            rLight.mFlags.set(0x10);
        }
        else
        {
            rLight.mFlags.reset(0x10);
            rLight.mTexture.setWrap(5, 5, 5);
            rLight.mTexture.setBorderColor(sead::Color4f(0.0f, 0.0f, 0.0f, 0.0f));
        }
    }
    else
    {
        rLight.mHasTexture = false;
    }

    rLight.setNormVec();
    rLight.mShadowType = cShadowType_Normal;
    rLight.mShadowParam = 0.5f;

    for (auto& rView : rLight.mView)
    {
        rView.mShadowMap = nullptr;
    }

    pMgr->updateParameters_(rLight);
}

/**
 * Sets the parameters of an orthographic projection light.
 * @param index light index
 * @param rPos position
 * @param rDir direction
 * @param rUp up vector
 * @param rColor diffuse color
 * @param near near distance
 * @param far far distance
 * @param top top of the projection volume
 * @param bottom bottom of the projection volume
 * @param left left of the projection volume
 * @param right right of the projection volume
 * @param attnPow attenuation power
 * @param useSpecColor whether rSpecColor is used
 * @param rSpecColor specular color
 * @param pTexture projected texture, or nullptr
 * @param texWrap whether the projected texture wraps
 * @param rTexScale projected texture scale
 * @param rTexOffset projected texture offset
 */
void LightPrePass::setProjLight_Ortho(s32 index, const sead::Vector3f& rPos,
                                      const sead::Vector3f& rDir, const sead::Vector3f& rUp,
                                      const sead::Color4f& rColor, f32 near, f32 far, f32 top,
                                      f32 bottom, f32 left, f32 right, f32 attnPow,
                                      bool useSpecColor, const sead::Color4f& rSpecColor,
                                      TextureSampler* pTexture, bool texWrap,
                                      const sead::Vector2f& rTexScale,
                                      const sead::Vector2f& rTexOffset)
{
    ProjLightMgr* pMgr = sead::DynamicCast<ProjLightMgr>(mProjLightMgr);
    ProjLight& rLight = pMgr->getLight(index);
    rLight.mPos = rPos;
    rLight.mDir = rDir;
    rLight.mUp = rUp;
    rLight.mColor = rColor;
    rLight.mSpecColor = rSpecColor;
    rLight.mParam[0] = near;
    rLight.mParam[1] = far;
    rLight.mParam[4] = top;
    rLight.mParam[5] = bottom;
    rLight.mParam[6] = left;
    rLight.mParam[7] = right;
    rLight.mAttnPow = attnPow;
    rLight.mTexScale = rTexScale;
    rLight.mTexOffset = rTexOffset;
    rLight.mFlags.change(2, useSpecColor);
    rLight.mFlags.set(4);
    rLight.mVisibleMask = 0xffffffff;

    if (pTexture != nullptr)
    {
        rLight.mHasTexture = true;
        rLight.mTexture = *pTexture;

        if (texWrap)
        {
            rLight.mFlags.set(0x10);
        }
        else
        {
            rLight.mFlags.reset(0x10);
            rLight.mTexture.setWrap(5, 5, 5);
            rLight.mTexture.setBorderColor(sead::Color4f(0.0f, 0.0f, 0.0f, 0.0f));
        }
    }
    else
    {
        rLight.mHasTexture = false;
    }

    rLight.setNormVec();
    rLight.mShadowType = cShadowType_Normal;
    rLight.mShadowParam = 0.5f;

    for (auto& rView : rLight.mView)
    {
        rView.mShadowMap = nullptr;
    }

    pMgr->updateParameters_(rLight);
}

/**
 * Sets the shadow map of a projection light for a view.
 * @param index light index
 * @param view view index
 * @param pShadowMap shadow map sampler
 * @param rShadowMtx shadow matrix
 * @param blackBorder whether the border color is black instead of white
 */
void LightPrePass::setProjLightShadowMap(s32 index, s32 view, const TextureSampler* pShadowMap,
                                         const sead::Matrix44f& rShadowMtx, bool blackBorder)
{
    ProjLight& rLight = sead::DynamicCast<ProjLightMgr>(mProjLightMgr)->getLight(index);
    rLight.mView[view].mShadowMap = pShadowMap;
    rLight.mView[view].mShadowMtx = rShadowMtx;
    rLight.mView[view].mShadowSampler->setBorderColorDirect(blackBorder ? sead::Color4f::cBlack :
                                                                          sead::Color4f::cWhite);
}

/**
 * Sets the shadow type of a projection light.
 * @param index light index
 * @param type shadow type
 * @param param shadow parameter
 */
void LightPrePass::setProjLightShadowType(s32 index, ShadowType type, f32 param)
{
    ProjLight& rLight = sead::DynamicCast<ProjLightMgr>(mProjLightMgr)->getLight(index);
    rLight.mShadowType = type;
    rLight.mShadowParam = param;
}

/**
 * Gets a projection light.
 * @param index light index
 * @return the projection light
 */
LightPrePass::ProjLight& LightPrePass::getProjLightStruct(s32 index)
{
    return sead::DynamicCast<ProjLightMgr>(mProjLightMgr)->getLight(index);
}

/**
 * Gets a projection light.
 * @param index light index
 * @return the projection light
 */
const LightPrePass::ProjLight& LightPrePass::getProjLightStruct(s32 index) const
{
    return sead::DynamicCast<ProjLightMgr>(mProjLightMgr)->getLight(index);
}

/**
 * Fills a light info with the frustum description of a spot light.
 * @param index light index
 * @param pInfo destination info
 */
void LightPrePass::getSpotLightInfo(s32 index, LightInfo* pInfo) const
{
    const SpotLight& rLight = sead::DynamicCast<SpotLightMgr>(mSpotLightMgr)->getLight(index);
    pInfo->mType = cLightType_Spot;
    pInfo->mIsOrtho = 0;
    pInfo->mPos = rLight.mPos;
    pInfo->mDir = rLight.mDir;
    pInfo->mUp.set(0.0f, 1.0f, 0.0f);
    pInfo->mParam[0] = 0.1f;
    pInfo->mParam[1] = rLight.mLength;
    pInfo->mParam[2] = rLight.mAngle * 2;
    pInfo->mParam[3] = 1.0f;
}

/**
 * Fills a light info with the frustum description of a projection light.
 * @param index light index
 * @param pInfo destination info
 */
void LightPrePass::getProjLightInfo(s32 index, LightInfo* pInfo) const
{
    const ProjLight& rLight = sead::DynamicCast<ProjLightMgr>(mProjLightMgr)->getLight(index);
    pInfo->mType = cLightType_Proj;
    pInfo->mIsOrtho = rLight.mFlags.isOn(4);
    pInfo->mPos = rLight.mPos;
    pInfo->mDir = rLight.mDir;
    pInfo->mUp = rLight.mUp;

    for (s32 i = 0; i < 8; i++)
    {
        pInfo->mParam[i] = rLight.mParam[i];
    }
}

/**
 * Copies a specular curve and rebuilds the specular power texture.
 * @param rCurve source curve
 */
void LightPrePass::setSpecularCurve(const utl::ParameterCurve<2>& rCurve)
{
    mSpecularCurve.reset();
    mSpecularCurve.copyUnsafe(rCurve);
    updateSpecPowTex_();
}

/**
 * Looks up the light shader program of a variation.
 * @param type light type
 * @param a variation flag
 * @param b variation flag
 * @param c variation flag
 * @param d variation flag
 * @param shadowType shadow type
 * @param e variation flag
 * @param f variation flag
 * @return the shader program
 */
const ShaderProgram* LightPrePass::getShader(LightType type, bool a, bool b, bool c, bool d,
                                             ShadowType shadowType, bool e, bool f)
{
    const ShaderProgram* pProgram = nullptr;
    GetShader_(&pProgram, type, a, b, c, d, shadowType, e, f);
    return pProgram;
}

/**
 * Handles a host IO property event.
 * @param pEvent property event
 */
void LightPrePass::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if (!(pEvent->getType() & 2) && pEvent->getId() < &mFlags + 1 && pEvent->getId() >= &mFlags)
    {
        changeTextureFilter_();
    }

    if (reinterpret_cast<uintptr_t>(pEvent->getId()) == 10001)
    {
        mDirtyFlags |= 1;
    }

    mPointLightMgr->listenPropertyEvent(pEvent);
    mSpotLightMgr->listenPropertyEvent(pEvent);
    mProjLightMgr->listenPropertyEvent(pEvent);
    updateSpecPowTex_();
}

}  // namespace agl::lght

