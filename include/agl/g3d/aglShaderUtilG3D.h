#pragma once

#include <nn/gfx/gfx_Types.h>
#include <prim/seadSafeString.h>

namespace nn::g3d {
class ResShaderProgram;
class ResShadingModel;
class ShaderSelector;
class ShadingModelObj;
}  // namespace nn::g3d

namespace agl {
class DrawContext;
class SamplerLocation;
class ShaderLocation;
class UniformBlockLocation;
}  // namespace agl

namespace agl::g3d {

class ShaderUtilG3D {
public:
    static void load(DrawContext* pDrawContext, const ShaderLocation& rLocation,
                     const nn::gfx::Buffer& rBuffer, u64 size, s32 unused);
    static void load(DrawContext* pDrawContext, const ShaderLocation& rLocation,
                     const u32& rSamplerId, const u32& rTextureId);
    static void search(UniformBlockLocation* pLocation, const nn::g3d::ResShadingModel* pModel,
                       const nn::g3d::ResShaderProgram* pProgram, const sead::SafeString& rName);
    static void searchUniformBlock(ShaderLocation* pLocation,
                                   const nn::g3d::ResShadingModel* pModel,
                                   const nn::g3d::ResShaderProgram* pProgram,
                                   const sead::SafeString& rName);
    static void search(SamplerLocation* pLocation, const nn::g3d::ResShadingModel* pModel,
                       const nn::g3d::ResShaderProgram* pProgram, const sead::SafeString& rName);
    static void searchSampler(ShaderLocation* pLocation, const nn::g3d::ResShadingModel* pModel,
                              const nn::g3d::ResShaderProgram* pProgram,
                              const sead::SafeString& rName);
    static void print(const nn::g3d::ShadingModelObj& rModel,
                      const nn::g3d::ShaderSelector& rSelector);
};

}  // namespace agl::g3d
