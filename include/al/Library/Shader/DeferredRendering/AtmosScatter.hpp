#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include "common/aglShaderEnum.h"

namespace al {
class GBufferArray;

class AtmosScatter {
public:
    void drawFarDeferred(s32 viewIndex, GBufferArray* pGBufferArray, const sead::Matrix34f& rViewMtx,
                         const sead::Matrix44f& rProjMtx, const sead::Vector2f& rProjOffset, f32 fovy,
                         f32 aspect, agl::ShaderMode shaderMode) const;
    agl::ShaderMode drawFarToCubeMap(s32 viewIndex, const sead::Matrix34f& rViewMtx,
                                     const sead::Matrix44f& rProjMtx,
                                     const sead::Vector2f& rProjOffset, f32 fovy, f32 aspect,
                                     agl::ShaderMode shaderMode) const;
};
}  // namespace al
