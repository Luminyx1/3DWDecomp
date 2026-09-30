#pragma once

#include <basis/seadTypes.h>

namespace al {
class EnvTexInfo;
class ModelAdditionalInfo;
class ShaderFresnelTextureKeeper;

class ShaderEnvTextureKeeper {
public:
    void activateEnvTexture(const EnvTexInfo& rInfo, ModelAdditionalInfo* pAdditionalInfo,
                            bool isForce) const;

    ShaderFresnelTextureKeeper* getFresnelTextureKeeper() const { return mFresnelTextureKeeper; }

private:
    u8 _0[0x10];
    ShaderFresnelTextureKeeper* mFresnelTextureKeeper;
};

}  // namespace al
