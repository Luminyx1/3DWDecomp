#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

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
    /**
     * A registered cube map. Partial layout; only the members used so far are named.
     */
    struct CubeMapInfo {
        u8 _0[0x18];
        sead::SafeString mName;
    };

    ShaderCubeMapKeeper(GraphicsSystemInfo* pGraphicsSystemInfo, PlayerHolder* pPlayerHolder);
    ~ShaderCubeMapKeeper();

    void initStageResource(const Resource* pResource, const char* pName, const LiveActorKit* pKit);
    void endInit();
    void updateCubeMapKeeper();
    bool activateCubeMapTexture(s32, s32, s32, bool) const;
    bool isDrawCubeMap() const;
    const agl::TextureSampler* getIrradiance(s32 index, const sead::Vector3f& rPos) const;
    const agl::TextureSampler* getRoughnessCubeMap(s32 roughness, s32 index) const;
    s32 findCubeMapIndexByName(const char* pName) const;
    const void* getCurrentCategoryLightInfo(s32 category) const;

    const CubeMapInfo* getForceCubeMapInfo() const { return mForceCubeMapInfo; }
    f32 getModelLightIntensity() const { return mModelLightIntensity; }

private:
    u8 _0[0x8];
    f32 mModelLightIntensity;
    u8 _c[0x1f8 - 0xc];
    CubeMapInfo* mForceCubeMapInfo;
    u8 _200[0x258 - 0x200];
};
}  // namespace al
