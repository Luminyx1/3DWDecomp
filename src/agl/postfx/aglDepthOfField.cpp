#include "postfx/aglDepthOfField.h"

#include <cmath>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <math/seadBoundBox.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadSafeString.h>
#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "detail/aglRootNode.h"
#include "detail/aglShaderHolder.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglDynamicTextureAllocator.h"
#include "utility/aglImageFilter2D.h"

namespace agl::pfx {

namespace {

inline void calcRange(f32* pMin, f32* pMax, f32 a, f32 b)
{
    *pMin = a < b ? a : b;
    *pMax = a > b ? a : b;
    if (*pMax - *pMin < 0.1f)
    {
        *pMax += 0.1f;
    }
}

inline void calcNearRange(f32* pMin, f32* pMax, f32 a, f32 b)
{
    *pMax = a > b ? a : b;
    *pMin = a < b ? a : b;
    if (*pMax - *pMin < 0.1f)
    {
        *pMax += 0.1f;
    }
}

inline s32 getMipHeight(const TextureData& rTexture, s32 mipLevel)
{
    s32 min = rTexture.getMinHeight_();
    s32 height = rTexture.getHeight() >> mipLevel;
    return min > height ? min : height;
}

inline f32 calcProjDepth(f32 z, f32 near, f32 far)
{
    f32 zNear = z * near;
    return (zNear * (near + far) - near * 2.0f * near * far) / (zNear * (far - near));
}

}  // namespace

DepthOfFieldParameter::DepthOfFieldParameter()
{
    mIndirectMatrix[0][0] = 1.0f;
    mIndirectMatrix[0][1] = 0.0f;
    mIndirectMatrix[0][2] = 0.0f;
    mIndirectMatrix[1][0] = 0.0f;
    mIndirectMatrix[1][1] = 1.0f;
    mIndirectMatrix[1][2] = 0.0f;
}

void DepthOfFieldParameter::initialize(utl::IParameterObj* pObj, sead::Heap* pHeap)
{
    mLevel.init(2.0f, "level", "Blur level", "Min=0, Max=6", pObj);
    mStart.init(192.0f, "start", "DOF start", pObj);
    mEnd.init(200.0f, "end", "DOF end", pObj);
    mNearEnable.init(false, "near_enable", "Near enable", pObj);
    mFarEnable.init(true, "far_enable", "Far enable", pObj);
    mDepthBlur.init(false, "depth_blur", "Depth blur", pObj);
    mEnableVignettingColor.init(false, "enable_vignetting_color", "Enable vignette color", pObj);
    mEnableVignettingBlur.init(false, "enable_vignetting_blur", "Enable vignette blur", pObj);
    mEnableVignetting2Shape.init(false, "enable_vignetting_2_shape", "Enable vignette shape",
                                 pObj);
    mEnableColorControl.init(false, "enable_color_control", "Enable color control", pObj);
    mEnableColorReverse.init(true, "enable_color_reverse", "Enable color reverse", pObj);
    mIndirectEnable.init(false, "indirect_enable", "Enable indirect", pObj);
    mIndirectDepthCancelEnable.init(false, "indirect_depth_cancelenable",
                                    "Enable indirect depth cancel", pObj);
    mEnableReduceDraw.init(false, "enable_reduce_draw", "Enable reduced draw", pObj);
    mFarStart.init(120.0f, "far_start", "DOF start", pObj);
    mFarEnd.init(0.0f, "far_end", "DOF end", pObj);
    mDepthBlurAdd.init(0.0f, "depth_blur_add", "Depth blur add", pObj);
    mSaturateMin.init(1.0f, "saturate_min", "Saturate min", pObj);
    mColorCtrlDepth.init(sead::Vector4f(1000.0f, 2000.0f, 3000.0f, 4000.0f), "color_ctrl_depth",
                         "Color control depth", pObj);
    mIndirectTexTrans.init(sead::Vector2f::zero, "indirect_tex_trans",
                           "Indirect texture translation", pObj);
    mIndirectTexRotate.init(0.0f, "indirect_tex_rotate", "Indirect texture rotation", pObj);
    mIndirectTexScale.init(sead::Vector2f::ones, "indirect_tex_scale", "Indirect texture scale",
                           pObj);
    mIndirectScale.init(0.2f, "indirect_scale", "Indirect scale", pObj);
    mVignettingBlur.init(1.0f, "vignetting_blur", "Vignette blur", pObj);
    mVignettingBlend.init(0, "vignetting_blend", "Vignette blend type", pObj);
    mVignettingColor.init(sead::Color4f(0.0f, 0.0f, 0.0f, 0.75f), "vignetting_color",
                          "Vignette color", pObj);
    mFarMulColor.init(sead::Color4f(1.0f, 1.0f, 1.0f, 1.0f), "far_mul_color",
                      "Far multiply color", pObj);
    mVignettingShape0.initialize("vignetting_shape_0", pObj, pHeap);
    mVignettingShape1.initialize("vignetting_shape_1", pObj, pHeap);
    mEnableDofFarMax.init(false, "enable_dof_far_max", "Enable DOF far max", pObj);
    mDofFarMax.init(200.0f, "dof_far_max", "DOF far max", "Max=100000", pObj);
    mEnableIndirectFromFull.init(false, "enable_indirect_from_full",
                                 "Enable indirect full buffer resolution", pObj);
}

void DepthOfFieldParameter::VignettingShapeParam::initialize(const sead::SafeString& rName,
                                                             utl::IParameterObj* pObj,
                                                             sead::Heap* pHeap)
{
    mType.init(0, sead::FormatFixedSafeString<32>("%s_type", rName.cstr()), "Shape", pObj);
    mRange.init(sead::Vector2f(0.25f, 1.0f),
                sead::FormatFixedSafeString<32>("%s_range", rName.cstr()), "Range", pObj);
    mScale.init(sead::Vector2f(1.0f, 1.0f),
                sead::FormatFixedSafeString<32>("%s_scale", rName.cstr()), "Scale", pObj);
    mTrans.init(sead::Vector2f(0.0f, 0.0f),
                sead::FormatFixedSafeString<32>("%s_trans", rName.cstr()), "Offset", pObj);
}

DepthOfField::DepthOfField() : IParameterIO("agldof", 0)
{
    agl::detail::RootNode::setNodeMeta(this, "Icon=SNAPSHOT");
}

DepthOfField::~DepthOfField() = default;

void DepthOfField::initialize(s32 contextNum, sead::Heap* pHeap)
{
    initializeContextParameterBuffer(contextNum, false, pHeap);
    mEnable.init(true, "enable", "Enable", &mParameterObjs[0]);
    addObj(&mParameterObjs[0], "dof");
    assignShaderProgram_();
    updateIndirectMatrix_();
    copyParameterToAllContext(0);
    initVertex_(pHeap);
    initIndex_(pHeap);
    mDebugTexturePage.setUp(contextNum, "DepthOfField", pHeap);
}

void DepthOfFieldParameter::assignShaderProgram_()
{
    auto* pHolder = agl::detail::ShaderHolder::instance();

    {
        const ShaderProgram* pProgram =
            pHolder->getShaderProgram(agl::detail::ShaderHolder::cDofMipmap);
        mpMipMapProgram = pProgram->getVariation(pProgram->getVariationMacroStride(0) * 2);
        mpDepthMipMapProgram = pProgram->getVariation(pProgram->getVariationMacroStride(1));
    }

    bool isDepthBlur = enableDepthBlur_();
    s32 nearMode = 0;
    if (*mNearEnable && *mLevel > 0.0f)
    {
        nearMode = isDepthBlur ? 2 : 1;
    }

    s32 vignettingMode = 0;
    if (*mEnableVignettingBlur && *mLevel > 0.0f)
    {
        if (!*mEnableVignettingColor || enableSeparateVignettingPass_())
        {
            vignettingMode = 1;
        }
        else
        {
            vignettingMode = mVignettingColor->r == 0.0f && mVignettingColor->g == 0.0f &&
                                     mVignettingColor->b == 0.0f ?
                                 2 :
                                 3;
        }
    }

    s32 farColorMode = 0;
    if (*mFarEnable && (*mLevel > 0.0f || enableMipFromZeroLevel_()))
    {
        farColorMode = *mEnableDofFarMax ? 2 : 1;
        if (*mEnableColorControl)
        {
            if (*mSaturateMin != 1.0f)
            {
                farColorMode += 2;
            }

            if (mFarMulColor->r != 1.0f || mFarMulColor->g != 1.0f || mFarMulColor->b != 1.0f)
            {
                farColorMode += 4;
            }
        }
    }

    s32 indirectMode = 0;
    if (enableIndirect_())
    {
        indirectMode = *mIndirectDepthCancelEnable ? 2 : 1;
    }

    {
        const ShaderProgram* pProgram =
            pHolder->getShaderProgram(agl::detail::ShaderHolder::cDofDepthMask);
        s32 variation = pProgram->getVariationMacroStride(0) *
                            (farColorMode != 0 ? (*mEnableDofFarMax ? 2 : 1) : 0) +
                        pProgram->getVariationMacroStride(1) * nearMode +
                        pProgram->getVariationMacroStride(2) * *mEnableVignettingBlur;
        mpDepthMaskPrograms[0] = pProgram->getVariation(variation);
        mpDepthMaskPrograms[1] = pProgram->getVariation(variation | 1);
    }

    {
        const ShaderProgram* pProgram =
            pHolder->getShaderProgram(agl::detail::ShaderHolder::cDofFinal);
        s32 variation = pProgram->getVariationMacroStride(0) * farColorMode +
                        pProgram->getVariationMacroStride(1) * nearMode +
                        pProgram->getVariationMacroStride(2) * vignettingMode +
                        pProgram->getVariationMacroStride(3) * indirectMode;
        mpFinalPrograms[0] = pProgram->getVariation(variation);
        mpFinalPrograms[1] = pProgram->getVariation(variation | 1);
    }

    {
        const ShaderProgram* pProgram =
            pHolder->getShaderProgram(agl::detail::ShaderHolder::cDofVignetting);
        bool isColor = mVignettingColor->r != 0.0f || mVignettingColor->g != 0.0f ||
                       mVignettingColor->b != 0.0f;
        s32 blendMode = *mVignettingBlend == cVignettingBlendType_Mul    ? 1 :
                        *mVignettingBlend == cVignettingBlendType_Screen ? 2 :
                                                                           0;
        s32 variation = pProgram->getVariationMacroStride(0) * isColor +
                        pProgram->getVariationMacroStride(1) * blendMode;
        mpVignettingProgram = pProgram->getVariation(variation);
    }

    {
        const ShaderProgram* pProgram =
            pHolder->getShaderProgramUnsafe(agl::detail::ShaderHolder::cDofExpandReduce);
        s32 variation =
            pProgram->getVariationMacroStride(0) * (!*mEnableVignettingBlur && *mFarEnable) +
            pProgram->getVariationMacroStride(1) *
                (*mNearEnable && !*mEnableVignettingBlur && !*mDepthBlur);
        mpExpandReduceProgram = pProgram->getVariation(variation);
    }
}

void DepthOfFieldParameter::updateIndirectMatrix_()
{
    sead::Matrix34f mtx;
    mtx.makeR(sead::Vector3f(0.0f, 0.0f, *mIndirectTexRotate));
    mtx.scaleBases(mIndirectTexScale->x, mIndirectTexScale->y, 0.0f);
    sead::Vector3f trans;
    trans.setMul(mtx, sead::Vector3f(mIndirectTexTrans->x, mIndirectTexTrans->y, 0.0f));
    mIndirectMatrix[0][0] = mtx(0, 0);
    mIndirectMatrix[0][1] = mtx(0, 1);
    mIndirectMatrix[0][2] = trans.x;
    mIndirectMatrix[1][0] = mtx(1, 0);
    mIndirectMatrix[1][1] = mtx(1, 1);
    mIndirectMatrix[1][2] = trans.y;
}

void DepthOfField::initVertex_(sead::Heap* pHeap)
{
    const s32 cDivNum[] = {32, 4};
    const f32 cRingScale[] = {1.0f, 1.0f, 0.0f, 0.0f};
    for (s32 i = 0; i < 2; i++)
    {
        s32 divNum = cDivNum[i];
        Shape& rShape = mShapes[i];
        rShape.mVertex.allocBuffer(divNum * 4, pHeap, 8, MemoryAttribute(0));
        GPUMemAddr<Vertex> addr(rShape.mVertex, 0);
        s32 index = 0;
        for (s32 ring = 0; ring < 4; ring++)
        {
            switch (i)
            {
            case 0:
                for (s32 j = 0; j < divNum; j++)
                {
                    f32 angle = j * sead::Mathf::pi2() / divNum;
                    detail::getBufferPtr<Vertex>(rShape.mVertex)[index].mPos = sead::Vector2f(sead::Mathf::cos(angle), sead::Mathf::sin(angle));
                    detail::getBufferPtr<Vertex>(rShape.mVertex)[index].mParam = sead::Vector2f(cRingScale[ring], ring);
                    index++;
                }

                break;
            case 1:
                detail::getBufferPtr<Vertex>(rShape.mVertex)[index].mPos = sead::Vector2f(-1.0f, 1.0f);
                detail::getBufferPtr<Vertex>(rShape.mVertex)[index].mParam = sead::Vector2f(cRingScale[ring], ring);
                detail::getBufferPtr<Vertex>(rShape.mVertex)[index + 1].mPos = sead::Vector2f(-1.0f, -1.0f);
                detail::getBufferPtr<Vertex>(rShape.mVertex)[index + 1].mParam = sead::Vector2f(cRingScale[ring], ring);
                detail::getBufferPtr<Vertex>(rShape.mVertex)[index + 2].mPos = sead::Vector2f(1.0f, -1.0f);
                detail::getBufferPtr<Vertex>(rShape.mVertex)[index + 2].mParam = sead::Vector2f(cRingScale[ring], ring);
                detail::getBufferPtr<Vertex>(rShape.mVertex)[index + 3].mPos = sead::Vector2f(1.0f, 1.0f);
                detail::getBufferPtr<Vertex>(rShape.mVertex)[index + 3].mParam = sead::Vector2f(cRingScale[ring], ring);
                index += 4;
                break;
            }
        }

        rShape.mVertexBuffer.setUpBuffer(ConstGPUMemVoidAddr(rShape.mVertex, 0), sizeof(Vertex),
                                         u32(rShape.mVertex.getSize()));
        rShape.mVertexBuffer.setUpStream(0, VertexStreamFormat(22), 0, false);
        rShape.mVertexBuffer.setUpStream(1, VertexStreamFormat(22), 8, false);
        rShape.mVertexAttribute.create(1, pHeap);
        rShape.mVertexAttribute.setVertexStream(0, &rShape.mVertexBuffer, 0);
        rShape.mVertexAttribute.setVertexStream(1, &rShape.mVertexBuffer, 1);
        rShape.mVertexAttribute.setUp();
    }
}

void DepthOfField::initIndex_(sead::Heap* pHeap)
{
    const s32 cDivNum[] = {32, 4};
    for (s32 i = 0; i < 2; i++)
    {
        s32 divNum = cDivNum[i];
        Shape& rShape = mShapes[i];
        rShape.mIndex.allocBuffer(divNum * 18, pHeap, 4, MemoryAttribute(0));
        GPUMemAddr<u16> addr(rShape.mIndex, 0);
        s32 index = 0;
        for (s32 ring = 0; ring < 3; ring++)
        {
            s32 inner = ring * divNum;
            s32 outer = inner + divNum;
            for (s32 j = 0; j < divNum; j++)
            {
                detail::getBufferPtr<u16>(rShape.mIndex)[index] = inner + j;
                s32 next = (j + 1) % divNum;
                detail::getBufferPtr<u16>(rShape.mIndex)[index + 1] = next + outer;
                detail::getBufferPtr<u16>(rShape.mIndex)[index + 2] = next + inner;
                detail::getBufferPtr<u16>(rShape.mIndex)[index + 3] = inner + j;
                detail::getBufferPtr<u16>(rShape.mIndex)[index + 4] = outer + j;
                detail::getBufferPtr<u16>(rShape.mIndex)[index + 5] = next + outer;
                index += 6;
            }
        }

        rShape.mIndexStream.setUpStream(GPUMemAddr<u16>(rShape.mIndex, 0),
                                        rShape.mIndex.getSize() / sizeof(u16));
        rShape.mIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLES);
    }
}

void DepthOfField::initializeContext(Context* pContext, sead::Heap* pHeap)
{
    pContext->mColorSampler.setFilter(1, 1, 0);
    pContext->mDepthSampler.setFilter(1, 1, 0);
    pContext->mBlurSampler.setFilter(1, 1, 1);
    pContext->mDepthBlurSampler.setFilter(1, 1, 1);
    pContext->mComposeSampler.setFilter(1, 1, 1);
    pContext->mpBlurTexture = nullptr;
    pContext->mpDepthBlurTexture = nullptr;
}

void DepthOfField::assignShaderProgramAll_()
{
    for (s32 i = 0; i < getBufferNum(); i++)
    {
        getParameter(i).assignShaderProgram_();
    }
}

void DepthOfField::allocBuffer(DrawContext* pDrawContext, s32 context,
                               const RenderBuffer& rRenderBuffer) const
{
    const RenderTargetColor* pColor = rRenderBuffer.getRenderTargetColor();
    allocBuffer(pDrawContext, context, TextureFormat(pColor->getTextureFormat()),
                pColor->getWidth(0), pColor->getHeight(0));
}

void DepthOfField::allocBuffer(DrawContext* pDrawContext, s32 context, TextureFormat format,
                               s32 width, s32 height) const
{
    Context& rContext = getContext_(context);
    const DepthOfFieldParameter& rParam = getParameter(context);
    auto* pAllocator = utl::DynamicTextureAllocator::instance();

    bool isFromZero = rParam.enableMipFromZeroLevel_();
    s32 level = sead::Mathf::ceil(*rParam.mLevel);
    u32 blurWidth;
    u32 blurHeight;
    u32 mipLevelNum;
    if (isFromZero)
    {
        blurWidth = width;
        blurHeight = height;
        mipLevelNum = level + 1;
    }
    else
    {
        blurWidth = rParam.mReduceScale * width * 0.5f;
        blurHeight = rParam.mReduceScale * height * 0.5f;
        mipLevelNum = level > 1 ? level : 1;
    }

    if (*mEnableReduceDraw && format == TextureFormat::cTextureFormat_R11_G11_B10_float)
    {
        format = TextureFormat::cTextureFormat_R10_G10_B10_A2_uNorm;
    }

    const TextureData* pBlur =
        pAllocator->alloc(pDrawContext, "dof_blur_mipmap", format, blurWidth, blurHeight,
                          mipLevelNum, nullptr, utl::DynamicTextureAllocator::AllocateType(0),
                          true, false);
    rContext.mpBlurTexture = pBlur;
    rContext.mBlurSampler.applyTextureData(*pBlur);

    if (rParam.enableDepthBlur_())
    {
        const TextureData* pDepth = pAllocator->alloc(
            pDrawContext, "dof_depth_mipmap", TextureFormat::cTextureFormat_R8_uNorm, width / 2,
            height / 2, sead::Mathf::ceil(*rParam.mDepthBlurAdd) + 1, nullptr,
            utl::DynamicTextureAllocator::AllocateType(0), true, false);
        rContext.mpDepthBlurTexture = pDepth;
        rContext.mDepthBlurSampler.applyTextureData(*pDepth);
    }
}

bool DepthOfFieldParameter::enableMipFromZeroLevel_() const
{
    return enableIndirect_() &&
           (sead::Mathf::floor(*mLevel) == 0 || *mEnableIndirectFromFull);
}

bool DepthOfFieldParameter::enableDepthBlur_() const
{
    return *mNearEnable && *mDepthBlur && *mLevel > 0.0f;
}

void DepthOfField::freeBuffer(s32 context) const
{
    Context& rContext = getContext_(context);
    auto* pAllocator = utl::DynamicTextureAllocator::instance();
    if (rContext.mpBlurTexture)
    {
        pAllocator->free(rContext.mpBlurTexture);
        rContext.mpBlurTexture = nullptr;
    }

    if (rContext.mpDepthBlurTexture)
    {
        pAllocator->free(rContext.mpDepthBlurTexture);
        rContext.mpDepthBlurTexture = nullptr;
    }
}

DepthOfField::DrawArg::DrawArg(s32 context, Context& rContext,
                               const DepthOfFieldParameter& rParam,
                               const RenderBuffer& rRenderBuffer, const TextureData& rDepth,
                               bool isLinearDepth, f32 near, f32 far)
    : mContext(context), mPass(0), mpContext(&rContext), mpParam(&rParam),
      mpRenderBuffer(&rRenderBuffer), mNear(near), mFar(far),
      mWidth(rRenderBuffer.getRenderTargetColor()->getWidth(0)),
      mHeight(rRenderBuffer.getRenderTargetColor()->getHeight(0)), mIsLinearDepth(isLinearDepth)
{
    mpContext->mColorSampler.applyTextureData(*mpRenderBuffer->getRenderTargetColor());
    mpContext->mDepthSampler.applyTextureData(rDepth);
}

void DepthOfField::draw(DrawContext* pDrawContext, s32 context, const RenderBuffer& rRenderBuffer,
                        f32 near, f32 far) const
{
    draw(pDrawContext, context, rRenderBuffer, *rRenderBuffer.getRenderTargetDepth(), false, near,
         far);
}

void DepthOfField::draw(DrawContext* pDrawContext, s32 context, const RenderBuffer& rRenderBuffer,
                        const TextureData& rDepth, bool isLinearDepth, f32 near, f32 far) const
{
    if (!*mEnable || !mEnableContext.isOnBit(context))
    {
        return;
    }

    DrawArg arg(context, getContext_(context), getParameter(context), rRenderBuffer, rDepth,
                isLinearDepth, near, far);
    const DepthOfFieldParameter& rParam = getParameter(context);
    if (rParam.enableBlurMipMapPass_())
    {
        allocBuffer(pDrawContext, context, rRenderBuffer);
        arg.mPass = 0;
        drawColorMipMap_(pDrawContext, arg);
        if (rParam.enableDepthBlur_())
        {
            arg.mPass = 1;
            drawDepthMipMap_(pDrawContext, arg);
        }

        if (mDebugMode == 2)
        {
            drawDebugBlur_(pDrawContext, arg);
        }
        else
        {
            arg.mPass = 2;
            drawCompose_(pDrawContext, arg);
        }

        freeBuffer(context);
    }

    if (rParam.enableSeparateVignettingPass_())
    {
        arg.mPass = 3;
        drawVignetting_(pDrawContext, arg);
    }
}

bool DepthOfFieldParameter::enableBlurMipMapPass_() const
{
    return (enableDepthOfField_() || *mEnableVignettingBlur) &&
           (*mLevel > 0.0f || enableMipFromZeroLevel_());
}

void DepthOfField::drawColorMipMap_(DrawContext* pDrawContext, const DrawArg& rArg) const
{
    bool isFromZero = rArg.mpParam->enableMipFromZeroLevel_();
    Context& rContext = *rArg.mpContext;
    const TextureData& rTexture = rContext.mBlurSampler.getTextureData();
    RenderBuffer& rRenderBuffer = rContext.mRenderBuffer;
    RenderTargetColor& rTarget = rContext.mColorTarget;
    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.setColorMask(true, true, true, *mEnableReduceDraw);
    graphicsContext.apply(pDrawContext);
    rRenderBuffer.setRenderTargetColorNullAll();
    rRenderBuffer.setRenderTargetDepth(nullptr);
    rRenderBuffer.setRenderTargetColor(&rTarget);
    rTarget.applyTextureData(rTexture, 0, 0);

    const ShaderProgram* pProgram = rArg.mpParam->mpMipMapProgram;
    pProgram->activate(pDrawContext, true);
    rContext.mBlurSampler.setFilter(1, 1, 1);
    u32 mipLevelNum = rTexture.getMipLevelNum();
    for (u32 i = 0; i < mipLevelNum; i++)
    {
        u32 width;
        if (i != 0)
        {
            rContext.mBlurSampler.setLod(i - 1, i - 1, 0.0f);
            rContext.mBlurSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1,
                                           false);
            width = rTexture.getMipWidth(i);
        }
        else if (isFromZero)
        {
            bindRenderBuffer_(pDrawContext, rRenderBuffer, 0, 0);
            utl::ImageFilter2D::drawTextureQuadTriangle(pDrawContext,
                                                        rArg.mpContext->mColorSampler);
            rArg.mpContext->mRenderBuffer.getRenderTargetColor()->invalidateGPUCache(
                pDrawContext);
            pProgram->activate(pDrawContext, true);
            continue;
        }
        else
        {
            rArg.mpContext->mColorSampler.activate(pDrawContext, pProgram->getSamplerLocation(0),
                                                   -1, false);
            width = rTexture.getWidth(0);
        }

        f32 invWidth = 0.5f / width;
        f32 invHeight = 0.5f / u32(getMipHeight(rTexture, i));
        sead::Vector4f param(invWidth * mMipBlurScale, mMipBlurScale * invHeight,
                             mMipBlurScale * 0.0f, mMipBlurScale * 0.0f);
        pProgram->getUniformLocation(7).setUniform(pDrawContext, 4, &param);
        bindRenderBuffer_(pDrawContext, rRenderBuffer, i, 0);
        drawKick_(pDrawContext, rArg);
        rArg.mpContext->mRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);
    }

    rContext.mBlurSampler.setFilter(1, 1, 2);
    rContext.mBlurSampler.setLod(0.0f, rTexture.getMipLevelNum() - 1.0f, 0.0f);
}

void DepthOfField::drawDepthMipMap_(DrawContext* pDrawContext, const DrawArg& rArg) const
{
    Context& rContext = *rArg.mpContext;
    const TextureData& rTexture = rContext.mDepthBlurSampler.getTextureData();
    RenderBuffer& rRenderBuffer = rContext.mRenderBuffer;
    RenderTargetColor& rTarget = rContext.mColorTarget;
    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.setColorMask(true, false, false, false);
    graphicsContext.apply(pDrawContext);
    rRenderBuffer.setRenderTargetColorNullAll();
    rRenderBuffer.setRenderTargetDepth(nullptr);
    rRenderBuffer.setRenderTargetColor(&rTarget);
    rTarget.applyTextureData(rTexture, 0, 0);
    rContext.mDepthBlurSampler.setFilter(1, 1, 1);

    s32 variation = rArg.mIsLinearDepth;
    const ShaderProgram* pProgram =
        agl::detail::ShaderHolder::instance()
            ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cDofNearMask)
            ->getVariation(variation);
    pProgram->activate(pDrawContext, true);
    {
        const DepthOfFieldParameter& rParam = *rArg.mpParam;
        f32 start;
        f32 end;
        calcNearRange(&start, &end, *rParam.mFarStart, *rParam.mFarEnd);
        f32 offset;
        f32 scale;
        if (rArg.mIsLinearDepth)
        {
            offset = (end - rArg.mNear) / (rArg.mFar - rArg.mNear);
            scale = (rArg.mFar - rArg.mNear) / (start - end);
        }
        else
        {
            offset = end;
            scale = 1.0f / (start - end);
        }

        sead::Vector4f param(1.0f / rArg.mNear, (1.0f - rArg.mNear / rArg.mFar) / rArg.mNear,
                             scale, -(offset * scale));
        pProgram->getUniformLocation(6).setUniform(pDrawContext, 4, &param);
    }

    rArg.mpContext->mDepthSampler.activate(pDrawContext, pProgram->getSamplerLocation(1), -1,
                                           false);
    {
        f32 invWidth = 0.5f / rTexture.getWidth(0);
        f32 invHeight = 0.5f / u32(getMipHeight(rTexture, 0));
        sead::Vector4f param(mMipBlurScale * invWidth, mMipBlurScale * invHeight,
                             mMipBlurScale * 0.0f, mMipBlurScale * 0.0f);
        pProgram->getUniformLocation(7).setUniform(pDrawContext, 4, &param);
    }

    bindRenderBuffer_(pDrawContext, rRenderBuffer, 0, 0);
    drawKick_(pDrawContext, rArg);
    rArg.mpContext->mRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);

    pProgram = rArg.mpParam->mpDepthMipMapProgram;
    pProgram->activate(pDrawContext, true);
    u32 mipLevelNum = rTexture.getMipLevelNum();
    for (u32 i = 1; i < mipLevelNum; i++)
    {
        rContext.mDepthBlurSampler.setLod(i - 1, i - 1, 0.0f);
        rContext.mDepthBlurSampler.activate(pDrawContext, pProgram->getSamplerLocation(1), -1,
                                            false);
        f32 invWidth = 0.5f / u32(rTexture.getMipWidth(i));
        f32 invHeight = 0.5f / u32(getMipHeight(rTexture, i));
        sead::Vector4f param(mMipBlurScale * invWidth, mMipBlurScale * invHeight,
                             mMipBlurScale * 0.0f, mMipBlurScale * 0.0f);
        pProgram->getUniformLocation(7).setUniform(pDrawContext, 4, &param);
        bindRenderBuffer_(pDrawContext, rRenderBuffer, i, 0);
        drawKick_(pDrawContext, rArg);
        rArg.mpContext->mRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);
    }

    rContext.mDepthBlurSampler.setLod(*rArg.mpParam->mDepthBlurAdd, *rArg.mpParam->mDepthBlurAdd,
                                      0.0f);
    rContext.mDepthBlurSampler.setFilter(1, 1, 1);
}

void DepthOfField::drawDebugBlur_(DrawContext* pDrawContext, const DrawArg& rArg) const
{
    sead::Viewport viewport(*rArg.mpRenderBuffer);
    rArg.mpRenderBuffer->bind(pDrawContext);
    viewport.apply(pDrawContext, *rArg.mpRenderBuffer);
    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setColorMask(true, true, true, true);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(pDrawContext);
    utl::ImageFilter2D::drawTextureMipLevel(
pDrawContext, rArg.mpContext->mBlurSampler, viewport,
                                            *rArg.mpParam->mLevel - 1.0f,
                                            sead::Vector2f::ones * 2.0f, sead::Vector2f::zero);
}

void DepthOfField::drawCompose_(DrawContext* pDrawContext, const DrawArg& rArg) const
{
    const DepthOfFieldParameter& rParam = *rArg.mpParam;
    bool isFarOnly = *rParam.mFarEnable && !*rParam.mNearEnable && !*rParam.mEnableVignettingBlur;
    bool isNearOnly = !*rParam.mFarEnable && *rParam.mNearEnable && !*rParam.mDepthBlur &&
                      !*rParam.mEnableVignettingBlur;

    sead::GraphicsContext graphicsContext;
    if (isFarOnly)
    {
        graphicsContext.setDepthEnable(true, false);
        graphicsContext.setDepthFunc(2);
    }
    else if (isNearOnly)
    {
        graphicsContext.setDepthEnable(true, false);
        graphicsContext.setDepthFunc(5);
    }
    else
    {
        graphicsContext.setDepthEnable(false, false);
    }

    graphicsContext.setColorMask(true, true, true, false);

    if (*mEnableReduceDraw)
    {
        Context& rContext = *rArg.mpContext;
        const TextureData& rBlurTexture = rContext.mBlurSampler.getTextureData();
        rContext.mRenderBuffer.setRenderTargetColor(&rContext.mColorTarget);
        rArg.mpContext->mColorTarget.applyTextureData(
            rArg.mpContext->mBlurSampler.getTextureData(), 0, 0);
        s32 mipLevel = rBlurTexture.getWidth(0) == u32(rArg.mWidth) &&
                       rBlurTexture.getHeight(0) == u32(rArg.mHeight);
        rArg.mpContext->mComposeSampler.applyTextureData(rBlurTexture);
        rArg.mpContext->mComposeSampler.setFilter(1, 1, 1);
        rArg.mpContext->mComposeSampler.setLod(mipLevel, mipLevel, 0.0f);
        bindRenderBuffer_(pDrawContext, rArg.mpContext->mRenderBuffer, mipLevel, 0);
        sead::GraphicsContext reduceContext = graphicsContext;
        reduceContext.setDepthEnable(false, false);
        reduceContext.setBlendEnable(false);
        reduceContext.setColorMask(true, true, true, true);
        reduceContext.apply(pDrawContext);
    }
    else
    {
        sead::Viewport viewport(*rArg.mpRenderBuffer);
        viewport.apply(pDrawContext, *rArg.mpRenderBuffer);
        rArg.mpRenderBuffer->bind(pDrawContext);
        graphicsContext.apply(pDrawContext);
    }

    s32 variation = rArg.mIsLinearDepth;
    const ShaderProgram* pProgram = mDebugMode == 1 ?
                                        rArg.mpParam->mpDepthMaskPrograms[variation] :
                                        rArg.mpParam->mpFinalPrograms[variation];
    pProgram->activate(pDrawContext, true);
    uniformComposeParam_(pDrawContext, rArg, pProgram);
    if (*rArg.mpParam->mEnableVignettingBlur)
    {
        uniformVignettingParam_(pDrawContext, rArg, pProgram);
    }

    drawKick_(pDrawContext, rArg);

    if (*mEnableReduceDraw)
    {
        DrawArg arg = rArg;
        arg.mPass = 4;
        graphicsContext.applyDepthAndStencilTest(pDrawContext);
        graphicsContext.applyBlendAndFastZ(pDrawContext);
        graphicsContext.applyColorMask(pDrawContext);
        rArg.mpContext->mRenderBuffer.getRenderTargetColor()->invalidateGPUCache(pDrawContext);
        sead::Viewport reduceViewport(arg.mpContext->mRenderBuffer);
        sead::Viewport viewport(*arg.mpRenderBuffer);
        viewport.apply(pDrawContext, *arg.mpRenderBuffer);
        arg.mpRenderBuffer->bind(pDrawContext);
        const ShaderProgram* pExpandProgram = arg.mpParam->mpExpandReduceProgram;
        pExpandProgram->activate(pDrawContext, true);
        uniformExpandReduceParam_(pDrawContext, arg, pExpandProgram);
        drawKick_(pDrawContext, arg);
    }
}

bool DepthOfFieldParameter::enableSeparateVignettingPass_() const
{
    if (*mEnableVignettingColor)
    {
        if (enableDifferntShape_() || *mVignettingBlend != 0 || !*mEnableVignettingBlur ||
            *mLevel <= 0.0f)
        {
            return true;
        }
    }

    return false;
}

void DepthOfField::drawVignetting_(DrawContext* pDrawContext, const DrawArg& rArg) const
{
    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setColorMask(true, true, true, false);
    switch (*rArg.mpParam->mVignettingBlend)
    {
    case DepthOfFieldParameter::cVignettingBlendType_Normal:
        graphicsContext.setBlendFactor(0, 5, 6);
        graphicsContext.setBlendEquation(0, 1);
        break;
    case DepthOfFieldParameter::cVignettingBlendType_Add:
        graphicsContext.setBlendFactor(0, 5, 2);
        graphicsContext.setBlendEquation(0, 1);
        break;
    case DepthOfFieldParameter::cVignettingBlendType_Mul:
        graphicsContext.setBlendFactor(0, 1, 3);
        graphicsContext.setBlendEquation(0, 1);
        break;
    case DepthOfFieldParameter::cVignettingBlendType_Screen:
        graphicsContext.setBlendFactor(0, 10, 2);
        graphicsContext.setBlendEquation(0, 1);
        break;
    }

    graphicsContext.apply(pDrawContext);

    sead::Viewport viewport(*rArg.mpRenderBuffer);
    viewport.apply(pDrawContext, *rArg.mpRenderBuffer);
    rArg.mpRenderBuffer->bind(pDrawContext);
    rArg.mpParam->mpVignettingProgram->activate(pDrawContext, true);
    uniformVignettingParam_(pDrawContext, rArg, rArg.mpParam->mpVignettingProgram);
    drawKick_(pDrawContext, rArg);
}

void DepthOfField::bindRenderBuffer_(DrawContext* pDrawContext, RenderBuffer& rRenderBuffer,
                                     s32 mipLevel, s32 depthMipOffset) const
{
    s32 width = -1;
    s32 height = -1;
    if (rRenderBuffer.getRenderTargetColor())
    {
        rRenderBuffer.getRenderTargetColor()->setMipLevel(mipLevel);
        width = rRenderBuffer.getRenderTargetColor()->getMipWidth(mipLevel);
        height = getMipHeight(*rRenderBuffer.getRenderTargetColor(), mipLevel);
    }

    if (rRenderBuffer.getRenderTargetDepth())
    {
        s32 depthMipLevel = mipLevel + depthMipOffset;
        rRenderBuffer.getRenderTargetDepth()->setMipLevel(depthMipLevel);
        width = rRenderBuffer.getRenderTargetDepth()->getMipWidth(depthMipLevel);
        height = getMipHeight(*rRenderBuffer.getRenderTargetDepth(), depthMipLevel);
    }

    rRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, width, height));
    rRenderBuffer.setVirtualSize(sead::Vector2f(width, height));
    sead::Viewport viewport(rRenderBuffer);
    viewport.apply(pDrawContext, rRenderBuffer);
    rRenderBuffer.bind(pDrawContext);
}

void DepthOfField::drawKick_(DrawContext* pDrawContext, const DrawArg& rArg) const
{
    switch (rArg.mPass)
    {
    case 3:
    {
        s32 type = rArg.mpParam->enableDifferntShape_() ?
                       *rArg.mpParam->mVignettingShape1.mType :
                       *rArg.mpParam->mVignettingShape0.mType;
        const Shape& rShape = mShapes[type];
        rShape.mVertexAttribute.activate(pDrawContext);
        detail::drawIndexStream(pDrawContext, rShape.mIndexStream);
        return;
    }
    case 2:
        if (*rArg.mpParam->mEnableVignettingBlur)
        {
            mShapes[*rArg.mpParam->mVignettingShape0.mType].mVertexAttribute.activate(
                pDrawContext);
            detail::drawIndexStream(pDrawContext,
                                    mShapes[*rArg.mpParam->mVignettingShape0.mType].mIndexStream);
            return;
        }

        break;
    }

    detail::drawQuadTriangle(pDrawContext);
}

void DepthOfField::uniformComposeParam_(DrawContext* pDrawContext, const DrawArg& rArg,
                                        const ShaderProgram* pProgram) const
{
    sead::Vector4f depthScale = sead::Vector4f::zero;
    sead::Vector4f depthOffset = sead::Vector4f::zero;
    const DepthOfFieldParameter& rParam = *rArg.mpParam;
    f32 width = rArg.mWidth;
    f32 height = rArg.mHeight;
    f32 start;
    f32 end;
    calcRange(&start, &end, *rParam.mStart, *rParam.mEnd);
    bool isLinearDepth = rArg.mIsLinearDepth;
    f32 farMax = *rParam.mDofFarMax;
    if (isLinearDepth)
    {
        farMax = (farMax - rArg.mNear) / (rArg.mFar - rArg.mNear);
    }

    f32 invWidth = 1.0f / width;
    f32 invHeight = 1.0f / height;
    {
        sead::Vector4f param(*rParam.mLevel, *rParam.mLevel, farMax, 1.0f - *rParam.mSaturateMin);
        pProgram->getUniformLocation(0).setUniform(pDrawContext, 4, &param);
    }

    {
        sead::Vector4f param(invWidth * mComposeBlurScale, invHeight * mComposeBlurScale,
                             invWidth, invHeight);
        pProgram->getUniformLocation(7).setUniform(pDrawContext, 4, &param);
    }

    f32 farOffset;
    f32 farScale;
    if (isLinearDepth)
    {
        farOffset = (start - rArg.mNear) / (rArg.mFar - rArg.mNear);
        farScale = (rArg.mFar - rArg.mNear) / (end - start);
    }
    else
    {
        farOffset = start;
        farScale = 1.0f / (end - start);
    }

    depthScale.x = farScale;
    depthOffset.x = -(farOffset * farScale);
    f32 near = rArg.mNear;
    f32 far = rArg.mFar;
    f32 farDepth = calcProjDepth(start, near, far);
    f32 nearDepth = 0.0f;
    if (*rArg.mpParam->mNearEnable)
    {
        f32 nearStart;
        f32 nearEnd;
        calcNearRange(&nearStart, &nearEnd, *rArg.mpParam->mFarStart,
                      *rArg.mpParam->mFarEnd);
        f32 nearOffset = nearEnd;
        f32 nearLength = 1.0f;
        if (isLinearDepth)
        {
            nearOffset = (nearEnd - near) / (far - near);
            nearLength = far - near;
        }

        f32 nearScale = nearLength / (nearStart - nearEnd);
        depthScale.y = nearScale;
        depthOffset.y = -(nearOffset * nearScale);
        nearDepth = calcProjDepth(nearEnd, near, far);
    }

    {
        sead::Vector4f param(1.0f / near, (1.0f - near / far) / near, farDepth, nearDepth);
        pProgram->getUniformLocation(6).setUniform(pDrawContext, 4, &param);
    }

    if (*rArg.mpParam->mEnableColorControl)
    {
        const sead::Vector4f& rDepth = *rArg.mpParam->mColorCtrlDepth;
        f32 colorStart;
        f32 colorEnd;
        calcRange(&colorStart, &colorEnd, rDepth.x, rDepth.y);
        f32 reverseStart = rDepth.z > rDepth.w ? rDepth.z : rDepth.w;
        f32 reverseEnd = rDepth.z < rDepth.w ? rDepth.z : rDepth.w;
        if (reverseEnd - reverseStart < 0.1f)
        {
            reverseEnd += 0.1f;
        }

        f32 colorRange;
        f32 reverseRange;
        if (isLinearDepth)
        {
            f32 length = rArg.mFar - rArg.mNear;
            f32 colorOffset = (colorStart - rArg.mNear) / length;
            f32 reverseOffset = (reverseStart - rArg.mNear) / length;
            colorRange = (colorEnd - colorStart) / length;
            reverseRange = (reverseEnd - reverseStart) / length;
            colorStart = colorOffset;
            reverseStart = reverseOffset;
        }
        else
        {
            colorRange = colorEnd - colorStart;
            reverseRange = reverseEnd - reverseStart;
        }

        depthScale.z = 1.0f / colorRange;
        depthOffset.z = -(colorStart * depthScale.z);
        depthScale.w = 1.0f / reverseRange;
        depthOffset.w = -(reverseStart * depthScale.w);
        if (!*rArg.mpParam->mEnableColorReverse)
        {
            depthScale.w = 1.0f;
            depthOffset.w = 1.0f;
        }
    }

    pProgram->getUniformLocation(4).setUniform(pDrawContext, 4, &depthScale);
    pProgram->getUniformLocation(5).setUniform(pDrawContext, 4, &depthOffset);
    pProgram->getUniformLocation(12).setUniform(pDrawContext, 4, &*rArg.mpParam->mFarMulColor);

    rArg.mpContext->mBlurSampler.activate(pDrawContext, pProgram->getSamplerLocation(2), -1,
                                          false);
    if (rArg.mpParam->enableDepthBlur_())
    {
        rArg.mpContext->mDepthBlurSampler.activate(pDrawContext, pProgram->getSamplerLocation(3),
                                                   -1, false);
    }

    rArg.mpContext->mDepthSampler.activate(pDrawContext, pProgram->getSamplerLocation(1), -1,
                                           false);
    if (rArg.mpParam->enableIndirect_())
    {
        mIndirectSampler.activate(pDrawContext, pProgram->getSamplerLocation(4), -1, false);
        rArg.mpParam->mIndirectParam.x = mpIndirectTexture->getWidth(0);
        rArg.mpParam->mIndirectParam.y = mpIndirectTexture->getHeight(0);
        rArg.mpParam->mIndirectParam.z = 1.0f / (invWidth * rArg.mHeight);
        rArg.mpParam->mIndirectParam.w = *rArg.mpParam->mIndirectScale;
        pProgram->getUniformLocation(1).setUniform(pDrawContext, 4,
                                                   &rArg.mpParam->mIndirectParam);
        pProgram->getUniformLocation(2).setUniform(pDrawContext, 3,
                                                   rArg.mpParam->mIndirectMatrix[0]);
        pProgram->getUniformLocation(3).setUniform(pDrawContext, 3,
                                                   rArg.mpParam->mIndirectMatrix[1]);
    }
}

void DepthOfField::uniformVignettingParam_(DrawContext* pDrawContext, const DrawArg& rArg,
                                           const ShaderProgram* pProgram) const
{
    bool isDepthOfField = false;
    const DepthOfFieldParameter::VignettingShapeParam* pShape;
    if (rArg.mPass == 2)
    {
        isDepthOfField = rArg.mpParam->enableDepthOfField_();
        pShape = &rArg.mpParam->mVignettingShape0;
    }
    else if (rArg.mPass == 3 && rArg.mpParam->enableDifferntShape_())
    {
        pShape = &rArg.mpParam->mVignettingShape1;
    }
    else
    {
        pShape = &rArg.mpParam->mVignettingShape0;
    }

    f32 scaleX = pShape->mScale->x < 0.001f ? 0.001f : pShape->mScale->x;
    f32 scaleY = pShape->mScale->y < 0.001f ? 0.001f : pShape->mScale->y;
    f32 range = sead::Mathf::clamp(1.0f - pShape->mRange->x, 0.0f, 1.0f);
    f32 transX = std::abs(pShape->mTrans->x);
    f32 transY = std::abs(pShape->mTrans->y);
    f32 trans = transX > transY ? transX : transY;
    f32 minScale = scaleX < scaleY ? scaleX : scaleY;
    {
        sead::Vector4f param(isDepthOfField ? 0.0f : range, range,
                             range + (1.0f - range) * pShape->mRange->y,
                             1.0f / minScale * 1.1f + trans);
        pProgram->getUniformLocation(8).setUniform(pDrawContext, 4, &param);
    }

    f32 aspectX;
    f32 aspectY;
    if (*pShape->mType != 0)
    {
        aspectX = 1.0f;
        aspectY = 1.0f;
    }
    else
    {
        f32 diagonal = sead::Mathf::sqrt(f32(rArg.mWidth * rArg.mWidth + rArg.mHeight * rArg.mHeight));
        aspectX = diagonal / rArg.mWidth;
        aspectY = diagonal / rArg.mHeight;
    }

    {
        sead::Vector4f param(scaleX * aspectX, scaleY * aspectY,
                             sead::Mathf::clamp(*rArg.mpParam->mVignettingBlur, 0.0f, 1.0f),
                             0.0f);
        pProgram->getUniformLocation(9).setUniform(pDrawContext, 4, &param);
    }

    pProgram->getUniformLocation(11).setUniform(pDrawContext, 4,
                                                &*rArg.mpParam->mVignettingColor);
    {
        sead::Vector4f param(pShape->mTrans->x, pShape->mTrans->y, 0.0f, 0.0f);
        pProgram->getUniformLocation(10).setUniform(pDrawContext, 4, &param);
    }
}

void DepthOfField::uniformExpandReduceParam_(DrawContext* pDrawContext, const DrawArg& rArg,
                                             const ShaderProgram* pProgram) const
{
    f32 near = rArg.mNear;
    f32 far = rArg.mFar;
    const DepthOfFieldParameter& rParam = *rArg.mpParam;
    f32 farDepth = calcProjDepth(sead::Mathf::min(*rParam.mStart, *rParam.mEnd), near, far);
    f32 nearDepth = 0.0f;
    if (*rParam.mNearEnable)
    {
        nearDepth =
            calcProjDepth(sead::Mathf::max(*rParam.mFarStart, *rParam.mFarEnd), near, far);
    }

    sead::Vector4f param(0.0f, 0.0f, farDepth, nearDepth);
    pProgram->getUniformLocation(6).setUniform(pDrawContext, 4, &param);
    rArg.mpContext->mComposeSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1,
                                             false);
}

bool DepthOfFieldParameter::enableDifferntShape_() const
{
    return *mEnableVignettingColor && *mEnableVignettingBlur && *mEnableVignetting2Shape;
}

bool DepthOfFieldParameter::enableIndirect_() const
{
    return *mIndirectEnable && mpIndirectTexture && *mFarEnable;
}

bool DepthOfFieldParameter::enableDepthOfField_() const
{
    return (*mNearEnable || *mFarEnable) && (*mLevel > 0.0f || enableMipFromZeroLevel_());
}

void DepthOfFieldParameter::setDepthParam(f32 start, f32 end)
{
    *mStart = start;
    *mEnd = end;
}

void DepthOfFieldParameter::setDepthNearParam(f32 start, f32 end)
{
    *mFarStart = start;
    *mFarEnd = end;
}

void DepthOfFieldParameter::setEnableDofNear(bool enable)
{
    if (*mNearEnable != enable)
    {
        *mNearEnable = enable;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setEnableDofFar(bool enable)
{
    if (*mFarEnable != enable)
    {
        *mFarEnable = enable;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setBlurParam(f32 level)
{
    level = sead::Mathf::clamp(level, 0.0f, 14.0f);
    if (*mLevel != level)
    {
        *mLevel = level;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setDepthBlurParam(bool enable, f32 add)
{
    add = sead::Mathf::clamp(add, 0.0f, 14.0f);
    bool isEnable = enable || add == 0.0f;
    if (*mDepthBlur != isEnable || *mDepthBlurAdd != add)
    {
        *mDepthBlur = isEnable;
        *mDepthBlurAdd = add;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setEnableVignettingBlur(bool enable)
{
    if (*mEnableVignettingBlur != enable)
    {
        *mEnableVignettingBlur = enable;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setVignettingBlur(f32 blur)
{
    if (*mVignettingBlur != blur)
    {
        *mVignettingBlur = blur;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setEnableVignettingColor(bool enable)
{
    if (*mEnableVignettingColor != enable)
    {
        *mEnableVignettingColor = enable;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setVignettingColor(sead::Color4f color)
{
    if (!(*mVignettingColor == color))
    {
        *mVignettingColor = color;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setVignettingBlendType(VignettingBlendType type)
{
    if (*mVignettingBlend != type)
    {
        *mVignettingBlend = type;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setIndirectEnable(bool enable)
{
    if (*mIndirectEnable != enable)
    {
        *mIndirectEnable = enable;
        assignShaderProgram_();
    }
}

void DepthOfField::setIndirectTextureData(const TextureData* pTexture)
{
    mpIndirectTexture = pTexture;
    if (pTexture)
    {
        mIndirectSampler.applyTextureData(*pTexture);
        mIndirectSampler.setWrapDirect(1, 1, 1);
    }

    for (s32 i = 0; i < getBufferNum(); i++)
    {
        getParameter(i).mpIndirectTexture = pTexture;
    }

    assignShaderProgram_();
}

void DepthOfFieldParameter::setIndirectTextureScale(const sead::Vector2f& rScale)
{
    *mIndirectTexScale = rScale;
    updateIndirectMatrix_();
}

void DepthOfFieldParameter::setIndirectTextureRotateRad(f32 rotate)
{
    *mIndirectTexRotate = rotate;
    updateIndirectMatrix_();
}

void DepthOfFieldParameter::setIndirectTextureTrans(const sead::Vector2f& rTrans)
{
    *mIndirectTexTrans = rTrans;
    updateIndirectMatrix_();
}

void DepthOfFieldParameter::setIndirectTextureSRT(const sead::Vector2f& rScale, f32 rotate,
                                                  const sead::Vector2f& rTrans)
{
    *mIndirectTexScale = rScale;
    *mIndirectTexRotate = rotate;
    *mIndirectTexTrans = rTrans;
    updateIndirectMatrix_();
}

void DepthOfFieldParameter::setEnableVignetting2Shape(bool enable)
{
    if (*mEnableVignetting2Shape != enable)
    {
        *mEnableVignetting2Shape = enable;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setVignettingShapeParam0(const VignettingShapeParam& rParam)
{
    if (mVignettingShape0.isDifferent(rParam))
    {
        mVignettingShape0.copy(rParam);
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setVignettingShapeParam1(const VignettingShapeParam& rParam)
{
    if (mVignettingShape1.isDifferent(rParam))
    {
        mVignettingShape1.copy(rParam);
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setEnableDofFarCancel(bool enable)
{
    if (*mEnableDofFarMax != enable)
    {
        *mEnableDofFarMax = enable;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setEnableIndirectDepthCancel(bool enable)
{
    if (*mIndirectDepthCancelEnable != enable)
    {
        *mIndirectDepthCancelEnable = enable;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setColorChangeEnable(bool enable)
{
    if (*mEnableColorControl != enable)
    {
        *mEnableColorControl = enable;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setColorChangeSaturateMin(f32 min)
{
    if (*mSaturateMin != min)
    {
        *mSaturateMin = min;
        assignShaderProgram_();
    }
}

void DepthOfFieldParameter::setColorChangeMulColor(const sead::Color4f& rColor)
{
    if (!(*mFarMulColor == rColor))
    {
        *mFarMulColor = rColor;
        assignShaderProgram_();
    }
}

void DepthOfField::postRead_()
{
    assignShaderProgram_();
    updateIndirectMatrix_();
    copyParameterToAllContext(0);
}

void DepthOfField::postCopyParameter(s32 index, const utl::IParameterObj* pObjA,
                                     const utl::IParameterObj* pObjB, f32 t)
{
    getParameter(index).assignShaderProgram_();
    getParameter(index).updateIndirectMatrix_();
}

void DepthOfField::genMessage(sead::hostio::Context* pContext)
{
    genMessageIO(pContext, 0xf);
    mDebugTexturePage.genMessagePage(pContext, this);
    mEnable.genMessageParameter(pContext, mEnable.getMeta());
    genMessageDepthOfFieldParameter(pContext);
}

void DepthOfFieldParameter::genMessageDepthOfFieldParameter(sead::hostio::Context* pContext)
{
    mLevel.genMessageParameter(pContext, mLevel.getMeta());
    mFarEnable.genMessageParameter(pContext, mFarEnable.getMeta());
    if (*mFarEnable)
    {
        mStart.genMessageParameter(pContext, "Min=-10000, Max=10000");
        mEnd.genMessageParameter(pContext, "Min=-10000, Max=10000");
        mEnableColorControl.genMessageParameter(pContext, mEnableColorControl.getMeta());
        if (*mEnableColorControl)
        {
            mSaturateMin.genMessageParameter(pContext, "Min=0, Max=1");
            mFarMulColor.genMessageParameter(pContext, "Mode=RGBOnly, Max=2");
            mEnableColorReverse.genMessageParameter(pContext, mEnableColorReverse.getMeta());
        }

        mIndirectEnable.genMessageParameter(pContext, mIndirectEnable.getMeta());
        if (*mIndirectEnable)
        {
            mIndirectScale.genMessageParameter(pContext, mIndirectScale.getMeta());
            {
                sead::SafeString label = mIndirectTexRotate.getLabel();
            }

            mIndirectTexScale.genMessageParameter(pContext, "Min=0.1,Max=10");
            mIndirectTexTrans.genMessageParameter(pContext, "Min=0,Max=1");
            mIndirectDepthCancelEnable.genMessageParameter(pContext,
                                                           mIndirectDepthCancelEnable.getMeta());
            mEnableIndirectFromFull.genMessageParameter(pContext,
                                                        mEnableIndirectFromFull.getMeta());
        }

        mEnableDofFarMax.genMessageParameter(pContext, mEnableDofFarMax.getMeta());
        if (*mEnableDofFarMax)
        {
            mDofFarMax.genMessageParameter(pContext, mDofFarMax.getMeta());
        }
    }

    mNearEnable.genMessageParameter(pContext, mNearEnable.getMeta());
    if (*mNearEnable)
    {
        mFarStart.genMessageParameter(pContext, "Min=-1000, Max=1000");
        mFarEnd.genMessageParameter(pContext, "Min=-1000, Max=1000");
        mDepthBlur.genMessageParameter(pContext, mDepthBlur.getMeta());
        if (*mDepthBlur)
        {
            mDepthBlurAdd.genMessageParameter(pContext, "Min=0.0, Max=10");
        }
    }

    mEnableVignettingBlur.genMessageParameter(pContext, mEnableVignettingBlur.getMeta());
    mEnableVignettingColor.genMessageParameter(pContext, mEnableVignettingColor.getMeta());
    if (*mEnableVignettingColor && *mEnableVignettingBlur)
    {
        mEnableVignetting2Shape.genMessageParameter(pContext, mEnableVignetting2Shape.getMeta());
    }

    if (*mEnableVignettingColor || *mEnableVignettingBlur)
    {
        mVignettingShape0.genMessage(pContext);
        if (*mEnableVignettingBlur)
        {
            mVignettingBlur.genMessageParameter(pContext, "Min=0, Max=1");
        }

        if (enableDifferntShape_())
        {
            mVignettingShape1.genMessage(pContext);
        }

        if (*mEnableVignettingColor)
        {
            mVignettingColor.genMessageParameter(pContext, mVignettingColor.getMeta());
            {
                sead::SafeString label = mVignettingBlend.getLabel();
            }
        }
    }

    mEnableReduceDraw.genMessageParameter(pContext, mEnableReduceDraw.getMeta());
}

void DepthOfFieldParameter::VignettingShapeParam::genMessage(sead::hostio::Context* pContext)
{
    {
        sead::SafeString label = mType.getLabel();
    }

    mScale.genMessageParameter(pContext, "Min=0.001, Max=2");
    mTrans.genMessageParameter(pContext, "Min=-1, Max=1");
}

void DepthOfField::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    listenPropertyEventIO(this, pEvent);
    listenPropertyEventDepthOfFieldParameter(this, pEvent);
    assignShaderProgramAll_();
    copyParameterToAllContext(0);
}

/**
 * Clamps the mip level and rebuilds the indirect matrix when an indirect texture parameter changes.
 * @param pNode Node that received the event.
 * @param pEvent Property event received from the host.
 */
void DepthOfFieldParameter::listenPropertyEventDepthOfFieldParameter(
    sead::hostio::Node* pNode, const sead::hostio::PropertyEvent* pEvent)
{
    *mLevel = sead::Mathf::clamp(*mLevel, 0.0f, 14.0f);
    uintptr_t id = pEvent->getIdValue();
    if (id == reinterpret_cast<uintptr_t>(&*mIndirectTexScale) ||
        id == reinterpret_cast<uintptr_t>(&*mIndirectTexRotate) ||
        id == reinterpret_cast<uintptr_t>(&*mIndirectTexTrans))
    {
        updateIndirectMatrix_();
    }
}

void DepthOfField::tempVignettingPostRead_(s32 index, const TempVignetting& rVignetting)
{
    if (index != 0)
    {
        *getParameter(0).mVignettingShape1.mType = *rVignetting.mType;
        *getParameter(0).mVignettingShape1.mScale = *rVignetting.mScale;
        *getParameter(0).mVignettingShape1.mTrans = *rVignetting.mTrans;
        *getParameter(0).mVignettingShape1.mRange = *rVignetting.mRange;
    }
    else
    {
        *getParameter(0).mVignettingShape0.mType = *rVignetting.mType;
        *getParameter(0).mVignettingShape0.mScale = *rVignetting.mScale;
        *getParameter(0).mVignettingShape0.mTrans = *rVignetting.mTrans;
        *getParameter(0).mVignettingShape0.mRange = *rVignetting.mRange;
    }

    assignShaderProgram_();
}

DepthOfField::TempVignetting::TempVignetting(DepthOfField* pOwner, s32 index,
                                             const sead::SafeString& rName)
    : mType(0, "type", "形状", this), mRange(sead::Vector2f(0.25f, 1.0f), "range", "変化幅", this),
      mScale(sead::Vector2f(1.0f, 1.0f), "scale", "スケール", this),
      mTrans(sead::Vector2f(0.0f, 0.0f), "trans", "オフセット", this), mpOwner(pOwner),
      mIndex(index)
{
    pOwner->addObj(this, rName);
}

void DepthOfField::TempVignetting::postRead_()
{
    mpOwner->tempVignettingPostRead_(mIndex, *this);
}

bool DepthOfField::TempVignetting::preWrite_() const
{
    return false;
}

}  // namespace agl::pfx
