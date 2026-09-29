#pragma once

#include <cmath>
#include <container/seadBuffer.h>
#include <gfx/seadColor.h>
#include <hostio/seadHostIONode.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
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
class ShaderProgramArchive;
namespace utl {
class DebugTexturePage;
}  // namespace utl
}  // namespace agl

namespace agl::pfx {

class Sky : public utl::IParameterIO, public sead::hostio::Node {
public:
    enum {
        cFlag_Initialized = 1 << 0,
        cFlag_TextureAllocated = 1 << 1,
        cFlag_Enable = 1 << 2,
        cFlag_GroundFog = 1 << 4,
        cFlag_Sky = 1 << 5,
        cFlag_Ground = 1 << 6,
        cFlag_SkyFog = 1 << 7,
        cFlag_UpdateBasis = 1 << 16,
        cFlag_UpdateBasisAlways = 1 << 17,
        cFlag_BakeIrradiance = 1 << 18,
        cFlag_Scattering = 1 << 19,
        cFlag_InfLoopCheckDisabled = 1 << 20,
        cFlag_SunDisk = 1 << 24,
    };

    struct TextureSize {
        u32 mTransmittanceWidth;
        u32 mTransmittanceHeight;
        u32 mIrradianceWidth;
        u32 mIrradianceHeight;
        u32 mAltitudeNum;
        u32 mSunZenithNum;
        u32 mViewZenithNum;
        u32 mSunViewNum;
        u32 mBakedInscatterWidth;
        u32 mBakedInscatterHeight;
        u32 mBakedRangeTransmittanceWidth;
        u32 mBakedRangeTransmittanceHeight;
    };
    static_assert(sizeof(TextureSize) == 0x30);

    struct InitializeArg {
        sead::Heap* mHeap;
        s32 mContextNum;
        u8 _c[0xc];
        TextureSize mTextureSize;
        const void* mBasisImage;
        u32 mBasisImageSize;
    };
    static_assert(sizeof(InitializeArg) == 0x58);

    struct RenderingParameterStatic {
        f32 mRayleighBaseHeight;
        f32 mMieBaseHeight;
        f32 mMieScatteringCoeff;
        f32 mMieSymmetricalProp;
    };

    struct RenderingParameter {
        f32 mRayleighAmplifier;
        f32 mMieAmplifier;
        f32 mMieSymmetricalProp;
    };

    struct ScatterFogParam {
        f32 mNear;
        f32 mFar;
        f32 mDensity;
        f32 mAtten;
        f32 mHorz;
    };

    struct AdhocFogParam {
        sead::Color4f mColor;
        f32 mNear;
        f32 mFar;
        f32 mAttenGround;
        f32 mAttenSky;
        f32 mAttenMinScaleSky;
    };

    struct BakeInfoParam {
        f32 mAltitude;
        f32 mHorizon;
    };

    struct Context {
        bool mIsEnable;
        bool mIsSkyEnable;
        bool mIsGroundEnable;
        bool mIsEffectiveBelowHorizon;
        TextureSampler mBakedInscatter;
        TextureSampler mBakedIrradiance;
        TextureSampler mBakedRangeTransmittance;
        const TextureSampler* mCloudSampler;
        f32 mCloudParam;
        RenderBuffer mRenderBuffer;
        RenderTargetColor mRenderTargets[2];
        TextureSampler mSamplers[4];
        u8 mDirtyTargets = 0;
        u32 _d84;
        BakeInfoParam mBakeInfo;
    };
    static_assert(sizeof(Context) == 0xd90);

    struct Sizes {
        sead::Vector4f mTransmittance;
        sead::Vector4f mIrradiance;
        sead::Vector4f mBakedInscatter;
        sead::Vector4f mBakedRangeTransmittance;
        sead::Vector4f mAltitude;
        sead::Vector4f mSunZenith;
        sead::Vector4f mViewZenith;
        sead::Vector4f mSunView;
    };

    struct OldParam {
        utl::Parameter<f32> mRayleighBaseHeight;
        utl::Parameter<f32> mMieBaseHeight;
        utl::Parameter<f32> mMieScatteringCoeff;
        utl::Parameter<f32> mMieSymmetricalProp;
        utl::Parameter<f32> mRayleighAmplifierRendering;
        utl::Parameter<f32> mMieSymmetricalPropRendering;
        utl::Parameter<f32> mMieAmplifierRendering;
        utl::Parameter<sead::Color4f> mSunColor;
    };
    static_assert(sizeof(OldParam) == 0x108);

    Sky();
    ~Sky() override;

    static void setUpShader(ShaderProgramArchive* pArchive, sead::Heap* pHeap);

    void initialize(const InitializeArg& rArg);
    void finalize();
    void allocateTexture(sead::Heap* pHeap);
    bool setBasisTextureImage(const void* pImage, u32 size);
    void freeTexture();
    bool setBasisTextureImage(const void* pTransmittance, u32 transmittanceSize,
                              const void* pInscatter, u32 inscatterSize,
                              const void* pIrradiance, u32 irradianceSize);
    void drawBasis(DrawContext* pDrawContext, u32 context);
    void drawBasisImpl(DrawContext* pDrawContext, u32 context);
    void drawContext(DrawContext* pDrawContext, u32 context, const sead::Matrix34f& rViewMtx,
                     f32 near, f32 far);
    void setUpRenderBuffer(DrawContext* pDrawContext, u32 context, const TextureData* pTexture,
                           u32 mipLevel, u32 slice, u32 target, u32 flags);
    void invalidateRenderBufferCache(DrawContext* pDrawContext, u32 context);
    void getRenderingParameterStatic(RenderingParameterStatic* pParam) const;
    void setRenderingParameterStatic(const RenderingParameterStatic& rParam, bool update);
    void getRenderingParameter(RenderingParameter* pParam) const;
    void setRenderingParameter(const RenderingParameter& rParam);
    void getScatterFogParam(ScatterFogParam* pParam) const;
    void setScatterFogParam(const ScatterFogParam& rParam);
    void getAdhocFogParam(AdhocFogParam* pParam) const;
    void setAdhocFogParam(const AdhocFogParam& rParam);
    void getBakeInfoParam(BakeInfoParam* pParam, u32 context) const;
    void drawBoth(DrawContext* pDrawContext, u32 context, const sead::Matrix44f& rProjMtx,
                  const sead::Matrix34f& rViewMtx, f32 near, f32 far, const TextureData& rColor,
                  const TextureData& rDepth, bool useColor, f32 amplifier);
    void drawSky(DrawContext* pDrawContext, u32 context, const sead::Matrix44f& rProjMtx,
                 const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 amplifier);
    void drawGround(DrawContext* pDrawContext, u32 context, const sead::Matrix44f& rProjMtx,
                    const sead::Matrix34f& rViewMtx, f32 near, f32 far,
                    const TextureData& rColor, const TextureData& rDepth, bool useColor,
                    f32 amplifier);
    void updateDirectionFromLatLong();
    void updateDirectionToLatLong();
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

protected:
    bool preWrite_() const override;
    bool preRead_() override;
    void postRead_() override;

public:
    Context& getContext_(u32 context) { return mContexts[context]; }
    const Context& getContext_(u32 context) const { return mContexts[context]; }
    OldParam& getOldParam_() { return *reinterpret_cast<OldParam*>(mOldParamBuffer); }

    void calcRayleighScatteringCoeff_()
    {
        sead::Vector3<double> wavelength(mWavelength.x, mWavelength.y, mWavelength.z);
        sead::Vector3<double> index(
            0.05792105 / (238.0185 - std::pow(wavelength.x, -2.0)) +
                0.00167917 / (57.362 - std::pow(wavelength.x, -2.0)),
            0.05792105 / (238.0185 - std::pow(wavelength.y, -2.0)) +
                0.00167917 / (57.362 - std::pow(wavelength.y, -2.0)),
            0.05792105 / (238.0185 - std::pow(wavelength.z, -2.0)) +
                0.00167917 / (57.362 - std::pow(wavelength.z, -2.0)));
        index += sead::Vector3<double>::ones;
        f64 x = (index.x * index.x - 1.0) * (index.x * index.x - 1.0) * 248.05021344239853 /
                (std::pow(wavelength.x, 4.0) * 76.5);
        f64 y = (index.y * index.y - 1.0) * (index.y * index.y - 1.0) * 248.05021344239853 /
                (std::pow(wavelength.y, 4.0) * 76.5);
        f64 z = (index.z * index.z - 1.0) * (index.z * index.z - 1.0) * 248.05021344239853 /
                (std::pow(wavelength.z, 4.0) * 76.5);
        mParam.mStaticRayleighScatteringCoeff.set(x * 1000.0, y * 1000.0, z * 1000.0);
    }

    sead::Buffer<const ShaderProgram*> mPrograms;
    TextureSize mTextureSize{};
    TextureSize mTextureSizeEdit{};
    TextureSampler mTransmittanceSampler;
    TextureSampler mInscatterSampler;
    TextureSampler mIrradianceSampler;
    sead::Buffer<Context> mContexts;
    sead::Vector3f mWavelength;
    s32 mScatteringOrder = 8;
    struct {
        utl::ParameterObj mObj;
        utl::Parameter<s32> mVersion;
        utl::Parameter<f32> mStaticRayleighBaseHeight;
        sead::Vector3f mStaticRayleighScatteringCoeff;
        utl::Parameter<f32> mStaticMieBaseHeight;
        utl::Parameter<f32> mStaticMieScatteringCoeff;
        utl::Parameter<f32> mStaticMieSymmetricalProp;
        utl::Parameter<f32> mDynamicRayleighAmplifier;
        utl::Parameter<f32> mDynamicMieSymmetricalProp;
        utl::Parameter<f32> mDynamicMieAmplifier;
        sead::Vector3f mSunDir;
        f32 mSunLatitude;
        f32 mSunLongitude;
        utl::Parameter<sead::Color4f> mDynamicColor;
        f32 mColorTemperature;
        f32 mFade;
        utl::Parameter<f32> mScatterFogNear;
        utl::Parameter<f32> mScatterFogFar;
        utl::Parameter<f32> mScatterFogDensity;
        utl::Parameter<f32> mScatterFogAtten;
        utl::Parameter<f32> mScatterFogHorz;
        utl::Parameter<f32> mAdhocFogNear;
        utl::Parameter<f32> mAdhocFogFar;
        utl::Parameter<f32> mAdhocFogAttenGround;
        utl::Parameter<f32> mAdhocFogAttenSky;
        utl::Parameter<f32> mAdhocFogAttenMinScaleSky;
        utl::Parameter<sead::Color4f> mAdhocFogColor;
        utl::Parameter<f32> mRenderSunIntensity;
        utl::Parameter<f32> mRenderSunSize;
        utl::Parameter<f32> mRenderSunLerp;
        utl::Parameter<sead::Color4f> mGroundColor;
        utl::Parameter<f32> mAmplifierForEnvmap;
        utl::Parameter<u32> mFlag;
    } mParam;
    sead::BitFlag32 mFlags = 0;
    alignas(4) u8 mOldParamBuffer[0x404];
    utl::DebugTexturePage* mDebugTexturePage = nullptr;
};
static_assert(sizeof(Sky) == 0xed0);

}  // namespace agl::pfx
