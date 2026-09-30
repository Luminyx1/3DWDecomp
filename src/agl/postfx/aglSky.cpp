#include "postfx/aglSky.h"

#include <cmath>
#include <framework/seadInfLoopChecker.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <math/seadBoundBox.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadMatrixCalcCommon.h>
#include <prim/seadSafeString.h>
#include "common/aglDrawContext.h"
#include "common/aglGPUMemAddr.h"
#include "common/aglGPUMemBlock.h"
#include "common/aglShaderProgram.h"
#include "common/aglShaderProgramArchive.h"
#include "detail/aglPrivateResource.h"
#include "detail/aglShaderHolder.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglDebugTexturePage.h"
#include "utility/aglDynamicTextureAllocator.h"
#include "utility/aglImageFilter2D.h"
#include "utility/aglPrimitiveTexture.h"

namespace agl::pfx {

namespace {

struct ProgramInfo {
    s32 mIndex;
    s32 mShaderHolderIndex;
    const char* mName;
};

const ProgramInfo cProgramInfo[] = {
    {0, agl::detail::ShaderHolder::cSkyTransmittance, "sky_transmittance"},
    {1, agl::detail::ShaderHolder::cSkyIrradiance, "sky_irradiance"},
    {2, agl::detail::ShaderHolder::cSkyInscatter, "sky_inscatter"},
    {3, agl::detail::ShaderHolder::cSkyDeltaInscatter, "sky_delta_inscatter"},
    {4, agl::detail::ShaderHolder::cSkyCopyIrradiance, "sky_copy_irradiance"},
    {5, agl::detail::ShaderHolder::cSkyCopyInscatter, "sky_copy_inscatter"},
    {6, agl::detail::ShaderHolder::cSkyBakeInscatter, "sky_bake_inscatter"},
    {7, agl::detail::ShaderHolder::cSkyBakeIrradiance, "sky_bake_irradiance"},
    {8, agl::detail::ShaderHolder::cSkyBakeRangeTransmittance, "sky_bake_range_transmittance"},
    {9, agl::detail::ShaderHolder::cSkyPostfxSky, "sky_postfx_sky"},
    {10, agl::detail::ShaderHolder::cSkyPostfxGround, "sky_postfx_ground"},
};

template <typename T>
inline void initParam(utl::Parameter<T>& rParam, const T& rValue, const char* rName,
                      utl::IParameterObj* pObj)
{
    rParam.init(rValue, rName, "", pObj);
}

const sead::Vector3f cDefaultWavelength(0.7f, 0.546f, 0.436f);

constexpr f32 cRadiusGround = 6360.0f;
constexpr f32 cRadiusTop = 6420.0f;

inline void setLayerParam(DrawContext* pDrawContext, const ShaderProgram* pProgram, s32 index,
                          f32 layer)
{
    f32 offset = layer <= 0.0f ? 0.001f : (layer >= 1.0f ? -0.001f : 0.0f);
    f32 r = sead::Mathf::sqrt(layer * layer * (cRadiusTop * cRadiusTop -
                                                cRadiusGround * cRadiusGround) +
                              cRadiusGround * cRadiusGround) +
            offset;
    f32 dmaxp = sead::Mathf::sqrt(r * r - cRadiusGround * cRadiusGround);
    f32 dmin = cRadiusTop - r;
    f32 dmax =
        dmaxp + sead::Mathf::sqrt(cRadiusTop * cRadiusTop - cRadiusGround * cRadiusGround);
    sead::Vector4f param(dmin, dmax, r - cRadiusGround, dmaxp);
    pProgram->getUniformLocation(index).setUniform(pDrawContext, 4, &param);
}

inline void setUniform(DrawContext* pDrawContext, const ShaderProgram* pProgram, s32 index,
                       f32 value)
{
    pProgram->getUniformLocation(index).setUniform(pDrawContext, value);
}

inline void setUniform(DrawContext* pDrawContext, const ShaderProgram* pProgram, s32 index,
                       s32 value)
{
    pProgram->getUniformLocation(index).setUniform(pDrawContext, 1, &value);
}

inline void drawIndexStream(DrawContext* pDrawContext, const IndexStream& rStream)
{
    u32 count = rStream.getCount();

    if (count == 0)
    {
        return;
    }

    NVNcommandBuffer* pCommandBuffer = pDrawContext->getNvnCommandBuffer();
    NVNdrawPrimitive primitive = rStream.getPrimitiveType();
    NVNbufferAddress address = nvnBufferGetAddress(rStream.getNvnBuffer());
    nvnCommandBufferDrawElements(pCommandBuffer, primitive, NVNindexType(rStream.getFormat()),
                                 count, address);
}

inline void drawQuad(DrawContext* pDrawContext)
{
    utl::VertexAttributeHolder::instance()
        ->getVertexAttribute(utl::VertexAttributeHolder::cAttribute_QuadTriangleTexCoord)
        .activate(pDrawContext);
    detail::drawIndexStream(pDrawContext,
                            utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
}

struct BasisDrawer {
    Sky* mSky;
    DrawContext* mDrawContext;
    const ShaderProgram* mProgram;

    void setProgram(const ShaderProgram* pProgram, const Sky::Sizes& rSizes);
    void drawLayer(f32 layer) const;
    void draw() const
    {
        utl::VertexAttributeHolder::instance()
            ->getVertexAttribute(utl::VertexAttributeHolder::cAttribute_QuadTriangleTexCoord)
            .activate(mDrawContext);
        detail::drawIndexStream(mDrawContext,
                                utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
    }

    void drawLayerQuad() const
    {
        utl::VertexAttributeHolder::instance()
            ->getVertexAttribute(utl::VertexAttributeHolder::cAttribute_QuadTriangleTexCoord)
            .activate(mDrawContext);
        drawIndexStream(mDrawContext,
                        utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
    }
};

struct BakeDrawer {
    DrawContext* mDrawContext;
    const ShaderProgram* mProgram;

    void drawLayer(bool isEffectiveBelowHorizon, f32 layer) const;
    void draw() const
    {
        utl::VertexAttributeHolder::instance()
            ->getVertexAttribute(utl::VertexAttributeHolder::cAttribute_QuadTriangleTexCoord)
            .activate(mDrawContext);
        drawIndexStream(mDrawContext,
                        utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
    }
};

inline void activateSampler(Sky* pSky, s32 context, s32 index, const TextureData& rTexture,
                            DrawContext* pDrawContext, const ShaderProgram* pProgram,
                            s32 location)
{
    pSky->getContext_(context).mSamplers[index].applyTextureData(rTexture);
    pSky->getContext_(context).mSamplers[index].activate(
        pDrawContext, pProgram->getSamplerLocation(location), -1, false);
}

struct PostFxDrawer {
    Sky* mSky;
    s32 mContext;
    const sead::Matrix44f* mProjMtx;
    const sead::Matrix34f* mViewMtx;
    f32 mNear;
    f32 mFar;
    f32 mAmplifier;
    const TextureData* mColor;
    const TextureData* mDepth;

    void draw(DrawContext* pDrawContext, const ShaderProgram* pProgram,
              const Sky::Sizes& rSizes) const;
};

/**
 * Creates a 2D texture of the given size and binds it to a sampler.
 * @param pSampler Sampler that receives the texture
 * @param pHeap Heap for the image memory
 * @param width Texture width
 * @param height Texture height
 */
void createTexture2D(TextureSampler* pSampler, sead::Heap* pHeap, u32 width, u32 height)
{
    TextureData texture;
    texture.initialize_(TextureType::cTextureType_2D, TextureFormat(0x2b), width, height, 1, 1,
                        TextureAttribute(0), MultiSampleType(0), true);
    u32 size = texture.getImageByteSize();
    u32 alignment = texture.getAlignment();
    auto* block = new (pHeap, 8) GPUMemBlock<u8>;
    block->allocBuffer_(size, pHeap, alignment, MemoryAttribute::CpuCached);
    texture.setImagePtr(GPUMemAddr<u8>(*block, 0));
    texture.getImagePtr().flushCPUCache(texture.getImageByteSize());
    pSampler->applyTextureData(texture);
}

/**
 * Creates a 3D texture of the given size and binds it to a sampler.
 * @param pSampler Sampler that receives the texture
 * @param pHeap Heap for the image memory
 * @param width Texture width
 * @param height Texture height
 * @param depth Texture depth
 */
inline void createTexture3D(TextureSampler* pSampler, sead::Heap* pHeap, u32 width, u32 height,
                            u32 depth)
{
    TextureData texture;
    texture.initialize_(TextureType::cTextureType_3D, TextureFormat(0x2b), width, height, depth,
                        1, TextureAttribute(0), MultiSampleType(0), true);
    u32 size = texture.getImageByteSize();
    u32 alignment = texture.getAlignment();
    auto* block = new (pHeap, 8) GPUMemBlock<u8>;
    block->allocBuffer_(size, pHeap, alignment, MemoryAttribute::CpuCached);
    texture.setImagePtr(GPUMemAddr<u8>(*block, 0));
    texture.getImagePtr().flushCPUCache(texture.getImageByteSize());
    pSampler->applyTextureData(texture);
}

/**
 * Calculates the normalized color of black body radiation at the given wavelengths.
 * @param rBase Color the radiance is divided by
 * @param temperature Temperature in kelvin
 * @param r Red wavelength in micrometers
 * @param g Green wavelength in micrometers
 * @param b Blue wavelength in micrometers
 * @return Normalized color
 */
sead::Color4f calcColorTemperature(const sead::Color4f& rBase, f32 temperature, f32 r, f32 g,
                                   f32 b)
{
    f64 lr = r * 1e-6;
    f64 lg = g * 1e-6;
    f64 lb = b * 1e-6;
    f64 cr = 4.992482532251199e-24 /
             (std::pow(lr, 5.0) *
              (std::exp(1.9864456832693028e-25 / (lr * 1.3806488e-23 * temperature)) - 1.0));
    f64 cg = 4.992482532251199e-24 /
             (std::pow(lg, 5.0) *
              (std::exp(1.9864456832693028e-25 / (lg * 1.3806488e-23 * temperature)) - 1.0));
    f64 cb = 4.992482532251199e-24 /
             (std::pow(lb, 5.0) *
              (std::exp(1.9864456832693028e-25 / (lb * 1.3806488e-23 * temperature)) - 1.0));
    cr /= rBase.r;
    cg /= rBase.g;
    cb /= rBase.b;
    f64 max = cr > cg ? (cr > cb ? cr : cb) : (cg > cb ? cg : cb);

    if (max == 0.0)
    {
        return sead::Color4f::cWhite;
    }

    return sead::Color4f(cr / max, cg / max, cb / max, 1.0f);
}

}  // namespace

Sky::Sky() : IParameterIO("ksky", 0), mWavelength(cDefaultWavelength), mParam()
{
    addObj(&mParam.mObj, "sky");
    mTextureSize = {};
    mTextureSizeEdit = mTextureSize;
    calcRayleighScatteringCoeff_();

    initParam(mParam.mVersion, 2, "version", &mParam.mObj);
    mParam.mStaticRayleighBaseHeight.init(16.0f, "static_rayleigh_base_height", "基準高さ",
                                          "Min = 1.0, Max = 25.0", &mParam.mObj);
    mParam.mStaticMieBaseHeight.init(1.2f, "static_mie_base_height", "基準高さ",
                                     "Min = 0.001, Max = 10.0", &mParam.mObj);
    mParam.mStaticMieScatteringCoeff.init(0.004f, "static_mie_scattering_coeff",
                                          "スキャッタリング係数", "Min = 0.0001, Max = 0.01",
                                          &mParam.mObj);
    mParam.mStaticMieSymmetricalProp.init(0.8f, "static_mie_symmetrical_prop", "対称性係数",
                                          "Min = 0.0, Max = 1.0, Mode = MinMaxLock", &mParam.mObj);
    mParam.mDynamicRayleighAmplifier.init(1.0f, "dynamic_rayleigh_amplifier",
                                          "強さ（物理的なパラメータではありません）",
                                          "Min = 0.0, Max = 2.0", &mParam.mObj);
    mParam.mDynamicMieSymmetricalProp.init(0.8f, "dynamic_mie_symmetrical_prop", "対称性係数",
                                           "Min = 0.0, Max = 1.0, Mode = MinMaxLock", &mParam.mObj);
    mParam.mDynamicMieAmplifier.init(1.0f, "dynamic_mie_amplifier",
                                     "強さ（物理的なパラメータではありません）",
                                     "Min = 0.0, Max = 2.0", &mParam.mObj);
    mParam.mDynamicColor.init(sead::Color4f(1.0f, 1.0f, 0.9f, 10.0f), "dynamic_color", "太陽光",
                              "", &mParam.mObj);
    mParam.mColorTemperature = 6500.0f;
    mParam.mFade = 0.0f;
    mParam.mSunDir.set(0.0f, -1.0f, 0.0f);
    mParam.mSunLatitude = 0.0f;
    mParam.mSunLongitude = 1.5707964f;
    mParam.mScatterFogNear.init(1000.0f, "scatter_fog_near", "フォグ開始距離",
                                "Min = -5000.0, Max = 20000.0", &mParam.mObj);
    mParam.mScatterFogFar.init(8000.0f, "scatter_fog_far", "フォグ終了距離",
                               "Min = -5000.0, Max = 20000.0", &mParam.mObj);
    mParam.mScatterFogDensity.init(1.0f, "scatter_fog_density", "フォグ濃度",
                                   "Min = 0.0, Max =  1.0", &mParam.mObj);
    mParam.mScatterFogAtten.init(2.5f, "scatter_fog_atten", "減衰（乗数）",
                                 "Min = 0.0, Max = 10.0", &mParam.mObj);
    mParam.mScatterFogHorz.init(1.0f, "scatter_fog_horz", "地平線色の出やすさ",
                                "Min = 0.0, Max = 10.0", &mParam.mObj);
    mParam.mAdhocFogNear.init(1000.0f, "adhoc_fog_near", "フォグ開始距離",
                              "Min = -5000.0, Max = 20000.0", &mParam.mObj);
    mParam.mAdhocFogFar.init(8000.0f, "adhoc_fog_far", "フォグ終了距離",
                             "Min = -5000.0, Max = 20000.0", &mParam.mObj);
    mParam.mAdhocFogAttenGround.init(1.0f, "adhoc_fog_atten_grd", "地面に対する減衰（乗数）",
                                     "Min = 0.0, Max = 10.0", &mParam.mObj);
    mParam.mAdhocFogAttenSky.init(0.5f, "adhoc_fog_atten_sky", "空に対する減衰（乗数）",
                                  "Min = 0.0, Max = 10.0", &mParam.mObj);
    mParam.mAdhocFogAttenMinScaleSky.init(0.5f, "adhoc_fog_atten_minscale_sky",
                                          "空に対する減衰の最低スケール",
                                          "Min = 0.0, Max = 1.0, Mode = MinMaxLock", &mParam.mObj);
    mParam.mAdhocFogColor.init(sead::Color4f::cWhite, "adhoc_fog_color", "色", "", &mParam.mObj);
    mParam.mRenderSunIntensity.init(100.0f, "render_sun_intensity", "インテンシティ",
                                    "Min = 0.0, Max = 10000.0", &mParam.mObj);
    mParam.mRenderSunSize.init(1.0f, "render_sun_size", "大きさ", "Min = 0.0, Max = 4.0",
                               &mParam.mObj);
    mParam.mRenderSunLerp.init(1.0f, "render_sun_lerp", "グラデーション係数",
                               "Min = 1.0, Max = 4.0", &mParam.mObj);
    mParam.mGroundColor.init(sead::Color4f(0.2f, 0.2f, 0.2f, 0.0f), "ground_color", "地面色", "",
                             &mParam.mObj);
    mParam.mAmplifierForEnvmap.init(1.0f, "amplifier_for_envmap",
                                    "キューブ環境マップ撮影時にのみ掛かる倍率",
                                    "Min = 0.0, Max = 10.0", &mParam.mObj);
    initParam(mParam.mFlag, 0u, "flg", &mParam.mObj);
    mFlags.set(cFlag_Sky | cFlag_Ground | cFlag_UpdateBasis | cFlag_Scattering | cFlag_SunDisk);

    OldParam* pOldParam = new (mOldParamBuffer) OldParam();
    initParam(pOldParam->mRayleighBaseHeight, 0.0f, "rayleigh_base_height", &mParam.mObj);
    initParam(pOldParam->mMieBaseHeight, 0.0f, "mie_base_height", &mParam.mObj);
    initParam(pOldParam->mMieScatteringCoeff, 0.0f, "mie_scattering_coeff", &mParam.mObj);
    initParam(pOldParam->mMieSymmetricalProp, 0.0f, "mie_symmetrical_prop", &mParam.mObj);
    initParam(pOldParam->mRayleighAmplifierRendering, 0.0f, "rayleigh_amplifier_rendering",
              &mParam.mObj);
    initParam(pOldParam->mMieSymmetricalPropRendering, 0.0f, "mie_symmetrical_prop_rendering",
              &mParam.mObj);
    initParam(pOldParam->mMieAmplifierRendering, 0.0f, "mie_amplifier_rendering", &mParam.mObj);
    initParam(pOldParam->mSunColor, sead::Color4f::cWhite, "sun_color", &mParam.mObj);
}

Sky::~Sky()
{
    finalize();
}

/**
 * Releases the textures, contexts, programs and debug page created by initialize.
 */
void Sky::finalize()
{
    if (!mFlags.isOn(cFlag_Initialized))
    {
        return;
    }

    freeTexture();
    mContexts.freeBuffer();
    mPrograms.freeBuffer();

    if (mDebugTexturePage)
    {
        mDebugTexturePage->cleanUp();
        delete mDebugTexturePage;
        mDebugTexturePage = nullptr;
    }

    mFlags.reset(cFlag_Initialized);
}

/**
 * Creates the sampler and uniform locations of every sky shader program.
 * @param pArchive Archive containing the sky programs
 * @param pHeap Heap for the location buffers
 */
void Sky::setUpShader(ShaderProgramArchive* pArchive, sead::Heap* pHeap)
{
    for (s32 i = 0; i < 11; i++)
    {
        s32 index = pArchive->searchShaderProgramIndex(cProgramInfo[i].mName);
        ShaderProgram* program = index >= 0 ? pArchive->getShaderProgramPtr(index) : nullptr;

        if (cProgramInfo[i].mIndex < 4)
        {
            program->createSamplerLocation(5, pHeap);
            program->createUniform(12, pHeap);
            program->setSamplerLocationName(0, "cTexTransmittance");
            program->setSamplerLocationName(1, "cTexDeltaE");
            program->setSamplerLocationName(2, "cTexDeltaJ");
            program->setSamplerLocationName(3, "cTexDeltaSR");
            program->setSamplerLocationName(4, "cTexDeltaSM");
            program->setUniformName(0, "cSizeTransmittance");
            program->setUniformName(1, "cSizeIrradiance");
            program->setUniformName(4, "cSizeViewZenithAngle");
            program->setUniformName(3, "cSizeSunZenithAngle");
            program->setUniformName(5, "cSizeSunViewAngle");
            program->setUniformName(2, "cSizeAltitude");
            program->setUniformName(6, "cScatteringCoeffRayleigh");
            program->setUniformName(7, "cScatteringCoeffMie");
            program->setUniformName(8, "cSymmetricalPropertyMie");
            program->setUniformName(9, "cHeightRayleigh");
            program->setUniformName(10, "cHeightMie");
            program->setUniformName(11, "cNonLinearParam");
        }

        switch (cProgramInfo[i].mIndex)
        {
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            program->createSamplerLocation(7, pHeap);
            program->createUniform(19, pHeap);
            program->setSamplerLocationName(0, "cTexDeltaE");
            program->setSamplerLocationName(1, "cTexDeltaJ");
            program->setSamplerLocationName(2, "cTexDeltaSR");
            program->setSamplerLocationName(3, "cTexDeltaSM");
            program->setSamplerLocationName(4, "cTexInscatter");
            program->setSamplerLocationName(6, "cTexTransmittance");
            program->setSamplerLocationName(5, "cTexIrradiance");
            program->setUniformName(1, "cSizeBakedInscatter");
            program->setUniformName(2, "cSizeBakedRangeTransmittance");
            program->setUniformName(0, "cSizeIrradiance");
            program->setUniformName(5, "cSizeViewZenithAngle");
            program->setUniformName(4, "cSizeSunZenithAngle");
            program->setUniformName(6, "cSizeSunViewAngle");
            program->setUniformName(3, "cSizeAltitude");
            program->setUniformName(7, "cScatteringCoeffRayleigh");
            program->setUniformName(8, "cScatteringCoeffMie");
            program->setUniformName(9, "cAmplifierReyleigh");
            program->setUniformName(10, "cAmplifierMie");
            program->setUniformName(11, "cSymmetricalPropertyMie");
            program->setUniformName(12, "cSunZenithAngle");
            program->setUniformName(13, "cSunColor");
            program->setUniformName(14, "cFade");
            program->setUniformName(15, "cNonLinearParam");
            program->setUniformName(16, "cNearFar");
            program->setUniformName(17, "cAltitude");
            program->setUniformName(18, "cEffectiveBelowHorizon");
            break;
        case 9:
        case 10:
            program->createSamplerLocation(9, pHeap);
            program->createUniform(27, pHeap);
            program->setSamplerLocationName(0, "cTexColor");
            program->setSamplerLocationName(1, "cTexDepth");
            program->setSamplerLocationName(2, "cTexTransmittance");
            program->setSamplerLocationName(3, "cTexIrradiance");
            program->setSamplerLocationName(4, "cTexInscatter");
            program->setSamplerLocationName(5, "cTexBakedInscatter");
            program->setSamplerLocationName(6, "cTexBakedIrradiance");
            program->setSamplerLocationName(7, "cTexBakedRangeTransmittance");
            program->setSamplerLocationName(8, "cTexCloud");
            program->setUniformName(1, "cSizeTransmittance");
            program->setUniformName(2, "cSizeIrradiance");
            program->setUniformName(5, "cSizeViewZenithAngle");
            program->setUniformName(4, "cSizeSunZenithAngle");
            program->setUniformName(6, "cSizeSunViewAngle");
            program->setUniformName(3, "cSizeAltitude");
            program->setUniformName(7, "cScatteringCoeffRayleigh");
            program->setUniformName(8, "cScatteringCoeffMie");
            program->setUniformName(9, "cSymmetricalPropertyMie");
            program->setUniformName(10, "cAmplifierReyleigh");
            program->setUniformName(11, "cAmplifierMie");
            program->setUniformName(12, "cAmplifierAdhoc");
            program->setUniformName(13, "cProjInverse[0]");
            program->setUniformName(14, "cViewInverse[0]");
            program->setUniformName(15, "cSunDir");
            program->setUniformName(16, "cSunColor");
            program->setUniformName(17, "cGroundColor");
            program->setUniformName(18, "cNearFar");
            program->setUniformName(19, "cSunInfo");
            program->setUniformName(20, "cScatterFogDistance");
            program->setUniformName(21, "cScatterFogCoeff");
            program->setUniformName(22, "cNormalFogDistance");
            program->setUniformName(23, "cNormalFogCoeff");
            program->setUniformName(24, "cNormalFogColor");
            program->setUniformName(25, "cCloudParam");
            program->setUniformName(26, "cExposure");
            break;
        }
    }
}

/**
 * Creates the programs, contexts, textures and debug page.
 * @param rArg Initialization settings
 */
void Sky::initialize(const InitializeArg& rArg)
{
    mPrograms.tryAllocBuffer(11, rArg.mHeap);
    mPrograms.fill(nullptr);

    for (s32 i = 0; i < 11; i++)
    {
        mPrograms[i] =
            agl::detail::ShaderHolder::instance()->getShaderProgram(cProgramInfo[i].mShaderHolderIndex);
    }

    mContexts.tryAllocBuffer(rArg.mContextNum, rArg.mHeap);
    u32 contextNum = mContexts.size();

    for (u32 i = 0; i < contextNum; i++)
    {
        mContexts[i].mIsEnable = true;
        mContexts[i].mIsSkyEnable = true;
        mContexts[i].mIsGroundEnable = true;
        mContexts[i].mIsEffectiveBelowHorizon = true;
        mContexts[i].mCloudSampler = nullptr;
        mContexts[i].mCloudParam = 1.0f;
        mContexts[i].mDirtyTargets = 0;
        mContexts[i].mBakeInfo.mAltitude = 0.0f;
        mContexts[i].mBakeInfo.mHorizon = 0.0f;
    }

    mTextureSize.mTransmittanceWidth = rArg.mTextureSize.mTransmittanceWidth;
    mTextureSize.mTransmittanceHeight = rArg.mTextureSize.mTransmittanceHeight;
    mTextureSize.mIrradianceWidth = rArg.mTextureSize.mIrradianceWidth;
    mTextureSize.mIrradianceHeight = rArg.mTextureSize.mIrradianceHeight;
    mTextureSize.mAltitudeNum = rArg.mTextureSize.mAltitudeNum;
    mTextureSize.mSunViewNum = rArg.mTextureSize.mSunViewNum;
    mTextureSize.mViewZenithNum = rArg.mTextureSize.mViewZenithNum;
    mTextureSize.mSunZenithNum = rArg.mTextureSize.mSunZenithNum;
    mTextureSize.mBakedInscatterWidth = rArg.mTextureSize.mBakedInscatterWidth;
    mTextureSize.mBakedInscatterHeight = rArg.mTextureSize.mBakedInscatterHeight;
    mTextureSize.mBakedRangeTransmittanceWidth = rArg.mTextureSize.mBakedRangeTransmittanceWidth;
    mTextureSize.mBakedRangeTransmittanceHeight =
        rArg.mTextureSize.mBakedRangeTransmittanceHeight;
    mTextureSizeEdit = mTextureSize;
    allocateTexture(rArg.mHeap);

    if (rArg.mBasisImage)
    {
        setBasisTextureImage(rArg.mBasisImage, rArg.mBasisImageSize);
    }

    mDebugTexturePage = new (rArg.mHeap, 8) utl::DebugTexturePage;
    mDebugTexturePage->setUp(rArg.mContextNum, "デバッグ表示", rArg.mHeap);
    mFlags.set(cFlag_Initialized);
}

/**
 * Allocates the basis textures and the baked textures of every context.
 * @param pHeap Heap for the image memory
 */
void Sky::allocateTexture(sead::Heap* pHeap)
{
    pHeap->getFreeSize();
    createTexture2D(&mTransmittanceSampler, pHeap, mTextureSize.mTransmittanceWidth,
                    mTextureSize.mTransmittanceHeight);
    pHeap->getFreeSize();
    createTexture3D(&mInscatterSampler, pHeap, mTextureSize.mSunViewNum * mTextureSize.mSunZenithNum,
                    mTextureSize.mViewZenithNum, mTextureSize.mAltitudeNum);
    pHeap->getFreeSize();
    {
        u32 width = mTextureSize.mIrradianceWidth;
        u32 height = mTextureSize.mIrradianceHeight;
        TextureData texture;
        createTexture2D(&mIrradianceSampler, pHeap, width, height);
        pHeap->getFreeSize();
    }

    for (s32 i = 0; i < mContexts.size(); i++)
    {
        createTexture2D(&mContexts[i].mBakedInscatter, pHeap, mTextureSize.mBakedInscatterHeight,
                        mTextureSize.mBakedInscatterWidth);
        pHeap->getFreeSize();
        createTexture2D(&mContexts[i].mBakedIrradiance, pHeap, mTextureSize.mIrradianceWidth,
                        mTextureSize.mIrradianceHeight);
        pHeap->getFreeSize();
        createTexture2D(&mContexts[i].mBakedRangeTransmittance, pHeap,
                        mTextureSize.mBakedRangeTransmittanceWidth,
                        mTextureSize.mBakedRangeTransmittanceHeight);
        pHeap->getFreeSize();
    }

    mFlags.set(cFlag_TextureAllocated);
}

/**
 * Copies a precomputed basis image that holds all three basis textures back to back.
 * @param pImage Image data
 * @param size Image size in bytes
 * @return Whether the size matched and the image was copied
 */
bool Sky::setBasisTextureImage(const void* pImage, u32 size)
{
    u32 transmittanceSize = mTransmittanceSampler.getTextureData().getImageByteSize();
    u32 inscatterSize = mInscatterSampler.getTextureData().getImageByteSize();
    u32 irradianceSize = mIrradianceSampler.getTextureData().getImageByteSize();

    if (transmittanceSize + inscatterSize + irradianceSize != size)
    {
        return false;
    }

    const u8* pInscatter =
        reinterpret_cast<const u8*>(reinterpret_cast<uintptr_t>(pImage) + transmittanceSize);
    const u8* pIrradiance = pInscatter + inscatterSize;
    return setBasisTextureImage(pImage, transmittanceSize, pInscatter, inscatterSize, pIrradiance,
                                irradianceSize);
}

/**
 * Frees the image memory of the basis textures and the baked textures.
 */
void Sky::freeTexture()
{
    mTransmittanceSampler.getTextureData().getImagePtr().deleteGPUMemBlock();
    mInscatterSampler.getTextureData().getImagePtr().deleteGPUMemBlock();
    mIrradianceSampler.getTextureData().getImagePtr().deleteGPUMemBlock();

    for (s32 i = 0; i < mContexts.size(); i++)
    {
        mContexts[i].mBakedInscatter.getTextureData().getImagePtr().deleteGPUMemBlock();
        mContexts[i].mBakedIrradiance.getTextureData().getImagePtr().deleteGPUMemBlock();
        mContexts[i].mBakedRangeTransmittance.getTextureData().getImagePtr().deleteGPUMemBlock();
    }

    mFlags.reset(cFlag_TextureAllocated);
}

/**
 * Copies precomputed basis images into the basis textures.
 * @param pTransmittance Transmittance image
 * @param transmittanceSize Transmittance image size
 * @param pInscatter Inscatter image
 * @param inscatterSize Inscatter image size
 * @param pIrradiance Irradiance image
 * @param irradianceSize Irradiance image size
 * @return Whether all sizes matched and the images were copied
 */
bool Sky::setBasisTextureImage(const void* pTransmittance, u32 transmittanceSize,
                               const void* pInscatter, u32 inscatterSize,
                               const void* pIrradiance, u32 irradianceSize)
{
    if (mTransmittanceSampler.getTextureData().getImageByteSize() != transmittanceSize ||
        mInscatterSampler.getTextureData().getImageByteSize() != inscatterSize ||
        mIrradianceSampler.getTextureData().getImageByteSize() != irradianceSize)
    {
        return false;
    }

    std::memcpy(mTransmittanceSampler.getTextureData().getImagePtr().getPtr(),
                pTransmittance, transmittanceSize);
    std::memcpy(mInscatterSampler.getTextureData().getImagePtr().getPtr(), pInscatter,
                inscatterSize);
    std::memcpy(mIrradianceSampler.getTextureData().getImagePtr().getPtr(), pIrradiance,
                irradianceSize);
    mTransmittanceSampler.getTextureData().getImagePtr().flushCPUCache(transmittanceSize);
    mInscatterSampler.getTextureData().getImagePtr().flushCPUCache(inscatterSize);
    mIrradianceSampler.getTextureData().getImagePtr().flushCPUCache(irradianceSize);
    mFlags.reset(cFlag_UpdateBasis);
    return true;
}

/**
 * Recomputes the basis textures when requested.
 * @param pDrawContext Draw context
 * @param context Context index
 */
void Sky::drawBasis(DrawContext* pDrawContext, u32 context)
{
    if (!mFlags.isOnAll(cFlag_Initialized | cFlag_Enable))
    {
        return;
    }

    if (!getContext_(context).mIsEnable)
    {
        return;
    }

    if (mFlags.isOn(cFlag_InfLoopCheckDisabled))
    {
        sead::InfLoopChecker::instance()->setEnabled(true);
        mFlags.reset(cFlag_InfLoopCheckDisabled);
    }

    if (mFlags.isOn(cFlag_UpdateBasis | cFlag_UpdateBasisAlways))
    {
        sead::InfLoopChecker* checker = sead::InfLoopChecker::instance();

        if (checker && checker->isEnabled())
        {
            mFlags.set(cFlag_InfLoopCheckDisabled);
            checker->setEnabled(false);
        }

        drawBasisImpl(pDrawContext, context);
        mFlags.reset(cFlag_UpdateBasis);
    }
}

/**
 * Precomputes the transmittance, irradiance and inscatter basis textures.
 * @param pDrawContext Draw context
 * @param context Context index
 */
void Sky::drawBasisImpl(DrawContext* pDrawContext, u32 context)
{
    BasisDrawer drawer = {this, pDrawContext, nullptr};
    BakeDrawer bakeDrawer = {pDrawContext, nullptr};
    utl::DynamicTextureAllocator* allocator = utl::DynamicTextureAllocator::instance();

    sead::GraphicsContext graphicsContext;
    graphicsContext.setAlphaTestEnable(false);
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEquation(0, 1);
    graphicsContext.setBlendFactor(0, 2, 2);
    graphicsContext.setBlendEquation(1, 1);
    graphicsContext.setBlendFactor(1, 2, 2);

    Sizes sizes;
    f32 transmittanceWidth = mTextureSize.mTransmittanceWidth;
    f32 transmittanceHeight = mTextureSize.mTransmittanceHeight;
    sizes.mTransmittance.set(transmittanceWidth, transmittanceHeight, 1.0f / transmittanceWidth,
                             1.0f / transmittanceHeight);
    u32 irradianceWidth = mTextureSize.mIrradianceWidth;
    u32 irradianceHeight = mTextureSize.mIrradianceHeight;
    sizes.mIrradiance.set(f32(irradianceWidth), f32(irradianceHeight),
                          1.0f / f32(irradianceWidth), 1.0f / f32(irradianceHeight));
    u32 altitudeNum = mTextureSize.mAltitudeNum;
    f32 altitude = altitudeNum;
    sizes.mAltitude.set(altitude, 1.0f / altitude, 1.0f / (altitude - 1.0f), 0.0f);
    u32 sunZenithNum = mTextureSize.mSunZenithNum;
    f32 sunZenith = sunZenithNum;
    sizes.mSunZenith.set(sunZenith, 1.0f / sunZenith, 1.0f / (sunZenith - 1.0f), 0.0f);
    u32 viewZenithNum = mTextureSize.mViewZenithNum;
    f32 viewZenith = viewZenithNum;
    sizes.mViewZenith.x = viewZenith;
    sizes.mViewZenith.y = 1.0f / viewZenith;
    sizes.mViewZenith.z = 1.0f / (viewZenith - 1.0f);
    u32 sunViewNum = mTextureSize.mSunViewNum;
    f32 sunView = sunViewNum;
    sizes.mSunView.set(sunView, 1.0f / sunView, 1.0f / (sunView - 1.0f), 0.0f);
    sizes.mViewZenith.w = 1.0f / (viewZenith * 0.5f - 1.0f);

    u32 width = sunZenithNum * sunViewNum;
    TextureData* pDeltaE =
        allocator->alloc(pDrawContext, "Single Irradiance", TextureFormat(0x2b), irradianceWidth,
                         irradianceHeight, 1, nullptr,
                         utl::DynamicTextureAllocator::cAllocateType_0, true, false);
    TextureData* pDeltaSR =
        allocator->alloc3D(pDrawContext, "Single Scattering Rayleigh", TextureFormat(0x2b), width,
                           viewZenithNum, altitudeNum, 1, nullptr,
                           utl::DynamicTextureAllocator::cAllocateType_0, true, false);
    TextureData* pDeltaSM =
        allocator->alloc3D(pDrawContext, "Single Scattering Mie", TextureFormat(0x2b), width,
                           viewZenithNum, altitudeNum, 1, nullptr,
                           utl::DynamicTextureAllocator::cAllocateType_0, true, false);
    TextureData* pDeltaJ =
        allocator->alloc3D(pDrawContext, "delta J", TextureFormat(0x2b), width, viewZenithNum,
                           altitudeNum, 1, nullptr, utl::DynamicTextureAllocator::cAllocateType_0,
                           true, false);

    graphicsContext.setBlendEnableMask(0);
    graphicsContext.apply(pDrawContext);

    setUpRenderBuffer(pDrawContext, context, &mIrradianceSampler.getTextureData(), 0, 0, 0, 7);
    utl::ImageFilter2D::drawTextureQuadTriangle(
        pDrawContext,
        *utl::PrimitiveTexture::instance()->getTextureSampler(utl::PrimitiveTexture::cType_Zero2D));
    invalidateRenderBufferCache(pDrawContext, context);

    setUpRenderBuffer(pDrawContext, context, &mTransmittanceSampler.getTextureData(), 0, 0, 0, 7);
    drawer.setProgram(mPrograms[0]->getVariation(0), sizes);
    drawer.draw();
    invalidateRenderBufferCache(pDrawContext, context);

    setUpRenderBuffer(pDrawContext, context, pDeltaE, 0, 0, 0, 7);
    drawer.setProgram(mPrograms[1]->getVariation(0), sizes);
    mTransmittanceSampler.activate(pDrawContext, drawer.mProgram->getSamplerLocation(0), -1,
                                   false);
    drawer.draw();
    invalidateRenderBufferCache(pDrawContext, context);

    drawer.setProgram(mPrograms[2]->getVariation(0), sizes);
    mTransmittanceSampler.activate(pDrawContext, drawer.mProgram->getSamplerLocation(0), -1,
                                   false);
    for (u32 layer = 0, flags = 5; layer < mTextureSize.mAltitudeNum;
         layer++, flags = 1)
    {
        setUpRenderBuffer(pDrawContext, context, pDeltaSR, 0, layer, 0, flags);
        setUpRenderBuffer(pDrawContext, context, pDeltaSM, 0, layer, 1, 2);
        drawer.drawLayer(f32(layer) / f32(mTextureSize.mAltitudeNum - 1));
    }

    invalidateRenderBufferCache(pDrawContext, context);

    {
        const ShaderProgram* program = mPrograms[5]->getVariation(0);
        bakeDrawer.mProgram = program;
        program->activate(pDrawContext, true);
        program->getUniformLocation(1).setUniform(pDrawContext, 4, &sizes.mBakedInscatter);
        program->getUniformLocation(2).setUniform(pDrawContext, 4,
                                                  &sizes.mBakedRangeTransmittance);
        program->getUniformLocation(0).setUniform(pDrawContext, 4, &sizes.mIrradiance);
        program->getUniformLocation(3).setUniform(pDrawContext, 4, &sizes.mAltitude);
        program->getUniformLocation(4).setUniform(pDrawContext, 4, &sizes.mSunZenith);
        program->getUniformLocation(5).setUniform(pDrawContext, 4, &sizes.mViewZenith);
        program->getUniformLocation(6).setUniform(pDrawContext, 4, &sizes.mSunView);
        getContext_(context).mSamplers[0].applyTextureData(*pDeltaSR);
        getContext_(context).mSamplers[0].activate(pDrawContext, program->getSamplerLocation(2),
                                                   -1, false);
        getContext_(context).mSamplers[1].applyTextureData(*pDeltaSM);
        getContext_(context).mSamplers[1].activate(pDrawContext, program->getSamplerLocation(3),
                                                   -1, false);
        for (u32 layer = 0, flags = 7; layer < mTextureSize.mAltitudeNum;
             layer++, flags = 3)
        {
            setUpRenderBuffer(pDrawContext, context, &mInscatterSampler.getTextureData(), 0,
                              layer, 0, flags);
            bakeDrawer.drawLayer(true, f32(layer) / f32(mTextureSize.mAltitudeNum - 1));
        }

        invalidateRenderBufferCache(pDrawContext, context);
    }

    for (u32 order = 0; order < u32(mScatteringOrder); order++)
    {
        graphicsContext.setBlendEnableMask(0);
        graphicsContext.apply(pDrawContext);

        drawer.setProgram(mPrograms[3]->getVariation(order == 0 ? 0 : 1), sizes);
        mTransmittanceSampler.activate(pDrawContext, drawer.mProgram->getSamplerLocation(0), -1,
                                       false);
        getContext_(context).mSamplers[0].applyTextureData(*pDeltaE);
        getContext_(context).mSamplers[0].activate(
            pDrawContext, drawer.mProgram->getSamplerLocation(1), -1, false);
        getContext_(context).mSamplers[1].applyTextureData(*pDeltaSR);
        getContext_(context).mSamplers[1].activate(
            pDrawContext, drawer.mProgram->getSamplerLocation(3), -1, false);
        getContext_(context).mSamplers[2].applyTextureData(*pDeltaSM);
        getContext_(context).mSamplers[2].activate(
            pDrawContext, drawer.mProgram->getSamplerLocation(4), -1, false);
        for (u32 layer = 0, flags = 7; layer < mTextureSize.mAltitudeNum;
             layer++, flags = 3)
        {
            setUpRenderBuffer(pDrawContext, context, pDeltaJ, 0, layer, 0, flags);
            drawer.drawLayer(f32(layer) / f32(mTextureSize.mAltitudeNum - 1));
        }

        invalidateRenderBufferCache(pDrawContext, context);

        drawer.setProgram(mPrograms[1]->getVariation(order == 0 ? 1 : 2), sizes);
        mTransmittanceSampler.activate(pDrawContext, drawer.mProgram->getSamplerLocation(0), -1,
                                       false);
        getContext_(context).mSamplers[0].applyTextureData(*pDeltaSR);
        getContext_(context).mSamplers[0].activate(
            pDrawContext, drawer.mProgram->getSamplerLocation(3), -1, false);
        getContext_(context).mSamplers[1].applyTextureData(*pDeltaSM);
        getContext_(context).mSamplers[1].activate(
            pDrawContext, drawer.mProgram->getSamplerLocation(4), -1, false);
        setUpRenderBuffer(pDrawContext, context, pDeltaE, 0, 0, 0, 7);
        drawer.draw();
        invalidateRenderBufferCache(pDrawContext, context);

        drawer.setProgram(mPrograms[2]->getVariation(1), sizes);
        mTransmittanceSampler.activate(pDrawContext, drawer.mProgram->getSamplerLocation(0), -1,
                                       false);
        getContext_(context).mSamplers[0].applyTextureData(*pDeltaJ);
        getContext_(context).mSamplers[0].activate(
            pDrawContext, drawer.mProgram->getSamplerLocation(2), -1, false);
        for (u32 layer = 0, flags = 7; layer < mTextureSize.mAltitudeNum;
             layer++, flags = 3)
        {
            setUpRenderBuffer(pDrawContext, context, pDeltaSR, 0, layer, 0, flags);
            drawer.drawLayer(f32(layer) / f32(mTextureSize.mAltitudeNum - 1));
        }

        invalidateRenderBufferCache(pDrawContext, context);

        graphicsContext.setBlendEnableMask(1);
        graphicsContext.apply(pDrawContext);

        {
            const ShaderProgram* program = mPrograms[5]->getVariation(1);
            bakeDrawer.mProgram = program;
            program->activate(pDrawContext, true);
            program->getUniformLocation(1).setUniform(pDrawContext, 4, &sizes.mBakedInscatter);
            program->getUniformLocation(2).setUniform(pDrawContext, 4,
                                                      &sizes.mBakedRangeTransmittance);
            program->getUniformLocation(0).setUniform(pDrawContext, 4, &sizes.mIrradiance);
            program->getUniformLocation(3).setUniform(pDrawContext, 4, &sizes.mAltitude);
            program->getUniformLocation(4).setUniform(pDrawContext, 4, &sizes.mSunZenith);
            program->getUniformLocation(5).setUniform(pDrawContext, 4, &sizes.mViewZenith);
            program->getUniformLocation(6).setUniform(pDrawContext, 4, &sizes.mSunView);
            getContext_(context).mSamplers[0].applyTextureData(*pDeltaSR);
            getContext_(context).mSamplers[0].activate(
                pDrawContext, program->getSamplerLocation(2), -1, false);
            for (u32 layer = 0, flags = 7; layer < mTextureSize.mAltitudeNum;
                 layer++, flags = 3)
            {
                setUpRenderBuffer(pDrawContext, context, &mInscatterSampler.getTextureData(), 0,
                                  layer, 0, flags);
                bakeDrawer.drawLayer(true, f32(layer) / f32(mTextureSize.mAltitudeNum - 1));
            }

            invalidateRenderBufferCache(pDrawContext, context);
        }

        {
            const ShaderProgram* program = mPrograms[4];
            bakeDrawer.mProgram = program;
            program->activate(pDrawContext, true);
            program->getUniformLocation(1).setUniform(pDrawContext, 4, &sizes.mBakedInscatter);
            program->getUniformLocation(2).setUniform(pDrawContext, 4,
                                                      &sizes.mBakedRangeTransmittance);
            program->getUniformLocation(0).setUniform(pDrawContext, 4, &sizes.mIrradiance);
            program->getUniformLocation(3).setUniform(pDrawContext, 4, &sizes.mAltitude);
            program->getUniformLocation(4).setUniform(pDrawContext, 4, &sizes.mSunZenith);
            program->getUniformLocation(5).setUniform(pDrawContext, 4, &sizes.mViewZenith);
            program->getUniformLocation(6).setUniform(pDrawContext, 4, &sizes.mSunView);
            getContext_(context).mSamplers[0].applyTextureData(*pDeltaE);
            getContext_(context).mSamplers[0].activate(
                pDrawContext, program->getSamplerLocation(0), -1, false);
            setUpRenderBuffer(pDrawContext, context, &mIrradianceSampler.getTextureData(), 0, 0,
                              0, 7);
            drawQuad(pDrawContext);
            invalidateRenderBufferCache(pDrawContext, context);
        }
    }

    allocator->free(pDeltaE);
    allocator->free(pDeltaJ);
    allocator->free(pDeltaSR);
    allocator->free(pDeltaSM);
}

/**
 * Bakes the inscatter, irradiance and range transmittance textures of a context.
 * @param pDrawContext Draw context
 * @param context Context index
 * @param rViewMtx View matrix of the camera
 * @param near Near clip distance
 * @param far Far clip distance
 */
void Sky::drawContext(DrawContext* pDrawContext, u32 context, const sead::Matrix34f& rViewMtx,
                      f32 near, f32 far)
{
    if (!mFlags.isOnAll(cFlag_Initialized | cFlag_Enable))
    {
        return;
    }

    Context& rContext = getContext_(context);

    if (!rContext.mIsEnable || mFlags.isOn(cFlag_UpdateBasis) ||
        !mFlags.isOn(cFlag_Sky | cFlag_Ground))
    {
        return;
    }

    BakeDrawer bakeDrawer = {pDrawContext, nullptr};
    Sizes sizes;
    f32 height;
    {
        sead::GraphicsContext graphicsContext;
        graphicsContext.setDepthEnable(false, false);
        graphicsContext.setAlphaTestEnable(false);
        graphicsContext.setBlendEnable(false);
        graphicsContext.apply(pDrawContext);

        f32 bakedWidth = rContext.mBakedInscatter.getTextureData().getWidth(0);
        f32 bakedHeight = rContext.mBakedInscatter.getTextureData().getHeight(0);
        f32 transmittanceWidth = mTextureSize.mTransmittanceWidth;
        f32 transmittanceHeight = mTextureSize.mTransmittanceHeight;
        sizes.mTransmittance.set(transmittanceWidth, transmittanceHeight, 1.0f / transmittanceWidth,
                                 1.0f / transmittanceHeight);
        f32 irradianceWidth = mTextureSize.mIrradianceWidth;
        f32 irradianceHeight = mTextureSize.mIrradianceHeight;
        sizes.mIrradiance.set(irradianceWidth, irradianceHeight, 1.0f / irradianceWidth,
                              1.0f / irradianceHeight);
        sizes.mBakedInscatter.set(bakedWidth, bakedHeight, 1.0f / bakedWidth, 1.0f / bakedHeight);
        f32 rangeWidth = mTextureSize.mBakedRangeTransmittanceWidth;
        f32 rangeHeight = mTextureSize.mBakedRangeTransmittanceHeight;
        sizes.mBakedRangeTransmittance.set(rangeWidth, rangeHeight, 1.0f / rangeWidth,
                                           1.0f / rangeHeight);
        f32 altitude = mTextureSize.mAltitudeNum;
        sizes.mAltitude.set(altitude, 1.0f / altitude, 1.0f / (altitude - 1.0f), 0.0f);
        f32 sunZenith = mTextureSize.mSunZenithNum;
        sizes.mSunZenith.set(sunZenith, 1.0f / sunZenith, 1.0f / (sunZenith - 1.0f), 0.0f);
        f32 viewZenith = mTextureSize.mViewZenithNum;
        sizes.mViewZenith.x = viewZenith;
        sizes.mViewZenith.y = 1.0f / viewZenith;
        sizes.mViewZenith.z = 1.0f / (viewZenith - 1.0f);
        f32 sunView = mTextureSize.mSunViewNum;
        sizes.mSunView.set(sunView, 1.0f / sunView, 1.0f / (sunView - 1.0f), 0.0f);
        sizes.mViewZenith.w = 1.0f / (viewZenith * 0.5f - 1.0f);

        sead::Matrix34f invView;
        invView.setInverse(rViewMtx);
        height = invView.m[1][3];
        rContext.mBakeInfo.mAltitude = height / 1000.0f + cRadiusGround;
        f32 ratio = cRadiusGround / rContext.mBakeInfo.mAltitude;
        rContext.mBakeInfo.mHorizon =
            -sead::Mathf::sqrt(sead::Mathf::clamp(1.0f - ratio * ratio, 0.0f, 1.0f));
    }

    setUpRenderBuffer(pDrawContext, context, &rContext.mBakedInscatter.getTextureData(), 0, 0, 0,
                      7);
    const ShaderProgram* base = mPrograms[6];
    s32 variation = mFlags.isOn(cFlag_Scattering) * base->getVariationMacroStride(1) +
                    mFlags.isOn(cFlag_SunDisk) * base->getVariationMacroStride(2);
    const ShaderProgram* program = base->getVariation(variation);
    bakeDrawer.mProgram = program;
    program->activate(pDrawContext, true);
    program->getUniformLocation(1).setUniform(pDrawContext, 4, &sizes.mBakedInscatter);
    program->getUniformLocation(2).setUniform(pDrawContext, 4, &sizes.mBakedRangeTransmittance);
    program->getUniformLocation(0).setUniform(pDrawContext, 4, &sizes.mIrradiance);
    program->getUniformLocation(3).setUniform(pDrawContext, 4, &sizes.mAltitude);
    program->getUniformLocation(4).setUniform(pDrawContext, 4, &sizes.mSunZenith);
    program->getUniformLocation(5).setUniform(pDrawContext, 4, &sizes.mViewZenith);
    program->getUniformLocation(6).setUniform(pDrawContext, 4, &sizes.mSunView);
    mInscatterSampler.activate(pDrawContext, program->getSamplerLocation(4), -1, false);

    f32 sunZenithCos = -1.0f;
    const sead::Vector3f& rSunDir = mParam.mSunDir;
    f32 length = sead::Mathf::sqrt(rSunDir.x * rSunDir.x + rSunDir.y * rSunDir.y +
                                   rSunDir.z * rSunDir.z);
    if (length > 0.0f)
    {
        sunZenithCos = rSunDir.y * (-1.0f / length);
    }

    const sead::Color4f& rColor = *mParam.mDynamicColor;
    sead::Vector3f sunColor(rColor.a * rColor.r, rColor.a * rColor.g, rColor.a * rColor.b);
    program->getUniformLocation(9).setUniform(pDrawContext, *mParam.mDynamicRayleighAmplifier);
    program->getUniformLocation(10).setUniform(pDrawContext, *mParam.mDynamicMieAmplifier);
    program->getUniformLocation(11).setUniform(pDrawContext, *mParam.mDynamicMieSymmetricalProp);
    program->getUniformLocation(12).setUniform(pDrawContext, sunZenithCos);
    program->getUniformLocation(13).setUniform(pDrawContext, 3, &sunColor);
    program->getUniformLocation(14).setUniform(pDrawContext, mParam.mFade);
    f32 layer = height / 60000.0f;
    program->getUniformLocation(7).setUniform(pDrawContext, 3,
                                              &mParam.mStaticRayleighScatteringCoeff);
    program->getUniformLocation(8).setUniform(pDrawContext, *mParam.mStaticMieScatteringCoeff);
    bakeDrawer.drawLayer(rContext.mIsEffectiveBelowHorizon, layer);
    invalidateRenderBufferCache(pDrawContext, context);

    if (!mFlags.isOn(cFlag_BakeIrradiance))
    {
        return;
    }

    sead::Vector4f nearFar(near, far, far - near, 1.0f / ((far - near) * 0.001f));
    setUpRenderBuffer(pDrawContext, context, &rContext.mBakedIrradiance.getTextureData(), 0, 0,
                      0, 7);
    program = mPrograms[7]->getVariation(0);
    bakeDrawer.mProgram = program;
    program->activate(pDrawContext, true);
    program->getUniformLocation(1).setUniform(pDrawContext, 4, &sizes.mBakedInscatter);
    program->getUniformLocation(2).setUniform(pDrawContext, 4, &sizes.mBakedRangeTransmittance);
    program->getUniformLocation(0).setUniform(pDrawContext, 4, &sizes.mIrradiance);
    program->getUniformLocation(3).setUniform(pDrawContext, 4, &sizes.mAltitude);
    program->getUniformLocation(4).setUniform(pDrawContext, 4, &sizes.mSunZenith);
    program->getUniformLocation(5).setUniform(pDrawContext, 4, &sizes.mViewZenith);
    program->getUniformLocation(6).setUniform(pDrawContext, 4, &sizes.mSunView);
    mIrradianceSampler.activate(pDrawContext, program->getSamplerLocation(5), -1, false);
    mTransmittanceSampler.activate(pDrawContext, program->getSamplerLocation(6), -1, false);
    bakeDrawer.drawLayer(rContext.mIsEffectiveBelowHorizon, layer);
    invalidateRenderBufferCache(pDrawContext, context);

    setUpRenderBuffer(pDrawContext, context, &rContext.mBakedRangeTransmittance.getTextureData(),
                      0, 0, 0, 7);
    program = mPrograms[8]->getVariation(0);
    bakeDrawer.mProgram = program;
    program->activate(pDrawContext, true);
    program->getUniformLocation(1).setUniform(pDrawContext, 4, &sizes.mBakedInscatter);
    program->getUniformLocation(2).setUniform(pDrawContext, 4, &sizes.mBakedRangeTransmittance);
    program->getUniformLocation(0).setUniform(pDrawContext, 4, &sizes.mIrradiance);
    program->getUniformLocation(3).setUniform(pDrawContext, 4, &sizes.mAltitude);
    program->getUniformLocation(4).setUniform(pDrawContext, 4, &sizes.mSunZenith);
    program->getUniformLocation(5).setUniform(pDrawContext, 4, &sizes.mViewZenith);
    program->getUniformLocation(6).setUniform(pDrawContext, 4, &sizes.mSunView);
    mTransmittanceSampler.activate(pDrawContext, program->getSamplerLocation(6), -1, false);
    program->getUniformLocation(16).setUniform(pDrawContext, 4, &nearFar);
    bakeDrawer.drawLayer(rContext.mIsEffectiveBelowHorizon, layer);
    invalidateRenderBufferCache(pDrawContext, context);
}

/**
 * Binds a texture as a render target of a context's render buffer.
 * @param pDrawContext Draw context
 * @param context Context index
 * @param pTexture Texture to render into
 * @param mipLevel Mip level to render into
 * @param slice Slice to render into
 * @param target Render target index
 * @param flags 1 clears the other targets, 2 binds the buffer, 4 applies the viewport
 */
void Sky::setUpRenderBuffer(DrawContext* pDrawContext, u32 context, const TextureData* pTexture,
                            u32 mipLevel, u32 slice, u32 target, u32 flags)
{
    Context& rContext = getContext_(context);

    if (flags & 1)
    {
        rContext.mRenderBuffer.setRenderTargetColorNullAll();
        rContext.mRenderBuffer.setRenderTargetDepth(nullptr);
    }

    if (flags & 4)
    {
        f32 width = u32(sead::Mathi::max(pTexture->getWidth() >> mipLevel, 1));
        f32 height = u32(sead::Mathi::max(s32(pTexture->getMinHeight_()),
                                          pTexture->getHeight() >> mipLevel));
        rContext.mRenderBuffer.setPhysicalArea(
            sead::BoundBox2f(sead::Vector2f(0.0f, 0.0f), sead::Vector2f(width, height)));
        rContext.mRenderBuffer.setVirtualSize(sead::Vector2f(width, height));
        sead::Viewport viewport(rContext.mRenderBuffer);
        viewport.apply(pDrawContext, rContext.mRenderBuffer);
    }

    RenderTargetColor& rTarget = rContext.mRenderTargets[target];
    rTarget.applyTextureData(*pTexture);
    rTarget.setSlice(slice);
    rTarget.setMipLevel(mipLevel);
    rContext.mRenderBuffer.setRenderTargetColor(&rTarget, target);

    if (flags & 2)
    {
        rContext.mRenderBuffer.bind(pDrawContext);
    }

    rContext.mDirtyTargets |= 1 << target;
}

/**
 * Invalidates the GPU cache of every render target drawn since the last call.
 * @param pDrawContext Draw context
 * @param context Context index
 */
void Sky::invalidateRenderBufferCache(DrawContext* pDrawContext, u32 context)
{
    Context& rContext = getContext_(context);

    for (s32 i = 0; i < 2; i++)
    {
        if (rContext.mDirtyTargets & (1 << i))
        {
            rContext.mRenderTargets[i].invalidateGPUCache(pDrawContext);
        }
    }

    rContext.mDirtyTargets = 0;
}

/**
 * Gets the parameters that affect the basis textures.
 * @param pParam Output parameters
 */
void Sky::getRenderingParameterStatic(RenderingParameterStatic* pParam) const
{
    pParam->mRayleighBaseHeight = *mParam.mStaticRayleighBaseHeight;
    pParam->mMieBaseHeight = *mParam.mStaticMieBaseHeight;
    pParam->mMieScatteringCoeff = sead::Mathf::max(0.0f, *mParam.mStaticMieScatteringCoeff);
    pParam->mMieSymmetricalProp = sead::Mathf::clamp(*mParam.mStaticMieSymmetricalProp, 0.0f, 1.0f);
}

/**
 * Sets the parameters that affect the basis textures.
 * @param rParam New parameters
 * @param update Whether to recompute the basis textures
 */
void Sky::setRenderingParameterStatic(const RenderingParameterStatic& rParam, bool update)
{
    *mParam.mStaticRayleighBaseHeight = rParam.mRayleighBaseHeight;
    *mParam.mStaticMieBaseHeight = rParam.mMieBaseHeight;
    *mParam.mStaticMieScatteringCoeff = sead::Mathf::max(0.0f, rParam.mMieScatteringCoeff);
    *mParam.mStaticMieSymmetricalProp = sead::Mathf::clamp(rParam.mMieSymmetricalProp, 0.0f, 1.0f);
    if (update)
    {
        mFlags.set(cFlag_UpdateBasis);
    }
}

/**
 * Gets the parameters used when baking a context.
 * @param pParam Output parameters
 */
void Sky::getRenderingParameter(RenderingParameter* pParam) const
{
    pParam->mRayleighAmplifier = sead::Mathf::max(0.0f, *mParam.mDynamicRayleighAmplifier);
    pParam->mMieAmplifier = sead::Mathf::max(0.0f, *mParam.mDynamicMieAmplifier);
    pParam->mMieSymmetricalProp = sead::Mathf::clamp(*mParam.mDynamicMieSymmetricalProp, 0.0f, 1.0f);
}

/**
 * Sets the parameters used when baking a context.
 * @param rParam New parameters
 */
void Sky::setRenderingParameter(const RenderingParameter& rParam)
{
    *mParam.mDynamicRayleighAmplifier = sead::Mathf::max(0.0f, rParam.mRayleighAmplifier);
    *mParam.mDynamicMieAmplifier = sead::Mathf::max(0.0f, rParam.mMieAmplifier);
    *mParam.mDynamicMieSymmetricalProp = sead::Mathf::clamp(rParam.mMieSymmetricalProp, 0.0f, 1.0f);
}

/**
 * Gets the scattering fog parameters.
 * @param pParam Output parameters
 */
void Sky::getScatterFogParam(ScatterFogParam* pParam) const
{
    pParam->mNear = *mParam.mScatterFogNear;
    pParam->mFar = *mParam.mScatterFogFar;
    pParam->mDensity = sead::Mathf::max(0.0f, *mParam.mScatterFogDensity);
    pParam->mAtten = sead::Mathf::max(0.0f, *mParam.mScatterFogAtten);
    pParam->mHorz = sead::Mathf::max(0.0f, *mParam.mScatterFogHorz);
}

/**
 * Sets the scattering fog parameters.
 * @param rParam New parameters
 */
void Sky::setScatterFogParam(const ScatterFogParam& rParam)
{
    *mParam.mScatterFogNear = rParam.mNear;
    *mParam.mScatterFogFar = rParam.mFar;
    *mParam.mScatterFogDensity = sead::Mathf::max(0.0f, rParam.mDensity);
    *mParam.mScatterFogAtten = sead::Mathf::max(0.0f, rParam.mAtten);
    *mParam.mScatterFogHorz = sead::Mathf::max(0.0f, rParam.mHorz);
}

/**
 * Gets the ad hoc fog parameters.
 * @param pParam Output parameters
 */
void Sky::getAdhocFogParam(AdhocFogParam* pParam) const
{
    pParam->mColor = *mParam.mAdhocFogColor;
    pParam->mNear = *mParam.mAdhocFogNear;
    pParam->mFar = *mParam.mAdhocFogFar;
    pParam->mAttenGround = sead::Mathf::max(0.0f, *mParam.mAdhocFogAttenGround);
    pParam->mAttenSky = sead::Mathf::max(0.5f, *mParam.mAdhocFogAttenSky);
    pParam->mAttenMinScaleSky = sead::Mathf::max(0.0f, *mParam.mAdhocFogAttenMinScaleSky);
}

/**
 * Sets the ad hoc fog parameters.
 * @param rParam New parameters
 */
void Sky::setAdhocFogParam(const AdhocFogParam& rParam)
{
    *mParam.mAdhocFogColor = rParam.mColor;
    *mParam.mAdhocFogNear = rParam.mNear;
    *mParam.mAdhocFogFar = rParam.mFar;
    *mParam.mAdhocFogAttenGround = sead::Mathf::max(0.0f, rParam.mAttenGround);
    *mParam.mAdhocFogAttenSky = sead::Mathf::max(0.5f, rParam.mAttenSky);
    *mParam.mAdhocFogAttenMinScaleSky = sead::Mathf::max(0.0f, rParam.mAttenMinScaleSky);
}

/**
 * Gets the values computed while baking a context.
 * @param pParam Output parameters
 * @param context Context index
 */
void Sky::getBakeInfoParam(BakeInfoParam* pParam, u32 context) const
{
    pParam->mAltitude = getContext_(context).mBakeInfo.mAltitude;
    pParam->mHorizon = getContext_(context).mBakeInfo.mHorizon;
}

/**
 * Draws the sky and the ground with atmospheric scattering.
 * @param pDrawContext Draw context
 * @param context Context index
 * @param rProjMtx Projection matrix
 * @param rViewMtx View matrix
 * @param near Near clip distance
 * @param far Far clip distance
 * @param rColor Scene color texture
 * @param rDepth Scene depth texture
 * @param useColor Whether to use the scene color variation for the ground
 * @param amplifier Brightness scale
 */
void Sky::drawBoth(DrawContext* pDrawContext, u32 context, const sead::Matrix44f& rProjMtx,
                   const sead::Matrix34f& rViewMtx, f32 near, f32 far,
                   const TextureData& rColor, const TextureData& rDepth, bool useColor,
                   f32 amplifier)
{
    if ((mFlags.getDirect() & (cFlag_Initialized | cFlag_Enable | cFlag_UpdateBasis)) !=
        (cFlag_Initialized | cFlag_Enable))
    {
        return;
    }

    Context& rContext = getContext_(context);

    if (!rContext.mIsEnable)
    {
        return;
    }

    PostFxDrawer drawer = {this,  s32(context), &rProjMtx, &rViewMtx, near,
                           far,   amplifier,    &rColor,   &rDepth};
    sead::GraphicsContext graphicsContext;
    u32 bakedWidth = rContext.mBakedInscatter.getTextureData().getWidth(0);
    u32 bakedHeight = rContext.mBakedInscatter.getTextureData().getHeight(0);
    graphicsContext.setDepthEnable(true, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.setAlphaTestEnable(false);

    Sizes sizes;
    f32 transmittanceWidth = mTextureSize.mTransmittanceWidth;
    f32 transmittanceHeight = mTextureSize.mTransmittanceHeight;
    sizes.mTransmittance.set(transmittanceWidth, transmittanceHeight, 1.0f / transmittanceWidth,
                             1.0f / transmittanceHeight);
    f32 irradianceWidth = mTextureSize.mIrradianceWidth;
    f32 irradianceHeight = mTextureSize.mIrradianceHeight;
    sizes.mIrradiance.set(irradianceWidth, irradianceHeight, 1.0f / irradianceWidth,
                          1.0f / irradianceHeight);
    sizes.mBakedInscatter.set(f32(bakedWidth), f32(bakedHeight), 1.0f / f32(bakedWidth),
                              1.0f / f32(bakedHeight));
    f32 altitude = mTextureSize.mAltitudeNum;
    sizes.mAltitude.set(altitude, 1.0f / altitude, 1.0f / (altitude - 1.0f), 0.0f);
    f32 sunZenith = mTextureSize.mSunZenithNum;
    sizes.mSunZenith.set(sunZenith, 1.0f / sunZenith, 1.0f / (sunZenith - 1.0f), 0.0f);
    f32 viewZenith = mTextureSize.mViewZenithNum;
    sizes.mViewZenith.x = viewZenith;
    sizes.mViewZenith.y = 1.0f / viewZenith;
    sizes.mViewZenith.z = 1.0f / (viewZenith - 1.0f);
    f32 sunView = mTextureSize.mSunViewNum;
    sizes.mSunView.set(sunView, 1.0f / sunView, 1.0f / (sunView - 1.0f), 0.0f);
    sizes.mViewZenith.w = 1.0f / (viewZenith * 0.5f - 1.0f);

    if (mFlags.isOn(cFlag_Sky) && rContext.mIsSkyEnable)
    {
        graphicsContext.setDepthFunc(3);
        graphicsContext.apply(pDrawContext);
        const ShaderProgram* base = mPrograms[9];
        bool fog = mParam.mAdhocFogColor->a > 0.0f && *mParam.mAdhocFogAttenSky > 0.0f;
        s32 variation =
            base->getVariationMacroStride(0) * mFlags.isOn(cFlag_Scattering) +
            base->getVariationMacroStride(1) * fog +
            base->getVariationMacroStride(2) * mFlags.isOn(cFlag_SkyFog) +
            base->getVariationMacroStride(3) * (rContext.mCloudSampler != nullptr) +
            base->getVariationMacroStride(4) * mFlags.isOn(cFlag_GroundFog);
        const ShaderProgram* program = base->getVariation(variation);
        program->activate(pDrawContext, true);

        if (rContext.mCloudSampler)
        {
            rContext.mCloudSampler->activate(pDrawContext, program->getSamplerLocation(8), -1,
                                             false);
            program->getUniformLocation(25).setUniform(pDrawContext, rContext.mCloudParam);
        }

        drawer.draw(pDrawContext, program, sizes);
    }

    if (mFlags.isOn(cFlag_Ground) && rContext.mIsGroundEnable)
    {
        graphicsContext.setDepthFunc(6);
        graphicsContext.apply(pDrawContext);
        const ShaderProgram* base = mPrograms[10];
        bool fog = mParam.mAdhocFogColor->a > 0.0f && *mParam.mAdhocFogAttenSky > 0.0f;
        s32 variation = base->getVariationMacroStride(0) * useColor +
                        base->getVariationMacroStride(1) * fog +
                        base->getVariationMacroStride(2) * mFlags.isOn(cFlag_Scattering) +
                        base->getVariationMacroStride(3) * mFlags.isOn(cFlag_GroundFog);
        const ShaderProgram* program = base->getVariation(variation);
        program->activate(pDrawContext, true);
        drawer.draw(pDrawContext, program, sizes);
    }
}

/**
 * Draws the sky with atmospheric scattering.
 * @param pDrawContext Draw context
 * @param context Context index
 * @param rProjMtx Projection matrix
 * @param rViewMtx View matrix
 * @param near Near clip distance
 * @param far Far clip distance
 * @param amplifier Brightness scale
 */
void Sky::drawSky(DrawContext* pDrawContext, u32 context, const sead::Matrix44f& rProjMtx,
                  const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 amplifier)
{
    if ((mFlags.getDirect() & (cFlag_Initialized | cFlag_Enable | cFlag_UpdateBasis)) !=
        (cFlag_Initialized | cFlag_Enable))
    {
        return;
    }

    Context& rContext = getContext_(context);

    if (!rContext.mIsEnable)
    {
        return;
    }

    PostFxDrawer drawer = {this, s32(context), &rProjMtx, &rViewMtx, near, far, amplifier,
                           nullptr, nullptr};
    sead::GraphicsContext graphicsContext;
    u32 bakedWidth = rContext.mBakedInscatter.getTextureData().getWidth(0);
    u32 bakedHeight = rContext.mBakedInscatter.getTextureData().getHeight(0);
    graphicsContext.setDepthEnable(true, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.setAlphaTestEnable(false);

    Sizes sizes;
    f32 transmittanceWidth = mTextureSize.mTransmittanceWidth;
    f32 transmittanceHeight = mTextureSize.mTransmittanceHeight;
    sizes.mTransmittance.set(transmittanceWidth, transmittanceHeight, 1.0f / transmittanceWidth,
                             1.0f / transmittanceHeight);
    f32 irradianceWidth = mTextureSize.mIrradianceWidth;
    f32 irradianceHeight = mTextureSize.mIrradianceHeight;
    sizes.mIrradiance.set(irradianceWidth, irradianceHeight, 1.0f / irradianceWidth,
                          1.0f / irradianceHeight);
    sizes.mBakedInscatter.set(f32(bakedWidth), f32(bakedHeight), 1.0f / f32(bakedWidth),
                              1.0f / f32(bakedHeight));
    f32 altitude = mTextureSize.mAltitudeNum;
    sizes.mAltitude.set(altitude, 1.0f / altitude, 1.0f / (altitude - 1.0f), 0.0f);
    f32 sunZenith = mTextureSize.mSunZenithNum;
    sizes.mSunZenith.set(sunZenith, 1.0f / sunZenith, 1.0f / (sunZenith - 1.0f), 0.0f);
    f32 viewZenith = mTextureSize.mViewZenithNum;
    sizes.mViewZenith.x = viewZenith;
    sizes.mViewZenith.y = 1.0f / viewZenith;
    sizes.mViewZenith.z = 1.0f / (viewZenith - 1.0f);
    f32 sunView = mTextureSize.mSunViewNum;
    sizes.mSunView.set(sunView, 1.0f / sunView, 1.0f / (sunView - 1.0f), 0.0f);
    sizes.mViewZenith.w = 1.0f / (viewZenith * 0.5f - 1.0f);

    if (mFlags.isOn(cFlag_Sky) && rContext.mIsSkyEnable)
    {
        graphicsContext.setDepthFunc(3);
        graphicsContext.apply(pDrawContext);
        const ShaderProgram* base = mPrograms[9];
        bool fog = mParam.mAdhocFogColor->a > 0.0f && *mParam.mAdhocFogAttenSky > 0.0f;
        s32 variation =
            base->getVariationMacroStride(0) * mFlags.isOn(cFlag_Scattering) +
            base->getVariationMacroStride(1) * fog +
            base->getVariationMacroStride(2) * mFlags.isOn(cFlag_SkyFog) +
            base->getVariationMacroStride(3) * (rContext.mCloudSampler != nullptr) +
            base->getVariationMacroStride(4) * mFlags.isOn(cFlag_GroundFog);
        const ShaderProgram* program = base->getVariation(variation);
        program->activate(pDrawContext, true);

        if (rContext.mCloudSampler)
        {
            rContext.mCloudSampler->activate(pDrawContext, program->getSamplerLocation(8), -1,
                                             false);
            program->getUniformLocation(25).setUniform(pDrawContext, rContext.mCloudParam);
        }

        drawer.draw(pDrawContext, program, sizes);
    }
}

/**
 * Draws the ground with atmospheric scattering.
 * @param pDrawContext Draw context
 * @param context Context index
 * @param rProjMtx Projection matrix
 * @param rViewMtx View matrix
 * @param near Near clip distance
 * @param far Far clip distance
 * @param rColor Scene color texture
 * @param rDepth Scene depth texture
 * @param useColor Whether to use the scene color variation
 * @param amplifier Brightness scale
 */
void Sky::drawGround(DrawContext* pDrawContext, u32 context, const sead::Matrix44f& rProjMtx,
                     const sead::Matrix34f& rViewMtx, f32 near, f32 far,
                     const TextureData& rColor, const TextureData& rDepth, bool useColor,
                     f32 amplifier)
{
    if ((mFlags.getDirect() & (cFlag_Initialized | cFlag_Enable | cFlag_UpdateBasis)) !=
        (cFlag_Initialized | cFlag_Enable))
    {
        return;
    }

    Context& rContext = getContext_(context);

    if (!rContext.mIsEnable)
    {
        return;
    }

    PostFxDrawer drawer = {this,  s32(context), &rProjMtx, &rViewMtx, near,
                           far,   amplifier,    &rColor,   &rDepth};
    sead::GraphicsContext graphicsContext;
    u32 bakedWidth = getContext_(context).mBakedInscatter.getTextureData().getWidth(0);
    u32 bakedHeight = getContext_(context).mBakedInscatter.getTextureData().getHeight(0);
    graphicsContext.setDepthEnable(true, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.setAlphaTestEnable(false);

    Sizes sizes;
    f32 transmittanceWidth = mTextureSize.mTransmittanceWidth;
    f32 transmittanceHeight = mTextureSize.mTransmittanceHeight;
    sizes.mTransmittance.set(transmittanceWidth, transmittanceHeight, 1.0f / transmittanceWidth,
                             1.0f / transmittanceHeight);
    f32 irradianceWidth = mTextureSize.mIrradianceWidth;
    f32 irradianceHeight = mTextureSize.mIrradianceHeight;
    sizes.mIrradiance.set(irradianceWidth, irradianceHeight, 1.0f / irradianceWidth,
                          1.0f / irradianceHeight);
    sizes.mBakedInscatter.set(f32(bakedWidth), f32(bakedHeight), 1.0f / f32(bakedWidth),
                              1.0f / f32(bakedHeight));
    f32 altitude = mTextureSize.mAltitudeNum;
    sizes.mAltitude.set(altitude, 1.0f / altitude, 1.0f / (altitude - 1.0f), 0.0f);
    f32 sunZenith = mTextureSize.mSunZenithNum;
    sizes.mSunZenith.set(sunZenith, 1.0f / sunZenith, 1.0f / (sunZenith - 1.0f), 0.0f);
    f32 viewZenith = mTextureSize.mViewZenithNum;
    sizes.mViewZenith.x = viewZenith;
    sizes.mViewZenith.y = 1.0f / viewZenith;
    sizes.mViewZenith.z = 1.0f / (viewZenith - 1.0f);
    f32 sunView = mTextureSize.mSunViewNum;
    sizes.mSunView.set(sunView, 1.0f / sunView, 1.0f / (sunView - 1.0f), 0.0f);
    sizes.mViewZenith.w = 1.0f / (viewZenith * 0.5f - 1.0f);

    if (mFlags.isOn(cFlag_Ground) && rContext.mIsGroundEnable)
    {
        graphicsContext.setDepthFunc(6);
        graphicsContext.apply(pDrawContext);
        const ShaderProgram* base = mPrograms[10];
        bool fog = mParam.mAdhocFogColor->a > 0.0f && *mParam.mAdhocFogAttenSky > 0.0f;
        s32 variation = base->getVariationMacroStride(0) * useColor +
                        base->getVariationMacroStride(1) * fog +
                        base->getVariationMacroStride(2) * mFlags.isOn(cFlag_Scattering) +
                        base->getVariationMacroStride(3) * mFlags.isOn(cFlag_GroundFog);
        const ShaderProgram* program = base->getVariation(variation);
        program->activate(pDrawContext, true);
        drawer.draw(pDrawContext, program, sizes);
    }
}

bool Sky::preWrite_() const
{
    *const_cast<utl::Parameter<u32>&>(mParam.mFlag) =
        mFlags.getDirect() & (cFlag_Enable | cFlag_Sky | cFlag_Ground);
    return true;
}

bool Sky::preRead_()
{
    *mParam.mVersion = -1;
    return true;
}

void Sky::postRead_()
{
    if (*mParam.mVersion < 2)
    {
        if (*mParam.mVersion < 1)
        {
            OldParam& rOldParam = getOldParam_();
            *mParam.mStaticRayleighBaseHeight = *rOldParam.mRayleighBaseHeight;
            *mParam.mStaticMieBaseHeight = *rOldParam.mMieBaseHeight;
            *mParam.mStaticMieScatteringCoeff = *rOldParam.mMieScatteringCoeff;
            *mParam.mStaticMieSymmetricalProp = *rOldParam.mMieSymmetricalProp;
            *mParam.mDynamicRayleighAmplifier = *rOldParam.mRayleighAmplifierRendering;
            *mParam.mDynamicMieSymmetricalProp = *rOldParam.mMieSymmetricalPropRendering;
            *mParam.mDynamicMieAmplifier = *rOldParam.mMieAmplifierRendering;
            *mParam.mDynamicColor = *rOldParam.mSunColor;
        }

        mParam.mGroundColor->a = 0.0f;
        *mParam.mVersion = 2;
    }

    *mParam.mVersion = 2;
    mFlags.setDirect((mFlags.getDirect() & ~(cFlag_Enable | cFlag_Sky | cFlag_Ground |
                                             cFlag_UpdateBasis)) |
                     (*mParam.mFlag & (cFlag_Enable | cFlag_Sky | cFlag_Ground)) |
                     cFlag_UpdateBasis);
    *mParam.mAmplifierForEnvmap = 1.0f;
}

/**
 * Generates the host IO messages of the parameters and contexts.
 * @param pContext Host IO context
 */
void Sky::genMessage(sead::hostio::Context* pContext)
{
    genMessageIO(pContext, 0xf);
    mParam.mDynamicColor.genMessageParameter(pContext, mParam.mDynamicColor.getMeta());
    mParam.mStaticRayleighBaseHeight.genMessageParameter(
        pContext, mParam.mStaticRayleighBaseHeight.getMeta());
    mParam.mStaticMieBaseHeight.genMessageParameter(pContext,
                                                    mParam.mStaticMieBaseHeight.getMeta());
    mParam.mStaticMieScatteringCoeff.genMessageParameter(
        pContext, mParam.mStaticMieScatteringCoeff.getMeta());
    mParam.mStaticMieSymmetricalProp.genMessageParameter(
        pContext, mParam.mStaticMieSymmetricalProp.getMeta());
    mParam.mDynamicRayleighAmplifier.genMessageParameter(
        pContext, mParam.mDynamicRayleighAmplifier.getMeta());
    mParam.mDynamicMieSymmetricalProp.genMessageParameter(
        pContext, mParam.mDynamicMieSymmetricalProp.getMeta());
    mParam.mDynamicMieAmplifier.genMessageParameter(pContext,
                                                    mParam.mDynamicMieAmplifier.getMeta());
    mParam.mScatterFogNear.genMessageParameter(pContext, mParam.mScatterFogNear.getMeta());
    mParam.mScatterFogFar.genMessageParameter(pContext, mParam.mScatterFogFar.getMeta());
    mParam.mScatterFogDensity.genMessageParameter(pContext, mParam.mScatterFogDensity.getMeta());
    mParam.mScatterFogAtten.genMessageParameter(pContext, mParam.mScatterFogAtten.getMeta());
    mParam.mScatterFogHorz.genMessageParameter(pContext, mParam.mScatterFogHorz.getMeta());
    mParam.mAdhocFogNear.genMessageParameter(pContext, mParam.mAdhocFogNear.getMeta());
    mParam.mAdhocFogFar.genMessageParameter(pContext, mParam.mAdhocFogFar.getMeta());
    mParam.mAdhocFogColor.genMessageParameter(pContext, mParam.mAdhocFogColor.getMeta());
    mParam.mAdhocFogAttenGround.genMessageParameter(pContext,
                                                    mParam.mAdhocFogAttenGround.getMeta());
    mParam.mAdhocFogAttenSky.genMessageParameter(pContext, mParam.mAdhocFogAttenSky.getMeta());
    mParam.mAdhocFogAttenMinScaleSky.genMessageParameter(
        pContext, mParam.mAdhocFogAttenMinScaleSky.getMeta());
    mParam.mAmplifierForEnvmap.genMessageParameter(pContext,
                                                   mParam.mAmplifierForEnvmap.getMeta());
    mDebugTexturePage->genMessagePage(pContext, this);
    mParam.mGroundColor.genMessageParameter(pContext, mParam.mGroundColor.getMeta());
    mParam.mRenderSunIntensity.genMessageParameter(pContext,
                                                   mParam.mRenderSunIntensity.getMeta());
    mParam.mRenderSunSize.genMessageParameter(pContext, mParam.mRenderSunSize.getMeta());
    mParam.mRenderSunLerp.genMessageParameter(pContext, mParam.mRenderSunLerp.getMeta());

    u32 contextNum = mContexts.size();

    for (u32 i = 0; i < contextNum; i++)
    {
        {
            sead::FormatFixedSafeString<256> str("GroupHeader = context %d, Layout = Wrap", i);
        }

        {
            sead::FormatFixedSafeString<256> str("Sky: %s",
                                                 getContext_(i).mIsSkyEnable ? "○" : "×");
        }

        {
            sead::FormatFixedSafeString<256> str("Ground: %s",
                                                 getContext_(i).mIsGroundEnable ? "○" : "×");
        }
    }
}

/**
 * Converts the sun latitude and longitude into the sun direction.
 */
void Sky::updateDirectionFromLatLong()
{
    f32 latCos = std::cos(mParam.mSunLatitude);
    f32 latSin = std::sin(mParam.mSunLatitude);
    f32 longCos = std::cos(mParam.mSunLongitude);
    f32 longSin = std::sin(mParam.mSunLongitude);
    mParam.mSunDir.set(-latCos * longSin, -latSin, -latCos * longCos);
}

/**
 * Converts the sun direction into the sun latitude and longitude.
 */
void Sky::updateDirectionToLatLong()
{
    f32 length = mParam.mSunDir.length();

    if (length > 0.0f)
    {
        f32 inv = 1.0f / length;
        sead::Vector3f dir(inv * mParam.mSunDir.x, inv * mParam.mSunDir.y,
                           inv * mParam.mSunDir.z);
        sead::Vector2f horizontal(dir.x, dir.z);

        if (horizontal.normalize() > 0.0f)
        {
            mParam.mSunLongitude = std::atan2(-horizontal.x, -horizontal.y);
        }

        mParam.mSunLatitude = std::asin(sead::Mathf::clamp(-dir.y, -1.0f, 1.0f));
    }
}

/**
 * Handles host IO property changes.
 * @param pEvent Property event
 */
void Sky::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    listenPropertyEventIO(this, pEvent);
    listenPropertyEventParameter(this, pEvent);

    bool isHandled = true;
    uintptr_t id = pEvent->getIdValue();

    if (id == uintptr_t(&*mParam.mStaticRayleighBaseHeight) || id == uintptr_t(&*mParam.mStaticMieBaseHeight) ||
        id == uintptr_t(&*mParam.mStaticMieScatteringCoeff) || id == uintptr_t(&*mParam.mStaticMieSymmetricalProp) ||
        id == uintptr_t(&mScatteringOrder))
    {
        mFlags.set(cFlag_UpdateBasis);
    }
    else if (id == uintptr_t(&mWavelength.x) || id == uintptr_t(&mWavelength.y) || id == uintptr_t(&mWavelength.z))
    {
        calcRayleighScatteringCoeff_();
        mFlags.set(cFlag_UpdateBasis);
    }
    else if (id == uintptr_t(&mParam.mStaticRayleighScatteringCoeff.x) ||
             id == uintptr_t(&mParam.mStaticRayleighScatteringCoeff.y) ||
             id == uintptr_t(&mParam.mStaticRayleighScatteringCoeff.z))
    {
        mFlags.set(cFlag_UpdateBasis);
    }
    else
    {
        isHandled = false;
    }

    id = pEvent->getIdValue();

    if (id == uintptr_t(&mParam.mSunLatitude) || id == uintptr_t(&mParam.mSunLongitude))
    {
        updateDirectionFromLatLong();
    }
    else if (id == uintptr_t(&mParam.mSunDir))
    {
        updateDirectionToLatLong();
    }
    else if (id == uintptr_t(&mParam.mColorTemperature))
    {
        static const sead::Color4f sBaseColor = calcColorTemperature(
            sead::Color4f::cWhite, 6500.0f, mWavelength.x, mWavelength.y, mWavelength.z);
        sead::Color4f& rColor = *mParam.mDynamicColor;
        f32 alpha = rColor.a;
        f32 intensity = rColor.r > rColor.g ? (rColor.r > rColor.b ? rColor.r : rColor.b) :
                                              (rColor.g > rColor.b ? rColor.g : rColor.b);
        sead::Color4f color = calcColorTemperature(sBaseColor, mParam.mColorTemperature,
                                                   mWavelength.x, mWavelength.y, mWavelength.z) *
                              intensity;
        rColor.r = color.r;
        rColor.g = color.g;
        rColor.b = color.b;
        rColor.a = alpha;
    }
    else if (!isHandled)
    {
        if (id == 0x66)
        {
            mTextureSize = mTextureSizeEdit;
            freeTexture();
            allocateTexture(agl::detail::PrivateResource::instance()->getDebugHeap());
            mFlags.set(cFlag_UpdateBasis);
        }

        if (id == 0x65)
        {
            mFlags.set(cFlag_UpdateBasis);
        }
    }
}

namespace {

/**
 * Activates a program and sets the uniforms shared by the basis programs.
 * @param pProgram Program to activate
 * @param rSizes Texture sizes
 */
void BasisDrawer::setProgram(const ShaderProgram* pProgram, const Sky::Sizes& rSizes)
{
    mProgram = pProgram;
    pProgram->activate(mDrawContext, true);
    mProgram->getUniformLocation(0).setUniform(mDrawContext, 4, &rSizes.mTransmittance);
    mProgram->getUniformLocation(1).setUniform(mDrawContext, 4, &rSizes.mIrradiance);
    mProgram->getUniformLocation(2).setUniform(mDrawContext, 4, &rSizes.mAltitude);
    mProgram->getUniformLocation(3).setUniform(mDrawContext, 4, &rSizes.mSunZenith);
    mProgram->getUniformLocation(4).setUniform(mDrawContext, 4, &rSizes.mViewZenith);
    mProgram->getUniformLocation(5).setUniform(mDrawContext, 4, &rSizes.mSunView);
    mProgram->getUniformLocation(6).setUniform(mDrawContext, 3,
                                               &mSky->mParam.mStaticRayleighScatteringCoeff);
    mProgram->getUniformLocation(7).setUniform(mDrawContext,
                                               *mSky->mParam.mStaticMieScatteringCoeff);
    mProgram->getUniformLocation(8).setUniform(mDrawContext,
                                               *mSky->mParam.mStaticMieSymmetricalProp);
    mProgram->getUniformLocation(9).setUniform(mDrawContext,
                                               *mSky->mParam.mStaticRayleighBaseHeight);
    mProgram->getUniformLocation(10).setUniform(mDrawContext, *mSky->mParam.mStaticMieBaseHeight);
}

/**
 * Draws one altitude layer of a 3D basis texture.
 * @param layer Normalized altitude of the layer
 */
void BasisDrawer::drawLayer(f32 layer) const
{
    setLayerParam(mDrawContext, mProgram, 11, layer);
    drawLayerQuad();
}

/**
 * Draws one altitude layer with a bake program.
 * @param isEffectiveBelowHorizon Whether the scattering below the horizon is used
 * @param layer Normalized altitude of the layer
 */
void BakeDrawer::drawLayer(bool isEffectiveBelowHorizon, f32 layer) const
{
    setLayerParam(mDrawContext, mProgram, 15, layer);
    setUniform(mDrawContext, mProgram, 17, layer);
    setUniform(mDrawContext, mProgram, 18, s32(isEffectiveBelowHorizon));
    draw();
}

/**
 * Sets the post effect uniforms and draws a full screen quad.
 * @param pDrawContext Draw context
 * @param pProgram Active program
 * @param rSizes Texture sizes
 */
void PostFxDrawer::draw(DrawContext* pDrawContext, const ShaderProgram* pProgram,
                        const Sky::Sizes& rSizes) const
{
    mSky->mTransmittanceSampler.activate(pDrawContext, pProgram->getSamplerLocation(2), -1, false);
    mSky->mIrradianceSampler.activate(pDrawContext, pProgram->getSamplerLocation(3), -1, false);
    mSky->mInscatterSampler.activate(pDrawContext, pProgram->getSamplerLocation(4), -1, false);
    mSky->getContext_(mContext).mBakedInscatter.activate(pDrawContext,
                                                         pProgram->getSamplerLocation(5), -1, false);
    mSky->getContext_(mContext).mBakedIrradiance.activate(pDrawContext,
                                                          pProgram->getSamplerLocation(6), -1,
                                                          false);
    mSky->getContext_(mContext).mBakedRangeTransmittance.activate(
        pDrawContext, pProgram->getSamplerLocation(7), -1, false);
    if (mColor)
    {
        activateSampler(mSky, mContext, 0, *mColor, pDrawContext, pProgram, 0);
    }

    if (mDepth)
    {
        activateSampler(mSky, mContext, 1, *mDepth, pDrawContext, pProgram, 1);
    }

    sead::Matrix44f projInv;
    sead::Matrix34f viewInv;
    sead::Vector3f sunColor;
    sead::Vector4f groundColor;
    sead::Vector4f sunInfo;
    sead::Vector4f nearFar;
    sead::Vector4f scatterFogDistance;
    sead::Vector4f scatterFogCoeff;
    sead::Vector4f normalFogDistance;
    sead::Vector4f normalFogCoeff;
    sead::Vector4f normalFogColor;

    const sead::Color4f& rColor = *mSky->mParam.mDynamicColor;
    sunColor.set(rColor.a * rColor.r, rColor.a * rColor.g, rColor.a * rColor.b);
    f32 intensity = *mSky->mParam.mRenderSunIntensity / rColor.a;
    f32 sunSize = *mSky->mParam.mRenderSunSize;
    f32 scaledSize = sead::Mathf::max(sunSize, 0.001f) * 0.0001f;
    f32 lerpScale = *mSky->mParam.mRenderSunLerp / scaledSize;
    sunInfo.set(intensity, sunSize, lerpScale, (1.0f - scaledSize) * lerpScale);
    groundColor.x = mSky->mParam.mGroundColor->r;
    groundColor.y = mSky->mParam.mGroundColor->g;
    groundColor.z = mSky->mParam.mGroundColor->b;
    groundColor.w = 1.0f - sead::Mathf::clamp(mSky->mParam.mGroundColor->a, 0.0f, 1.0f);

    projInv.setInverse(*mProjMtx);
    viewInv.setInverse(*mViewMtx);

    nearFar.x = mNear;
    nearFar.y = mFar;
    nearFar.z = nearFar.y - nearFar.x;
    nearFar.w = 1.0f / (nearFar.z * 0.001f);
    f32 scatterRange = *mSky->mParam.mScatterFogFar - *mSky->mParam.mScatterFogNear;
    f32 scatterInv = 1.0f / scatterRange;
    scatterFogDistance.set(scatterInv, *mSky->mParam.mScatterFogNear * scatterInv,
                           *mSky->mParam.mScatterFogNear / nearFar.z, nearFar.z / scatterRange);
    scatterFogCoeff.x = *mSky->mParam.mScatterFogAtten;
    scatterFogCoeff.y = *mSky->mParam.mScatterFogHorz;
    scatterFogCoeff.z = *mSky->mParam.mScatterFogDensity;
    scatterFogCoeff.w = 0.0f;
    f32 adhocRange = *mSky->mParam.mAdhocFogFar - *mSky->mParam.mAdhocFogNear;
    f32 adhocInv = 1.0f / adhocRange;
    normalFogDistance.set(adhocInv, *mSky->mParam.mAdhocFogNear * adhocInv,
                          *mSky->mParam.mAdhocFogNear / nearFar.z, nearFar.z / adhocRange);
    if (mSky->mParam.mAdhocFogColor->a > 0.0f)
    {
        normalFogCoeff.x = *mSky->mParam.mAdhocFogAttenGround;
        normalFogCoeff.y = *mSky->mParam.mAdhocFogAttenSky;
        normalFogCoeff.z = *mSky->mParam.mAdhocFogAttenMinScaleSky;
    }
    else
    {
        normalFogCoeff.x = 0.0f;
        normalFogCoeff.y = 0.0f;
        normalFogCoeff.z = 1.0f;
    }

    normalFogCoeff.w = mSky->mParam.mAdhocFogColor->a;
    normalFogColor.x = mSky->mParam.mAdhocFogColor->r;
    normalFogColor.y = mSky->mParam.mAdhocFogColor->g;
    normalFogColor.z = mSky->mParam.mAdhocFogColor->b;
    normalFogColor.w = 1.0f;

    pProgram->getUniformLocation(1).setUniform(pDrawContext, 4, &rSizes.mTransmittance);
    pProgram->getUniformLocation(2).setUniform(pDrawContext, 4, &rSizes.mIrradiance);
    pProgram->getUniformLocation(3).setUniform(pDrawContext, 4, &rSizes.mAltitude);
    pProgram->getUniformLocation(4).setUniform(pDrawContext, 4, &rSizes.mSunZenith);
    pProgram->getUniformLocation(5).setUniform(pDrawContext, 4, &rSizes.mViewZenith);
    pProgram->getUniformLocation(6).setUniform(pDrawContext, 4, &rSizes.mSunView);
    pProgram->getUniformLocation(7).setUniform(pDrawContext, 3,
                                               &mSky->mParam.mStaticRayleighScatteringCoeff);
    setUniform(pDrawContext, pProgram, 8, *mSky->mParam.mStaticMieScatteringCoeff);
    setUniform(pDrawContext, pProgram, 10, *mSky->mParam.mDynamicRayleighAmplifier);
    setUniform(pDrawContext, pProgram, 9, *mSky->mParam.mDynamicMieSymmetricalProp);
    setUniform(pDrawContext, pProgram, 11, *mSky->mParam.mDynamicMieAmplifier);
    setUniform(pDrawContext, pProgram, 12, mAmplifier);
    pProgram->getUniformLocation(13).setUniform(pDrawContext, 16, &projInv);
    pProgram->getUniformLocation(14).setUniform(pDrawContext, 12, &viewInv);
    {
        sead::Vector3f sunDir(-mSky->mParam.mSunDir.x, -mSky->mParam.mSunDir.y,
                              -mSky->mParam.mSunDir.z);
        pProgram->getUniformLocation(15).setUniform(pDrawContext, 3, &sunDir);
    }

    pProgram->getUniformLocation(16).setUniform(pDrawContext, 3, &sunColor);
    pProgram->getUniformLocation(17).setUniform(pDrawContext, 4, &groundColor);
    pProgram->getUniformLocation(18).setUniform(pDrawContext, 4, &nearFar);
    pProgram->getUniformLocation(19).setUniform(pDrawContext, 4, &sunInfo);
    pProgram->getUniformLocation(20).setUniform(pDrawContext, 4, &scatterFogDistance);
    pProgram->getUniformLocation(21).setUniform(pDrawContext, 4, &scatterFogCoeff);
    pProgram->getUniformLocation(22).setUniform(pDrawContext, 4, &normalFogDistance);
    pProgram->getUniformLocation(23).setUniform(pDrawContext, 4, &normalFogCoeff);
    pProgram->getUniformLocation(24).setUniform(pDrawContext, 4, &normalFogColor);
    setUniform(pDrawContext, pProgram, 26, 0.4f);
    drawQuad(pDrawContext);
}

}  // namespace

}  // namespace agl::pfx
