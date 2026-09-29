#pragma once

#include <container/seadPtrArray.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadSafeString.h>
#include "common/aglShaderProgramArchive.h"

namespace sead {
class ArchiveRes;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {

class ShaderProgram;

namespace detail {

class ShaderHolder : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(ShaderHolder)

public:
    enum ArchiveType {
        cArchive_Common = 0,
        cArchive_Technique,
        cArchive_TechniquePfx,
        cArchive_TechniqueLght,
        cArchive_TechniqueLghtLpp,
        cArchive_TechniqueShdw,
        cArchive_Num
    };

    enum ProgramType {
        cDevUtil = 0,
        cDepthVisualize,
        cTexture,
        cReduce2,
        cReduce4,
        cReduce8,
        cReduce16,
        cColorCorrection,
        cColorCorrectionMap,
        cXluSnap,
        cImageFilterGaussian,
        cImageFilterCubic,
        cImageFilterBlur,
        cTextureMultColor,
        cRed,
        cGreen,
        cBlue,
        cAlpha,
        cDepthRaw,
        cDepthLinear,
        cDepthLinearArray,
        cDepth2d,
        cDepth2dArrayQuad,
        cDepth2dMin,
        cDepth2dArrayMin,
        cDepth2dMax,
        cDepth2dArrayMax,
        cColor2dArrayQuad,
        cColor2dMin,
        cColor2dArrayMin,
        cColor2dMax,
        cColor2dArrayMax,
        cUint,
        cUintArray,
        cDepthMask,
        cLuminance,
        cTexture2dArray,
        cTexture3d,
        cTextureCubeMap,
        cTextureCubeMapArray,
        cTextureClrmtx2d,
        cTextureClrmtx2dArray,
        cTextureTexcoord,
        cTextureTexcoordMultColor,
        cTextureMultiSample1x,
        cTextureMultiSample2x,
        cTextureMultiSample4x,
        cTextureMultiSample8x,
        cAlphaModify,
        cTextureColorDrift,
        cColorQuad,
        cClearQuad,
        cTopBottomColor,
        cTextureGamma,
        cClear,
        cDrawImm,
        cDrawFan,
        cDrawCapsule,
        cFrameBufferFlipY,
        cFrameBufferNoFlip,
        cBloomMask,
        cBloomGaussian,
        cBloomCompose,
        cBloomReduce,
        cHdrCompose,
        cDofNearMask,
        cDofMipmap,
        cDofDepthMask,
        cDofFinal,
        cDofVignetting,
        cDofExpandReduce,
        cFxaa,
        cFxaaColoronly,
        cFxaaLuma,
        cFxaaReprojection,
        cFilterAaReprojection,
        cReduceAa,
        cSmaaLineDetection,
        cSmaaWeightCalculation,
        cSmaaCompose,
        cSmaaResolve,
        cSmaacLineDetection,
        cSmaacWeightCalculation,
        cSmaacUtility,
        cLightmap,
        cLightmapClear,
        cLocalLightmap,
        cLightPrePassPointLight,
        cLightPrePassSpotLight,
        cLightPrePassProjLight,
        cLightPrePassDev,
        cMultiFilterReduce,
        cMultiFilterExpand,
        cStaticDepthShadow,
        cCubemapGaussian,
        cCubemapHeadExtract,
        cCubemapHeadConvoluteFirst,
        cCubemapHeadConvoluteOther,
        cCubemapHdrEncode,
        cCubemapDrawIlluminant,
        cNv12decode,
        cScreenPick,
        cDebugCubemap,
        cDepthShadowDebug,
        cVsm,
        cRaymarchDepthShadow,
        cSsaoAoBuffer,
        cSsaoBlur,
        cSsaoReduce,
        cSsaoMask,
        cAlchemyAoBuffer,
        cOcclusionQuery,
        cOcclusionRendererClearBuf,
        cOcclusionRenderer,
        cOccludedEffectLensflare,
        cTextureCompress,
        cTextureCompressHq,
        cShadowMask,
        cRadialBlur,
        cRadialBlurCompose,
        cSsii,
        cSsiiRef,
        cSsiiReduce,
        cSsiiReduceG,
        cSsiiPreRender,
        cSsiiExpand,
        cSsiiAntiHowling,
        cNormalMap,
        cAutoExposure,
        cSssssBlur,
        cSssssExpandSss,
        cSssssReduce,
        cSssssMerge,
        cSssssMergeWeight,
        cGlareFilterSeed,
        cGlareFilterBlur,
        cGlareFilterDepth,
        cGlareFilterClear,
        cFlareFilterFlare,
        cFlareFilterCopy,
        cPlanarReflection,
        cShadowPrePass,
        cScreenSpaceSelfShadow,
        cScreenSpaceSelfShadowCreateHiz,
        cDecalSimple,
        cDecalTrail,
        cDecalTextureDrawer,
        cNormalDrawerPost,
        cDebugPrimitive,
        cDebugShapeInstanced,
        cDebugPointInstanced,
        cDebugLineInstanced,
        cDebugTriangleInstanced,
        cCubemapIrradiance,
        cCubemapSh,
        cCubemapShPoint,
        cCubemapShOcclusion,
        cLocalReflectionMask,
        cLocalReflectionFilter,
        cLocalReflectionGaussianFilter,
        cLocalReflectionDepthFilter,
        cLocalReflectionSpecular,
        cLocalReflectionSpecularVec4,
        cLocalReflectionJitteredCopy,
        cLocalReflectionDebug,
        cDebugShVolumeVtx,
        cDebugShVolumeFrag,
        cDebugShVolumeHemiLight,
        cShVolumeLighting,
        cShVolumeCubemapLighting,
        cShVolumeClearAllSlices,
        cShVolumeClearSingleSlice,
        cShVolumePoint,
        cShVolumePointMrt,
        cShVolumeRectMrt,
        cShVolumeCopyMrt,
        cShVolumeCopyMrtSingleSlice,
        cShVolumeDiffUpdateMrt,
        cShVolumeCopyCubemapMrt,
        cShVolumeCubemapFaceFilter,
        cVdm,
        cCloud,
        cVolumeMaskReducedepth,
        cVolumeMaskLayer,
        cVolumeMaskRaymarch,
        cVolumeMaskDrawtex,
        cVolumeMaskDebug,
        cSkyTransmittance,
        cSkyIrradiance,
        cSkyInscatter,
        cSkyDeltaInscatter,
        cSkyCopyIrradiance,
        cSkyCopyInscatter,
        cSkyBakeInscatter,
        cSkyBakeIrradiance,
        cSkyBakeRangeTransmittance,
        cSkyPostfxSky,
        cSkyPostfxGround,
        cStarRender,
        cProgram_Num
    };

    ShaderHolder();
    virtual ~ShaderHolder();

    void initialize(sead::ArchiveRes* pArchive, sead::Heap* pHeap);
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    ShaderProgram* getShaderProgram(s32 type) const { return mShaderPrograms[type]; }
    ShaderProgram* getShaderProgramUnsafe(s32 type) const { return mShaderPrograms(type); }
    ShaderProgramArchive& getArchive(s32 type) { return mArchives[type]; }
    void setNoOption(bool noOption) { mNoOption = noOption; }

private:
    sead::FixedPtrArray<ShaderProgram, cProgram_Num> mShaderPrograms;
    ShaderProgramArchive mArchives[cArchive_Num];
    bool mNoOption = true;
};
static_assert(sizeof(ShaderHolder) == 0xAF8);

}  // namespace detail
}  // namespace agl
