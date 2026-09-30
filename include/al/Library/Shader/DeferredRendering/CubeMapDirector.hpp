#pragma once

#include <basis/seadTypes.h>
#include "common/aglShaderEnum.h"

namespace agl {
class TextureSampler;
}

namespace al {
class AtmosScatterCubeMap;
class GraphicsSystemInfo;
class LiveActorKit;
class PlayerHolder;
class Resource;
class ShaderCubeMapKeeper;

class CubeMapDirector {
public:
    CubeMapDirector(GraphicsSystemInfo* pGraphicsSystemInfo);
    ~CubeMapDirector();

    void initStageResource(const Resource* pResource, const char* pName, const LiveActorKit* pKit);
    void preDrawGraphics();
    void endInit();
    void initByCapturePoint(PlayerHolder* pPlayerHolder);
    void initByAtmosScatter();
    bool activateCubeMapTexture(s32, s32, s32, bool) const;
    bool isDrawCapturePointCubeMap() const;
    agl::ShaderMode renderToCubeMap(agl::ShaderMode shaderMode) const;
    const agl::TextureSampler* getIrradianceSampler(s32 index) const;
    const agl::TextureSampler* getCubeMapMirrorSampler(s32 index) const;

    ShaderCubeMapKeeper* getShaderCubeMapKeeper() const { return mShaderCubeMapKeeper; }

private:
    ShaderCubeMapKeeper* mShaderCubeMapKeeper = nullptr;
    GraphicsSystemInfo* mGraphicsSystemInfo;
    AtmosScatterCubeMap* mAtmosScatterCubeMap = nullptr;
};
}  // namespace al
