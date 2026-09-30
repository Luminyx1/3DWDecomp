#pragma once
#include <nn/g3d/g3d_Resources.h>
#include <nn/g3d/g3d_ResShader.h>

namespace nn::g3d {
class ShaderUtility {
public:
    // key receives defaults and assignments for model; capacity and check are unchecked in this build.
    static void InitializeShaderKey(u32* key, int capacity, const ResShadingModel* model,
                                    const ResShaderAssign* assignment, bool check);
    // object receives assignment's static options; check is unchecked in this build.
    static void InitializeShaderKey(ShadingModelObj* object, const ResShaderAssign* assignment, bool check);
    // material receives parameter offsets resolved against model's material uniform block.
    static void BindShaderParam(ResMaterial* material, const ResShadingModel* model);
    // material receives model's default values in each buffered material block.
    static void InitializeShaderParam(MaterialObj* material, const ResShadingModel* model);
};
}
