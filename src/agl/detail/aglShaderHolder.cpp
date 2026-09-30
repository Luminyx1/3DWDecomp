#include "detail/aglShaderHolder.h"

#include <heap/seadExpHeap.h>
#include <resource/seadArchiveRes.h>

namespace agl {

namespace pfx {
class Sky {
public:
    static void setUpShader(ShaderProgramArchive* pArchive, sead::Heap* pHeap);
};
}  // namespace pfx

namespace eft {
class Star {
public:
    static void setUpShader(ShaderProgramArchive* pArchive, sead::Heap* pHeap);
};
}  // namespace eft

namespace detail {

SEAD_SINGLETON_DISPOSER_IMPL(ShaderHolder)

namespace {

struct ProgramInfo {
    s32 mArchive;
    const char* mName;
};

const ProgramInfo cProgramInfo[ShaderHolder::cProgram_Num] = {
    {ShaderHolder::cArchive_Common, "dev_util"},
    {ShaderHolder::cArchive_Common, "depth_visualize"},
    {ShaderHolder::cArchive_Common, "texture"},
    {ShaderHolder::cArchive_Common, "reduce_2"},
    {ShaderHolder::cArchive_Common, "reduce_4"},
    {ShaderHolder::cArchive_Common, "reduce_8"},
    {ShaderHolder::cArchive_Common, "reduce_16"},
    {ShaderHolder::cArchive_TechniquePfx, "color_correction"},
    {ShaderHolder::cArchive_TechniquePfx, "color_correction_map"},
    {ShaderHolder::cArchive_Common, "xlu_snap"},
    {ShaderHolder::cArchive_Common, "image_filter_gaussian"},
    {ShaderHolder::cArchive_Common, "image_filter_cubic"},
    {ShaderHolder::cArchive_Common, "image_filter_blur"},
    {ShaderHolder::cArchive_Common, "texture_mult_color"},
    {ShaderHolder::cArchive_Common, "red"},
    {ShaderHolder::cArchive_Common, "green"},
    {ShaderHolder::cArchive_Common, "blue"},
    {ShaderHolder::cArchive_Common, "alpha"},
    {ShaderHolder::cArchive_Common, "depth_raw"},
    {ShaderHolder::cArchive_Common, "depth_linear"},
    {ShaderHolder::cArchive_Common, "depth_linear_array"},
    {ShaderHolder::cArchive_Common, "depth_2d"},
    {ShaderHolder::cArchive_Common, "depth_2d_array_quad"},
    {ShaderHolder::cArchive_Common, "depth_2d_min"},
    {ShaderHolder::cArchive_Common, "depth_2d_array_min"},
    {ShaderHolder::cArchive_Common, "depth_2d_max"},
    {ShaderHolder::cArchive_Common, "depth_2d_array_max"},
    {ShaderHolder::cArchive_Common, "color_2d_array_quad"},
    {ShaderHolder::cArchive_Common, "color_2d_min"},
    {ShaderHolder::cArchive_Common, "color_2d_array_min"},
    {ShaderHolder::cArchive_Common, "color_2d_max"},
    {ShaderHolder::cArchive_Common, "color_2d_array_max"},
    {ShaderHolder::cArchive_Common, "uint"},
    {ShaderHolder::cArchive_Common, "uint_array"},
    {ShaderHolder::cArchive_Common, "depth_mask"},
    {ShaderHolder::cArchive_Common, "luminance"},
    {ShaderHolder::cArchive_Common, "texture_2d_array"},
    {ShaderHolder::cArchive_Common, "texture_3d"},
    {ShaderHolder::cArchive_Common, "texture_cube_map"},
    {ShaderHolder::cArchive_Common, "texture_cube_map_array"},
    {ShaderHolder::cArchive_Common, "texture_clrmtx_2d"},
    {ShaderHolder::cArchive_Common, "texture_clrmtx_2d_array"},
    {ShaderHolder::cArchive_Common, "texture_texcoord"},
    {ShaderHolder::cArchive_Common, "texture_texcoord_mult_color"},
    {ShaderHolder::cArchive_Common, "texture_multi_sample_1x"},
    {ShaderHolder::cArchive_Common, "texture_multi_sample_2x"},
    {ShaderHolder::cArchive_Common, "texture_multi_sample_4x"},
    {ShaderHolder::cArchive_Common, "texture_multi_sample_8x"},
    {ShaderHolder::cArchive_Common, "alpha_modify"},
    {ShaderHolder::cArchive_Common, "texture_color_drift"},
    {ShaderHolder::cArchive_Common, "color_quad"},
    {ShaderHolder::cArchive_Common, "clear_quad"},
    {ShaderHolder::cArchive_Common, "top_bottom_color"},
    {ShaderHolder::cArchive_Common, "texture_gamma"},
    {ShaderHolder::cArchive_Common, "clear"},
    {ShaderHolder::cArchive_Common, "draw_imm"},
    {ShaderHolder::cArchive_Common, "draw_fan"},
    {ShaderHolder::cArchive_Common, "draw_capsule"},
    {ShaderHolder::cArchive_Common, "frame_buffer_flip_y"},
    {ShaderHolder::cArchive_Common, "frame_buffer_no_flip"},
    {ShaderHolder::cArchive_TechniquePfx, "bloom_mask"},
    {ShaderHolder::cArchive_TechniquePfx, "bloom_gaussian"},
    {ShaderHolder::cArchive_TechniquePfx, "bloom_compose"},
    {ShaderHolder::cArchive_TechniquePfx, "bloom_reduce"},
    {ShaderHolder::cArchive_TechniquePfx, "hdr_compose"},
    {ShaderHolder::cArchive_TechniquePfx, "dof_near_mask"},
    {ShaderHolder::cArchive_TechniquePfx, "dof_mipmap"},
    {ShaderHolder::cArchive_TechniquePfx, "dof_depth_mask"},
    {ShaderHolder::cArchive_TechniquePfx, "dof_final"},
    {ShaderHolder::cArchive_TechniquePfx, "dof_vignetting"},
    {ShaderHolder::cArchive_TechniquePfx, "dof_expand_reduce"},
    {ShaderHolder::cArchive_TechniquePfx, "fxaa"},
    {ShaderHolder::cArchive_TechniquePfx, "fxaa_coloronly"},
    {ShaderHolder::cArchive_TechniquePfx, "fxaa_luma"},
    {ShaderHolder::cArchive_TechniquePfx, "fxaa_reprojection"},
    {ShaderHolder::cArchive_TechniquePfx, "filter_aa_reprojection"},
    {ShaderHolder::cArchive_TechniquePfx, "reduce_aa"},
    {ShaderHolder::cArchive_TechniquePfx, "smaa_line_detection"},
    {ShaderHolder::cArchive_TechniquePfx, "smaa_weight_calculation"},
    {ShaderHolder::cArchive_TechniquePfx, "smaa_compose"},
    {ShaderHolder::cArchive_TechniquePfx, "smaa_resolve"},
    {ShaderHolder::cArchive_TechniquePfx, "smaac_line_detection"},
    {ShaderHolder::cArchive_TechniquePfx, "smaac_weight_calculation"},
    {ShaderHolder::cArchive_TechniquePfx, "smaac_utility"},
    {ShaderHolder::cArchive_TechniqueLght, "lightmap"},
    {ShaderHolder::cArchive_TechniqueLght, "lightmap_clear"},
    {ShaderHolder::cArchive_TechniqueLght, "local_lightmap"},
    {ShaderHolder::cArchive_TechniqueLghtLpp, "light_pre_pass_point_light"},
    {ShaderHolder::cArchive_TechniqueLghtLpp, "light_pre_pass_spot_light"},
    {ShaderHolder::cArchive_TechniqueLghtLpp, "light_pre_pass_proj_light"},
    {ShaderHolder::cArchive_TechniqueLghtLpp, "light_pre_pass_dev"},
    {ShaderHolder::cArchive_Technique, "multi_filter_reduce"},
    {ShaderHolder::cArchive_Technique, "multi_filter_expand"},
    {ShaderHolder::cArchive_TechniqueShdw, "static_depth_shadow"},
    {ShaderHolder::cArchive_Common, "cubemap_gaussian"},
    {ShaderHolder::cArchive_Common, "cubemap_head_extract"},
    {ShaderHolder::cArchive_Common, "cubemap_head_convolute_first"},
    {ShaderHolder::cArchive_Common, "cubemap_head_convolute_other"},
    {ShaderHolder::cArchive_Common, "cubemap_hdr_encode"},
    {ShaderHolder::cArchive_Common, "cubemap_draw_illuminant"},
    {ShaderHolder::cArchive_Common, "nv12decode"},
    {ShaderHolder::cArchive_Technique, "screen_pick"},
    {ShaderHolder::cArchive_Common, "debug_cubemap"},
    {ShaderHolder::cArchive_TechniqueShdw, "depth_shadow_debug"},
    {ShaderHolder::cArchive_TechniqueShdw, "vsm"},
    {ShaderHolder::cArchive_TechniqueShdw, "raymarch_depth_shadow"},
    {ShaderHolder::cArchive_TechniqueShdw, "ssao_ao_buffer"},
    {ShaderHolder::cArchive_TechniqueShdw, "ssao_blur"},
    {ShaderHolder::cArchive_TechniqueShdw, "ssao_reduce"},
    {ShaderHolder::cArchive_TechniqueShdw, "ssao_mask"},
    {ShaderHolder::cArchive_TechniqueShdw, "alchemy_ao_buffer"},
    {ShaderHolder::cArchive_Technique, "occlusion_query"},
    {ShaderHolder::cArchive_Technique, "occlusion_renderer_clear_buf"},
    {ShaderHolder::cArchive_Technique, "occlusion_renderer"},
    {ShaderHolder::cArchive_Technique, "occluded_effect_lensflare"},
    {ShaderHolder::cArchive_Common, "texture_compress"},
    {ShaderHolder::cArchive_Common, "texture_compress_hq"},
    {ShaderHolder::cArchive_TechniqueShdw, "shadow_mask"},
    {ShaderHolder::cArchive_Technique, "radial_blur"},
    {ShaderHolder::cArchive_Technique, "radial_blur_compose"},
    {ShaderHolder::cArchive_TechniqueLght, "ssii"},
    {ShaderHolder::cArchive_TechniqueLght, "ssii_ref"},
    {ShaderHolder::cArchive_TechniqueLght, "ssii_reduce"},
    {ShaderHolder::cArchive_TechniqueLght, "ssii_reduce_g"},
    {ShaderHolder::cArchive_TechniqueLght, "ssii_pre_render"},
    {ShaderHolder::cArchive_TechniqueLght, "ssii_expand"},
    {ShaderHolder::cArchive_TechniqueLght, "ssii_anti_howling"},
    {ShaderHolder::cArchive_Technique, "normal_map"},
    {ShaderHolder::cArchive_TechniquePfx, "auto_exposure"},
    {ShaderHolder::cArchive_TechniqueLght, "sssss_blur"},
    {ShaderHolder::cArchive_TechniqueLght, "sssss_expand_sss"},
    {ShaderHolder::cArchive_TechniqueLght, "sssss_reduce"},
    {ShaderHolder::cArchive_TechniqueLght, "sssss_merge"},
    {ShaderHolder::cArchive_TechniqueLght, "sssss_merge_weight"},
    {ShaderHolder::cArchive_TechniquePfx, "glare_filter_seed"},
    {ShaderHolder::cArchive_TechniquePfx, "glare_filter_blur"},
    {ShaderHolder::cArchive_TechniquePfx, "glare_filter_depth"},
    {ShaderHolder::cArchive_TechniquePfx, "glare_filter_clear"},
    {ShaderHolder::cArchive_TechniquePfx, "flare_filter_flare"},
    {ShaderHolder::cArchive_TechniquePfx, "flare_filter_copy"},
    {ShaderHolder::cArchive_TechniqueLght, "planar_reflection"},
    {ShaderHolder::cArchive_TechniqueShdw, "shadow_pre_pass"},
    {ShaderHolder::cArchive_TechniqueShdw, "screen_space_self_shadow"},
    {ShaderHolder::cArchive_TechniqueShdw, "screen_space_self_shadow_create_hiz"},
    {ShaderHolder::cArchive_Technique, "decal_simple"},
    {ShaderHolder::cArchive_Technique, "decal_trail"},
    {ShaderHolder::cArchive_Technique, "decal_texture_drawer"},
    {ShaderHolder::cArchive_Technique, "normal_drawer_post"},
    {ShaderHolder::cArchive_Common, "debug_primitive"},
    {ShaderHolder::cArchive_Common, "debug_shape_instanced"},
    {ShaderHolder::cArchive_Common, "debug_point_instanced"},
    {ShaderHolder::cArchive_Common, "debug_line_instanced"},
    {ShaderHolder::cArchive_Common, "debug_triangle_instanced"},
    {ShaderHolder::cArchive_Common, "cubemap_irradiance"},
    {ShaderHolder::cArchive_Common, "cubemap_sh"},
    {ShaderHolder::cArchive_Common, "cubemap_sh_point"},
    {ShaderHolder::cArchive_Common, "cubemap_sh_occlusion"},
    {ShaderHolder::cArchive_TechniqueLght, "local_reflection_mask"},
    {ShaderHolder::cArchive_TechniqueLght, "local_reflection_filter"},
    {ShaderHolder::cArchive_TechniqueLght, "local_reflection_gaussian_filter"},
    {ShaderHolder::cArchive_TechniqueLght, "local_reflection_depth_filter"},
    {ShaderHolder::cArchive_TechniqueLght, "local_reflection_specular"},
    {ShaderHolder::cArchive_TechniqueLght, "local_reflection_specular_vec4"},
    {ShaderHolder::cArchive_TechniqueLght, "local_reflection_jittered_copy"},
    {ShaderHolder::cArchive_TechniqueLght, "local_reflection_debug"},
    {ShaderHolder::cArchive_Common, "debug_sh_volume_vtx"},
    {ShaderHolder::cArchive_Common, "debug_sh_volume_frag"},
    {ShaderHolder::cArchive_Common, "debug_sh_volume_hemi_light"},
    {ShaderHolder::cArchive_TechniqueLght, "sh_volume_lighting"},
    {ShaderHolder::cArchive_TechniqueLght, "sh_volume_cubemap_lighting"},
    {ShaderHolder::cArchive_TechniqueLght, "sh_volume_clear_all_slices"},
    {ShaderHolder::cArchive_TechniqueLght, "sh_volume_clear_single_slice"},
    {ShaderHolder::cArchive_TechniqueLght, "sh_volume_point"},
    {ShaderHolder::cArchive_TechniqueLght, "sh_volume_point_mrt"},
    {ShaderHolder::cArchive_TechniqueLght, "sh_volume_rect_mrt"},
    {ShaderHolder::cArchive_TechniqueLght, "sh_volume_copy_mrt"},
    {ShaderHolder::cArchive_TechniqueLght, "sh_volume_copy_mrt_single_slice"},
    {ShaderHolder::cArchive_TechniqueLght, "sh_volume_diff_update_mrt"},
    {ShaderHolder::cArchive_TechniqueLght, "sh_volume_copy_cubemap_mrt"},
    {ShaderHolder::cArchive_TechniqueLght, "sh_volume_cubemap_face_filter"},
    {ShaderHolder::cArchive_TechniquePfx, "vdm"},
    {ShaderHolder::cArchive_Technique, "cloud"},
    {ShaderHolder::cArchive_Technique, "volume_mask_reducedepth"},
    {ShaderHolder::cArchive_Technique, "volume_mask_layer"},
    {ShaderHolder::cArchive_Technique, "volume_mask_raymarch"},
    {ShaderHolder::cArchive_Technique, "volume_mask_drawtex"},
    {ShaderHolder::cArchive_Technique, "volume_mask_debug"},
    {ShaderHolder::cArchive_TechniquePfx, "sky_transmittance"},
    {ShaderHolder::cArchive_TechniquePfx, "sky_irradiance"},
    {ShaderHolder::cArchive_TechniquePfx, "sky_inscatter"},
    {ShaderHolder::cArchive_TechniquePfx, "sky_delta_inscatter"},
    {ShaderHolder::cArchive_TechniquePfx, "sky_copy_irradiance"},
    {ShaderHolder::cArchive_TechniquePfx, "sky_copy_inscatter"},
    {ShaderHolder::cArchive_TechniquePfx, "sky_bake_inscatter"},
    {ShaderHolder::cArchive_TechniquePfx, "sky_bake_irradiance"},
    {ShaderHolder::cArchive_TechniquePfx, "sky_bake_range_transmittance"},
    {ShaderHolder::cArchive_TechniquePfx, "sky_postfx_sky"},
    {ShaderHolder::cArchive_TechniquePfx, "sky_postfx_ground"},
    {ShaderHolder::cArchive_Technique, "star_render"},
};

}  // namespace

/**
 * Constructs an empty shader holder.
 */
ShaderHolder::ShaderHolder() {}

/**
 * Destroys the shader holder.
 */
ShaderHolder::~ShaderHolder() = default;

/**
 * Loads the built-in shader archives and binds the uniform and sampler names of every program.
 * @param pArchive archive containing the shader archives
 * @param pHeap parent heap of the temporary work heap
 */
void ShaderHolder::initialize(sead::ArchiveRes* pArchive, sead::Heap* pHeap) {
    static const char* const cArchiveName[cArchive_Num] = {
        "agl_common",         "agl_technique",          "agl_technique_pfx",
        "agl_technique_lght", "agl_technique_lght_lpp", "agl_technique_shdw",
    };

    sead::Heap* heap = sead::ExpHeap::create(pHeap->getMaxAllocatableSize(8),
                                             "agl::detail::ShaderHolder", pHeap, 8,
                                             sead::Heap::cHeapDirection_Forward, false);
    const u32 option = !mNoOption;

    for (s32 i = 0; i < cArchive_Num; i++) {
        ResShaderArchive archive(pArchive->getFile(sead::FormatFixedSafeString<1024>(
            "%s.%s", cArchiveName[i], ResShaderArchiveData::getExtension())));
        ResBinaryShaderArchive binaryArchive(pArchive->getFile(sead::FormatFixedSafeString<1024>(
            "%s.%s", cArchiveName[i], ResBinaryShaderArchiveData::getExtension())));
        if (archive.isValid() || binaryArchive.isValid()) {
            mArchives[i].createWithOption(binaryArchive, archive, option, heap);
        }
    }

    for (s32 i = 0; i < cProgram_Num; i++) {
        ShaderProgramArchive& rArchive = mArchives[cProgramInfo[i].mArchive];
        if (!rArchive.isValid()) {
            mShaderPrograms.pushBack(nullptr);
            continue;
        }

        s32 index = rArchive.searchShaderProgramIndex(cProgramInfo[i].mName);
        ShaderProgram* pProgram = rArchive.getShaderProgramPtr(index);
        mShaderPrograms.pushBack(pProgram);

    switch (i) {
    case cSmaaLineDetection:
    case cSmaaWeightCalculation:
    case cSmaaCompose:
    case cSmaaResolve:
    case cSmaacLineDetection:
    case cSmaacWeightCalculation:
    case cSmaacUtility:
        pProgram->createImageLocation(3, heap);
        pProgram->setImageLocationName(0, "imOutputColor");
        pProgram->setImageLocationName(1, "imEdgeCounter");
        pProgram->setImageLocationName(2, "imEdgePosStorage");
        pProgram->createShaderStorageBlock(1, heap);
        pProgram->setShaderStorageBlockName(0, "outputSSBO");
        pProgram->createUniformBlock(1, heap);
        pProgram->setUniformBlockName(0, "Common");
        pProgram->createUniform(6, heap);
        pProgram->setUniformName(0, "cScreenParam");
        pProgram->setUniformName(1, "cSMAAEdgeParam");
        pProgram->setUniformName(2, "cSMAAWeightCalcParam");
        pProgram->setUniformName(3, "cSMAASubsampleIndices");
        pProgram->setUniformName(4, "cVelocityMapParam");
        pProgram->setUniformName(5, "cSMAAComposeParam");
        pProgram->createSamplerLocation(8, heap);
        pProgram->setSamplerLocationName(0, "cTextureCurColor");
        pProgram->setSamplerLocationName(1, "cTexturePrevColor");
        pProgram->setSamplerLocationName(2, "cTextureVelocityMap");
        pProgram->setSamplerLocationName(3, "cTextureLuminanceMap");
        pProgram->setSamplerLocationName(3, "cTextureLuminanceMap");
        pProgram->setSamplerLocationName(4, "cTextureEdgeMap");
        pProgram->setSamplerLocationName(5, "cTextureBlendWeightMap");
        pProgram->setSamplerLocationName(6, "cTextureSMAAArea");
        pProgram->setSamplerLocationName(7, "cTextureSMAASearch");
        break;
    case cDofNearMask:
    case cDofMipmap:
    case cDofDepthMask:
    case cDofFinal:
    case cDofVignetting:
    case cDofExpandReduce:
        pProgram->createUniform(13, heap);
        pProgram->setUniformName(0, "cParam0");
        pProgram->setUniformName(1, "cIndirectTexParam");
        pProgram->setUniformName(2, "cIndirectTexMtx0");
        pProgram->setUniformName(3, "cIndirectTexMtx1");
        pProgram->setUniformName(4, "cMulParam");
        pProgram->setUniformName(5, "cAddParam");
        pProgram->setUniformName(6, "cNearFarParam");
        pProgram->setUniformName(7, "cTexParam");
        pProgram->setUniformName(8, "cVignettingRadius");
        pProgram->setUniformName(9, "cVignettingParam");
        pProgram->setUniformName(10, "cVignettingTrans");
        pProgram->setUniformName(11, "cVignettingColor");
        pProgram->setUniformName(12, "cFarMulColor");
        pProgram->createSamplerLocation(5, heap);
        pProgram->setSamplerLocationName(0, "cTexColor");
        pProgram->setSamplerLocationName(1, "cTexDepth");
        pProgram->setSamplerLocationName(2, "cTexMipMap");
        pProgram->setSamplerLocationName(3, "cTexMipMapDepth");
        pProgram->setSamplerLocationName(4, "cTexIndirect");
        break;
    case cFxaa:
    case cFxaaColoronly:
    case cFxaaLuma:
    case cFxaaReprojection:
    case cFilterAaReprojection:
        pProgram->createUniform(11, heap);
        pProgram->setUniformName(0, "cFrameSize");
        pProgram->setUniformName(1, "cFrameRCP1H");
        pProgram->setUniformName(2, "cFrameRCPOffset");
        pProgram->setUniformName(3, "cLumaCoeff");
        pProgram->setUniformName(4, "cEdgeThreshold");
        pProgram->setUniformName(5, "cQualityParam");
        pProgram->setUniformName(6, "cAlphaOut");
        pProgram->setUniformName(7, "cFinalPowerGL");
        pProgram->setUniformName(8, "cPrevScreenCoordTrx[0]");
        pProgram->setUniformName(9, "cNearFar");
        pProgram->setUniformName(10, "cNearFarCache");
        pProgram->createSamplerLocation(5, heap);
        pProgram->setSamplerLocationName(0, "cTexBase");
        pProgram->setSamplerLocationName(1, "cTexLuma");
        pProgram->setSamplerLocationName(2, "cTexReduce");
        pProgram->setSamplerLocationName(3, "cTexDepth");
        pProgram->setSamplerLocationName(4, "cTexPrev");
        break;
    case cVolumeMaskReducedepth:
    case cVolumeMaskLayer:
    case cVolumeMaskRaymarch:
    case cVolumeMaskDrawtex:
    case cVolumeMaskDebug:
        pProgram->createUniform(12, heap);
        pProgram->setUniformName(0, "cArrayIndex");
        pProgram->setUniformName(9, "cSizeRCP");
        pProgram->setUniformName(1, "cViewInvMtx[0]");
        pProgram->setUniformName(2, "cLayerTrans");
        pProgram->setUniformName(3, "cLayerRenderInfo");
        pProgram->setUniformName(4, "cLayerNum");
        pProgram->setUniformName(5, "cHeightAttenuation");
        pProgram->setUniformName(6, "cDepthShadowProj[0]");
        pProgram->setUniformName(7, "cDepthShadowLength[0]");
        pProgram->setUniformName(8, "cDepthShadowAmplifier");
        pProgram->setUniformName(10, "cViewProjMtx[0]");
        pProgram->setUniformName(11, "cColor");
        pProgram->createSamplerLocation(4, heap);
        pProgram->setSamplerLocationName(0, "cDepthTex");
        pProgram->setSamplerLocationName(1, "cDepthShadowTex");
        pProgram->setSamplerLocationName(2, "cMaskTex");
        pProgram->setSamplerLocationName(3, "cVolumeTex");
        break;
    case cBloomMask:
    case cBloomGaussian:
    case cBloomCompose:
    case cBloomReduce:
        pProgram->createUniform(10, heap);
        pProgram->setUniformName(0, "cLuminanceWeight");
        pProgram->setUniformName(1, "cThresholdParam");
        pProgram->setUniformName(5, "cComposeColor");
        pProgram->setUniformName(2, "cDepthParam0");
        pProgram->setUniformName(3, "cDepthParam1");
        pProgram->setUniformName(4, "cNearFarTexSize");
        pProgram->setUniformName(6, "cGaussianParam");
        pProgram->setUniformName(7, "cLuminanceParam");
        pProgram->setUniformName(8, "cParam6");
        pProgram->setUniformName(9, "cParam7");
        pProgram->createSamplerLocation(3, heap);
        pProgram->setSamplerLocationName(0, "cTextureColor");
        pProgram->setSamplerLocationName(1, "cTextureDepth");
        pProgram->setSamplerLocationName(2, "cTextureLuminanceOffset");
        break;
    case cSsaoAoBuffer:
    case cSsaoBlur:
    case cSsaoReduce:
    case cSsaoMask:
        pProgram->createUniform(7, heap);
        pProgram->setUniformName(0, "cAOParam0");
        pProgram->setUniformName(1, "cAOParam1");
        pProgram->setUniformName(2, "cAOParam2");
        pProgram->setUniformName(3, "cNearFar");
        pProgram->setUniformName(4, "cInvTexSize");
        pProgram->setUniformName(5, "cSamplePoint[0]");
        pProgram->setUniformName(6, "cRotate[0]");
        pProgram->createSamplerLocation(4, heap);
        pProgram->setSamplerLocationName(0, "cTexture");
        pProgram->setSamplerLocationName(1, "cTextureRotate");
        pProgram->setSamplerLocationName(2, "cTextureDepth");
        pProgram->setSamplerLocationName(3, "cTextureMask");
        pProgram->createUniformBlock(1, heap);
        pProgram->setUniformBlockName(0, "cAO_UB");
        break;
    case cDebugPointInstanced:
    case cDebugLineInstanced:
    case cDebugTriangleInstanced:
        pProgram->createUniformBlock(5, heap);
        pProgram->setUniformBlockName(0, "View");
        pProgram->setUniformBlockName(3, "PointMask");
        pProgram->setUniformBlockName(4, "PointInfo");
        break;
    case cLightPrePassPointLight:
    case cLightPrePassSpotLight:
    case cLightPrePassProjLight:
        pProgram->createUniformBlock(4, heap);
        pProgram->setUniformBlockName(0, "Context");
        pProgram->setUniformBlockName(1, "PointLightView");
        pProgram->setUniformBlockName(2, "SpotLightView");
        pProgram->setUniformBlockName(3, "ProjLightView");
        pProgram->createSamplerLocation(6, heap);
        pProgram->setSamplerLocationName(0, "cDepth");
        pProgram->setSamplerLocationName(1, "cNormal");
        pProgram->setSamplerLocationName(2, "cSpecPowTable");
        pProgram->setSamplerLocationName(3, "cProjTex");
        pProgram->setSamplerLocationName(4, "cDepthShadow");
        pProgram->setSamplerLocationName(5, "cAlbedo");
        break;
    case cCubemapSh:
    case cCubemapShPoint:
    case cCubemapShOcclusion:
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cColorCube");
        pProgram->setSamplerLocationName(1, "cLinearDepthCube");
        pProgram->createUniform(7, heap);
        pProgram->setUniformName(0, "cAngleParam");
        pProgram->setUniformName(1, "cMipLevel");
        pProgram->setUniformName(2, "cCalcSHParam");
        pProgram->setUniformName(3, "cDistanceParam");
        pProgram->setUniformName(4, "cPointSampleWorldNormal");
        pProgram->setUniformName(5, "cNearFarParam");
        pProgram->setUniformName(6, "cOcclusionParam");
        break;
    case cLocalReflectionSpecular:
    case cLocalReflectionSpecularVec4:
    case cLocalReflectionDebug:
        pProgram->createUniformBlock(3, heap);
        pProgram->setUniformBlockName(0, "ViewContext");
        pProgram->setUniformBlockName(1, "ReflectionContext");
        pProgram->setUniformBlockName(2, "FilterContext");
        pProgram->createSamplerLocation(9, heap);
        pProgram->setSamplerLocationName(0, "cViewNormalTexture");
        pProgram->setSamplerLocationName(1, "cLinearDepthTexture");
        pProgram->setSamplerLocationName(3, "cColorTexture");
        pProgram->setSamplerLocationName(4, "cReflectionCacheTexture");
        pProgram->setSamplerLocationName(6, "cJitteredTexture");
        pProgram->setSamplerLocationName(2, "cRoughnessTexture");
        pProgram->setSamplerLocationName(7, "cReflectionHistoryTexture");
        break;
    case cDebugShVolumeVtx:
    case cDebugShVolumeFrag:
    case cDebugShVolumeHemiLight:
        pProgram->createUniformBlock(1, heap);
        pProgram->setUniformBlockName(0, "DebugContext");
        pProgram->createSamplerLocation(4, heap);
        pProgram->setSamplerLocationName(0, "cLightTexture0");
        pProgram->setSamplerLocationName(1, "cLightTexture1");
        pProgram->setSamplerLocationName(2, "cLightTexture2");
        pProgram->setSamplerLocationName(3, "cLightTexture3");
        break;
    case cTextureCompress:
    case cTextureCompressHq:
        pProgram->createUniform(4, heap);
        pProgram->setUniformName(0, "cTexSizeRCP");
        pProgram->setUniformName(1, "cReferLevel");
        pProgram->setUniformName(2, "cReferSlice");
        pProgram->setUniformName(3, "cReferFace");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cTexture");
        break;
    case cRadialBlur:
    case cRadialBlurCompose:
        pProgram->createUniform(3, heap);
        pProgram->setUniformName(0, "cRadius");
        pProgram->setUniformName(1, "cBlurPosition");
        pProgram->setUniformName(2, "cVtxColor[0]");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cTexture");
        break;
    case cSsiiReduce:
    case cSsiiReduceG:
        pProgram->createUniform(1, heap);
        pProgram->setUniformName(0, "cStep");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cSrc");
        break;
    case cShVolumeLighting:
    case cShVolumeCubemapLighting:
        pProgram->createUniformBlock(2, heap);
        pProgram->setUniformBlockName(0, "VoxelContext");
        pProgram->setUniformBlockName(1, "ProbeContext");
        pProgram->createSamplerLocation(7, heap);
        pProgram->setSamplerLocationName(0, "cViewNormalTexture");
        pProgram->setSamplerLocationName(1, "cLinearDepthTexture");
        pProgram->setSamplerLocationName(2, "cLightTexture0");
        pProgram->setSamplerLocationName(3, "cLightTexture1");
        pProgram->setSamplerLocationName(4, "cLightTexture2");
        pProgram->setSamplerLocationName(5, "cLightTexture3");
        pProgram->setSamplerLocationName(6, "cNormalCubeMapTexture");
        break;
    case cShVolumeClearAllSlices:
    case cShVolumeClearSingleSlice:
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cSHTexture");
        break;
    case cColorCorrection:
    case cColorCorrectionMap:
        pProgram->createUniform(13, heap);
        pProgram->setUniformName(0, "c3DTexCoordOffset");
        pProgram->setUniformName(1, "cHSBG");
        pProgram->setUniformName(2, "cCurveA[0]");
        pProgram->setUniformName(3, "cCurveB[0]");
        pProgram->setUniformName(4, "cToyCam_Offset1");
        pProgram->setUniformName(5, "cToyCam_Offset2");
        pProgram->setUniformName(6, "cToyCam_Level1");
        pProgram->setUniformName(7, "cToyCam_Level2");
        pProgram->setUniformName(8, "cToyCam_Saturation1");
        pProgram->setUniformName(9, "cToyCam_Saturation2");
        pProgram->setUniformName(10, "cToyCam_Brightness");
        pProgram->setUniformName(11, "cToyCam_Contrast");
        pProgram->setUniformName(12, "cToyCam_MulColor");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cTexColor");
        pProgram->setSamplerLocationName(1, "cTexCorrectionMap");
        break;
    case cFrameBufferFlipY:
    case cFrameBufferNoFlip:
        pProgram->createAttribute(1, heap);
        pProgram->setAttributeName(0, "aPosition");
        pProgram->createUniform(26, heap);
        pProgram->setUniformName(0, "uProjViewWorld[0]");
        pProgram->setUniformName(2, "uMipLevel");
        pProgram->createSamplerLocation(9, heap);
        pProgram->setSamplerLocationName(0, "uTextureColor_0");
        pProgram->setSamplerLocationName(1, "uTextureColor_1");
        pProgram->setSamplerLocationName(2, "uTextureColor_2");
        pProgram->setSamplerLocationName(3, "uTextureColor_3");
        pProgram->setSamplerLocationName(4, "uTextureColor_4");
        pProgram->setSamplerLocationName(5, "uTextureColor_5");
        pProgram->setSamplerLocationName(6, "uTextureColor_6");
        pProgram->setSamplerLocationName(7, "uTextureColor_7");
        pProgram->setSamplerLocationName(8, "uTextureDepth");
        break;
    case cLightmap:
    case cLightmapClear:
        pProgram->createUniformBlock(3, heap);
        pProgram->setUniformBlockName(0, "Light");
        pProgram->setUniformBlockName(1, "LightView");
        pProgram->setUniformBlockName(2, "LightMip");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cLUT");
        pProgram->setSamplerLocationName(1, "cNormal");
        break;
    case cMultiFilterReduce:
    case cMultiFilterExpand:
        pProgram->createAttribute(1, heap);
        pProgram->setAttributeName(0, "aPosition");
        pProgram->createUniform(4, heap);
        pProgram->setUniformName(0, "cTexOffsetScale");
        pProgram->setUniformName(1, "cTexWidthInv");
        pProgram->setUniformName(2, "cTexOffsetAdjust");
        pProgram->setUniformName(3, "c3DTexCoordOffset");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cTexture");
        break;
    case cScreenSpaceSelfShadow:
    case cScreenSpaceSelfShadowCreateHiz:
        pProgram->createUniform(7, heap);
        pProgram->setUniformName(0, "cW[0]");
        pProgram->setUniformName(1, "cBP[0]");
        pProgram->setUniformName(2, "cRayArray[0]");
        pProgram->setUniformName(3, "cRayParamS");
        pProgram->setUniformName(4, "cRayParamF");
        pProgram->setUniformName(5, "cParamCamera");
        pProgram->setUniformName(6, "cFrustumCorner");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cSamplerDepth");
        pProgram->setSamplerLocationName(1, "cSamplerMask");
        break;
    case cLocalReflectionFilter:
    case cLocalReflectionGaussianFilter:
        pProgram->createUniformBlock(1, heap);
        pProgram->setUniformBlockName(0, "FilterContext");
        pProgram->createSamplerLocation(9, heap);
        pProgram->setSamplerLocationName(0, "cViewNormalTexture");
        pProgram->setSamplerLocationName(1, "cLinearDepthTexture");
        pProgram->setSamplerLocationName(5, "cFilterInputTexture");
        break;
    case cDevUtil:
        pProgram->createUniform(6, heap);
        pProgram->setUniformName(1, "cWorldViewMatrix[0]");
        pProgram->setUniformName(0, "cWorldViewProjectionMatrix[0]");
        pProgram->setUniformName(2, "cDiffuseColor0");
        pProgram->setUniformName(3, "cDiffuseColor1");
        pProgram->setUniformName(4, "cSpecularColor");
        pProgram->setUniformName(5, "cViewLightDir");
        break;
    case cDepthVisualize:
        pProgram->createUniform(5, heap);
        pProgram->setUniformName(0, "cNear");
        pProgram->setUniformName(1, "cFar");
        pProgram->setUniformName(2, "cDepth[0]");
        pProgram->setUniformName(3, "cColor[0]");
        pProgram->setUniformName(4, "cLineNum");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cTexDepth");
        break;
    case cXluSnap:
        pProgram->createUniform(27, heap);
        pProgram->setUniformName(0, "uProjViewWorld[0]");
        pProgram->setUniformName(1, "cTexSize");
        pProgram->setUniformName(4, "uTexCoordScale");
        pProgram->setUniformName(2, "uMipLevel");
        pProgram->setUniformName(26, "uAlpha");
        pProgram->createSamplerLocation(6, heap);
        pProgram->setSamplerLocationName(0, "uTexture");
        pProgram->setSamplerLocationName(2, "uSnapColor");
        pProgram->setSamplerLocationName(3, "uSnapDepth");
        pProgram->setSamplerLocationName(5, "uTargetDepth");
        break;
    case cClear:
        pProgram->createUniform(2, heap);
        pProgram->setUniformName(0, "cPositions[0]");
        pProgram->setUniformName(1, "cClearColor");
        break;
    case cDrawImm:
        pProgram->createUniform(5, heap);
        pProgram->setUniformName(0, "cWorld[0]");
        pProgram->setUniformName(1, "cView[0]");
        pProgram->setUniformName(2, "cProj[0]");
        pProgram->setUniformName(3, "cColor");
        pProgram->setUniformName(4, "cPosition[0]");
        break;
    case cDrawFan:
        pProgram->createUniform(3, heap);
        pProgram->setUniformName(0, "cProjViewWorld[0]");
        pProgram->setUniformName(1, "cArcInfo");
        pProgram->setUniformName(2, "cColor");
        break;
    case cDrawCapsule:
        pProgram->createUniform(4, heap);
        pProgram->setUniformName(0, "cPVW_Cylinder");
        pProgram->setUniformName(1, "cPVW_Hemisphere");
        pProgram->setUniformName(2, "cColor");
        pProgram->setUniformName(3, "cHemisphereOffsetLocal");
        break;
    case cHdrCompose:
        pProgram->createUniform(5, heap);
        pProgram->setUniformName(1, "cParam");
        pProgram->setUniformName(2, "cTexCoordCoeff0");
        pProgram->setUniformName(3, "cTexCoordCoeff1");
        pProgram->setUniformName(4, "cColorCorrectionCoeff");
        pProgram->createSamplerLocation(5, heap);
        pProgram->setSamplerLocationName(0, "cExposureTexture");
        pProgram->setSamplerLocationName(1, "cHDRImage");
        pProgram->setSamplerLocationName(2, "cBloom");
        pProgram->setSamplerLocationName(3, "cColorCorrectionTable");
        pProgram->setSamplerLocationName(4, "cNormalizedLinearDepth");
        break;
    case cReduceAa:
        pProgram->createUniform(1, heap);
        pProgram->setUniformName(0, "cFrameRCP");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cTexNormal");
        pProgram->setSamplerLocationName(1, "cTexReduce");
        break;
    case cLocalLightmap:
        pProgram->createUniformBlock(2, heap);
        pProgram->setUniformBlockName(0, "BlendInfo");
        pProgram->setUniformBlockName(1, "ProbeSH");
        pProgram->createSamplerLocation(3, heap);
        pProgram->setSamplerLocationName(0, "cNormal");
        pProgram->setSamplerLocationName(1, "cLightMap0");
        pProgram->setSamplerLocationName(2, "cLightMap1");
        break;
    case cLightPrePassDev:
        pProgram->createUniform(3, heap);
        pProgram->setUniformName(0, "cWVP[0]");
        pProgram->setUniformName(1, "cColor");
        pProgram->setUniformName(2, "cSpotCone_CenterScale");
        break;
    case cStaticDepthShadow:
        pProgram->createUniform(1, heap);
        pProgram->setUniformName(0, "uParam0");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "uTexture");
        break;
    case cCubemapGaussian:
        pProgram->createUniform(7, heap);
        pProgram->setUniformName(0, "cTexFlipY");
        pProgram->setUniformName(1, "cCubeMapFace");
        pProgram->setUniformName(2, "cCubeMapArrayIndex");
        pProgram->setUniformName(3, "cMipLevel");
        pProgram->setUniformName(4, "cTextureWidth");
        pProgram->setUniformName(5, "cAngleScale");
        pProgram->setUniformName(6, "cOffsetScale");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "sTexCube");
        break;
    case cCubemapHeadExtract:
        pProgram->createUniform(5, heap);
        pProgram->setUniformName(0, "cTexFlipY");
        pProgram->setUniformName(2, "cCubeMapFace");
        pProgram->setUniformName(3, "cCubeMapArrayIndex");
        pProgram->setUniformName(1, "cMipLevel");
        pProgram->setUniformName(4, "cTextureWidth");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "sTexCube");
        pProgram->setSamplerLocationName(1, "sTexCorrectValue");
        break;
    case cCubemapHeadConvoluteFirst:
        pProgram->createUniform(2, heap);
        pProgram->setUniformName(0, "cTexFlipY");
        pProgram->setUniformName(1, "cTextureWidth");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "sTexHeadArray");
        break;
    case cCubemapHeadConvoluteOther:
        pProgram->createUniform(2, heap);
        pProgram->setUniformName(0, "cTexFlipY");
        pProgram->setUniformName(1, "cTextureWidth");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "sTexHead");
        break;
    case cCubemapHdrEncode:
        pProgram->createUniform(7, heap);
        pProgram->setUniformName(0, "cTexFlipY");
        pProgram->setUniformName(1, "cMipLevel");
        pProgram->setUniformName(2, "cCubeMapFace");
        pProgram->setUniformName(3, "cCubeMapArrayIndex");
        pProgram->setUniformName(4, "cTextureWidth");
        pProgram->setUniformName(5, "cDynamicRange");
        pProgram->setUniformName(6, "cHDRDecodePower");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "sTexCube");
        pProgram->setSamplerLocationName(1, "sTexCorrectValue");
        break;
    case cCubemapDrawIlluminant:
        pProgram->createUniform(8, heap);
        pProgram->setUniformName(0, "cProjMtx[0]");
        pProgram->setUniformName(1, "cViewMtx[0]");
        pProgram->setUniformName(2, "cCenterPos");
        pProgram->setUniformName(3, "cAspect");
        pProgram->setUniformName(4, "cScale");
        pProgram->setUniformName(5, "cColor");
        pProgram->setUniformName(6, "cIntensity");
        pProgram->setUniformName(7, "cTexFlipY");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "sTexture");
        break;
    case cScreenPick:
        pProgram->createAttribute(4, heap);
        pProgram->setAttributeName(0, "aPosition");
        pProgram->setAttributeName(1, "aBlendWeight");
        pProgram->setAttributeName(2, "aBlendIndex");
        pProgram->setAttributeName(3, "aTexCoord0");
        pProgram->createUniformBlock(4, heap);
        pProgram->setUniformBlockName(0, "View");
        pProgram->setUniformBlockName(1, "Skeleton");
        pProgram->setUniformBlockName(2, "Material");
        pProgram->setUniformBlockName(3, "Shape");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cAlbedo");
        break;
    case cDebugCubemap:
        pProgram->createUniform(10, heap);
        pProgram->setUniformName(0, "cProjViewMtx[0]");
        pProgram->setUniformName(1, "cWorldMtx[0]");
        pProgram->setUniformName(2, "cCamWorld");
        pProgram->setUniformName(3, "cUseReflection");
        pProgram->setUniformName(4, "cUseMipLevel");
        pProgram->setUniformName(5, "cDynamicRange");
        pProgram->setUniformName(6, "cHDRDecodePower");
        pProgram->setUniformName(7, "cAlphaAdd");
        pProgram->setUniformName(8, "cMipLevel");
        pProgram->setUniformName(9, "cCubeMapArrayIndex");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "sTexCube");
        pProgram->setSamplerLocationName(1, "sTexCorrectValue");
        break;
    case cDepthShadowDebug:
        pProgram->createUniform(5, heap);
        pProgram->setUniformName(0, "cPVW[0]");
        pProgram->setUniformName(1, "cNearFarParam");
        pProgram->setUniformName(2, "cInvTextureSize");
        pProgram->setUniformName(3, "cTextureIndex");
        pProgram->setUniformName(4, "cTextureIndexY");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cTexture");
        break;
    case cVsm:
        pProgram->createUniform(2, heap);
        pProgram->setUniformName(0, "cInvTexSize");
        pProgram->setUniformName(1, "cSlice");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cTexture");
        break;
    case cRaymarchDepthShadow:
        pProgram->createUniformBlock(2, heap);
        pProgram->setUniformBlockName(0, "RaymarchContext");
        pProgram->setUniformBlockName(1, "ShadowContext");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cSamplerLinearDepth");
        pProgram->setSamplerLocationName(1, "cSamplerDepthShadow");
        break;
    case cAlchemyAoBuffer:
        pProgram->createUniform(6, heap);
        pProgram->setUniformName(0, "cCameraParam");
        pProgram->setUniformName(1, "cFrustumCorner");
        pProgram->setUniformName(2, "cFrustumCorner2");
        pProgram->setUniformName(3, "cTextureSize");
        pProgram->setUniformName(4, "cParam0");
        pProgram->setUniformName(5, "cParam1");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cSamplerLinearDepth");
        pProgram->setSamplerLocationName(1, "cSamplerNormal");
        break;
    case cOcclusionQuery:
        pProgram->createUniformBlock(4, heap);
        pProgram->setUniformBlockName(0, "View");
        pProgram->setUniformBlockName(1, "AABB");
        pProgram->setUniformBlockName(2, "Sphere");
        pProgram->setUniformBlockName(3, "Cylinder");
        break;
    case cOcclusionRenderer:
        pProgram->createUniformBlock(1, heap);
        pProgram->setUniformBlockName(0, "Context");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cDepth");
        break;
    case cOccludedEffectLensflare:
        pProgram->createUniformBlock(1, heap);
        pProgram->setUniformBlockName(0, "Element");
        pProgram->createSamplerLocation(3, heap);
        pProgram->setSamplerLocationName(0, "cOcclusion");
        pProgram->setSamplerLocationName(1, "cTexture");
        pProgram->setSamplerLocationName(2, "cTexture2");
        break;
    case cShadowMask:
        pProgram->createUniformBlock(3, heap);
        pProgram->setUniformBlockName(0, "Common");
        pProgram->setUniformBlockName(1, "SphereBlock");
        pProgram->setUniformBlockName(2, "CylinderBlock");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cLinearDepth");
        pProgram->setSamplerLocationName(1, "cTexColor");
        break;
    case cSsii:
        pProgram->createUniform(8, heap);
        pProgram->setUniformName(0, "cTanFovyHalf");
        pProgram->setUniformName(1, "cProjOffset");
        pProgram->setUniformName(2, "cSrcStep");
        pProgram->setUniformName(3, "cDstStep");
        pProgram->setUniformName(4, "cIntensity");
        pProgram->setUniformName(5, "cRange");
        pProgram->setUniformName(7, "cSteep");
        pProgram->setUniformName(6, "cNear");
        pProgram->createSamplerLocation(8, heap);
        pProgram->setSamplerLocationName(0, "cSrcLight");
        pProgram->setSamplerLocationName(1, "cSrcAlbedo");
        pProgram->setSamplerLocationName(2, "cSrcDepth");
        pProgram->setSamplerLocationName(3, "cSrcNormal");
        pProgram->setSamplerLocationName(4, "cSrcPos");
        pProgram->setSamplerLocationName(5, "cDstAlbedo");
        pProgram->setSamplerLocationName(6, "cDstDepth");
        pProgram->setSamplerLocationName(7, "cDstNormal");
        break;
    case cSsiiRef:
        pProgram->createUniform(10, heap);
        pProgram->setUniformName(0, "cTanFovyHalf");
        pProgram->setUniformName(1, "cProjOffset");
        pProgram->setUniformName(2, "cSrcStep");
        pProgram->setUniformName(3, "cDstStep");
        pProgram->setUniformName(4, "cIntensity");
        pProgram->setUniformName(5, "cRange");
        pProgram->setUniformName(7, "cSteep");
        pProgram->setUniformName(6, "cNear");
        pProgram->setUniformName(8, "cPow");
        pProgram->setUniformName(9, "cInflate");
        pProgram->createSamplerLocation(8, heap);
        pProgram->setSamplerLocationName(0, "cSrcLight");
        pProgram->setSamplerLocationName(1, "cSrcAlbedo");
        pProgram->setSamplerLocationName(2, "cSrcDepth");
        pProgram->setSamplerLocationName(3, "cSrcNormal");
        pProgram->setSamplerLocationName(4, "cSrcPos");
        pProgram->setSamplerLocationName(5, "cDstAlbedo");
        pProgram->setSamplerLocationName(6, "cDstDepth");
        pProgram->setSamplerLocationName(7, "cDstNormal");
        break;
    case cSsiiPreRender:
        pProgram->createUniform(4, heap);
        pProgram->setUniformName(0, "cTanFovyHalf");
        pProgram->setUniformName(1, "cProjOffset");
        pProgram->setUniformName(2, "cRange");
        pProgram->setUniformName(3, "cNear");
        pProgram->createSamplerLocation(3, heap);
        pProgram->setSamplerLocationName(0, "cLight");
        pProgram->setSamplerLocationName(1, "cAlbedo");
        pProgram->setSamplerLocationName(2, "cDepth");
        break;
    case cSsiiExpand:
        pProgram->createUniform(6, heap);
        pProgram->setUniformName(0, "cTanFovyHalf");
        pProgram->setUniformName(1, "cProjOffset");
        pProgram->setUniformName(2, "cSrcStep");
        pProgram->setUniformName(3, "cDstStep");
        pProgram->setUniformName(4, "cRange");
        pProgram->setUniformName(5, "cNear");
        pProgram->createSamplerLocation(7, heap);
        pProgram->setSamplerLocationName(0, "cSrcLight0");
        pProgram->setSamplerLocationName(1, "cSrcLight1");
        pProgram->setSamplerLocationName(2, "cSrcNormal");
        pProgram->setSamplerLocationName(3, "cSrcDepth");
        pProgram->setSamplerLocationName(4, "cDstAlbedo");
        pProgram->setSamplerLocationName(5, "cDstNormal");
        pProgram->setSamplerLocationName(6, "cDstDepth");
        break;
    case cSsiiAntiHowling:
        pProgram->createUniform(1, heap);
        pProgram->setUniformName(0, "cThreshold");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cSrc0");
        pProgram->setSamplerLocationName(1, "cSrc1");
        break;
    case cNormalMap:
        pProgram->createUniform(1, heap);
        pProgram->setUniformName(0, "cParam0");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cHeightTexture");
        break;
    case cAutoExposure:
        pProgram->createUniform(4, heap);
        pProgram->setUniformName(0, "cParam0");
        pProgram->setUniformName(1, "cParam1");
        pProgram->setUniformName(2, "cParam2");
        pProgram->setUniformName(3, "cParam3");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cTexture");
        pProgram->setSamplerLocationName(1, "cPrevTexture");
        break;
    case cSssssBlur:
        pProgram->createUniform(6, heap);
        pProgram->setUniformName(0, "cTanFovyHalf");
        pProgram->setUniformName(1, "cProjOffset");
        pProgram->setUniformName(2, "cBias");
        pProgram->setUniformName(3, "cStep");
        pProgram->setUniformName(4, "cDepthCorrection");
        pProgram->setUniformName(5, "cAlpha");
        pProgram->createSamplerLocation(3, heap);
        pProgram->setSamplerLocationName(0, "cSrcLight");
        pProgram->setSamplerLocationName(1, "cSrcDepth");
        pProgram->setSamplerLocationName(2, "cBiasTex");
        break;
    case cSssssExpandSss:
        pProgram->createUniform(2, heap);
        pProgram->setUniformName(0, "cWeight");
        pProgram->setUniformName(1, "cAlpha");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cSrc");
        break;
    case cSssssReduce:
        pProgram->createUniform(2, heap);
        pProgram->setUniformName(0, "cSrcStep");
        pProgram->setUniformName(1, "cAlpha");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cSrc0");
        pProgram->setSamplerLocationName(1, "cSrc1");
        break;
    case cSssssMerge:
        pProgram->createUniform(4, heap);
        pProgram->setUniformName(0, "cBaseDepth");
        pProgram->setUniformName(1, "cNear");
        pProgram->setUniformName(2, "cRange");
        pProgram->setUniformName(3, "cOneMinusNearDivFar");
        pProgram->createSamplerLocation(6, heap);
        pProgram->setSamplerLocationName(0, "cSss0");
        pProgram->setSamplerLocationName(1, "cSss1");
        pProgram->setSamplerLocationName(2, "cSss2");
        pProgram->setSamplerLocationName(3, "cNoSss");
        pProgram->setSamplerLocationName(4, "cDepth");
        pProgram->setSamplerLocationName(5, "cTrans");
        break;
    case cSssssMergeWeight:
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cSrc");
        pProgram->setSamplerLocationName(1, "cWeight");
        break;
    case cGlareFilterSeed:
        pProgram->createUniformBlock(1, heap);
        pProgram->setUniformBlockName(0, "Seed");
        break;
    case cGlareFilterBlur:
        pProgram->createUniform(3, heap);
        pProgram->setUniformName(0, "cSrcStep");
        pProgram->setUniformName(1, "cAlpha");
        pProgram->setUniformName(2, "cColor");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cSrc");
        break;
    case cGlareFilterDepth:
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cSrc");
        break;
    case cGlareFilterClear:
        pProgram->createUniform(1, heap);
        pProgram->setUniformName(0, "cColor");
        break;
    case cFlareFilterFlare:
        pProgram->createUniform(6, heap);
        pProgram->setUniformName(0, "cGhostDispersal");
        pProgram->setUniformName(1, "cGhostColor[0]");
        pProgram->setUniformName(2, "cHaloWidth");
        pProgram->setUniformName(3, "cHaloColor");
        pProgram->setUniformName(4, "cChromaDistortion");
        pProgram->setUniformName(5, "cColor");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cSrc");
        break;
    case cFlareFilterCopy:
        pProgram->createUniform(4, heap);
        pProgram->setUniformName(0, "cSrcStep");
        pProgram->setUniformName(1, "cAlpha");
        pProgram->setUniformName(2, "cThreshold");
        pProgram->setUniformName(3, "cColor");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cSrc");
        break;
    case cPlanarReflection:
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cColor");
        pProgram->createUniform(1, heap);
        pProgram->setUniformName(0, "cParam0");
        break;
    case cShadowPrePass:
        pProgram->createUniformBlock(1, heap);
        pProgram->setUniformBlockName(0, "Common");
        pProgram->createSamplerLocation(6, heap);
        pProgram->setSamplerLocationName(0, "cLinearDepth");
        pProgram->setSamplerLocationName(1, "cAoBuffer");
        pProgram->setSamplerLocationName(2, "texDynamicShadowCascade");
        pProgram->setSamplerLocationName(3, "texDynamicShadow");
        pProgram->setSamplerLocationName(4, "texStaticDepthShadow");
        pProgram->setSamplerLocationName(5, "texModifyOffset");
        break;
    case cDecalSimple:
        pProgram->createUniformBlock(1, heap);
        pProgram->setUniformBlockName(0, "Common");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cDepthTexture");
        pProgram->setSamplerLocationName(1, "cTexture");
        break;
    case cDecalTrail:
        pProgram->createUniformBlock(3, heap);
        pProgram->setUniformBlockName(0, "Common");
        pProgram->setUniformBlockName(1, "Trail");
        pProgram->setUniformBlockName(2, "Counter");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cDepthTexture");
        pProgram->setSamplerLocationName(1, "cTexture");
        break;
    case cDecalTextureDrawer:
        pProgram->createUniformBlock(1, heap);
        pProgram->setUniformBlockName(0, "Common");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cTexture");
        break;
    case cNormalDrawerPost:
        pProgram->createUniformBlock(1, heap);
        pProgram->setUniformBlockName(0, "Common");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cTexture");
        pProgram->setSamplerLocationName(1, "cLinearDepthTexture");
        break;
    case cDebugPrimitive:
        pProgram->createUniform(3, heap);
        pProgram->setUniformName(0, "cPVW[0]");
        pProgram->setUniformName(1, "cWorld[0]");
        pProgram->setUniformName(2, "cColor");
        break;
    case cDebugShapeInstanced:
        pProgram->createUniformBlock(5, heap);
        pProgram->setUniformBlockName(0, "View");
        pProgram->setUniformBlockName(1, "ShapeMask");
        pProgram->setUniformBlockName(2, "ShapeInfo");
        break;
    case cCubemapIrradiance:
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "sTexCube");
        pProgram->createUniform(6, heap);
        pProgram->setUniformName(0, "cTexFlipY");
        pProgram->setUniformName(1, "cCubeMapFace");
        pProgram->setUniformName(2, "cCubeMapArrayIndex");
        pProgram->setUniformName(3, "cAngleParam");
        pProgram->setUniformName(4, "cMipLevel");
        break;
    case cLocalReflectionMask:
        pProgram->createUniformBlock(3, heap);
        pProgram->setUniformBlockName(0, "ViewContext");
        pProgram->setUniformBlockName(2, "FilterContext");
        pProgram->setUniformBlockName(1, "ReflectionContext");
        pProgram->createSamplerLocation(9, heap);
        pProgram->setSamplerLocationName(0, "cViewNormalTexture");
        pProgram->setSamplerLocationName(1, "cLinearDepthTexture");
        pProgram->setSamplerLocationName(2, "cRoughnessTexture");
        break;
    case cLocalReflectionDepthFilter:
        pProgram->createUniformBlock(2, heap);
        pProgram->setUniformBlockName(0, "ViewContext");
        pProgram->setUniformBlockName(1, "FilterContext");
        pProgram->createSamplerLocation(9, heap);
        pProgram->setSamplerLocationName(0, "cViewNormalTexture");
        pProgram->setSamplerLocationName(1, "cLinearDepthTexture");
        pProgram->setSamplerLocationName(5, "cFilterInputTexture");
        break;
    case cLocalReflectionJitteredCopy:
        pProgram->createUniformBlock(1, heap);
        pProgram->setUniformBlockName(0, "FilterContext");
        pProgram->createSamplerLocation(9, heap);
        pProgram->setSamplerLocationName(5, "cFilterInputTexture");
        break;
    case cShVolumePoint:
        pProgram->createUniform(1, heap);
        pProgram->setUniformName(0, "cPointInfoArray");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cSHTexture");
        break;
    case cShVolumePointMrt:
        pProgram->createUniform(2, heap);
        pProgram->setUniformName(0, "cSHParam3");
        pProgram->setUniformName(1, "cPointInfoArray");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cSHTexture");
        break;
    case cShVolumeRectMrt:
        pProgram->createUniform(3, heap);
        pProgram->setUniformName(0, "cSrcSHParam3");
        pProgram->setUniformName(1, "cSliceParam");
        pProgram->setUniformName(2, "cRectInfoArray");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cSHTexture");
        pProgram->setSamplerLocationName(1, "cLightTexture3");
        break;
    case cShVolumeCopyMrt:
        pProgram->createUniform(1, heap);
        pProgram->setUniformName(0, "cCopyRectInfoArray");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cTexture3d_0");
        break;
    case cShVolumeCopyMrtSingleSlice:
        pProgram->createUniform(2, heap);
        pProgram->setUniformName(0, "cCopyRectInfoArray");
        pProgram->setUniformName(1, "cCopyParam");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cTexture3d_0");
        break;
    case cShVolumeDiffUpdateMrt:
        pProgram->createUniform(2, heap);
        pProgram->setUniformName(0, "cCopyRectInfoArray");
        pProgram->setUniformName(1, "cUpdateParam");
        pProgram->createSamplerLocation(4, heap);
        pProgram->setSamplerLocationName(0, "cLightTexture0");
        pProgram->setSamplerLocationName(1, "cLightTexture1");
        pProgram->setSamplerLocationName(2, "cLightTexture2");
        pProgram->setSamplerLocationName(3, "cLightTexture3");
        break;
    case cShVolumeCopyCubemapMrt:
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "cNormal");
        pProgram->setSamplerLocationName(1, "cSrcSamplerCube");
        break;
    case cShVolumeCubemapFaceFilter:
        pProgram->createUniform(1, heap);
        pProgram->setUniformName(0, "cNearFarParam");
        pProgram->createSamplerLocation(1, heap);
        pProgram->setSamplerLocationName(0, "cNonLinearDepthTexture");
        break;
    case cVdm:
        pProgram->createUniform(2, heap);
        pProgram->setUniformName(0, "cInvTexSize");
        pProgram->setUniformName(1, "cVarianceMax");
        pProgram->createSamplerLocation(3, heap);
        pProgram->setSamplerLocationName(0, "cTextureColor");
        pProgram->setSamplerLocationName(1, "cTextureDepth");
        pProgram->setSamplerLocationName(2, "cTextureLinearDepth");
        break;
    case cCloud:
        pProgram->createUniformBlock(2, heap);
        pProgram->setUniformBlockName(0, "View");
        pProgram->setUniformBlockName(1, "Common");
        pProgram->createSamplerLocation(5, heap);
        pProgram->setSamplerLocationName(0, "cBaseTexture");
        pProgram->setSamplerLocationName(1, "cNoiseTexture");
        pProgram->setSamplerLocationName(2, "cBaseTexture_Blend");
        pProgram->setSamplerLocationName(3, "cNoiseTexture_Blend");
        pProgram->setSamplerLocationName(4, "cScatterTexture");
        break;
    case cTexture:
    case cReduce2:
    case cReduce4:
    case cReduce8:
    case cReduce16:
    case cImageFilterGaussian:
    case cImageFilterCubic:
    case cImageFilterBlur:
    case cTextureMultColor:
    case cRed:
    case cGreen:
    case cBlue:
    case cAlpha:
    case cDepthRaw:
    case cDepthLinear:
    case cDepthLinearArray:
    case cDepth2d:
    case cDepth2dArrayQuad:
    case cDepth2dMin:
    case cDepth2dArrayMin:
    case cDepth2dMax:
    case cDepth2dArrayMax:
    case cColor2dArrayQuad:
    case cColor2dMin:
    case cColor2dArrayMin:
    case cColor2dMax:
    case cColor2dArrayMax:
    case cUint:
    case cUintArray:
    case cDepthMask:
    case cLuminance:
    case cTexture2dArray:
    case cTexture3d:
    case cTextureCubeMap:
    case cTextureCubeMapArray:
    case cTextureClrmtx2d:
    case cTextureClrmtx2dArray:
    case cTextureTexcoord:
    case cTextureTexcoordMultColor:
    case cTextureMultiSample1x:
    case cTextureMultiSample2x:
    case cTextureMultiSample4x:
    case cTextureMultiSample8x:
    case cAlphaModify:
    case cTextureColorDrift:
    case cColorQuad:
    case cClearQuad:
    case cTopBottomColor:
    case cTextureGamma:
    case cNv12decode:
        pProgram->createAttribute(1, heap);
        pProgram->setAttributeName(0, "aPosition");
        pProgram->createUniform(26, heap);
        pProgram->setUniformName(0, "uProjViewWorld[0]");
        pProgram->setUniformName(1, "cTexSize");
        pProgram->setUniformName(2, "uMipLevel");
        pProgram->setUniformName(3, "uSlice");
        pProgram->setUniformName(4, "uTexScale");
        pProgram->setUniformName(5, "uTexRotate");
        pProgram->setUniformName(6, "uTexTrans");
        pProgram->setUniformName(7, "uMultiSampleNum");
        pProgram->setUniformName(8, "uColor");
        pProgram->setUniformName(9, "uColorMatrix[0]");
        pProgram->setUniformName(10, "uTexFlipY");
        pProgram->setUniformName(11, "uTexFetchAdjust");
        pProgram->setUniformName(12, "cColorDriftR");
        pProgram->setUniformName(13, "cColorDriftG");
        pProgram->setUniformName(14, "cColorDriftB");
        pProgram->setUniformName(15, "uColorT");
        pProgram->setUniformName(16, "uColorB");
        pProgram->setUniformName(17, "uCubeMapFace");
        pProgram->setUniformName(18, "uBlurOffset");
        pProgram->setUniformName(19, "uDepthNear");
        pProgram->setUniformName(21, "uInvDepthRange");
        pProgram->setUniformName(20, "uOneMinusNearDivFar");
        pProgram->setUniformName(22, "cNV12DecodeParam");
        pProgram->setUniformName(23, "uGamma");
        pProgram->setUniformName(24, "uQuadTriangleZ");
        pProgram->setUniformName(25, "uNormalizeValue");
        pProgram->createSamplerLocation(2, heap);
        pProgram->setSamplerLocationName(0, "uTexture");
        pProgram->setSamplerLocationName(1, "uTexture_1");
        break;
    default:
        break;
    }
    }

    if (mArchives[cArchive_TechniquePfx].isValid()) {
        pfx::Sky::setUpShader(&mArchives[cArchive_TechniquePfx], heap);
    }

    if (mArchives[cArchive_Technique].isValid()) {
        eft::Star::setUpShader(&mArchives[cArchive_Technique], heap);
    }

    for (auto& rArchive : mArchives) {
        if (rArchive.isValid()) {
            rArchive.setUp();
        }
    }

    heap->adjust();
}

/**
 * Generates the host IO message with the program and variation counts.
 * @param pContext host IO context
 */
void ShaderHolder::genMessage(sead::hostio::Context* pContext) {
    s32 programNum = 0;
    s32 variationNum = 0;
    for (const auto& rArchive : mArchives) {
        programNum += rArchive.getShaderProgramNum();
        variationNum += rArchive.getVariationNum();
    }

    sead::FormatFixedSafeString<1024> message("プログラム数     :%d\nバリエーション数 :%d",
                                              programNum, variationNum);
}

/**
 * Handles a host IO property event (no-op in release builds).
 * @param pEvent property event
 */
void ShaderHolder::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

}  // namespace detail
}  // namespace agl
