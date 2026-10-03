#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class EnvTexInfo;
class GraphicsAreaDirector;
class GraphicsSystemInfo;
class ModelAdditionalInfo;
class PlayerHolder;
class ShaderFresnelTextureKeeper;
class ShaderHolder;

class ShaderEnvTextureKeeper {
public:
    ShaderEnvTextureKeeper(GraphicsSystemInfo* pInfo, PlayerHolder* pPlayerHolder);
    ~ShaderEnvTextureKeeper();

    void initTexture(ShaderHolder* pShaderHolder);
    void initGraphicsAreaParam(GraphicsAreaDirector* pAreaDirector, const char* pStageName);
    void endInit();
    void updateEnvTexture();
    void execute();
    void activateEnvTexture(const EnvTexInfo& rInfo, ModelAdditionalInfo* pAdditionalInfo,
                            bool isForce) const;

    ShaderFresnelTextureKeeper* getFresnelTextureKeeper() const { return mFresnelTextureKeeper; }

    bool isUseViewMtx() const { return mIsUseViewMtx; }

    const sead::Vector3f& getFrontDir() const { return mFrontDir; }

private:
    u8 _0[0x10];
    ShaderFresnelTextureKeeper* mFresnelTextureKeeper;
    u8 _18[0x20 - 0x18];
    bool mIsUseViewMtx;
    sead::Vector3f mFrontDir;
    u8 _30[0x68 - 0x30];
};

static_assert(sizeof(ShaderEnvTextureKeeper) == 0x68);

}  // namespace al
