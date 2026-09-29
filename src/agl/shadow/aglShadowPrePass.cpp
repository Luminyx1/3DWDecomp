#include "shadow/aglShadowPrePass.h"

#include <gfx/seadViewport.h>

#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "detail/aglRootNode.h"
#include "shadow/aglShadowMathUtil.h"
#include "utility/aglDynamicTextureAllocator.h"

namespace agl::sdw
{

/**
 * Constructs the shadow pre-pass with its default parameters.
 */
ShadowPrePass::ShadowPrePass()
    : utl::IParameterIO("aglshpp", 0), mIsEnable(true, "is_enable", "有効", &mParamObj),
      mResolutionMode(0, "resolutionMode", "バッファ解像度", &mParamObj),
      mScreenSpaceBlurType(0, "screenSpaceBlurType", "ブラータイプ", &mParamObj),
      mScreenSpaceBlurWidth(0, "screenSpaceBlurWidth", "ブラーサンプリング数", &mParamObj),
      mScreenSpaceBlurRepNum(1, "mScreenSpaceBlurRepNum", "ブラー回数", &mParamObj),
      mPcfWidth(5.0f, "pcfWidth", "PCFブラー幅", &mParamObj),
      mUseStaticDepthShadow(false, "is_useStaticDepthShadow", "静的デプスシャドウを使用",
                            &mParamObj),
      mUseDecalAo(false, "is_useDecalAo", "デカールAOを使用", &mParamObj),
      mUseDecalTrailSigned(false, "is_UseDecalTrailSigned", "Rチャンネルに符号付きで濃淡を追加描画",
                           &mParamObj),
      mUseFarFade(false, "is_useFarFade", "遠距離フェードを使用", &mParamObj),
      mDynamicShadowFarFadeStart(100.0f, "dynamicShadowFarFadeStart",
                                 "動的シャドウ遠距離フェード開始位置", &mParamObj),
      mDynamicShadowFarFadeEnd(1000.0f, "dynamicShadowFarFadeEnd",
                               "動的シャドウ遠距離フェード終了位置", &mParamObj),
      mStaticShadowFarFadeStart(100.0f, "staticShadowFarFadeStart",
                                "静的シャドウ遠距離フェード開始位置", &mParamObj),
      mStaticShadowFarFadeEnd(1000.0f, "staticShadowFarFadeEnd",
                              "静的シャドウ遠距離フェード終了位置", &mParamObj),
      mUseDepth2Normal(false, "is_useDepth2Normal", "デプスから法線を生成", &mParamObj),
      mUseDepth2NormalBlur(false, "is_useDepth2NormalBlur", "生成した法線を一緒にぼかす",
                           &mParamObj),
      mNormal2ShadowRatio(0.0f, "normal2ShadowRatio", "法線をシャドウに反映する角度", &mParamObj),
      mNormal2ShadowMul(1.0f, "normal2ShadowMul", "法線をシャドウに反映する影濃度", &mParamObj),
      mFaceNormalBias(20.0f, "faceNormalBias", "光源に直行した面のシャドウ補正距離", &mParamObj),
      mUseMipLevelBlur(false, "is_useMipLevelBlur", "ミップマップにぼけた絵を生成", &mParamObj),
      mUseMipLevelBlurReduce(false, "is_useMipLevelBlurReduce", "ミップマップのぼけを縮小して生成",
                             &mParamObj),
      mMipBlurWidth(0, "mipBlurWidth", "ミップマップのブラーサンプリング数", &mParamObj),
      mMipBlurRepNum(1, "mipBlurRepNum", "ミップマップのブラー回数", &mParamObj),
      mUsePreCombSsao(false, "is_usePreCombSsao", "SSAOを一緒にぼかす", &mParamObj),
      mUseFarDepthTest(false, "is_useFarDepthTest", "指定Z座標でデプステスト", &mParamObj),
      mFarDepthTestDist(1000.0f, "is_farDepthTestDist", "デプステストを行うZ座標", &mParamObj),
      mPcfShaderType(0, "is_PcfShaderType", "PCF設定", &mParamObj),
      mPcfSampleNum(0, "is_PcfSampleNum", "PCFサンプリング数設定", &mParamObj)
{
    addObj(&mParamObj, "ShadowPrePass");
    agl::detail::RootNode::setNodeMeta(this, "Icon=EFFECT");
    *mIsEnable = true;
    _830 = 0;
    _834 = sead::Vector4f::zero;
    mBufferIndex = 0;
}

/**
 * Destroys the contexts and the debug page.
 */
ShadowPrePass::~ShadowPrePass()
{
    mContexts.freeBuffer();
    mDebugTexturePage.cleanUp();
}

/**
 * Allocates the per-view contexts and sets up their uniform blocks and samplers.
 * @param contextNum number of contexts
 * @param pHeap heap to allocate from
 */
void ShadowPrePass::initialize(s32 contextNum, sead::Heap* pHeap)
{
    mContexts.tryAllocBuffer(contextNum, pHeap);

    for (auto it = mContexts.begin(), end = mContexts.end(); it != end; ++it)
    {
        Context& context = *it;
        if (it.getIndex() == 0)
        {
            UniformBlock& block = context.mUniformBlock;
            block.startDeclare(21, pHeap);
            block.declare(UniformBlock::cType_Vec4, 12);
            block.declare(UniformBlock::cType_Vec4, 4);
            block.declare(UniformBlock::cType_Vec4, 1);
            block.declare(UniformBlock::cType_Vec4, 1);
            block.declare(UniformBlock::cType_Vec2, 1);
            block.declare(UniformBlock::cType_Vec2, 1);
            block.declare(UniformBlock::cType_Vec2, 1);
            block.declare(UniformBlock::cType_Vec2, 1);
            block.declare(UniformBlock::cType_Float, 3);
            block.declare(UniformBlock::cType_Float, 1);
            block.declare(UniformBlock::cType_Float, 1);
            block.declare(UniformBlock::cType_Float, 1);
            block.declare(UniformBlock::cType_Float, 1);
            block.declare(UniformBlock::cType_Float, 1);
            block.declare(UniformBlock::cType_Float, 1);
            block.declare(UniformBlock::cType_Float, 1);
            block.declare(UniformBlock::cType_Float, 1);
            block.declare(UniformBlock::cType_Float, 1);
            block.declare(UniformBlock::cType_Float, 1);
            block.declare(UniformBlock::cType_Float, 1);
            block.declare(UniformBlock::cType_Float, 1);
        }
        else
        {
            context.mUniformBlock.declare(mContexts.front().mUniformBlock);
        }
        context.mUniformBlock.create(pHeap, 2, 1);

        context.mDepthSampler.setFilter(0, 0, 0);
        context.mShadowSampler.setWrap(5, 5, 5);
        context.mShadowSampler.setBorderColorAsColor(sead::Color4f::cWhite);
        context.mShadowSampler.setDepthCompareEnable(true);
        context.mShadowSampler.setDepthCompareFunc(4);
        context.mStaticShadowSampler = context.mShadowSampler;

        context.mLightBuffer = nullptr;
        context.mTempBuffer = nullptr;
        context.mIsCreated = false;
        context.mIsReleased = false;
        context.mBufferState = 0xffffffff;
        context.mHalfBuffers[1] = nullptr;
        context.mHalfBuffers[0] = nullptr;
        context.mStaticDepth = nullptr;
        context._1880 = nullptr;
        context.mResolutionScale = 1.0f;
    }

    mDebugTexturePage.setUp(contextNum, "ShadowPrePass", pHeap);
}

/**
 * Flips the uniform block buffers of every context.
 */
void ShadowPrePass::calc()
{
    mBufferIndex = 1 - mBufferIndex;
    for (auto& context : mContexts)
    {
        context.mUniformBlock.setCurrentBufferIndex(mBufferIndex);
    }
}

/**
 * Does nothing.
 */
void ShadowPrePass::calcGPU() const {}

/**
 * Uploads the static depth shadow matrix of a context.
 * @param index context index
 * @param pProjMtx static shadow projection matrix
 * @param rViewMtx camera view matrix
 */
void ShadowPrePass::calcGPU_StaticDepthShadow(s32 index, const sead::Matrix44f* pProjMtx,
                                              const sead::Matrix34f& rViewMtx)
{
    Context& context = mContexts[index];
    const sead::Matrix44f viewMtx(rViewMtx);
    sead::Matrix44f invViewMtx;
    invViewMtx.setInverse(viewMtx);
    sead::Matrix44f mtx;
    detail::multiplyMtx44(mtx, *pProjMtx, invViewMtx);
    context.mUniformBlock.setData(1, &mtx, 0, 4);
    context.mUniformBlock.flushCurrentBuffer();
}

/**
 * Selects the texture formats of the light buffer and the temporary buffer.
 * @param index context index
 * @param rFormat light buffer format
 * @param rTempFormat temporary buffer format
 */
void ShadowPrePass::getTextureFormat_(s32 index, TextureFormat& rFormat,
                                      TextureFormat& rTempFormat) const
{
    const bool hasStaticDepth = mFlags.isOnBit(0) && mContexts[index].mStaticDepth != nullptr;
    rFormat = TextureFormat::cTextureFormat_R8_G8_uNorm;
    rTempFormat = TextureFormat::cTextureFormat_R8_G8_uNorm;
    if (hasStaticDepth)
    {
        if (*mUseStaticDepthShadow || *mUseDecalAo || *mUseDepth2Normal || *mUsePreCombSsao)
        {
            rFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
            rTempFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
        }
        else
        {
            rFormat = TextureFormat::cTextureFormat_R8_G8_uNorm;
        }
    }
    else if (*mUseStaticDepthShadow || *mUseDecalAo || *mUseDepth2Normal || *mUsePreCombSsao)
    {
        rFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
        rTempFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
    }
}

/**
 * Releases the light buffer of a context.
 * @param index context index
 */
void ShadowPrePass::release(s32 index) const
{
    Context& context = mContexts[index];
    if (context.mLightBuffer)
    {
        utl::DynamicTextureAllocator::instance()->free(context.mLightBuffer);
        context.mLightBuffer = nullptr;
        context.mIsReleased = true;
    }
    context.mIsCreated = false;
}

/**
 * Clears the temporary buffer of a context to white.
 * @param pDrawContext draw context
 * @param index context index
 */
void ShadowPrePass::clearShadowBuffer(DrawContext* pDrawContext, s32 index) const
{
    const Context& context = mContexts[index];
    context.mTempRenderBuffer.bind(pDrawContext);
    sead::Viewport viewport(context.mTempRenderBuffer);
    viewport.apply(pDrawContext, context.mTempRenderBuffer);
    context.mTempRenderBuffer.clear(pDrawContext, 1, sead::Color4f(1.0f, 1.0f, 1.0f, 1.0f), 1.0f,
                                    0);
}

/**
 * Returns whether the screen space blur is enabled.
 * @return 1 when a screen space blur pass is used, 0 otherwise
 */
s32 ShadowPrePass::getPassType() const
{
    return *mScreenSpaceBlurType != 0;
}

/**
 * Returns the sampler holding the result of the previous pass.
 * @param pDrawContext draw context
 * @param index context index
 * @param pass pass index
 * @return the sampler of the previous pass
 */
const TextureSampler* ShadowPrePass::getPrevSampler(DrawContext* pDrawContext, s32 index,
                                                    s32 pass) const
{
    const Context& context = mContexts[index];
    if (getPassType() != 0)
    {
        switch (pass)
        {
        case 2:
            context.mRenderTarget.invalidateGPUCache(pDrawContext);
            return &context.mLightBufferSampler;
        case 1:
            context.mTempRenderTarget.invalidateGPUCache(pDrawContext);
            return &context.mTempSampler;
        default:
            break;
        }
    }
    if (pass == 0)
    {
        context.mRenderTarget.invalidateGPUCache(pDrawContext);
    }
    return &context.mLightBufferSampler;
}

/**
 * Returns the render buffer to render a pass into.
 * @param pDrawContext draw context
 * @param index context index
 * @param pass pass index
 * @return the render buffer of the pass
 */
RenderBuffer* ShadowPrePass::getRenderTarget(DrawContext* pDrawContext, s32 index, s32 pass) const
{
    Context& context = mContexts[index];
    if (getPassType() != 0)
    {
        switch (pass)
        {
        case 2:
            context.mRenderTarget.invalidateGPUCache(pDrawContext);
            return &context.mRenderBuffer;
        case 1:
            context.mTempRenderTarget.invalidateGPUCache(pDrawContext);
            return &context.mTempRenderBuffer;
        default:
            break;
        }
    }
    if (pass == 0)
    {
        context.mRenderTarget.invalidateGPUCache(pDrawContext);
    }
    return &context.mRenderBuffer;
}

/**
 * Binds the render buffer of a pass and applies its viewport.
 * @param pDrawContext draw context
 * @param index context index
 * @param pass pass index
 * @return the bound render buffer
 */
RenderBuffer* ShadowPrePass::bindBuffer(DrawContext* pDrawContext, s32 index, s32 pass) const
{
    RenderBuffer* pRenderBuffer = getRenderTarget(pDrawContext, index, pass);
    pRenderBuffer->bind(pDrawContext);
    sead::Viewport viewport(*pRenderBuffer);
    viewport.apply(pDrawContext, *pRenderBuffer);
    return pRenderBuffer;
}

/**
 * Computes the flags describing which buffers a context needs.
 * @param index context index
 * @return the buffer state flags
 */
u32 ShadowPrePass::getBufferState(s32 index) const
{
    u32 state = *mResolutionMode == 0 ? 1 : 0;
    if (*mResolutionMode == 1)
    {
        state |= 2;
    }
    if (mFlags.isOnBit(0) && mContexts[index].mStaticDepth != nullptr)
    {
        state |= 8;
    }
    if (*mScreenSpaceBlurType != 0)
    {
        state |= 0x10;
    }
    if (*mUseMipLevelBlur)
    {
        state |= 0x20;
    }
    return state;
}

/**
 * Creates (if needed) the buffers of a context for the given screen size.
 * @param pDrawContext draw context
 * @param index context index
 * @param width screen width
 * @param height screen height
 * @return the light buffer render buffer, or nullptr when disabled
 */
RenderBuffer* ShadowPrePass::createShadowBuffer(DrawContext* pDrawContext, s32 index, u32 width,
                                                u32 height) const
{
    if (!*mIsEnable)
    {
        return nullptr;
    }

    Context& context = mContexts[index];
    const u32 state = getBufferState(index);
    bool needCreate = true;
    if (context.mBufferState == state)
    {
        needCreate = !context.mIsCreated;
    }
    else
    {
        context.mBufferState = state;
        context.mIsCreated = false;
    }

    const f32 scale = context.mResolutionScale;
    const u32 bufferWidth = scale * width;
    const u32 bufferHeight = scale * height;
    context.mInvWidth = 1.0f / bufferWidth;
    context.mInvHeight = 1.0f / bufferHeight;

    if (needCreate)
    {
        createShadowBuffer_core_(pDrawContext, index, bufferWidth, bufferHeight, false);
    }
    createTempBuffer_(pDrawContext, index, bufferWidth, bufferHeight);

    Context& result = mContexts[index];
    result.mRenderTarget.invalidateGPUCache(pDrawContext);
    return &result.mRenderBuffer;
}

/**
 * Returns the PCF shader variation number.
 * @return the variation number, 0 when the default shader is used
 */
s32 ShadowPrePass::getPcfShaderNo() const
{
    if (*mPcfShaderType == 1)
    {
        switch (*mPcfSampleNum)
        {
        case 0:
            return 1;
        case 1:
            return 2;
        case 2:
            return 3;
        default:
            break;
        }
    }
    return 0;
}

/**
 * Computes the shader variation index from the macro values.
 * @param rProgram shader program
 * @param value0 value of macro 3
 * @param value1 value of macro 2
 * @param value2 value of macro 1
 * @param value3 value of macro 0
 * @param value4 value of macro 4
 * @return the variation index
 */
s32 ShadowPrePass::getShaderIndex(const ShaderProgram& rProgram, u32 value0, u32 value1, u32 value2,
                                  u32 value3, u32 value4) const
{
    return rProgram.getVariationMacroStride(0) * value3 +
           rProgram.getVariationMacroStride(1) * value2 +
           rProgram.getVariationMacroStride(2) * value1 +
           rProgram.getVariationMacroStride(3) * value0 +
           rProgram.getVariationMacroStride(4) * value4;
}

/**
 * Generates the host IO message.
 * @param pContext host IO context
 */
void ShadowPrePass::genMessage(sead::hostio::Context* pContext)
{
    genMessageIO(pContext, 0xf);
    mDebugTexturePage.genMessagePage(pContext, this);

    mIsEnable.genMessageParameter(pContext, mIsEnable.getMeta());
    {
        const sead::SafeString label = mResolutionMode.getLabel();
    }
    mUseStaticDepthShadow.genMessageParameter(pContext, mUseStaticDepthShadow.getMeta());
    mUseDecalAo.genMessageParameter(pContext, mUseDecalAo.getMeta());
    mUseDecalTrailSigned.genMessageParameter(pContext, mUseDecalTrailSigned.getMeta());
    {
        const sead::SafeString label = mPcfShaderType.getLabel();
    }
    {
        const sead::SafeString label = mPcfSampleNum.getLabel();
    }
    mPcfWidth.genMessageParameter(pContext, "Min = 0,Max = 10.0");
    {
        const sead::SafeString label = mScreenSpaceBlurType.getLabel();
    }
    mScreenSpaceBlurWidth.genMessageParameter(pContext, "Min = 0,Max = 5");
    mScreenSpaceBlurRepNum.genMessageParameter(pContext, "Min = 1,Max = 20");
    mUseMipLevelBlur.genMessageParameter(pContext, mUseMipLevelBlur.getMeta());
    mUseMipLevelBlurReduce.genMessageParameter(pContext, mUseMipLevelBlurReduce.getMeta());
    mMipBlurWidth.genMessageParameter(pContext, "Min = 0,Max = 5");
    mMipBlurRepNum.genMessageParameter(pContext, "Min = 1,Max = 20");
    mUsePreCombSsao.genMessageParameter(pContext, mUsePreCombSsao.getMeta());
    mUseFarFade.genMessageParameter(pContext, mUseFarFade.getMeta());
    mDynamicShadowFarFadeStart.genMessageParameter(pContext, "Min = 0,Max = 1000.0");
    mDynamicShadowFarFadeEnd.genMessageParameter(pContext, "Min = 0,Max = 1000.0");
    mStaticShadowFarFadeStart.genMessageParameter(pContext, "Min = 0,Max = 1000.0");
    mStaticShadowFarFadeEnd.genMessageParameter(pContext, "Min = 0,Max = 1000.0");
    mUseFarDepthTest.genMessageParameter(pContext, mUseFarDepthTest.getMeta());
    mFarDepthTestDist.genMessageParameter(pContext, mFarDepthTestDist.getMeta());
    mUseDepth2Normal.genMessageParameter(pContext, mUseDepth2Normal.getMeta());
    mUseDepth2NormalBlur.genMessageParameter(pContext, mUseDepth2NormalBlur.getMeta());
    mNormal2ShadowRatio.genMessageParameter(pContext, "Min = 0,Max = 1.0");
    mNormal2ShadowMul.genMessageParameter(pContext, "Min = 0,Max = 5.0");
    mFaceNormalBias.genMessageParameter(pContext, "Min = 0,Max = 50.0");
    mIsEnable.genMessageParameter(pContext, mIsEnable.getMeta());
}

/**
 * Handles a host IO property event.
 * @param pEvent property event
 */
void ShadowPrePass::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    listenPropertyEventIO(this, pEvent);
}

}  // namespace agl::sdw
