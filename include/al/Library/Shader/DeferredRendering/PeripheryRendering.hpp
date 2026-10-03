#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjArray.h>
#include "common/aglShaderEnum.h"

namespace agl {
class UniformBlockLocation;
}  // namespace agl

namespace nn::g3d {
class ModelObj;
class ResShaderAssign;
class ResShaderProgram;
class ResShadingModel;
class ShaderSelector;
class ShadingModelObj;
class ShapeObj;
}  // namespace nn::g3d

namespace al {
class SimpleModelG3D;
class UniformBlock;
struct UniformBlockLayout;

/**
 * @brief A per-model uniform block together with the location it is bound to.
 */
struct UniformBlockAssign {
    UniformBlock* mUniformBlock;
    agl::UniformBlockLocation* mLocation;
};

static_assert(sizeof(UniformBlockAssign) == 0x10);

class UniformBlockAssignArray : public sead::ObjArray<UniformBlockAssign> {};

static_assert(sizeof(UniformBlockAssignArray) == 0x20);

void activateUniformBlockAssign(const UniformBlockAssign& rAssign);
void activateUniformBlockAssignArray(const UniformBlockAssignArray& rArray);
UniformBlockAssign* tryCreateUniformBlockAssign(SimpleModelG3D* pModel, const char* pName,
                                                const UniformBlockLayout* pLayout, s32 layoutNum);
UniformBlockAssign* findUniformBlockAssign(const UniformBlockAssignArray* pArray,
                                           const char* pName);
void swapUniformBlockAssignArray(const UniformBlockAssignArray* pArray);

agl::ShaderMode activateShader(const nn::g3d::ResShaderProgram* pProgram,
                               agl::ShaderMode shaderMode);
void initShaderKeyStatic(nn::g3d::ShadingModelObj* pShadingModel,
                         const nn::g3d::ResShaderAssign* pShaderAssign,
                         const nn::g3d::ModelObj* pModel, const nn::g3d::ShapeObj* pShape,
                         bool isEnableAlphaOutput);
void initShaderKeyDynamic(nn::g3d::ShaderSelector* pSelector,
                          const nn::g3d::ResShaderAssign* pShaderAssign,
                          const nn::g3d::ModelObj* pModel, const nn::g3d::ShapeObj* pShape);
const nn::g3d::ResShaderProgram* searchVariation(const nn::g3d::ResShadingModel* pShadingModel,
                                                 s32 macroNum, const char* const* pMacros,
                                                 const char* const* pValues);
const nn::g3d::ResShaderProgram* trySearchVariation(const nn::g3d::ResShadingModel* pShadingModel,
                                                    s32 macroNum, const char* const* pMacros,
                                                    const char* const* pValues);
const char* searchVariationMacroValue(const nn::g3d::ShadingModelObj* pShadingModel,
                                      const char* pName);
const char* searchVariationMacroValue(const nn::g3d::ShaderSelector* pSelector,
                                      const char* pName);
const char* getShaderProgramName(const nn::g3d::ShadingModelObj* pShadingModel);

}  // namespace al
