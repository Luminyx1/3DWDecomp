#pragma once

#include <basis/seadTypes.h>

namespace agl {
class ShaderProgram;
}

namespace nn::g3d {
class ResShadingModel;
}

namespace al {

class ShaderHolder {
public:
    static ShaderHolder* sInstance;

    void setupShaderArchives();
    void cleanupShaderArchives();
    nn::g3d::ResShadingModel* getShadingModel(const char* pName) const;
    nn::g3d::ResShadingModel* getShadingModelUber(const char* pName) const;
    agl::ShaderProgram* getShaderProgram(const char* pName) const;
    agl::ShaderProgram* tryGetShaderProgram(const char* pName) const;
};

}  // namespace al
