#pragma once

#include <math/seadVector.h>
#include "common/aglShaderEnum.h"

namespace agl {
class DrawContext;
class SamplerLocation;
class UniformBlockLocation;
}  // namespace agl

namespace al {
class UniformBlock;

const agl::UniformBlockLocation& getUniformBlockLocationMdlEnvView();
const agl::UniformBlockLocation& getUniformBlockLocationMdlMtx();
const agl::UniformBlockLocation& getUniformBlockLocationShp();
const agl::UniformBlockLocation& getUniformBlockLocationRenderGBuffer();
const agl::UniformBlockLocation& getUniformBlockLocationRenderSky();
const agl::UniformBlockLocation& getUniformBlockLocationEchoBlock();
const agl::UniformBlockLocation& getUniformBlockLocationShaderOption();
const agl::UniformBlockLocation& getUniformBlockLocationOtherFirst();
const agl::UniformBlockLocation& getUniformBlockLocationOtherSecond();
const agl::UniformBlockLocation& getUniformBlockLocationOtherThird();
const agl::SamplerLocation& getSamplerLocationLinearDepth();
const agl::SamplerLocation& getSamplerLocationModelLight();
const agl::SamplerLocation& getSamplerLocationCubeMapRoughness();
const agl::SamplerLocation& getSamplerLocationCubeMapIrradiance();
const agl::SamplerLocation& getSamplerLocationFresnelCurve();
const agl::SamplerLocation& getSamplerLocationTransformHeightMap();
const agl::SamplerLocation& getSamplerLocationCubeMapRoughnessRefract();
const agl::SamplerLocation& getSamplerLocationIndirect();
const agl::SamplerLocation& getSamplerLocationThicknessCurve();
const agl::SamplerLocation& getSamplerLocationSilhouetteCurve();
const agl::SamplerLocation& getSamplerLocationMirrorTex();
const agl::SamplerLocation& getSamplerLocationMultiTexMask();
const agl::SamplerLocation& getSamplerLocationAlphaProjMaskTex();
const agl::SamplerLocation& getSamplerLocationDirLitTex();
const agl::SamplerLocation& getSamplerLocationDither();
void tryChangeShaderMode(agl::DrawContext* pDrawContext, agl::ShaderMode shaderMode);
void forceChangeShaderMode(agl::DrawContext* pDrawContext, agl::ShaderMode shaderMode);
UniformBlock* createMakeHalfTextureUbo();
void calcTanFovyHalf(sead::Vector2f* pTanFovyHalf, sead::Vector2f* pOffset, f32 fovy, f32 aspect,
                     const sead::Vector2f& rProjOffset);
}  // namespace al
