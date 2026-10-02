#pragma once

#include <basis/seadTypes.h>

namespace nn::g3d {
class ResMaterial;
}

namespace al {
class EnvTexInfo;
class GraphicsSystemInfo;
class UniformBlockAssignArray;

void activateUniformBlockAssignArray(const UniformBlockAssignArray& rArray);

class ModelLightDirector {
public:
    ModelLightDirector(GraphicsSystemInfo* pInfo);
    ~ModelLightDirector();

    void endInit();
    void activateModelLightTexture(const nn::g3d::ResMaterial* pMaterial,
                                   const EnvTexInfo* pEnvTexInfo, bool isForce) const;
    void activateModelLightTexture(s32 index) const;

private:
    u8 _0[0x330];
};

static_assert(sizeof(ModelLightDirector) == 0x330);

}  // namespace al
