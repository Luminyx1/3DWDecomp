#pragma once
#include <nn/gfx/gfx_Types.h>

namespace nn {
namespace gfx {
class ResShaderProgram;
class ResShaderContainer;
class ResShaderVariation;
};  // namespace gfx
namespace ui2d {
struct BuildArgSet;
// args contains the nested layout names; depth selects how many form the prefix.
size_t CalcDynamicGenerateTexturePrefixLength(const BuildArgSet& args, int depth);
// buffer/size describe the destination for the prefix selected by args and depth.
void ConcatDynamicGenerateTexturePrefixString(char* buffer, size_t size, const BuildArgSet& args, int depth);
bool IsResShaderProgramInitialized(nn::gfx::ResShaderProgram*);

bool IsResShaderContainerInitialized(nn::gfx::ResShaderContainer*);

nn::gfx::ShaderCodeType TryInitializeAndGetShaderCodeType(nn::gfx::Device*,
                                                          nn::gfx::ResShaderVariation*);
};  // namespace ui2d
};  // namespace nn