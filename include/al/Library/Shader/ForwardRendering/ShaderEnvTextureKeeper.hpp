#pragma once

#include <basis/seadTypes.h>

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

private:
    u8 _0[0x10];
    ShaderFresnelTextureKeeper* mFresnelTextureKeeper;
    u8 _18[0x68 - 0x18];
};

static_assert(sizeof(ShaderEnvTextureKeeper) == 0x68);

}  // namespace al
