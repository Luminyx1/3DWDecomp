#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"

#include <math/seadMathCalcCommon.h>
#include "common/aglDrawContext.h"
#include "common/aglShaderLocation.h"

#include "Library/Shader/Block/UniformBlockUtil.hpp"

namespace {
agl::ShaderLocation makeLocation(s32 vertex, s32 fragment, s32 geometry) {
    agl::ShaderLocation location;
    location.setLocation(agl::cShaderType_Vertex, vertex);
    location.setLocation(agl::cShaderType_Fragment, fragment);
    location.setLocation(agl::cShaderType_Geometry, geometry);
    return location;
}

agl::UniformBlockLocation sMdlEnvView(makeLocation(1, 1, -1), "MdlEnvView");
agl::UniformBlockLocation sMdlMtx(makeLocation(3, -1, -1), "MdlMtx");
agl::UniformBlockLocation sShp(makeLocation(4, -1, -1), "Shp");
agl::UniformBlockLocation sRenderGBuffer(makeLocation(5, -1, -1), "RenderGBuffer");
agl::UniformBlockLocation sModelAdditionalInfo(makeLocation(-1, 6, -1), "ModelAdditionalInfo");
agl::UniformBlockLocation sRenderSky(makeLocation(9, -1, -1), "RenderSky");
agl::UniformBlockLocation sEchoBlock(makeLocation(-1, 9, -1), "EchoBlockUniform");
agl::UniformBlockLocation sShaderOption(makeLocation(13, 13, -1), "cShaderOption");
agl::UniformBlockLocation sOtherFirst(makeLocation(-1, 9, -1), "Undefined");
agl::UniformBlockLocation sOtherSecond(makeLocation(-1, 10, -1), "Undefined");
agl::UniformBlockLocation sOtherThird(makeLocation(-1, 11, -1), "Undefined");
agl::SamplerLocation sLinearDepth(makeLocation(-1, 4, -1), "cTextureLinearDepth");
agl::SamplerLocation sModelLight(makeLocation(-1, 5, -1), "cTexModelLight");
agl::SamplerLocation sCubeMapRoughness(makeLocation(6, 6, -1), "cTexCubeMapRoughness");
agl::SamplerLocation sCubeMapIrradiance(makeLocation(7, 7, -1), "cTexCubeMapIrradiance");
agl::SamplerLocation sFresnelCurve(makeLocation(-1, 8, -1), "cFresnelCurve");
agl::SamplerLocation sTransformHeightMap(makeLocation(-1, 8, -1), "cTexTransformHeightMap");
agl::SamplerLocation sCubeMapRoughnessRefract(makeLocation(23, 23, -1),
                                              "cTexCubeMapRoughnessRefract");
agl::SamplerLocation sIndirect(makeLocation(-1, 9, -1), "cTextureIndirect");
agl::SamplerLocation sThicknessCurve(makeLocation(-1, 10, -1), "cThicknessCurve");
agl::SamplerLocation sSilhouetteCurve(makeLocation(-1, 0, -1), "cSilhouetteCurve");
agl::SamplerLocation sAlphaProjMaskTex(makeLocation(-1, 12, -1), "cTexAlphaMaskProj");
agl::SamplerLocation sMultiTexMask(makeLocation(-1, 24, -1), "cTexMultiTexMask");
agl::SamplerLocation sMirrorTex(makeLocation(-1, 19, -1), "cMirrorTex");
agl::SamplerLocation sDirLitTex(makeLocation(15, -1, -1), "cDirectionalLightColor");
agl::SamplerLocation sDither(makeLocation(-1, 25, -1), "cDitherTexture");

const al::UniformBlockLayout cMakeHalfTextureLayout[] = {
    {0, agl::UniformBlock::cType_Vec2, 1},
    {1, agl::UniformBlock::cType_Float, 1},
};
}  // namespace

namespace al {
/**
 * Gets the location of the model environment view uniform block.
 * @return The uniform block location.
 */
const agl::UniformBlockLocation& getUniformBlockLocationMdlEnvView() {
    return sMdlEnvView;
}

/**
 * Gets the location of the model matrix uniform block.
 * @return The uniform block location.
 */
const agl::UniformBlockLocation& getUniformBlockLocationMdlMtx() {
    return sMdlMtx;
}

/**
 * Gets the location of the shape uniform block.
 * @return The uniform block location.
 */
const agl::UniformBlockLocation& getUniformBlockLocationShp() {
    return sShp;
}

/**
 * Gets the location of the GBuffer rendering uniform block.
 * @return The uniform block location.
 */
const agl::UniformBlockLocation& getUniformBlockLocationRenderGBuffer() {
    return sRenderGBuffer;
}

/**
 * Gets the location of the sky rendering uniform block.
 * @return The uniform block location.
 */
const agl::UniformBlockLocation& getUniformBlockLocationRenderSky() {
    return sRenderSky;
}

/**
 * Gets the location of the echo block uniform block.
 * @return The uniform block location.
 */
const agl::UniformBlockLocation& getUniformBlockLocationEchoBlock() {
    return sEchoBlock;
}

/**
 * Gets the location of the shader option uniform block.
 * @return The uniform block location.
 */
const agl::UniformBlockLocation& getUniformBlockLocationShaderOption() {
    return sShaderOption;
}

/**
 * Gets the location of the first user uniform block.
 * @return The uniform block location.
 */
const agl::UniformBlockLocation& getUniformBlockLocationOtherFirst() {
    return sOtherFirst;
}

/**
 * Gets the location of the second user uniform block.
 * @return The uniform block location.
 */
const agl::UniformBlockLocation& getUniformBlockLocationOtherSecond() {
    return sOtherSecond;
}

/**
 * Gets the location of the third user uniform block.
 * @return The uniform block location.
 */
const agl::UniformBlockLocation& getUniformBlockLocationOtherThird() {
    return sOtherThird;
}

/**
 * Gets the location of the linear depth sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationLinearDepth() {
    return sLinearDepth;
}

/**
 * Gets the location of the model light sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationModelLight() {
    return sModelLight;
}

/**
 * Gets the location of the roughness cube map sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationCubeMapRoughness() {
    return sCubeMapRoughness;
}

/**
 * Gets the location of the irradiance cube map sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationCubeMapIrradiance() {
    return sCubeMapIrradiance;
}

/**
 * Gets the location of the fresnel curve sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationFresnelCurve() {
    return sFresnelCurve;
}

/**
 * Gets the location of the transform height map sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationTransformHeightMap() {
    return sTransformHeightMap;
}

/**
 * Gets the location of the refraction roughness cube map sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationCubeMapRoughnessRefract() {
    return sCubeMapRoughnessRefract;
}

/**
 * Gets the location of the indirect texture sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationIndirect() {
    return sIndirect;
}

/**
 * Gets the location of the thickness curve sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationThicknessCurve() {
    return sThicknessCurve;
}

/**
 * Gets the location of the silhouette curve sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationSilhouetteCurve() {
    return sSilhouetteCurve;
}

/**
 * Gets the location of the mirror texture sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationMirrorTex() {
    return sMirrorTex;
}

/**
 * Gets the location of the multi texture mask sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationMultiTexMask() {
    return sMultiTexMask;
}

/**
 * Gets the location of the projected alpha mask sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationAlphaProjMaskTex() {
    return sAlphaProjMaskTex;
}

/**
 * Gets the location of the directional light color sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationDirLitTex() {
    return sDirLitTex;
}

/**
 * Gets the location of the dither texture sampler.
 * @return The sampler location.
 */
const agl::SamplerLocation& getSamplerLocationDither() {
    return sDither;
}

/**
 * Changes the shader mode if it differs from the current one.
 * @param pDrawContext Draw context.
 * @param shaderMode New shader mode.
 */
void tryChangeShaderMode(agl::DrawContext* pDrawContext, agl::ShaderMode shaderMode) {
    if (pDrawContext->getShaderMode() == shaderMode) {
        return;
    }
    pDrawContext->changeShaderMode(shaderMode, agl::ShaderOptimizeType(0));
}

/**
 * Changes the shader mode.
 * @param pDrawContext Draw context.
 * @param shaderMode New shader mode.
 */
void forceChangeShaderMode(agl::DrawContext* pDrawContext, agl::ShaderMode shaderMode) {
    pDrawContext->changeShaderMode(shaderMode, agl::ShaderOptimizeType(0));
}

/**
 * Creates the uniform block used to make half resolution textures.
 * @return The created uniform block.
 */
UniformBlock* createMakeHalfTextureUbo() {
    return createUniformBlock(cMakeHalfTextureLayout, 2, nullptr, 2);
}

/**
 * Calculates the tangents of the half field of view.
 * @param pTanFovyHalf Output tangents, may be nullptr.
 * @param pOffset Output projection offset, may be nullptr.
 * @param fovy Vertical field of view.
 * @param aspect Aspect ratio.
 * @param rProjOffset Projection offset.
 */
void calcTanFovyHalf(sead::Vector2f* pTanFovyHalf, sead::Vector2f* pOffset, f32 fovy, f32 aspect,
                     const sead::Vector2f& rProjOffset) {
    f32 tanY = sead::Mathf::tan(fovy * 0.5f);
    f32 tanX = tanY * aspect;
    if (pOffset) {
        pOffset->set(tanX * (rProjOffset.x + rProjOffset.x), tanY * (rProjOffset.y + rProjOffset.y));
    }
    if (pTanFovyHalf) {
        pTanFovyHalf->set(tanX, -tanY);
    }
}
}  // namespace al
