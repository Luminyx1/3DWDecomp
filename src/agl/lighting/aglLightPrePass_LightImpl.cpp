#include "lighting/aglLightPrePass.h"

#include <hostio/seadHostIOPropertyEvent.h>

#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "detail/aglShaderHolder.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglPrimitiveShape.h"

namespace agl::utl::DevTools {

void beginDrawImm(DrawContext*, const sead::Matrix34f&, const sead::Matrix44f&);
void drawAxisImm(DrawContext*, const sead::Matrix34f&, float, float, float);
void drawSpotLight(DrawContext*, const sead::Vector3f&, const sead::Vector3f&, const sead::Color4f&,
                   float, float, const sead::Matrix34f&, const sead::Matrix44f&);
void drawProjLight(DrawContext*, const sead::Vector3f&, const sead::Vector3f&,
                   const sead::Vector3f&, const sead::Color4f&, float, float, float, float, float,
                   float, const sead::Vector3f&, bool, const sead::Matrix34f&,
                   const sead::Matrix44f&);
void drawPointLight(DrawContext*, const sead::Vector3f&, float, const sead::Color4f&,
                    const sead::Matrix34f&, const sead::Matrix44f&);

}  // namespace agl::utl::DevTools

namespace agl::lght {

namespace {

struct MacroInfo {
    s32 mIndex;
    s32 mValueNum;
};

struct ShaderInfo {
    LightPrePass::LightType mType;
    s32 mMacroNum;
    const MacroInfo* mMacros;
    s32 mShaderIndex;
    s32* mStrides;
};

const MacroInfo cPointLightMacros[] = {{0, 2}, {1, 3}};
const MacroInfo cSpotLightMacros[] = {{0, 2}, {1, 3}, {2, 2}};
const MacroInfo cProjLightMacros[] = {{0, 2}, {1, 3}, {2, 2}, {3, 3}, {4, 2}};

s32 sVariationStride[10];

const ShaderInfo cShaderInfo[LightPrePass::cLightType_Num] = {
    {LightPrePass::cLightType_Point, 2, cPointLightMacros, 0x57, &sVariationStride[0]},
    {LightPrePass::cLightType_Spot, 3, cSpotLightMacros, 0x58, &sVariationStride[2]},
    {LightPrePass::cLightType_Proj, 5, cProjLightMacros, 0x59, &sVariationStride[5]},
};

const f32 cQualityRadiusScale[] = {1.0f, 1.03f, 1.15f};

}  // namespace

/**
 * Looks up the light shader program of a variation.
 * @param ppProgram destination of the shader program
 * @param type light type
 * @param a variation flag
 * @param b variation flag
 * @param c variation flag
 * @param d variation flag
 * @param shadowType shadow type
 * @param e variation flag
 * @param f variation flag
 */
void LightPrePass::GetShader_(const ShaderProgram** ppProgram, LightType type, bool a, bool b,
                              bool c, bool d, ShadowType shadowType, bool e, bool f)
{
    const ShaderInfo& rInfo = cShaderInfo[type];
    const ShaderProgram* pProgram =
        detail::ShaderHolder::instance()->getShaderProgram(rInfo.mShaderIndex);
    *ppProgram = pProgram;

    s32 variation = 0;
    switch (type)
    {
    case cLightType_Point:
        variation = rInfo.mStrides[0] * f + rInfo.mStrides[1] * (a * (b + 1));
        break;
    case cLightType_Spot:
        variation = rInfo.mStrides[0] * f + rInfo.mStrides[1] * shadowType + rInfo.mStrides[2] * a;
        break;
    case cLightType_Proj:
        variation = rInfo.mStrides[0] * e + rInfo.mStrides[1] * (c * (d + 1)) + rInfo.mStrides[2] * f +
                    rInfo.mStrides[3] * shadowType + rInfo.mStrides[4] * a;
        break;
    default:
        break;
    }

    *ppProgram = pProgram->getVariation(variation);
}

void LightPrePass::InitializeShaderVariationTable_()
{
    for (const auto& rInfo : cShaderInfo)
    {
        s32 stride = 1;
        for (s32 i = rInfo.mMacroNum - 1; i >= 0; i--)
        {
            rInfo.mStrides[i] = stride;
            stride *= rInfo.mMacros[i].mValueNum;
        }
    }
}

/**
 * Declares the members of a point light uniform block.
 * @param pUbo uniform block
 * @param pHeap heap to allocate from
 */
void LightPrePass::createPointLightUBO(UniformBlock* pUbo, sead::Heap* pHeap)
{
    pUbo->startDeclare(5, pHeap);
    pUbo->declare(UniformBlock::cType_Vec4, 1);
    pUbo->declare(UniformBlock::cType_Vec4, 1);
    pUbo->declare(UniformBlock::cType_Vec4, 1);
    pUbo->declare(UniformBlock::cType_Vec4, 1);
    pUbo->declare(UniformBlock::cType_Vec4, 1);
}

/**
 * Declares the members of a spot light uniform block.
 * @param pUbo uniform block
 * @param pHeap heap to allocate from
 */
void LightPrePass::createSpotLightUBO(UniformBlock* pUbo, sead::Heap* pHeap)
{
    pUbo->startDeclare(3, pHeap);
    pUbo->declare(UniformBlock::cType_Vec4, 4);
    pUbo->declare(UniformBlock::cType_Vec4, 4);
    pUbo->declare(UniformBlock::cType_Vec4, 7);
}

/**
 * Declares the members of a projection light uniform block.
 * @param pUbo uniform block
 * @param pHeap heap to allocate from
 */
void LightPrePass::createProjLightUBO(UniformBlock* pUbo, sead::Heap* pHeap)
{
    pUbo->startDeclare(4, pHeap);
    pUbo->declare(UniformBlock::cType_Vec4, 4);
    pUbo->declare(UniformBlock::cType_Vec4, 4);
    pUbo->declare(UniformBlock::cType_Vec4, 4);
    pUbo->declare(UniformBlock::cType_Vec4, 7);
}

void LightPrePass::PointLightMgr::initPrepareImpl_(sead::Heap* pHeap)
{
    mVertexAttribute.create(1, pHeap);
    mVertexAttribute.setVertexStream(0, &utl::PrimitiveShape::instance()->getSphereVertexBuffer(),
                                     0);
    mVertexAttribute.setUp();
}

void LightPrePass::PointLightMgr::initImpl_(PointLight& rLight, sead::Heap* pHeap)
{
    rLight.mFlags = 3;
    rLight.mPos = sead::Vector3f::zero;
    rLight.mRadius = 1.0f;
    rLight.mColor = sead::Color4f::cWhite;
    rLight.mSpecColor = sead::Color4f::cBlack;
    rLight.mAttnPow = 2.0f;
    rLight.mAttnStart = 0.1f;
}

void LightPrePass::PointLightMgr::destroyFinishImpl_()
{
    mVertexAttribute.destroy();
}

void LightPrePass::PointLightMgr::initViewUboImpl_(UniformBlock* pUbo, sead::Heap* pHeap)
{
    createPointLightUBO(pUbo, pHeap);
}

bool LightPrePass::PointLightMgr::calcViewImpl_(PointLight& rLight, s32 view,
                                                const Context& rContext)
{
    return rContext.mCulling.isInside(rLight.mPos, rLight.mRadius);
}

void LightPrePass::PointLightMgr::updateUBO_(const PointLight& rLight, s32 view,
                                             const Context& rContext) const
{
    const sead::Matrix34f& rViewMtx = rContext.mViewMtx;
    sead::Vector3f viewPos;
    viewPos.setMul(rViewMtx, rLight.mPos);

    f32 screenX;
    f32 screenY;
    f32 scale;
    if (rLight.mFlags.isOn(2))
    {
        const sead::Matrix44f& rViewProjMtx = rContext.mViewProjMtx;
        const sead::Vector3f& rPos = rLight.mPos;
        f32 x = rViewProjMtx(0, 0) * rPos.x + rViewProjMtx(0, 1) * rPos.y +
                rViewProjMtx(0, 2) * rPos.z + rViewProjMtx(0, 3);
        f32 y = rViewProjMtx(1, 0) * rPos.x + rViewProjMtx(1, 1) * rPos.y +
                rViewProjMtx(1, 2) * rPos.z + rViewProjMtx(1, 3);
        f32 w = rViewProjMtx(3, 0) * rPos.x + rViewProjMtx(3, 1) * rPos.y +
                rViewProjMtx(3, 2) * rPos.z + rViewProjMtx(3, 3);
        screenX = -(rContext.mProjScale * (x / w)) - rContext.mScreenScaleX;
        screenY = -(rContext.mProjSign * ((y / w) * (rContext.mProjSign *
                                                      rContext.mCulling.mTanHalfFovy))) -
                  rContext.mScreenScaleY;
        scale = w * w * (1.0f / (rLight.mRadius * rLight.mRadius));
    }

    sead::Vector3f specColor(rLight.mSpecColor.r, rLight.mSpecColor.g, rLight.mSpecColor.b);
    if (rLight.mFlags.isOn(2) && !mLightPrePass->getFlags().isOn(1 << 8))
    {
        specColor.x = rLight.mSpecColor.a * (rLight.mSpecColor.r * 0.298912f +
                                             rLight.mSpecColor.g * 0.586611f +
                                             rLight.mSpecColor.b * 0.114478f);
    }

    f32 radiusScale = mLightPrePass->getFlags().isOn(1 << 5) ? 1.01f : 1.0f;
    radiusScale = cQualityRadiusScale[mLightPrePass->getQuality()] * radiusScale;
    radiusScale = radiusScale + radiusScale;

    const UniformBlock& rUbo = rLight.mView[view].mUbo;
    rUbo.dcbz(0);
    {
        sead::Vector4f data(rLight.mPos.x, rLight.mPos.y, rLight.mPos.z,
                            radiusScale * rLight.mRadius);
        rUbo.setData(0, &data, 0, 1);
    }

    {
        sead::Vector4f data(rLight.mColor.r, rLight.mColor.g, rLight.mColor.b,
                            1.0f / rLight.mRadius);
        rUbo.setData(1, &data, 0, 1);
    }

    {
        sead::Vector4f data(viewPos.x, viewPos.y, viewPos.z,
                            rLight.mAttnPow > 0.0f ? rLight.mAttnPow : 0.001f);
        rUbo.setData(3, &data, 0, 1);
    }

    if (rLight.mFlags.isOn(2))
    {
        {
            sead::Vector4f data(specColor.x, specColor.y, specColor.z, rLight.mAttnStart);
            rUbo.setData(2, &data, 0, 1);
        }

        {
            sead::Vector4f data(screenX, screenY, scale, 0.0f);
            rUbo.setData(4, &data, 0, 1);
        }
    }
}

void LightPrePass::PointLightMgr::drawImpl_(DrawContext* pDrawContext, const PointLight& rLight,
                                            s32 view, const Context& rContext,
                                            CallbackArg& rArg) const
{
    if (mDrawCallback)
    {
        if (!mDrawCallback->invoke(rArg, rLight))
        {
            return;
        }

        mVertexAttribute.activate(pDrawContext);
    }
    else
    {
        bool useSpec;
        if (rLight.mFlags.isOn(2) &&
            (rLight.mSpecColor.r != 0.0f || rLight.mSpecColor.g != 0.0f ||
             rLight.mSpecColor.b != 0.0f) &&
            !mLightPrePass->getFlags().isOn(1 << 16))
        {
            useSpec = true;
        }
        else
        {
            if (rLight.mColor.r == 0.0f && rLight.mColor.g == 0.0f && rLight.mColor.b == 0.0f)
            {
                return;
            }

            useSpec = false;
        }

        const sead::BitFlag32& rFlags = mLightPrePass->getFlags();
        bool splitSpec = rFlags.isOn(1 << 9);
        const ShaderProgram* pProgram =
            getShader(cLightType_Point, useSpec & splitSpec,
                      (rLight.mAttnStart > 0.0f) & splitSpec, false, false, cShadowType_None,
                      false, rFlags.isOn(1 << 8));
        mLightPrePass->applyGraphicsContext(pDrawContext, rArg, useSpec, false);
        pProgram->activate(rArg.mDrawContext, true);
        mVertexAttribute.activate(pDrawContext);
        rArg.mDepthSampler->activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
        rArg.mNormalSampler->activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
        rArg.mSpecPowSampler->activate(pDrawContext, pProgram->getSamplerLocation(2), -1, false);
        rArg.mViewUbo->activate(pDrawContext, pProgram->getUniformBlockLocation(0));
        rLight.mView[view].mUbo.activate(pDrawContext, pProgram->getUniformBlockLocation(1));
    }

    pfx::detail::drawIndexStream(
        pDrawContext,
        utl::PrimitiveShape::instance()->getSphereIndexStream(mLightPrePass->getQuality()));
}

void LightPrePass::PointLightMgr::drawDebugImpl_(DrawContext* pDrawContext,
                                                 const PointLight& rLight, s32 view,
                                                 const Context& rContext) const
{
    utl::DevTools::drawPointLight(pDrawContext, rLight.mPos, rLight.mRadius, rLight.mColor,
                                  rContext.mCulling.mViewMtx, rContext.mCulling.mProjMtx);
    utl::DevTools::drawPointLight(pDrawContext, rLight.mPos, rLight.mAttnStart,
                                  rLight.mSpecColor, rContext.mCulling.mViewMtx,
                                  rContext.mCulling.mProjMtx);
    utl::DevTools::beginDrawImm(pDrawContext, rContext.mCulling.mViewMtx,
                                rContext.mCulling.mProjMtx);
    sead::Matrix34f mtx(1.0f, 0.0f, 0.0f, rLight.mPos.x, 0.0f, 1.0f, 0.0f, rLight.mPos.y, 0.0f,
                        0.0f, 1.0f, rLight.mPos.z);
    utl::DevTools::drawAxisImm(pDrawContext, mtx, rLight.mRadius, 1.0f, 1.0f);
}

void LightPrePass::PointLightMgr::drawDebugTestImpl_(DrawContext* pDrawContext,
                                                     const PointLight& rLight, s32 view,
                                                     const Context& rContext) const
{
    utl::DevTools::drawPointLight(pDrawContext, rLight.mPos, rLight.mAttnStart,
                                  rLight.mSpecColor, rContext.mCulling.mViewMtx,
                                  rContext.mCulling.mProjMtx);
}

void LightPrePass::PointLightMgr::genMessageImpl_(sead::hostio::Context* pContext,
                                                  PointLight& rLight, s32 index)
{
}

void LightPrePass::SpotLightMgr::initPrepareImpl_(sead::Heap* pHeap)
{
    mVertexAttribute.create(1, pHeap);
    mVertexAttribute.setVertexStream(0, &utl::PrimitiveShape::instance()->getConeVertexBuffer(), 0);
    mVertexAttribute.setUp();
}

void LightPrePass::SpotLightMgr::initImpl_(SpotLight& rLight, sead::Heap* pHeap)
{
    rLight.mFlags = 3;
    rLight.mPos = sead::Vector3f::zero;
    rLight.mDir = -sead::Vector3f::ey;
    rLight.mAngle = 1.5707964f;
    rLight.mLength = 5.0f;
    rLight.mAttnPow = 1.0f;
    rLight.mAngleAttnPow = 0.5f;
    for (auto& rView : rLight.mView)
    {
        rView.mShadowMap = nullptr;
        rView.mShadowSampler = new (pHeap) TextureSampler();
        rView.mShadowSampler->setWrap(5, 5, 5);
        rView.mShadowSampler->setBorderColor(sead::Color4f::cWhite);
        rView.mShadowSampler->setDepthCompareEnable(true);
        rView.mShadowSampler->setDepthCompareFunc(4);
    }
}

void LightPrePass::SpotLightMgr::initViewUboImpl_(UniformBlock* pUbo, sead::Heap* pHeap)
{
    createSpotLightUBO(pUbo, pHeap);
}

void LightPrePass::SpotLightMgr::destroyImpl_(SpotLight& rLight)
{
    for (auto& rView : rLight.mView)
    {
        delete rView.mShadowSampler;
    }
}

void LightPrePass::SpotLightMgr::destroyFinishImpl_()
{
    mVertexAttribute.destroy();
}

bool LightPrePass::SpotLightMgr::calcViewImpl_(SpotLight& rLight, s32 view,
                                               const Context& rContext)
{
    return rContext.mCulling.isInside(rLight.mPos, rLight.mLength);
}

void LightPrePass::SpotLightMgr::drawImpl_(DrawContext* pDrawContext, const SpotLight& rLight,
                                           s32 view, const Context& rContext,
                                           CallbackArg& rArg) const
{
    bool useSpec;
    if (rLight.mFlags.isOff(2) || (rLight.mSpecColor.r == 0.0f && rLight.mSpecColor.g == 0.0f &&
                                   rLight.mSpecColor.b == 0.0f))
    {
        useSpec = false;
    }
    else
    {
        useSpec = !mLightPrePass->getFlags().isOn(1 << 16);
    }

    if (mDrawCallback)
    {
        if (!mDrawCallback->invoke(rArg, rLight))
        {
            return;
        }

        mVertexAttribute.activate(pDrawContext);
    }
    else
    {
        if (!useSpec &&
            (rLight.mColor.r == 0.0f && rLight.mColor.g == 0.0f && rLight.mColor.b == 0.0f))
        {
            return;
        }

        mLightPrePass->applyGraphicsContext(pDrawContext, rArg, useSpec, false);
        ShadowType shadowType =
            rLight.mView[view].mShadowMap != nullptr ? rLight.mShadowType : cShadowType_None;
        const sead::BitFlag32& rFlags = mLightPrePass->getFlags();
        const ShaderProgram* pProgram =
            getShader(cLightType_Spot, useSpec & rFlags.isOn(1 << 9), false, false, false,
                      shadowType, false, rFlags.isOn(1 << 8));
        pProgram->activate(rArg.mDrawContext, true);
        mVertexAttribute.activate(pDrawContext);
        rArg.mDepthSampler->activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
        rArg.mNormalSampler->activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
        rArg.mSpecPowSampler->activate(pDrawContext, pProgram->getSamplerLocation(2), -1, false);
        rArg.mViewUbo->activate(pDrawContext, pProgram->getUniformBlockLocation(0));
        if (rLight.mView[view].mShadowMap != nullptr)
        {
            rLight.mView[view].mShadowSampler->applyTextureData(
                rLight.mView[view].mShadowMap->getTextureData());
            rLight.mView[view].mShadowSampler->activate(pDrawContext,
                                                        pProgram->getSamplerLocation(4), -1, false);
        }

        rLight.mView[view].mUbo.activate(pDrawContext, pProgram->getUniformBlockLocation(2));
    }

    pfx::detail::drawIndexStream(
        pDrawContext,
        utl::PrimitiveShape::instance()->getConeTriangleIndexStream(mLightPrePass->getQuality()));
}

void LightPrePass::SpotLightMgr::drawDebugImpl_(DrawContext* pDrawContext,
                                                const SpotLight& rLight, s32 view,
                                                const Context& rContext) const
{
    sead::Vector3f dir = rLight.mDir;
    if (dir.equals(sead::Vector3f::ey, 0.01f))
    {
        dir.set(0.01f, 0.99f, -0.01f);
    }

    dir.normalize();
    utl::DevTools::drawSpotLight(pDrawContext, rLight.mPos, dir, rLight.mColor, rLight.mAngle,
                                 rLight.mLength, rContext.mCulling.mViewMtx,
                                 rContext.mCulling.mProjMtx);
}

void LightPrePass::SpotLightMgr::genMessageImpl_(sead::hostio::Context* pContext,
                                                 SpotLight& rLight, s32 index)
{
}

void LightPrePass::ProjLightMgr::initPrepareImpl_(sead::Heap* pHeap)
{
    mVertexAttribute.create(1, pHeap);
    mVertexAttribute.setVertexStream(0, &utl::PrimitiveShape::instance()->getCubeVertexBuffer(), 0);
    mVertexAttribute.setUp();
}

void LightPrePass::ProjLightMgr::initImpl_(ProjLight& rLight, sead::Heap* pHeap)
{
    rLight.mFlags = 0xb;
    rLight.mPos = sead::Vector3f::zero;
    rLight.mDir = -sead::Vector3f::ey;
    rLight.mUp = sead::Vector3f::ey;
    rLight.mColor = sead::Color4f::cWhite;
    rLight.mSpecColor = sead::Color4f::cBlack;
    rLight.mParam[0] = 0.4f;
    rLight.mParam[1] = 2.0f;
    rLight.mParam[2] = 0.7853982f;
    rLight.mParam[3] = 0.7f;
    rLight.mParam[4] = 0.5f;
    rLight.mParam[5] = -0.5f;
    rLight.mParam[6] = -0.8f;
    rLight.mParam[7] = 0.8f;
    rLight.mAttnPow = 1.0f;
    rLight.mTexScale = sead::Vector2f::zero;
    rLight.mTexOffset.set(1.0f, 1.0f);
    rLight.mHasTexture = false;
    rLight._268 = 0;
    rLight._260 = false;
    for (auto& rView : rLight.mView)
    {
        rView.mShadowMap = nullptr;
        rView.mShadowSampler = new (pHeap) TextureSampler();
        rView.mShadowSampler->setWrap(5, 5, 5);
        rView.mShadowSampler->setBorderColor(sead::Color4f::cBlack);
        rView.mShadowSampler->setDepthCompareEnable(true);
        rView.mShadowSampler->setDepthCompareFunc(4);
    }
}

void LightPrePass::ProjLightMgr::initViewUboImpl_(UniformBlock* pUbo, sead::Heap* pHeap)
{
    createProjLightUBO(pUbo, pHeap);
}

void LightPrePass::ProjLightMgr::destroyImpl_(ProjLight& rLight)
{
    for (auto& rView : rLight.mView)
    {
        delete rView.mShadowSampler;
    }
}

void LightPrePass::ProjLightMgr::destroyFinishImpl_()
{
    mVertexAttribute.destroy();
}

bool LightPrePass::ProjLightMgr::calcViewImpl_(ProjLight& rLight, s32 view,
                                               const Context& rContext)
{
    return rContext.mCulling.isInside(rLight.mBoundBox.getMin(), rLight.mBoundBox.getMax());
}

void LightPrePass::ProjLightMgr::drawImpl_(DrawContext* pDrawContext, const ProjLight& rLight,
                                           s32 view, const Context& rContext,
                                           CallbackArg& rArg) const
{
    bool useSpec;
    if (rLight.mFlags.isOff(2) || (rLight.mSpecColor.r == 0.0f && rLight.mSpecColor.g == 0.0f &&
                                   rLight.mSpecColor.b == 0.0f))
    {
        useSpec = false;
    }
    else
    {
        useSpec = !mLightPrePass->getFlags().isOn(1 << 16);
    }

    if (mDrawCallback)
    {
        if (!mDrawCallback->invoke(rArg, rLight))
        {
            return;
        }

        mVertexAttribute.activate(pDrawContext);
    }
    else
    {
        if (!useSpec &&
            (rLight.mColor.r == 0.0f && rLight.mColor.g == 0.0f && rLight.mColor.b == 0.0f))
        {
            return;
        }

        mLightPrePass->applyGraphicsContext(pDrawContext, rArg, useSpec, false);
        bool useTexture = rLight._260 || rLight.mHasTexture;
        ShadowType shadowType =
            rLight.mView[view].mShadowMap != nullptr ? rLight.mShadowType : cShadowType_None;
        const sead::BitFlag32& rFlags = mLightPrePass->getFlags();
        const ShaderProgram* pProgram = getShader(
            cLightType_Proj, useSpec & rFlags.isOn(1 << 9), false, useTexture,
            rLight.mFlags.isOn(1 << 4), shadowType, rLight.mFlags.isOn(1 << 2),
            rFlags.isOn(1 << 8));
        pProgram->activate(rArg.mDrawContext, true);
        mVertexAttribute.activate(pDrawContext);
        rArg.mDepthSampler->activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
        rArg.mNormalSampler->activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
        rArg.mSpecPowSampler->activate(pDrawContext, pProgram->getSamplerLocation(2), -1, false);
        rArg.mViewUbo->activate(pDrawContext, pProgram->getUniformBlockLocation(0));
        if (rLight._260 && rLight.mFlags.isOn(1 << 3))
        {
            rLight.mTempTextureSampler.activate(pDrawContext, pProgram->getSamplerLocation(3), -1,
                                                false);
        }
        else if (rLight.mHasTexture)
        {
            rLight.mTexture.activate(pDrawContext, pProgram->getSamplerLocation(3), -1, false);
        }

        if (rLight.mView[view].mShadowMap != nullptr)
        {
            rLight.mView[view].mShadowSampler->applyTextureData(
                rLight.mView[view].mShadowMap->getTextureData());
            rLight.mView[view].mShadowSampler->activate(pDrawContext,
                                                        pProgram->getSamplerLocation(4), -1, false);
        }

        rLight.mView[view].mUbo.activate(pDrawContext, pProgram->getUniformBlockLocation(3));
    }

    pfx::detail::drawIndexStream(pDrawContext,
                                 utl::PrimitiveShape::instance()->getCubeIndexStream());
}

void LightPrePass::ProjLightMgr::drawDebugImpl_(DrawContext* pDrawContext,
                                                const ProjLight& rLight, s32 view,
                                                const Context& rContext) const
{
    utl::DevTools::drawProjLight(pDrawContext, rLight.mDebugPos, rLight.mNormDir, rLight.mNormUp,
                                 rLight.mColor, rLight.mParam[2], rLight.mParam[3],
                                 rLight.mParam[0], rLight.mParam[1], rLight.mDebugParam1,
                                 rLight.mDebugParam0, rLight.mPos, rLight.mFlags.isOn(4),
                                 rContext.mCulling.mViewMtx, rContext.mCulling.mProjMtx);
}

void LightPrePass::ProjLightMgr::genMessageImpl_(sead::hostio::Context* pContext,
                                                 ProjLight& rLight, s32 index)
{
}

void LightPrePass::ProjLightMgr::listenPropertyEventImpl_(
    const sead::hostio::PropertyEvent* pEvent, ProjLight& rLight, s32 index)
{
    if (!(pEvent->getType() & 2) &&
        ((pEvent->getId() < &rLight.mPos + 1 && pEvent->getId() >= &rLight.mPos) ||
         (pEvent->getId() < &rLight.mDir + 1 && pEvent->getId() >= &rLight.mDir) ||
         (pEvent->getId() < &rLight.mUp + 1 && pEvent->getId() >= &rLight.mUp) ||
         (pEvent->getId() < &rLight.mParam[2] + 1 && pEvent->getId() >= &rLight.mParam[2]) ||
         (pEvent->getId() < &rLight.mParam[4] + 1 && pEvent->getId() >= &rLight.mParam[4]) ||
         (pEvent->getId() < &rLight.mParam[5] + 1 && pEvent->getId() >= &rLight.mParam[5]) ||
         (pEvent->getId() < &rLight.mParam[6] + 1 && pEvent->getId() >= &rLight.mParam[6]) ||
         (pEvent->getId() < &rLight.mParam[7] + 1 && pEvent->getId() >= &rLight.mParam[7])))
    {
        updateParameters_(rLight);
    }

    if (!(pEvent->getType() & 2) &&
        ((pEvent->getId() < &rLight.mDir + 1 && pEvent->getId() >= &rLight.mDir) ||
         (pEvent->getId() < &rLight.mUp + 1 && pEvent->getId() >= &rLight.mUp)))
    {
        rLight.setNormVec();
    }

    if (reinterpret_cast<uintptr_t>(pEvent->getId()) - index == 1000)
    {
        loadTextureOR(&rLight);
    }
}


}  // namespace agl::lght
