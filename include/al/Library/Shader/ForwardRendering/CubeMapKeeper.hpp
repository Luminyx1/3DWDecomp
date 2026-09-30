#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Shader/ForwardRendering/EnvTextureKeeper.hpp"

namespace agl {
class TextureSampler;
}

namespace al {
class GraphicsSystemInfo;
class LiveActorKit;
class PlayerHolder;
class Resource;

class ShaderCubeMapKeeper {
public:
    ShaderCubeMapKeeper(GraphicsSystemInfo* pGraphicsSystemInfo, PlayerHolder* pPlayerHolder);
    ~ShaderCubeMapKeeper();

    void initStageResource(const Resource* pResource, const char* pName, const LiveActorKit* pKit);
    void endInit();
    bool activateCubeMapTexture(s32, s32, s32, bool) const;
    bool isDrawCubeMap() const;
    const agl::TextureSampler* getIrradiance(s32 index, const sead::Vector3f& rPos) const;
    const agl::TextureSampler* getRoughnessCubeMap(s32 roughness, s32 index) const;
    s32 findCubeMapIndexByName(const char* pName) const;
    const void* getCurrentCategoryLightInfo(s32 category) const;

private:
    u8 _0[0x258];
};
}  // namespace al
