#include "postfx/aglHDRCompose.h"

#include <cmath>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadSafeString.h>
#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "detail/aglRootNode.h"
#include "detail/aglShaderHolder.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglPrimitiveTexture.h"

namespace agl::pfx {

HDRCompose::HDRCompose()
{
    agl::detail::RootNode::setNodeMeta(this, "Icon=EFFECT");
}

HDRCompose::~HDRCompose()
{
    mContexts.freeBuffer();
}

void HDRCompose::initialize(s32 contextNum, sead::Heap* pHeap)
{
    mContexts.tryAllocBuffer(contextNum, pHeap);

    for (auto& rContext : mContexts)
    {
        rContext.mpSampler0 = nullptr;
        rContext.mpSampler1 = nullptr;
        rContext.mSampler1Param = sead::Vector2f::zero;
        rContext.mpSampler2 = nullptr;
        rContext.mExposure = 0.0f;
        rContext.mTexCoordScale = sead::Vector2f::ones;
        rContext.mTexCoordOffset = sead::Vector2f::zero;
        rContext.mTexCoordRotate = 0.0f;
    }

    mDebugTexturePage.setUp(contextNum, "HDRCompose", pHeap);
}

void HDRCompose::calc() {}

void HDRCompose::calcGPU() const {}

void HDRCompose::calcGPU(s32 context) const {}

void HDRCompose::draw(DrawContext* pDrawContext, s32 context, const RenderBuffer& rRenderBuffer,
                      const sead::Viewport& rViewport, const TextureData& rTexture) const
{
    const Context& rContext = mContexts[context];
    rContext.mSampler.applyTextureData(rTexture);

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(pDrawContext);
    rRenderBuffer.bind(pDrawContext);
    rViewport.apply(pDrawContext, rRenderBuffer);

    const ShaderProgram* pProgram = agl::detail::ShaderHolder::instance()->getShaderProgram(
        agl::detail::ShaderHolder::cHdrCompose);
    bool isEnable0 = rContext.mpSampler0 != nullptr && (mFlags >> 4 & 1);
    bool isEnable1 = rContext.mpSampler1 != nullptr && (mFlags >> 1 & 1);
    bool isEnable2 = rContext.mpSampler2 != nullptr && (mFlags & 1);
    s32 variation = (isEnable2 ? (mSampler2Variation + 1) * pProgram->getVariationMacroStride(0) : 0);

    if (isEnable1)
    {
        variation += pProgram->getVariationMacroStride(1);
    }

    variation += mVariation2 * pProgram->getVariationMacroStride(2);

    if (mFlags & cFlag_Mode2)
    {
        variation += pProgram->getVariationMacroStride(3) * 2;
    }
    else if (mFlags & cFlag_Mode1)
    {
        variation += pProgram->getVariationMacroStride(3);
    }

    pProgram = pProgram->getVariation(variation);

    f32 exposure = mFlags & cFlag_EnableExposure ? std::exp2f(mContexts[context].mExposure) : 1.0f;
    pProgram->activate(pDrawContext, true);

    sead::Vector4f param(exposure, 1.0f, mParam.x, mParam.y);
    rContext.mSampler.activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
    pProgram->getUniformLocation(1).setUniform(pDrawContext, 4, &param);

    f32 rotate = sead::Mathf::deg2rad(rContext.mTexCoordRotate);
    f32 c = std::cos(rotate);
    f32 s = std::sin(rotate);
    const sead::Vector2f& rScale = rContext.mTexCoordScale;
    const sead::Vector2f& rOffset = rContext.mTexCoordOffset;
    f32 m00 = c * rScale.x;
    f32 m01 = -s * rScale.y;
    f32 m10 = s * rScale.x;
    f32 m11 = c * rScale.y;
    sead::Vector4f texMtx[2];
    texMtx[0].set(m00, m01, m10, m11);
    f32 offsetX = rOffset.x + 0.5f;
    f32 offsetY = rOffset.y - 0.5f;
    texMtx[1].set(m10 * offsetY - m00 * offsetX + 0.5f, -m01 * offsetX + m11 * offsetY + 0.5f,
                  0.0f, 0.0f);
    pProgram->getUniformLocation(2).setUniform(pDrawContext, 4, &texMtx[0]);
    pProgram->getUniformLocation(3).setUniform(pDrawContext, 4, &texMtx[1]);

    if (isEnable0)
    {
        rContext.mpSampler0->activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    }
    else
    {
        utl::PrimitiveTexture::instance()
            ->getTextureSampler(utl::PrimitiveTexture::cType_White2D)
            ->activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    }

    if (isEnable1)
    {
        pProgram->getUniformLocation(4).setUniform(pDrawContext, 2, &rContext.mSampler1Param);
        rContext.mpSampler1->activate(pDrawContext, pProgram->getSamplerLocation(3), -1, false);
    }

    if (isEnable2)
    {
        rContext.mpSampler2->activate(pDrawContext, pProgram->getSamplerLocation(2), -1, false);
    }

    detail::drawQuadTriangle(pDrawContext);
}

void HDRCompose::setTexCoordCoeff(s32 context, const sead::Vector2f& rScale,
                                  const sead::Vector2f& rOffset, f32 rotate)
{
    Context& rContext = mContexts[context];
    rContext.mTexCoordScale = rScale;
    rContext.mTexCoordOffset = rOffset;
    rContext.mTexCoordRotate = rotate;
}

void HDRCompose::genMessage(sead::hostio::Context* pContext)
{
    mDebugTexturePage.genMessagePage(pContext, this);
    s32 i = 0;

    for (auto& rContext : mContexts)
    {
        sead::FormatFixedSafeString<1024> header("GroupHeader= viewpoint: %d", i);
        i++;
    }

    i = 0;

    for (auto& rContext : mContexts)
    {
        sead::FormatFixedSafeString<1024> header("GroupHeader= viewpoint: %d", i);
        i++;
    }
}

void HDRCompose::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

}  // namespace agl::pfx
