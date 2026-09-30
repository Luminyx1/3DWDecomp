#pragma once

#include <basis/seadTypes.h>

namespace nn::g3d {
class ResMaterial;
}

namespace al {
class EnvTexInfo;
class UniformBlockAssignArray;

void activateUniformBlockAssignArray(const UniformBlockAssignArray& rArray);

class ModelLightDirector {
public:
    void activateModelLightTexture(const nn::g3d::ResMaterial* pMaterial,
                                   const EnvTexInfo* pEnvTexInfo, bool isForce) const;
    void activateModelLightTexture(s32 index) const;
};

}  // namespace al
