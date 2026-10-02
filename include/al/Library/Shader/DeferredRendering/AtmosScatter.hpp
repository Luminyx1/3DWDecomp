#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include "common/aglShaderEnum.h"

namespace al {
class GBufferArray;
class GraphicsSystemInfo;
class Resource;

class AtmosScatter {
public:
    AtmosScatter(GraphicsSystemInfo* pInfo, s32 viewNum, f32 far);
    ~AtmosScatter();

    void initStageResource(const Resource* pResource, const char* pStageName);
    void updateAtmosScatter();
    void preDrawGraphics();
    void calcInfo(sead::Vector3f* pPos, sead::Vector3f* pDir, sead::Vector3f* pLightDir) const;
    void drawFarDeferred(s32 viewIndex, GBufferArray* pGBufferArray, const sead::Matrix34f& rViewMtx,
                         const sead::Matrix44f& rProjMtx, const sead::Vector2f& rProjOffset, f32 fovy,
                         f32 aspect, agl::ShaderMode shaderMode) const;
    agl::ShaderMode drawFarToCubeMap(s32 viewIndex, const sead::Matrix34f& rViewMtx,
                                     const sead::Matrix44f& rProjMtx,
                                     const sead::Vector2f& rProjOffset, f32 fovy, f32 aspect,
                                     agl::ShaderMode shaderMode) const;

private:
    u8 _0[0x200];
};

static_assert(sizeof(AtmosScatter) == 0x200);
}  // namespace al
