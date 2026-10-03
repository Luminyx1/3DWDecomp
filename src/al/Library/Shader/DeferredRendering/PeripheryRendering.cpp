#include "Library/Shader/DeferredRendering/PeripheryRendering.hpp"

#include <common/aglShaderLocation.h>
#include <driver/aglGraphicsDriverMgr.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResShader.h>
#include <nn/g3d/g3d_ShapeObj.h>

#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace {
const char* const cSkinWeightNumNames[] = {"0", "1", "2", "3", "4"};

/**
 * Gets the graphics device of the graphics driver.
 * @return The graphics device.
 */
nn::gfx::Device* getGfxDevice() {
    return static_cast<nn::gfx::Device*>(
        agl::driver::GraphicsDriverMgr::instance()->getGfxDevice());
}
}  // namespace

namespace al {

/**
 * Binds a uniform block to its location for the current draw.
 * @param rAssign Uniform block and location.
 */
void activateUniformBlockAssign(const UniformBlockAssign& rAssign) {
    rAssign.mUniformBlock->activate(GameFrameworkNx::getAglDrawContext(), *rAssign.mLocation);
}

/**
 * Binds every uniform block of an array to its location for the current draw.
 * @param rArray Uniform block assignments.
 */
void activateUniformBlockAssignArray(const UniformBlockAssignArray& rArray) {
    for (s32 i = 0; i < rArray.size(); i++) {
        activateUniformBlockAssign(*rArray.unsafeAt(i));
    }
}

/**
 * Creates a uniform block for a model if one of its shader programs uses a block with that name.
 * @param pModel Model whose shapes are searched.
 * @param pName Name of the uniform block.
 * @param pLayout Layout of the uniform block.
 * @param layoutNum Number of layout entries.
 * @return The created assignment, or nullptr if no shader uses the block.
 */
UniformBlockAssign* tryCreateUniformBlockAssign(SimpleModelG3D* pModel, const char* pName,
                                                const UniformBlockLayout* pLayout,
                                                s32 layoutNum) {
    UniformBlockAssignArray* assignArray = pModel->getUniformBlockAssignArray();

    for (s32 i = 0; i < pModel->getModelObj()->GetNumShapes(); i++) {
        const nn::g3d::ShaderSelector* selector = pModel->getShape(i).mShaderSelector;
        const nn::g3d::ResShadingModel* shadingModel =
            selector->GetShadingModel()->GetResource();

        if (shadingModel->FindUniformBlock(pName) == nullptr) {
            continue;
        }

        const nn::g3d::ResShaderProgram* program = selector->GetProgram();
        s32 index = shadingModel->FindUniformBlockIndex(pName);
        s32 vertex = program->GetUniformBlockLocation(index, nn::g3d::Stage_Vertex);
        s32 geometry = program->GetUniformBlockLocation(index, nn::g3d::Stage_Geometry);
        s32 pixel = program->GetUniformBlockLocation(index, nn::g3d::Stage_Pixel);

        if (vertex < 0 && geometry < 0 && pixel < 0) {
            continue;
        }

        UniformBlockAssign* assign = assignArray->emplaceBack();
        agl::ShaderLocation location;
        location.setLocation(agl::cShaderType_Vertex, vertex);
        location.setLocation(agl::cShaderType_Fragment, pixel);
        location.setLocation(agl::cShaderType_Geometry, geometry);
        assign->mLocation = new agl::UniformBlockLocation(location, pName);
        assign->mUniformBlock = createUniformBlock(pLayout, layoutNum, nullptr, 2);
        return assign;
    }

    return nullptr;
}

/**
 * Finds the uniform block assignment with the given name.
 * @param pArray Uniform block assignments.
 * @param pName Name of the uniform block.
 * @return The assignment, or nullptr if none has that name.
 */
UniformBlockAssign* findUniformBlockAssign(const UniformBlockAssignArray* pArray,
                                           const char* pName) {
    for (s32 i = 0; i < pArray->size(); i++) {
        if (isEqualString(pArray->unsafeAt(i)->mLocation->getName().cstr(), pName)) {
            return pArray->at(i);
        }
    }

    return nullptr;
}

/**
 * Swaps the buffers of every uniform block of an array.
 * @param pArray Uniform block assignments.
 */
void swapUniformBlockAssignArray(const UniformBlockAssignArray* pArray) {
    for (s32 i = 0; i < pArray->size(); i++) {
        pArray->unsafeAt(i)->mUniformBlock->swap();
    }
}

/**
 * Updates a shader program and loads it into the command buffer.
 * @param pProgram Shader program.
 * @param shaderMode Current shader mode.
 * @return The shader mode after activation.
 */
agl::ShaderMode activateShader(const nn::g3d::ResShaderProgram* pProgram,
                               agl::ShaderMode shaderMode) {
    const_cast<nn::g3d::ResShaderProgram*>(pProgram)->Update(getGfxDevice());
    pProgram->Load(GameFrameworkNx::getAglDrawContext()->getCommandBuffer());
    return shaderMode;
}

/**
 * Initializes the static shader key from the shader assignment and the shape.
 * @param pShadingModel Shading model object.
 * @param pShaderAssign Shader assignment of the material.
 * @param pModel Model, unused.
 * @param pShape Shape.
 * @param isEnableAlphaOutput Whether the shader writes alpha.
 */
void initShaderKeyStatic(nn::g3d::ShadingModelObj* pShadingModel,
                         const nn::g3d::ResShaderAssign* pShaderAssign,
                         const nn::g3d::ModelObj* pModel, const nn::g3d::ShapeObj* pShape,
                         bool isEnableAlphaOutput) {
    pShadingModel->ClearStaticKey();

    s32 optionNum = pShaderAssign->GetShaderOptionCount();

    for (s32 i = 0; i < optionNum; i++) {
        s32 optionIndex = pShadingModel->FindStaticOptionIndex(pShaderAssign->GetOptionName(i));

        if (optionIndex < 0) {
            continue;
        }

        s32 choiceIndex = pShadingModel->GetResource()
                              ->GetStaticOption(optionIndex)
                              ->FindChoiceIndex(pShaderAssign->GetShaderOption(i));

        if (choiceIndex < 0) {
            continue;
        }

        pShadingModel->WriteStaticKey(optionIndex, choiceIndex);
    }

    s32 optionIndex = pShadingModel->FindStaticOptionIndex("cSkinWeightNum");

    if (optionIndex >= 0) {
        s32 choiceIndex =
            pShadingModel->GetResource()->GetStaticOption(optionIndex)->FindChoiceIndex(
                cSkinWeightNumNames[pShape->GetResource()->GetVertexSkinCount()]);

        if (choiceIndex >= 0) {
            pShadingModel->WriteStaticKey(optionIndex, choiceIndex);
        }
    }

    optionIndex = pShadingModel->FindStaticOptionIndex("cIsEnableAlphaOutput");

    if (optionIndex >= 0) {
        s32 choiceIndex =
            pShadingModel->GetResource()->GetStaticOption(optionIndex)->FindChoiceIndex(
                isEnableAlphaOutput ? "1" : "0");

        if (choiceIndex >= 0) {
            pShadingModel->WriteStaticKey(optionIndex, choiceIndex);
        }
    }

    pShadingModel->UpdateShaderRange();
    pShadingModel->CalculateOptionBlock(0);
}

/**
 * Initializes the dynamic shader key from the shader assignment and the shape, and selects the
 * matching shader program.
 * @param pSelector Shader selector.
 * @param pShaderAssign Shader assignment of the material.
 * @param pModel Model, unused.
 * @param pShape Shape.
 */
void initShaderKeyDynamic(nn::g3d::ShaderSelector* pSelector,
                          const nn::g3d::ResShaderAssign* pShaderAssign,
                          const nn::g3d::ModelObj* pModel, const nn::g3d::ShapeObj* pShape) {
    pSelector->ClearDynamicKey();

    s32 optionNum = pShaderAssign->GetShaderOptionCount();

    for (s32 i = 0; i < optionNum; i++) {
        s32 optionIndex = pSelector->FindDynamicOptionIndex(pShaderAssign->GetOptionName(i));

        if (optionIndex < 0) {
            continue;
        }

        s32 choiceIndex = pSelector->GetShadingModel()
                              ->GetResource()
                              ->GetDynamicOption(optionIndex)
                              ->FindChoiceIndex(pShaderAssign->GetShaderOption(i));

        if (choiceIndex < 0) {
            continue;
        }

        pSelector->WriteDynamicKey(optionIndex, choiceIndex);
    }

    s32 optionIndex = pSelector->FindDynamicOptionIndex("cSkinWeightNum");

    if (optionIndex >= 0) {
        s32 choiceIndex = pSelector->GetShadingModel()
                              ->GetResource()
                              ->GetDynamicOption(optionIndex)
                              ->FindChoiceIndex(
                                  cSkinWeightNumNames[pShape->GetResource()->GetVertexSkinCount()]);

        if (choiceIndex >= 0) {
            pSelector->WriteDynamicKey(optionIndex, choiceIndex);
        }
    }

    optionIndex = pSelector->FindDynamicOptionIndex("cIsEnableAlphaOutput");

    if (optionIndex >= 0) {
        s32 choiceIndex = pSelector->GetShadingModel()
                              ->GetResource()
                              ->GetDynamicOption(optionIndex)
                              ->FindChoiceIndex("0");

        if (choiceIndex >= 0) {
            pSelector->WriteDynamicKey(optionIndex, choiceIndex);
        }
    }

    pSelector->UpdateVariation(getGfxDevice());
}

/**
 * Searches the shader program matching a set of static option values.
 * @param pShadingModel Shading model.
 * @param macroNum Number of options.
 * @param pMacros Names of the options.
 * @param pValues Values of the options.
 * @return The shader program, or nullptr if none matches.
 */
const nn::g3d::ResShaderProgram* searchVariation(const nn::g3d::ResShadingModel* pShadingModel,
                                                 s32 macroNum, const char* const* pMacros,
                                                 const char* const* pValues) {
    return trySearchVariation(pShadingModel, macroNum, pMacros, pValues);
}

/**
 * Searches the shader program matching a set of static option values.
 * @param pShadingModel Shading model.
 * @param macroNum Number of options.
 * @param pMacros Names of the options.
 * @param pValues Values of the options.
 * @return The shader program, or nullptr if none matches.
 */
const nn::g3d::ResShaderProgram* trySearchVariation(const nn::g3d::ResShadingModel* pShadingModel,
                                                    s32 macroNum, const char* const* pMacros,
                                                    const char* const* pValues) {
    u32 key[32];
    pShadingModel->WriteDefaultStaticKey(key);
    pShadingModel->WriteDefaultDynamicKey(key + pShadingModel->GetStaticKeyLength());

    for (s32 i = 0; i < macroNum; i++) {
        const nn::g3d::ResShaderOption* option = pShadingModel->FindStaticOption(pMacros[i]);

        if (option != nullptr) {
            option->WriteStaticKey(key, option->FindChoiceIndex(pValues[i]));
        }
    }

    s32 programIndex = pShadingModel->FindProgramIndex(key);

    if (programIndex < 0) {
        return nullptr;
    }

    return pShadingModel->GetProgram(programIndex);
}

/**
 * Gets the value of a static option of a shading model.
 * @param pShadingModel Shading model object.
 * @param pName Name of the option.
 * @return The name of the selected choice, or an empty string if the option doesn't exist.
 */
const char* searchVariationMacroValue(const nn::g3d::ShadingModelObj* pShadingModel,
                                      const char* pName) {
    s32 optionIndex = pShadingModel->FindStaticOptionIndex(pName);

    if (optionIndex < 0) {
        return "";
    }

    const nn::g3d::ResShaderOption* option =
        pShadingModel->GetResource()->GetStaticOption(optionIndex);
    return option->GetChoiceName(pShadingModel->ReadStaticKey(optionIndex));
}

/**
 * Gets the value of a dynamic option of a shader selector, falling back to the static options.
 * @param pSelector Shader selector.
 * @param pName Name of the option.
 * @return The name of the selected choice, or an empty string if the option doesn't exist.
 */
const char* searchVariationMacroValue(const nn::g3d::ShaderSelector* pSelector,
                                      const char* pName) {
    const nn::g3d::ShadingModelObj* shadingModel = pSelector->GetShadingModel();
    s32 optionIndex = pSelector->FindDynamicOptionIndex(pName);

    if (optionIndex >= 0) {
        const nn::g3d::ResShaderOption* option =
            pSelector->GetShadingModel()->GetResource()->GetDynamicOption(optionIndex);
        return option->GetChoiceName(pSelector->ReadDynamicKey(optionIndex));
    }

    return searchVariationMacroValue(shadingModel, pName);
}

/**
 * Gets the name of the shading model of a shading model object.
 * @param pShadingModel Shading model object.
 * @return The name.
 */
const char* getShaderProgramName(const nn::g3d::ShadingModelObj* pShadingModel) {
    return pShadingModel->GetResource()->GetName();
}

}  // namespace al
