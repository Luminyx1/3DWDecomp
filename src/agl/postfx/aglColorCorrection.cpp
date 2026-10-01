#include "postfx/aglColorCorrection.h"

#include <gfx/seadViewport.h>
#include <hostio/seadHostIONodeEvent.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <math/seadMathCalcCommon.h>
#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "common/aglTextureDataInitializer.h"
#include "detail/aglRootNode.h"
#include "detail/aglShaderHolder.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglDebugTexturePage.h"

namespace agl::pfx {

namespace {

constexpr f32 cLumaR = 0.298912f;
constexpr f32 cLumaG = 0.586611f;
constexpr f32 cLumaB = 0.114478f;

}  // namespace

ColorCorrection::ColorCorrection() : IParameterIO("aglccr", 0)
{
    mGraphicsContext.setDepthEnable(false, false);
    mGraphicsContext.setBlendEnable(false);
    mLevelCurve.setChannelNum(3);
    agl::detail::RootNode::setNodeMeta(this, "Icon=COLOR");
    addObj(&mParamObj, "color_correction");
    resetAll();
}

void ColorCorrection::resetAll()
{
    *mHue = 0.0f;
    *mSaturation = 1.0f;
    *mBrightness = 1.0f;
    *mGamma = 1.0f;
    mLevelCurve.getCurve().reset();
    *mToyCameraFirst = false;
    for (s32 i = 0; i < cLevelTableNum - 1; i++)
    {
        f32 value = f32(i) / 7.0f;
        mLevelTable[i].set(value, value, value, value);
    }

    mLevelTable[cLevelTableNum - 1] = mLevelTable[cLevelTableNum - 2];
    *mToyCameraOffset1 = sead::Color4f::cBlack;
    *mToyCameraOffset2 = sead::Color4f::cBlack;
    *mToyCameraLevel1 = sead::Color4f::cWhite;
    *mToyCameraLevel2 = sead::Color4f::cWhite;
    *mToyCameraSaturation1 = 1.0f;
    *mToyCameraSaturation2 = 1.0f;
    *mToyCameraBrightness = 1.0f;
    *mToyCameraContrast = 1.0f;
    *mToyCameraMulColor = sead::Color4f::cWhite;
    mVariationIndex = 0;
    mFlags.setDirect(cFlag_UpdateMap);
}

ColorCorrection::~ColorCorrection()
{
    destroy_();
}

void ColorCorrection::destroy_()
{
    mSamplers.freeBuffer();
    mMapImage.deleteGPUMemBlock();

    if (mDebugTexturePage != nullptr)
    {
        mDebugTexturePage->cleanUp();
        delete mDebugTexturePage;
        mDebugTexturePage = nullptr;
    }
}

void ColorCorrection::initialize(s32 contextNum, sead::Heap* pHeap, bool unused)
{
    mSamplers.tryAllocBuffer(contextNum, pHeap);

    mMapTexture.initialize_(TextureType::cTextureType_3D,
                            TextureFormat::cTextureFormat_R11_G11_B10_float, cMapSize, cMapSize,
                            cMapSize, 1, TextureAttribute(0), MultiSampleType(0), true);
    u32 size = mMapTexture.getSurface().mStorageSize;
    u32 alignment = mMapTexture.getSurface().mAlignment;
    auto* block = new (pHeap, 8) GPUMemBlockU8;
    block->allocBuffer_(size, pHeap, alignment, MemoryAttribute::CompressibleMemory);
    mMapImage = GPUMemAddr<u8>(*block, 0);
    mMapTexture.setDebugLabel("agl::pfx::ColorCorrection");
    mMapTexture.setImagePtr(mMapImage);

    mRenderBuffer.setPhysicalArea(0.0f, 0.0f, f32(cMapSize), f32(cMapSize));
    mRenderBuffer.setVirtualSize(sead::Vector2f(f32(cMapSize), f32(cMapSize)));

    for (s32 i = 0; i < cMapSize; i++)
    {
        mRenderTarget[i].applyTextureData(mMapTexture);
        mRenderTarget[i].setSlice(i);
        mRenderTarget[i].setMipLevel(0);
        mRenderBuffer.setRenderTargetColor(&mRenderTarget[i], i);
    }

    mMapSampler.applyTextureData(mMapTexture);
    mMapScaleOffset.set(0.875f, 0.0625f);
}

void ColorCorrection::drawMap(DrawContext* pDrawContext) const
{
    if (!*mEnable || !mFlags.isOn(cFlag_UpdateMap | cFlag_ForceUpdateMap))
    {
        return;
    }

    if (detail::isDynamicTextureCaching())
    {
        return;
    }

    if (mFlags.isOn(cFlag_UpdateProgram))
    {
        updateProgram_();
    }

    const ShaderProgram* program =
        agl::detail::ShaderHolder::instance()
            ->getShaderProgramUnsafe(agl::detail::ShaderHolder::cColorCorrectionMap)
            ->getVariation(mVariationIndex);
    program->activate(pDrawContext, true);

    {
        sead::Vector4f param(0.571428597f, 0.500199974f, 0.500100017f, 0.500100017f);
        program->getUniformLocation(0).setUniform(pDrawContext, 4, &param);
    }

    {
        sead::Vector4f param(*mHue / 60.0f, *mSaturation, *mBrightness, 1.0f / *mGamma);
        program->getUniformLocation(1).setUniform(pDrawContext, 4, &param);
    }

    program->getUniformLocation(2).setUniform(pDrawContext, 0x20, &mLevelTable[0]);
    program->getUniformLocation(3).setUniform(pDrawContext, 0x20, &mLevelTable[1]);

    if (mFlags.isOn(cFlag_ToyCamera))
    {
        sead::Color4f level1 = *mToyCameraLevel1 * mToyCameraLevel1->a;
        sead::Color4f level2 = *mToyCameraLevel2 * mToyCameraLevel2->a;
        sead::Vector4f invLevel1(1.0f / level1.r, 1.0f / level1.g, 1.0f / level1.b, 1.0f);
        sead::Vector4f invLevel2(1.0f / level2.r, 1.0f / level2.g, 1.0f / level2.b, 1.0f);
        sead::Color4f mulColor = *mToyCameraMulColor * mToyCameraMulColor->a;

        program->getUniformLocation(4).setUniform(pDrawContext, 4, &*mToyCameraOffset1);
        program->getUniformLocation(5).setUniform(pDrawContext, 4, &*mToyCameraOffset2);
        program->getUniformLocation(6).setUniform(pDrawContext, 4, &invLevel1);
        program->getUniformLocation(7).setUniform(pDrawContext, 4, &invLevel2);
        f32 saturation1 = *mToyCameraSaturation1;
        program->getUniformLocation(8).setUniform(pDrawContext, saturation1);
        f32 saturation2 = *mToyCameraSaturation2;
        program->getUniformLocation(9).setUniform(pDrawContext, saturation2);
        f32 brightness = 0.5f - (1.0f - *mToyCameraBrightness) * 0.5f;
        program->getUniformLocation(10).setUniform(pDrawContext, brightness);
        f32 contrast = *mToyCameraContrast;
        program->getUniformLocation(11).setUniform(pDrawContext, contrast);
        program->getUniformLocation(12).setUniform(pDrawContext, 4, &mulColor);
    }

    sead::GraphicsContext context;
    context.setBlendEnableMask(0);
    context.setDepthEnable(false, false);
    context.apply(pDrawContext);

    sead::Viewport viewport(mRenderBuffer);
    viewport.apply(pDrawContext, mRenderBuffer);
    mRenderBuffer.bind(pDrawContext);
    detail::drawQuad(pDrawContext);
    mRenderTarget[0].invalidateGPUCache(pDrawContext);
    mFlags.reset(cFlag_UpdateMap);
}

void ColorCorrection::updateProgram_() const
{
    const ShaderProgram* program = agl::detail::ShaderHolder::instance()->getShaderProgram(
        agl::detail::ShaderHolder::cColorCorrectionMap);

    mVariationIndex = 0;

    if (mFlags.isOn(cFlag_Hue))
    {
        mVariationIndex = program->getVariationMacroStride(0) * 2;
    }
    else if (mFlags.isOn(cFlag_Saturation | cFlag_Brightness))
    {
        mVariationIndex = program->getVariationMacroStride(0);
    }

    if (mFlags.isOn(cFlag_Level))
    {
        mVariationIndex += program->getVariationMacroStride(1);
    }

    if (mFlags.isOn(cFlag_Gamma))
    {
        mVariationIndex += program->getVariationMacroStride(2);
    }

    if (mFlags.isOn(cFlag_ToyCamera))
    {
        mVariationIndex += program->getVariationMacroStride(3);
    }

    if (mFlags.isOn(cFlag_ToyCameraFirst))
    {
        mVariationIndex += program->getVariationMacroStride(4);
    }

    mFlags.reset(cFlag_UpdateProgram);
}

void ColorCorrection::draw(DrawContext* pDrawContext, s32 context,
                           const RenderBuffer& rRenderBuffer) const
{
    if (!*mEnable)
    {
        return;
    }

    TextureSampler& sampler = const_cast<TextureSampler&>(mSamplers[context]);
    sampler.applyTextureData(*reinterpret_cast<const TextureData*>(
        rRenderBuffer.getRenderTargetColor()));
    draw(pDrawContext, context, rRenderBuffer, mSamplers[context]);
}

void ColorCorrection::draw(DrawContext* pDrawContext, s32 context,
                           const RenderBuffer& rRenderBuffer, const TextureData& rTexture) const
{
    if (!*mEnable)
    {
        return;
    }

    TextureSampler& sampler = const_cast<TextureSampler&>(mSamplers[context]);
    sampler.applyTextureData(rTexture);
    draw(pDrawContext, context, rRenderBuffer, mSamplers[context]);
}

void ColorCorrection::draw(DrawContext* pDrawContext, s32 context,
                           const RenderBuffer& rRenderBuffer, const TextureSampler& rSampler) const
{
    if (!*mEnable)
    {
        return;
    }

    mGraphicsContext.apply(pDrawContext);
    sead::Viewport viewport(rRenderBuffer);
    viewport.apply(pDrawContext, rRenderBuffer);
    rRenderBuffer.bind(pDrawContext);

    const ShaderProgram* program = agl::detail::ShaderHolder::instance()->getShaderProgram(
        agl::detail::ShaderHolder::cColorCorrection);
    program->activate(pDrawContext, true);

    sead::Vector4f param(mMapScaleOffset.x, mMapScaleOffset.y, 0.0f, 0.0f);
    program->getUniformLocation(0).setUniform(pDrawContext, 4, &param);
    rSampler.activate(pDrawContext, program->getSamplerLocation(0), -1, false);
    mMapSampler.activate(pDrawContext, program->getSamplerLocation(1), -1, false);
    detail::drawQuad(pDrawContext);
}

void ColorCorrection::genMessage(sead::hostio::Context* pContext)
{
    genMessageIO(pContext, 0xf);

    if (mDebugTexturePage != nullptr)
    {
        mDebugTexturePage->genMessagePage(pContext, this);
    }

    mEnable.genMessageParameter(pContext, mEnable.getMeta());
    mToyCameraFirst.genMessageParameter(pContext, mToyCameraFirst.getMeta());
    genMessageParameters(pContext);
}

void ColorCorrection::genMessageParameters(sead::hostio::Context* pContext)
{
    if (mFlags.isOn(cFlag_ToyCameraFirst))
    {
        genMessageToyCameraParameters(pContext);
        genMessageHsbParameters(pContext);
    }
    else
    {
        genMessageHsbParameters(pContext);
        genMessageToyCameraParameters(pContext);
    }

    mLevelCurve.genMessageParameters(pContext);
    mGamma.genMessageParameter(pContext, mGamma.getMeta());
}

void ColorCorrection::genMessageToyCameraParameters(sead::hostio::Context* pContext)
{
    mToyCameraEnable.genMessageParameter(pContext, mToyCameraEnable.getMeta());

    if (!mFlags.isOn(cFlag_ToyCamera))
    {
        return;
    }

    mToyCameraOffset1.genMessageParameter(pContext, mToyCameraOffset1.getMeta());
    mToyCameraLevel1.genMessageParameter(pContext, mToyCameraLevel1.getMeta());
    mToyCameraSaturation1.genMessageParameter(pContext, mToyCameraSaturation1.getMeta());
    mToyCameraOffset2.genMessageParameter(pContext, mToyCameraOffset2.getMeta());
    mToyCameraLevel2.genMessageParameter(pContext, mToyCameraLevel2.getMeta());
    mToyCameraSaturation2.genMessageParameter(pContext, mToyCameraSaturation2.getMeta());
    mToyCameraBrightness.genMessageParameter(pContext, mToyCameraBrightness.getMeta());
    mToyCameraContrast.genMessageParameter(pContext, mToyCameraContrast.getMeta());
    mToyCameraMulColor.genMessageParameter(pContext, mToyCameraMulColor.getMeta());
}

void ColorCorrection::genMessageHsbParameters(sead::hostio::Context* pContext)
{
    mHue.genMessageParameter(pContext, mHue.getMeta());
    mSaturation.genMessageParameter(pContext, mSaturation.getMeta());
    mBrightness.genMessageParameter(pContext, mBrightness.getMeta());
}

/**
 * Handles a host property edit, resetting all parameters or updating the level curves as needed.
 * @param pEvent Property event received from the host.
 */
void ColorCorrection::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    listenPropertyEventIO(this, pEvent);

    if (pEvent->getIdValue() == 100001)
    {
        resetAll();
    }

    uintptr_t id = pEvent->getIdValue();

    if (id >= reinterpret_cast<uintptr_t>(&mLevelCurve) &&
        id < reinterpret_cast<uintptr_t>(&mDebugTexturePage))
    {
        mLevelCurve.listenPropertyEventParameters(this, pEvent);
        updateCurves_();
    }

    updateFlags_();
}

void ColorCorrection::updateCurves_()
{
    bool isLevel = false;

    for (s32 i = 0; i < cLevelTableNum - 1; i++)
    {
        f32 t = f32(i) / 7.0f;
        sead::Vector4f& level = mLevelTable[i];

        for (s32 j = 0; j < 4; j++)
        {
            auto& curve = mLevelCurve.getCurve().getCurve(j);
            (&level.x)[j] = curve.sead::hostio::Curve<f32>::interpolateToF32(t);
        }

        if (!sead::Mathf::equalsEpsilon(t, level.x) ||
            !sead::Mathf::equalsEpsilon(t, level.y) ||
            !sead::Mathf::equalsEpsilon(t, level.z))
        {
            isLevel = true;
        }
    }

    mLevelTable[cLevelTableNum - 1] = mLevelTable[cLevelTableNum - 2];

    if (mFlags.isOn(cFlag_Level) != isLevel)
    {
        mFlags.change(cFlag_Level, isLevel);
        mFlags.set(cFlag_UpdateProgram);
    }

    mFlags.set(cFlag_UpdateMap);
}

void ColorCorrection::updateFlags_()
{
    mFlags.change(cFlag_Hue, enablePassHue_());
    mFlags.change(cFlag_Saturation, enablePassSaturation_());
    mFlags.change(cFlag_Brightness, enablePassBrightness_());
    mFlags.change(cFlag_Gamma, enablePassGamma_());
    mFlags.change(cFlag_ToyCamera, *mToyCameraEnable);
    mFlags.change(cFlag_ToyCameraFirst, *mToyCameraFirst);
    mFlags.set(cFlag_UpdateMap | cFlag_UpdateProgram);
}

void ColorCorrection::listenNodeEvent(const sead::hostio::NodeEvent* pEvent)
{
    if (pEvent->getId() == 0 && mFlags.isOn(cFlag_Loaded))
    {
        mFlags.reset(cFlag_Loaded);
    }
}

void ColorCorrection::postRead_()
{
    mFlags.set(cFlag_Loaded);
    updateCurves_();
    updateFlags_();
}

bool ColorCorrection::enablePassHue_() const
{
    return *mHue != 0.0f;
}

bool ColorCorrection::enablePassSaturation_() const
{
    return *mSaturation != 1.0f;
}

bool ColorCorrection::enablePassGamma_() const
{
    return *mGamma != 1.0f;
}

bool ColorCorrection::enablePassBrightness_() const
{
    return *mBrightness != 1.0f;
}

void ColorCorrection::setHue(f32 hue)
{
    if (*mHue != hue)
    {
        bool prev = mFlags.isOn(cFlag_Hue);
        bool changed = (hue != 0.0f) != prev;
        *mHue = hue;
        if (changed)
        {
            mFlags.change(cFlag_Hue, enablePassHue_());
            mFlags.set(cFlag_UpdateProgram);
        }

        mFlags.set(cFlag_UpdateMap);
    }
}

void ColorCorrection::setSaturation(f32 saturation)
{
    if (*mSaturation != saturation)
    {
        bool enable = saturation != 1.0f;
        bool changed = enable != mFlags.isOn(cFlag_Saturation);
        *mSaturation = saturation;
        if (changed)
        {
            mFlags.change(cFlag_Saturation, enablePassSaturation_());
            mFlags.set(cFlag_UpdateProgram);
        }

        mFlags.set(cFlag_UpdateMap);
    }
}

void ColorCorrection::setBrightness(f32 brightness)
{
    if (*mBrightness != brightness)
    {
        bool enable = brightness != 1.0f;
        bool changed = enable != mFlags.isOn(cFlag_Brightness);
        *mBrightness = brightness;
        if (changed)
        {
            mFlags.change(cFlag_Brightness, enablePassBrightness_());
            mFlags.set(cFlag_UpdateProgram);
        }

        mFlags.set(cFlag_UpdateMap);
    }
}

void ColorCorrection::setGamma(f32 gamma)
{
    if (*mGamma != gamma)
    {
        bool enable = gamma != 1.0f;
        bool changed = enable != mFlags.isOn(cFlag_Gamma);
        *mGamma = gamma;
        if (changed)
        {
            mFlags.change(cFlag_Gamma, enablePassGamma_());
            mFlags.set(cFlag_UpdateProgram);
        }

        mFlags.set(cFlag_UpdateMap);
    }
}

void ColorCorrection::setLevelCurve_(u32 index, sead::hostio::CurveType type, const f32* pData,
                                     u32 num)
{
    auto& curve = mLevelCurve.getCurve().getCurve(index);
    curve.mInfo.curveType = u8(type);
    curve.mInfo.numUse = num;
    auto* data = reinterpret_cast<u32*>(&mLevelCurve.getCurve().getCurveData(0));
    data[index * 32 + 1] = u32(type);
    data[index * 32] = num;
    f32* pDst = reinterpret_cast<f32*>(&data[index * 32 + 2]);

    for (u32 i = 0; i < num; i++)
    {
        pDst[i] = pData[i];
    }

    updateCurves_();
}

void ColorCorrection::updateMapCPU_()
{
    s32 index = 0;

    for (s32 b = 0; b < cMapSize; b++)
    {
        f32 fb = f32(b) / 7.0f;

        for (s32 g = 0; g < cMapSize; g++)
        {
            f32 fg = f32(g) / 7.0f;

            for (s32 r = 0; r < cMapSize; r++)
            {
                convRGB_(&static_cast<u32*>(mMapImage.getPtr())[index++], f32(r) / 7.0f, fg,
                         fb);
            }
        }
    }

    TextureData texture;
    texture.initialize_(TextureType::cTextureType_3D,
                        TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, cMapSize, cMapSize,
                        cMapSize, 1, TextureAttribute(0), MultiSampleType(0), true);
    texture.setImagePtr(mMapImage);
    TextureDataInitializerRAW::copyTileImage(&texture, mMapImage, 0);
    mMapSampler.applyTextureData(texture);
}

void ColorCorrection::convRGB_(u32* pDst, f32 r, f32 g, f32 b) const
{
    if (mFlags.isOn(cFlag_ToyCamera))
    {
        calcToyCamera_(&r, &g, &b);
    }

    f32 max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    f32 min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    f32 hue = calcHue_(r, g, b);
    f32 saturation = max == 0.0f ? 0.0f : (max - min) / max;
    calcRGB_(pDst, hue, saturation, max);
}

void ColorCorrection::calcToyCamera_(f32* pR, f32* pG, f32* pB) const
{
    *pR = *pR + mToyCameraOffset1->r * (1.0f - *pR);
    *pG = *pG + mToyCameraOffset1->g * (1.0f - *pG);
    *pB = *pB + mToyCameraOffset1->b * (1.0f - *pB);
    *pR = std::pow(*pR, 1.0f / (mToyCameraLevel1->r * mToyCameraLevel1->a));
    *pG = std::pow(*pR, 1.0f / (mToyCameraLevel1->g * mToyCameraLevel1->a));
    *pB = std::pow(*pR, 1.0f / (mToyCameraLevel1->b * mToyCameraLevel1->a));
    calcSaturation_(pR, pG, pB, *mToyCameraSaturation1);

    *pR = *pR + mToyCameraOffset2->r * (1.0f - *pR);
    *pG = *pG + mToyCameraOffset2->g * (1.0f - *pG);
    *pB = *pB + mToyCameraOffset2->b * (1.0f - *pB);
    *pR = std::pow(*pR, 1.0f / (mToyCameraLevel2->r * mToyCameraLevel2->a));
    *pG = std::pow(*pR, 1.0f / (mToyCameraLevel2->g * mToyCameraLevel2->a));
    *pB = std::pow(*pR, 1.0f / (mToyCameraLevel2->b * mToyCameraLevel2->a));
    calcSaturation_(pR, pG, pB, *mToyCameraSaturation2);

    calcContrast_(pR, *mToyCameraContrast, *mToyCameraBrightness);
    calcContrast_(pG, *mToyCameraContrast, *mToyCameraBrightness);
    calcContrast_(pB, *mToyCameraContrast, *mToyCameraBrightness);

    *pR *= mToyCameraMulColor->r * mToyCameraMulColor->a;
    *pG *= mToyCameraMulColor->g * mToyCameraMulColor->a;
    *pB *= mToyCameraMulColor->b * mToyCameraMulColor->a;
}

f32 ColorCorrection::calcHue_(f32 r, f32 g, f32 b) const
{
    f32 max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    f32 min = r < g ? (r < b ? r : b) : (g < b ? g : b);

    f32 cr, cg, cb;

    if (max != min)
    {
        cr = (max - r) / (max - min);
        cg = (max - g) / (max - min);
        cb = (max - b) / (max - min);
    }
    else
    {
        cr = 0.0f;
        cg = 0.0f;
        cb = 0.0f;
    }

    f32 hue;

    if (max == r)
    {
        hue = cb - cg;
    }
    else if (max == g)
    {
        hue = cr + 2.0f - cb;
    }
    else if (max == b)
    {
        hue = cg + 4.0f - cr;
    }
    else
    {
        hue = 0.0f;
    }

    hue /= 6.0f;

    if (hue < 0.0f)
    {
        hue += 1.0f;
    }

    return hue;
}

void ColorCorrection::calcRGB_(u32* pDst, f32 h, f32 s, f32 v) const
{
    h += *mHue / 360.0f;

    if (h >= 1.0f)
    {
        h -= 1.0f;
    }

    if (h < 0.0f)
    {
        h += 1.0f;
    }

    h *= 6.0f;
    s32 i = sead::Mathf::floor(h);
    s *= *mSaturation;
    v *= *mBrightness;
    f32 f = h - f32(i);
    f32 p = v * (1.0f - s);
    f32 q = v * (1.0f - s * f);
    f32 t = v * (1.0f - s * (1.0f - f));

    f32 r, g, b;

    switch (i)
    {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    case 5:
        r = v;
        g = p;
        b = q;
        break;
    default:
        r = 0.0f;
        g = 0.0f;
        b = 0.0f;
        break;
    }

    auto& curve = mLevelCurve.getCurve();
    r = const_cast<sead::hostio::Curve<f32>&>(curve.getCurve(0))
            .sead::hostio::Curve<f32>::interpolateToF32(r);
    g = const_cast<sead::hostio::Curve<f32>&>(curve.getCurve(1))
            .sead::hostio::Curve<f32>::interpolateToF32(g);
    b = const_cast<sead::hostio::Curve<f32>&>(curve.getCurve(2))
            .sead::hostio::Curve<f32>::interpolateToF32(b);
    std::pow(r, 1.0f / *mGamma);
    std::pow(g, 1.0f / *mGamma);
    std::pow(b, 1.0f / *mGamma);
}

void ColorCorrection::calcSaturation_(f32* pR, f32* pG, f32* pB, f32 saturation) const
{
    f32 luma = *pR * cLumaR + *pG * cLumaG + *pB * cLumaB;
    *pR = luma + (*pR - luma) * saturation;
    *pG = luma + (*pG - luma) * saturation;
    *pB = luma + (*pB - luma) * saturation;
}

void ColorCorrection::calcContrast_(f32* pValue, f32 contrast, f32 brightness) const
{
    *pValue = sead::Mathf::clamp(
        (*pValue - 0.5f) * contrast - (1.0f - brightness) * 0.5f + 0.5f, 0.0f, 1.0f);
}

}  // namespace agl::pfx
