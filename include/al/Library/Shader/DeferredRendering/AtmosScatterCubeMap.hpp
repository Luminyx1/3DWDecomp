#pragma once

#include <container/seadPtrArray.h>
#include <math/seadVector.h>
#include "common/aglShaderEnum.h"
#include "common/aglTextureSampler.h"

#include "Library/Nerve/NerveExecutor.hpp"

namespace agl::env {
class CubeMap;
}

namespace al {
class AtmosScatter;
class CubeMapDrawInfo;
class GraphicsSystemInfo;

class AtmosScatterCubeMap : public NerveExecutor {
public:
    AtmosScatterCubeMap(GraphicsSystemInfo* pGraphicsSystemInfo);
    ~AtmosScatterCubeMap() override;

    void preDrawGraphics();
    void exeInitial();
    void exeDrawFace();
    void exeDrawRoughness();
    void exeFlipCubeMap();
    agl::ShaderMode renderToCubeMap(agl::ShaderMode shaderMode) const;
    const agl::TextureSampler* getIrradianceSampler(s32 index) const;
    const agl::TextureSampler* getCubeMapMirrorSampler(s32 index) const;
    bool activateCubeMapTexture(s32 type, bool isRefract) const;

private:
    GraphicsSystemInfo* mGraphicsSystemInfo;
    AtmosScatter* mAtmosScatter;
    void* _20;
    agl::TextureSampler mIrradianceSampler;
    agl::TextureSampler mMirrorSampler;
    sead::FixedPtrArray<agl::env::CubeMap, 2> mCubeMaps;
    CubeMapDrawInfo* mDrawInfo = nullptr;
    sead::Vector3f mCubeMapPos = {0.0f, 0.0f, 0.0f};
    s32 mCubeMapIndex = 0;
    s32 mFaceIndex = -1;
};

static_assert(sizeof(AtmosScatterCubeMap) == 0x348);
}  // namespace al
