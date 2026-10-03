#include "Library/Model/Function/alModelFunction.hpp"

#include <cmath>

#include <common/aglDrawContext.h>
#include <common/aglGPUMemAddr.h>
#include <common/aglGPUMemBlock.h>
#include <common/aglShaderLocation.h>
#include <driver/aglGraphicsDriverMgr.h>
#include <g3d/aglShaderUtilG3D.h>
#include <gfx/seadGraphicsContextMRT.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResShader.h>
#include <nn/g3d/g3d_ShaderUtility.h>
#include <nn/g3d/g3d_ShapeObj.h>
#include <nn/g3d/g3d_SkeletonObj.h>
#include <nn/g3d/g3d_ViewVolume.h>
#include <prim/seadEnum.h>

#include "Library/Draw/GraphicsFunction.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Model/ModelShaderAssign.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Library/Shader/DeferredRendering/PeripheryRendering.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/RenderState.hpp"
#include "Project/Draw/GpuMemAllocator.hpp"
#include "Project/Model/MeshDrawer.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

// clang-format off
SEAD_ENUM(RoughnessType, Mirror , HighGlossy , MiddleGlossy , LowGlossy , Matte , Irradiance , TypeNum)
// clang-format on

}  // namespace al

namespace {

/**
 * @brief Source representation of a texture SRT shader parameter.
 */
struct TexSrt {
    u32 mode;
    f32 scaleX;
    f32 scaleY;
    f32 rotate;
    f32 translateX;
    f32 translateY;
};

const f32 sRoughnessTable[] = {0.0f, 0.1f, 0.3f, 0.5f, 1.0f, 0.0f};

/**
 * Converts a texture SRT into the 2x3 texture matrix used by the shaders.
 * @param pDst Converted matrix.
 * @param pSrc Texture SRT.
 * @param pParam Converted shader parameter.
 * @param pDependency Unused.
 * @return Size of the converted matrix.
 */
size_t convertTexMtxCallback(void* pDst, const void* pSrc, const nn::g3d::ResShaderParam* pParam,
                             const void* pDependency) {
    const TexSrt* srt = static_cast<const TexSrt*>(pSrc);
    f32* mtx = static_cast<f32*>(pDst);
    f32 sinR = std::sin(srt->rotate);
    f32 cosR = std::cos(srt->rotate);
    f32 sinHalf = sinR * 0.5f - 0.5f;
    f32 cosHalf = cosR * -0.5f;
    mtx[0] = cosR * srt->scaleX;
    mtx[1] = -(sinR * srt->scaleY);
    mtx[2] = sinR * srt->scaleX;
    mtx[3] = cosR * srt->scaleY;
    mtx[4] = (cosHalf - sinHalf) * srt->scaleX - srt->translateX * al::sgn(srt->scaleX);
    mtx[5] = (sinHalf + cosHalf) * srt->scaleY + srt->translateY * al::sgn(srt->scaleY) + 1.0f;
    return sizeof(f32) * 6;
}

/**
 * Gets the graphics device.
 * @return Graphics device.
 */
nn::gfx::Device* getGfxDevice() {
    return static_cast<nn::gfx::Device*>(
        agl::driver::GraphicsDriverMgr::instance()->getGfxDevice());
}

/**
 * Gets the command buffer of a draw context.
 * @param pContext Draw context.
 * @return Command buffer.
 */
nn::gfx::CommandBuffer* getCommandBuffer(agl::DrawContext* pContext) {
    return reinterpret_cast<nn::gfx::CommandBuffer*>(reinterpret_cast<uintptr_t>(pContext) + 8);
}

}  // namespace

namespace alModelFunction {

/**
 * Updates the roughness and wrap coefficient parameters of a material from its render infos.
 * @param pMaterial Material.
 */
void updateRenderMaterialUbo(nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResMaterial* resMaterial = pMaterial->GetResource();
    const nn::g3d::ResRenderInfo* roughnessInfo = resMaterial->FindRenderInfo("roughness_preset");
    const nn::g3d::ResRenderInfo* refractRoughnessInfo =
        resMaterial->FindRenderInfo("refract_roughness_preset");
    s32 reflectIndex = pMaterial->FindShaderParamIndex("uRoughnessMipReflect");
    s32 refractIndex = pMaterial->FindShaderParamIndex("uRoughnessMipRefract");

    if (reflectIndex != -1) {
        f32* roughnessMip = pMaterial->EditShaderParam<f32>(reflectIndex);
        *roughnessMip = getRoughnessPresetIndex(roughnessInfo);
    }

    if (refractIndex != -1) {
        f32* roughnessMip = pMaterial->EditShaderParam<f32>(refractIndex);
        *roughnessMip = getRoughnessPresetIndex(refractRoughnessInfo);
    }

    s32 roughnessIndex = pMaterial->FindShaderParamIndex("Roughness");

    if (roughnessIndex != -1) {
        f32* roughness = pMaterial->EditShaderParam<f32>(roughnessIndex);
        *roughness = sRoughnessTable[getRoughnessPresetIndex(roughnessInfo)];
    }

    s32 wrapCoefIndex = pMaterial->FindShaderParamIndex("WrapCoef");

    if (wrapCoefIndex != -1) {
        f32* wrapCoef = pMaterial->EditShaderParam<f32>(wrapCoefIndex);
        const nn::g3d::ResRenderInfo* info = resMaterial->FindRenderInfo("wrap_coef");
        f32 coef = info->GetFloat() != nullptr ? *info->GetFloat() : 1.0f;
        wrapCoef[0] = coef;
        wrapCoef[1] = 1.0 / (coef + 1.0);
    }
}

/**
 * Gets the roughness preset of a roughness render info.
 * @param pRenderInfo Render info holding the name of the preset.
 * @return Roughness preset index (Matte if the render info is empty or unknown).
 */
s32 getRoughnessPresetIndex(const nn::g3d::ResRenderInfo* pRenderInfo) {
    if (pRenderInfo->GetArrayLength() == 0) {
        return al::RoughnessType::Matte;
    }

    const char* name = pRenderInfo->GetString(0);

    s32 index = al::RoughnessType::Matte;

    for (s32 i = 0; i < al::RoughnessType::size(); i++) {
        if (al::isEqualString(name, al::RoughnessType::text(i))) {
            index = i;
            break;
        }
    }

    return al::RoughnessType(index);
}

/**
 * Updates the wrap coefficient parameter of a material from its render info.
 * @param pMaterial Material.
 */
void updateRenderCloudLayerUbo(nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResMaterial* resMaterial = pMaterial->GetResource();
    s32 wrapCoefIndex = pMaterial->FindShaderParamIndex("WrapCoef");

    if (wrapCoefIndex == -1) {
        return;
    }

    f32* wrapCoef = pMaterial->EditShaderParam<f32>(wrapCoefIndex);
    const nn::g3d::ResRenderInfo* info = resMaterial->FindRenderInfo("wrap_coef");
    f32 coef = info->GetFloat() != nullptr ? *info->GetFloat() : 1.0f;
    wrapCoef[0] = coef;
    wrapCoef[1] = 1.0 / (coef + 1.0);
}

/**
 * Applies the deferred rendering render state of a model shape to a draw context.
 * @param pContext Draw context.
 * @param pModel Model.
 * @param index Shape index.
 * @param isAlphaMask Whether the shape is alpha masked.
 * @param isXlu Whether the shape is translucent.
 */
void prepareModelShapeDrawDeferredGraphicsContext(agl::DrawContext* pContext,
                                                  const al::SimpleModelG3D* pModel, s32 index,
                                                  bool isAlphaMask, bool isXlu) {
    const nn::g3d::MaterialObj* material = pModel->getMaterialObj(index);
    sead::GraphicsContextMRT context;

    if (isXlu) {
        al::GBufferArray::setContextMRTXlu(&context);
    } else if (isAlphaMask) {
        al::GBufferArray::setContextMRTAlphaMask(&context);
    } else {
        al::GBufferArray::setContextMRT(&context);
    }

    al::setPolygonCtrlToContext(&context, material);
    al::setDepthCtrlToContext(&context, material);
    al::setAlphaTestToContext(&context, material);
    al::setPolygonOffsetToContext(pContext, &context, material, 0.0f);
    context.apply(pContext);
}

/**
 * Applies the deferred rendering render state of a model shape given by its render state resource.
 * @param pContext Draw context.
 * @param pModel Model.
 * @param index Shape index.
 */
void prepareModelShapeDrawDeferredGraphicsContextByRenderState(agl::DrawContext* pContext,
                                                               const al::SimpleModelG3D* pModel,
                                                               s32 index) {
    const al::RenderState* renderState = pModel->getResRenderState(index);
    bool isXlu;
    bool isAlphaMask;

    switch (renderState->getMode()) {
    case 0:
        isXlu = false;
        isAlphaMask = false;
        break;
    case 1:
        isXlu = false;
        isAlphaMask = true;
        break;
    case 2:
        isXlu = true;
        isAlphaMask = false;
        break;
    default:
        isAlphaMask = renderState->isAlphaTest();
        isXlu = renderState->getBlendMode() == 0;
        break;
    }

    prepareModelShapeDrawDeferredGraphicsContext(pContext, pModel, index, isAlphaMask, isXlu);
}

/**
 * Applies a custom deferred rendering blend state of a model shape to a draw context.
 * @param pContext Draw context.
 * @param pModel Model.
 * @param index Shape index.
 * @param blendType Blend type.
 */
void prepareModelShapeDrawDeferredGraphicsContextByCustom(agl::DrawContext* pContext,
                                                          const al::SimpleModelG3D* pModel,
                                                          s32 index, al::BlendType blendType) {
    sead::GraphicsContextMRT context;
    const nn::g3d::MaterialObj* material = pModel->getMaterialObj(index);

    switch (blendType) {
    case al::BlendType_Xlu:
        al::GBufferArray::setContextMRTCustom(&context, material, false);
        break;
    case al::BlendType_OnlyLightBuf:
        al::GBufferArray::setContextMRTCustom(&context, material, true);
        break;
    case al::BlendType_XluWithNrm:
        al::GBufferArray::setContextMRTXluNrm(&context);
        break;
    }

    context.apply(pContext);
}

/**
 * Draws a model shape.
 * @param pSkeleton Skeleton of the model.
 * @param pMaterial Material of the shape.
 * @param pShape Shape.
 * @param pShadingModel Shading model of the shape.
 * @param pShaderAssign Shader assignment of the shape.
 * @param pViewVolume View volume used to cull the sub meshes, or nullptr.
 * @param viewIndex View index.
 * @param bufferIndex Buffer index.
 * @param lodIndex LOD index.
 */
void drawModelShape(const nn::g3d::SkeletonObj* pSkeleton, const nn::g3d::MaterialObj* pMaterial,
                    const nn::g3d::ShapeObj* pShape, const nn::g3d::ShadingModelObj* pShadingModel,
                    const al::ModelShaderAssign* pShaderAssign,
                    const nn::g3d::ViewVolume* pViewVolume, s32 viewIndex, s32 bufferIndex,
                    s32 lodIndex) {
    if (pShape->GetResource()->GetMeshCount() <= lodIndex) {
        return;
    }

    agl::DrawContext* drawContext = al::GameFrameworkNx::getAglDrawContext();
    pShaderAssign->activateMaterialUniformBlock(drawContext, pMaterial, bufferIndex);
    size_t mtxBlockSize = pSkeleton->GetMtxBlockSize();

    if (mtxBlockSize != 0) {
        agl::ShaderLocation location(
            al::getUniformBlockLocationMdlMtx().getLocation(agl::cShaderType_Vertex));
        agl::g3d::ShaderUtilG3D::load(drawContext, location, *pSkeleton->GetMtxBlock(bufferIndex),
                                      mtxBlockSize, bufferIndex);
    }

    size_t shapeBlockSize = pShape->CalculateBlockBufferSize(getGfxDevice());

    if (shapeBlockSize != 0) {
        agl::ShaderLocation location(
            al::getUniformBlockLocationShp().getLocation(agl::cShaderType_Vertex));
        agl::g3d::ShaderUtilG3D::load(
            drawContext, location,
            static_cast<const nn::gfx::Buffer&>(*pShape->GetShapeBlock(viewIndex, bufferIndex)),
            shapeBlockSize, bufferIndex);
    }

    if (pShadingModel->IsBlockBufferValid()) {
        const nn::g3d::ResShadingModel* resShadingModel = pShadingModel->GetResource();
        s32 blockIndex = resShadingModel->GetOptionBlockIndex();
        const nn::g3d::ResShaderProgram* program = pShaderAssign->getResShaderProgram();
        s32 vertexLocation = program->GetUniformBlockLocation(blockIndex, nn::g3d::Stage_Vertex);
        s32 pixelLocation = program->GetUniformBlockLocation(blockIndex, nn::g3d::Stage_Pixel);
        size_t blockSize = blockIndex >= 0 ? resShadingModel->GetUniformBlockSize(blockIndex) : 0;

        if (vertexLocation >= 0) {
            agl::ShaderLocation location(vertexLocation);
            agl::g3d::ShaderUtilG3D::load(drawContext, location, *pShadingModel->GetOptionBlock(),
                                          blockSize, 0);
        }

        if (pixelLocation >= 0) {
            agl::ShaderLocation location(pixelLocation);
            agl::g3d::ShaderUtilG3D::load(drawContext, location, *pShadingModel->GetOptionBlock(),
                                          blockSize, 0);
        }
    }

    const agl::g3d::ModelShaderAttribute& attribute = pShaderAssign->getAttribute();
    attribute.activateVertexAttribute(drawContext);
    attribute.activateVertexBuffer(drawContext);

    if (pViewVolume != nullptr && isExistBoundingNode(pShape->GetResource())) {
        nn::g3d::CullingContext cullingContext;

        while (pShape->TestSubMeshIntersection(&cullingContext, *pViewVolume, lodIndex)) {
            pShape->GetResource()->GetMesh(lodIndex)->DrawSubMesh(
                getCommandBuffer(drawContext), cullingContext.submeshIndex,
                cullingContext.submeshCount, 1);
        }
    } else {
        const nn::g3d::ResMesh* resMesh = pShape->GetResource()->GetMesh(lodIndex);
        resMesh->DrawSubMesh(getCommandBuffer(drawContext), 0, resMesh->GetSubMeshCount(), 1);
    }
}

/**
 * Checks if a shape has sub mesh bounding nodes.
 * @param pShape Shape resource.
 * @return Whether the first mesh is split into several sub meshes.
 */
bool isExistBoundingNode(const nn::g3d::ResShape* pShape) {
    return pShape->GetMesh()->GetSubMeshCount() > 1;
}

/**
 * Checks if a model shape is visible and inside a view volume.
 * @param pModel Model.
 * @param index Shape index.
 * @param rViewVolume View volume.
 * @return Whether the shape needs to be drawn.
 */
bool isModelShapeDraw(const al::SimpleModelG3D* pModel, s32 index,
                      const nn::g3d::ViewVolume& rViewVolume) {
    if (!pModel->isVisible() || !pModel->isShapeVisible(index)) {
        return false;
    }

    return rViewVolume.TestIntersection(*pModel->getModelObj()->GetBounding());
}

/**
 * Creates a table of mesh drawers using the shaders assigned to the materials of a model.
 * @param pModel Model.
 * @param meshNum Maximum number of model instances per mesh drawer.
 * @return Mesh drawer table.
 */
al::MeshDrawerTable* createMeshDrawerTableFromAssignShader(const al::SimpleModelG3D* pModel,
                                                           s32 meshNum) {
    s32 shapeNum = pModel->getModelObj()->GetNumShapes();
    al::GpuMemAllocator* allocator = pModel->getGpuMemAllocator();
    al::MeshDrawerTable* table = new al::MeshDrawerTable();
    table->allocBuffer(shapeNum, nullptr);

    for (s32 i = 0; i < shapeNum; i++) {
        const nn::g3d::ModelObj* modelObj = pModel->getModelObj();
        const nn::g3d::ShapeObj* shape = modelObj->GetShape(i);
        const nn::g3d::MaterialObj* material =
            modelObj->GetMaterial(shape->GetResource()->GetMaterialIndex());
        nn::g3d::ShaderSelector* selector =
            createShaderSelectorFromAssignShader(allocator, shape, material);
        table->insert(new al::MeshDrawer(shape->GetResource()->GetName(), pModel->getModelObj(),
                                         shape, selector, meshNum));
    }

    return table;
}

/**
 * Creates a shader selector using the shading model assigned to a material.
 * @param pAllocator GPU memory allocator.
 * @param pShape Shape.
 * @param pMaterial Material.
 * @return Shader selector.
 */
nn::g3d::ShaderSelector* createShaderSelectorFromAssignShader(al::GpuMemAllocator* pAllocator,
                                                              const nn::g3d::ShapeObj* pShape,
                                                              const nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResShaderAssign* shaderAssign = pMaterial->GetResource()->GetShaderAssign();
    nn::g3d::ResShadingModel* shadingModel =
        al::ShaderHolder::sInstance->getShadingModel(shaderAssign->GetShadingModelName());
    return createShaderSelector(pAllocator, pShape, pMaterial, shadingModel, 0, nullptr, nullptr,
                                false);
}

/**
 * Checks if the materials of a model need to be activated by the display lists.
 * @param pModel Model.
 * @param shapeIndex Shape index.
 * @return Whether the material is animated or uses a fixed material block.
 */
static bool isActivateMaterial(const alModelCafe* pModel, s32 shapeIndex) {
    return pModel->getModelG3D()->getShape(shapeIndex)._2a ||
           pModel->getAnimPlayerMat2() != nullptr || pModel->getAnimPlayerMat0() != nullptr;
}

/**
 * Creates the display lists of all mesh drawers of a table.
 * @param pTable Mesh drawer table.
 * @param pModel Model.
 * @param isUseUniformRegister Passed to the display list creation.
 */
void createMeshDrawerTableDisplayList(const al::MeshDrawerTable* pTable, const alModelCafe* pModel,
                                      bool isUseUniformRegister) {
    for (s32 i = 0; i < pTable->size(); i++) {
        al::MeshDrawer* drawer = pTable->at(i);
        al::GpuMemAllocator* allocator = pModel->getModelG3D()->getGpuMemAllocator();
        drawer->createDisplayList(
            allocator,
            al::MeshDrawer::RENDER_STATE_ACTIVATE_TYPE(
                !pModel->getModelG3D()->isCreateResRenderState(drawer->getShapeIndex())),
            al::MeshDrawer::TEXTURE_ACTIVATE_TYPE(pModel->getAnimPlayerMat1() == nullptr),
            al::MeshDrawer::MATERIAL_ACTIVATE_TYPE(
                !isActivateMaterial(pModel, drawer->getShapeIndex())),
            isUseUniformRegister);
    }
}

/**
 * Creates the display lists of all mesh drawers of a table without render state activation.
 * @param pTable Mesh drawer table.
 * @param pModel Model.
 * @param isUseUniformRegister Passed to the display list creation.
 */
void createMeshDrawerTableDisplayListRenderStateInvalidate(const al::MeshDrawerTable* pTable,
                                                           const alModelCafe* pModel,
                                                           bool isUseUniformRegister) {
    for (s32 i = 0; i < pTable->size(); i++) {
        al::MeshDrawer* drawer = pTable->at(i);
        drawer->createDisplayList(
            pModel->getModelG3D()->getGpuMemAllocator(),
            al::MeshDrawer::RENDER_STATE_ACTIVATE_TYPE(3),
            al::MeshDrawer::TEXTURE_ACTIVATE_TYPE(pModel->getAnimPlayerMat1() == nullptr),
            al::MeshDrawer::MATERIAL_ACTIVATE_TYPE(
                !isActivateMaterial(pModel, drawer->getShapeIndex())),
            isUseUniformRegister);
    }
}

/**
 * Gets the material of a model shape.
 * @param pModel Model.
 * @param index Shape index.
 * @return Material of the shape.
 */
static const nn::g3d::MaterialObj* getShapeMaterial(const nn::g3d::ModelObj* pModel, s32 index) {
    return pModel->GetMaterial(pModel->GetShape(index)->GetResource()->GetMaterialIndex());
}

/**
 * Checks if the shader assigned to a model shape is alpha masked.
 * @param pModel Model.
 * @param index Shape index.
 * @return Whether the shader is alpha masked.
 */
bool isShaderAssignAlphaMask(const nn::g3d::ModelObj* pModel, s32 index) {
    return isShaderAssignAlphaMask(getShapeMaterial(pModel, index));
}

/**
 * Checks if the shader assigned to a material is alpha masked.
 * @param pMaterial Material.
 * @return Whether the shader is alpha masked.
 */
bool isShaderAssignAlphaMask(const nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResShaderAssign* shaderAssign = pMaterial->GetResource()->GetShaderAssign();
    const char* alphaMaskType = shaderAssign->FindShaderOption("AlphaMaskType");

    if (alphaMaskType != nullptr &&
        (al::isEqualString("1", alphaMaskType) || al::isEqualString("2", alphaMaskType))) {
        return true;
    }

    if (al::isEqualString("RenderMaterialAlphaMask", shaderAssign->GetShadingModelName())) {
        return true;
    }

    return al::isEqualString("RenderMaterialEcho", shaderAssign->GetShadingModelName());
}

/**
 * Gets the alpha test function option of the shader assigned to a model shape.
 * @param pModel Model.
 * @param index Shape index.
 * @return Alpha test function option value, or nullptr.
 */
const char* getShaderAssignAlphaFunc(const nn::g3d::ModelObj* pModel, s32 index) {
    return getShaderAssignAlphaFunc(getShapeMaterial(pModel, index));
}

/**
 * Gets the alpha test function option of the shader assigned to a material.
 * @param pMaterial Material.
 * @return Alpha test function option value, or nullptr.
 */
const char* getShaderAssignAlphaFunc(const nn::g3d::MaterialObj* pMaterial) {
    return pMaterial->GetResource()->GetShaderAssign()->FindShaderOption("alpha_test_func");
}

/**
 * Checks if a shading model uses the thickness of the model.
 * @param rShadingModel Shading model.
 * @return Whether the refraction, render or emission type uses the thickness.
 */
bool isShaderUsingThickness(const nn::g3d::ShadingModelObj& rShadingModel) {
    if (al::isEqualString("3", al::searchVariationMacroValue(&rShadingModel, "cRefractionType")) ||
        al::isEqualString("4", al::searchVariationMacroValue(&rShadingModel, "cRefractionType")) ||
        al::isEqualString("6", al::searchVariationMacroValue(&rShadingModel, "cRefractionType"))) {
        return true;
    }

    if (al::isEqualString("2", al::searchVariationMacroValue(&rShadingModel, "cRenderType")) ||
        al::isEqualString("4", al::searchVariationMacroValue(&rShadingModel, "cRenderType")) ||
        al::isEqualString("6", al::searchVariationMacroValue(&rShadingModel, "cRenderType"))) {
        return true;
    }

    if (al::isEqualString("2", al::searchVariationMacroValue(&rShadingModel, "EmissionType")) ||
        al::isEqualString("4", al::searchVariationMacroValue(&rShadingModel, "EmissionType"))) {
        return true;
    }

    return al::isEqualString("8", al::searchVariationMacroValue(&rShadingModel, "EmissionType"));
}

/**
 * Checks if a shading model uses indirect refraction.
 * @param rShadingModel Shading model.
 * @return Whether the shading model is indirect.
 */
bool isShaderIndirect(const nn::g3d::ShadingModelObj& rShadingModel) {
    if (al::isEqualString("2", al::searchVariationMacroValue(&rShadingModel, "cRefractionType")) ||
        al::isEqualString("4", al::searchVariationMacroValue(&rShadingModel, "cRefractionType"))) {
        return true;
    }

    return al::isEqualSubString(al::getShaderProgramName(&rShadingModel), "Indirect");
}

/**
 * Checks if a shading model uses the refraction texture.
 * @param rShadingModel Shading model.
 * @return Whether the refraction type uses the refraction texture.
 */
bool isShaderUsingRefractTex(const nn::g3d::ShadingModelObj& rShadingModel) {
    if (al::isEqualString("5", al::searchVariationMacroValue(&rShadingModel, "cRefractionType")) ||
        al::isEqualString("6", al::searchVariationMacroValue(&rShadingModel, "cRefractionType"))) {
        return true;
    }

    return al::isEqualString("7", al::searchVariationMacroValue(&rShadingModel, "cRefractionType"));
}

/**
 * Checks if a material uses a model light preset.
 * @param pMaterial Material resource.
 * @return Whether the model light preset option is set and non-zero.
 */
bool isMaterialUsingModelLight(const nn::g3d::ResMaterial* pMaterial) {
    const char* preset = pMaterial->GetShaderAssign()->FindShaderOption("cUsingModelLightPreset");

    if (preset == nullptr) {
        return false;
    }

    return !al::isEqualString(preset, "0");
}

/**
 * Gets the draw priority of a material.
 * @param pMaterial Material resource.
 * @return Draw priority render info value, or 0.
 */
s32 getMaterialDrawPriority(const nn::g3d::ResMaterial* pMaterial) {
    const nn::g3d::ResRenderInfo* info = pMaterial->FindRenderInfo("draw_priority");

    if (info == nullptr || info->GetInt() == nullptr) {
        return 0;
    }

    return *info->GetInt();
}

/**
 * Checks the render state application mode of a material.
 * @param pMaterial Material resource.
 * @param pMode Mode to check.
 * @return Whether the apply_render_state render info equals the mode.
 */
static bool isMaterialApplyRenderStateMode(const nn::g3d::ResMaterial* pMaterial,
                                           const char* pMode) {
    const nn::g3d::ResRenderInfo* info = pMaterial->FindRenderInfo("apply_render_state");

    if (info == nullptr) {
        return false;
    }

    return al::isEqualString(info->GetString(0), pMode);
}

/**
 * Checks if a material applies its render state.
 * @param pMaterial Material resource.
 * @return Whether the material applies its render state.
 */
bool isMaterialApplyRenderState(const nn::g3d::ResMaterial* pMaterial) {
    return isMaterialApplyRenderStateMode(pMaterial, "applyRenderState");
}

/**
 * Checks if a material is translucent blended with normals.
 * @param pMaterial Material resource.
 * @return Whether the material blends with normals.
 */
bool isMaterialXluWithNrm(const nn::g3d::ResMaterial* pMaterial) {
    return isMaterialApplyRenderStateMode(pMaterial, "blendWithNormal");
}

/**
 * Checks if a material only blends into the light buffer.
 * @param pMaterial Material resource.
 * @return Whether the material only blends into the light buffer.
 */
bool isMaterialOnlyBlendLightBuf(const nn::g3d::ResMaterial* pMaterial) {
    return isMaterialApplyRenderStateMode(pMaterial, "blendOnlyLightBuf");
}

/**
 * Binds the shader parameters of all materials of a model and sets the texture matrix conversion
 * callbacks.
 * @param pModel Model resource.
 * @param pShaderHolder Shader holder providing the shading models.
 */
void bindShaderParamAndConvertParamCallback(nn::g3d::ResModel* pModel,
                                            const al::ShaderHolder* pShaderHolder) {
    if (pModel->GetUserPtr() == pShaderHolder) {
        return;
    }

    pModel->SetUserPtr(const_cast<al::ShaderHolder*>(pShaderHolder));

    for (s32 i = 0; i < pModel->GetMaterialCount(); i++) {
        nn::g3d::ResMaterial* material = pModel->GetMaterial(i);
        const nn::g3d::ResShadingModel* shadingModel =
            pShaderHolder->getShadingModel(material->GetShaderAssign()->GetShadingModelName());
        nn::g3d::ShaderUtility::BindShaderParam(material, shadingModel);
        const nn::g3d::ResUniformBlock* block = shadingModel->FindUniformBlock("cMat");

        if (block == nullptr) {
            continue;
        }

        for (s32 j = 0; j < block->GetUniformCount(); j++) {
            if (!al::isEqualSubString(block->GetUniformName(j), "cTexMtx")) {
                continue;
            }

            nn::g3d::ResShaderParam* param = material->FindShaderParam(block->GetUniformName(j));

            if (param != nullptr) {
                param->SetConvertCallback(convertTexMtxCallback);
            }
        }
    }
}

/**
 * Checks if the shader assigned to a model shape is translucent.
 * @param pModel Model.
 * @param index Shape index.
 * @return Whether the shader is translucent.
 */
bool isShaderAssignXlu(const nn::g3d::ModelObj* pModel, s32 index) {
    return isShaderAssignXlu(getShapeMaterial(pModel, index));
}

/**
 * Checks if the shader assigned to a material is translucent.
 * @param pMaterial Material.
 * @return Whether the shader is translucent.
 */
bool isShaderAssignXlu(const nn::g3d::MaterialObj* pMaterial) {
    const char* renderType =
        pMaterial->GetResource()->GetShaderAssign()->FindShaderOption("cRenderType");

    if (renderType == nullptr) {
        return false;
    }

    if (al::isEqualString("1", renderType) || al::isEqualString("2", renderType) ||
        al::isEqualString("4", renderType)) {
        return true;
    }

    return al::isEqualString("5", renderType);
}

/**
 * Checks if the shader assigned to a model shape is drawn by deferred rendering.
 * @param pModel Model.
 * @param index Shape index.
 * @return Whether the shader is deferred.
 */
bool isShaderAssignDeferred(const nn::g3d::ModelObj* pModel, s32 index) {
    return isShaderAssignDeferred(getShapeMaterial(pModel, index));
}

/**
 * Checks if the shader assigned to a material is drawn by deferred rendering.
 * @param pMaterial Material.
 * @return Whether the shader is deferred.
 */
bool isShaderAssignDeferred(const nn::g3d::MaterialObj* pMaterial) {
    const char* renderType =
        pMaterial->GetResource()->GetShaderAssign()->FindShaderOption("cRenderType");

    if (renderType == nullptr) {
        return true;
    }

    if (al::isEqualString("0", renderType) || al::isEqualString("1", renderType) ||
        al::isEqualString("2", renderType) || al::isEqualString("5", renderType)) {
        return true;
    }

    return al::isEqualString("6", renderType);
}

/**
 * Checks if the shader assigned to a model shape is drawn by forward rendering.
 * @param pModel Model.
 * @param index Shape index.
 * @return Whether the shader is forward.
 */
bool isShaderAssignForward(const nn::g3d::ModelObj* pModel, s32 index) {
    return isShaderAssignForward(getShapeMaterial(pModel, index));
}

/**
 * Checks if the shader assigned to a material is drawn by forward rendering.
 * @param pMaterial Material.
 * @return Whether the shader is forward.
 */
bool isShaderAssignForward(const nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResShaderAssign* shaderAssign = pMaterial->GetResource()->GetShaderAssign();

    if (al::isEqualString("PostEffectMask", shaderAssign->GetShadingModelName())) {
        return true;
    }

    if (al::isEqualString("alRenderCloudLayer", shaderAssign->GetShadingModelName())) {
        return true;
    }

    return !isShaderAssignDeferred(pMaterial);
}

/**
 * Creates the shader assignment of a shape with the program selected by static options.
 * @param pShape Shape.
 * @param pMaterial Material.
 * @param pShadingModel Shading model.
 * @param optionNum Number of static options.
 * @param pOptionNames Names of the static options.
 * @param pOptionValues Values of the static options.
 * @return Shader assignment.
 */
al::ModelShaderAssign* createModelShaderAssign(const nn::g3d::ShapeObj* pShape,
                                               const nn::g3d::MaterialObj* pMaterial,
                                               nn::g3d::ResShadingModel* pShadingModel,
                                               s32 optionNum, const char* const* pOptionNames,
                                               const char* const* pOptionValues) {
    const nn::g3d::ResShape* resShape = pShape->GetResource();
    const nn::g3d::ResMaterial* resMaterial = pMaterial->GetResource();
    u32 key[32];
    nn::g3d::ShaderUtility::InitializeShaderKey(key, 32, pShadingModel,
                                                resMaterial->GetShaderAssign(), false);
    nn::g3d::ResShaderOption* skinWeightNum = pShadingModel->FindStaticOption("cSkinWeightNum");

    if (skinWeightNum != nullptr) {
        skinWeightNum->WriteStaticKey(key, resShape->GetVertexSkinCount());
    }

    for (s32 i = 0; i < optionNum; i++) {
        nn::g3d::ResShaderOption* option = pShadingModel->FindStaticOption(pOptionNames[i]);

        if (option != nullptr) {
            option->WriteStaticKey(key, option->FindChoiceIndex(pOptionValues[i]));
        }
    }

    nn::g3d::ResShaderProgram* program =
        pShadingModel->GetProgram(pShadingModel->FindProgramIndex(key));
    al::ModelShaderAssign* shaderAssign = new al::ModelShaderAssign();
    program->Setup(getGfxDevice());
    program->Update(getGfxDevice());
    shaderAssign->create(nullptr);
    shaderAssign->bind(resMaterial, resShape, pShadingModel, program);
    return shaderAssign;
}

/**
 * Creates a shader selector of a shape with static and dynamic options.
 * @param pAllocator GPU memory allocator.
 * @param pShape Shape.
 * @param pMaterial Material.
 * @param pShadingModel Shading model.
 * @param optionNum Number of options.
 * @param pOptionNames Names of the options.
 * @param pOptionValues Values of the options.
 * @param isUseDefault Passed to the static shader key initialization.
 * @return Shader selector.
 */
nn::g3d::ShaderSelector* createShaderSelector(al::GpuMemAllocator* pAllocator,
                                              const nn::g3d::ShapeObj* pShape,
                                              const nn::g3d::MaterialObj* pMaterial,
                                              nn::g3d::ResShadingModel* pShadingModel,
                                              s32 optionNum, const char* const* pOptionNames,
                                              const char* const* pOptionValues,
                                              bool isUseDefault) {
    nn::g3d::ShadingModelObj* shadingModelObj = new nn::g3d::ShadingModelObj();
    {
        nn::g3d::ShadingModelObj::Builder builder(pShadingModel);
        builder.CalculateMemorySize();
        size_t memorySize = builder.GetWorkMemorySize();
        builder.Build(shadingModelObj, new (8) u8[memorySize], memorySize);
    }

    size_t blockBufferSize = shadingModelObj->CalculateBlockBufferSize(getGfxDevice());
    s32 alignment = shadingModelObj->GetBlockBufferAlignment(getGfxDevice());

    if (blockBufferSize == 0) {
        blockBufferSize = alignment;
    }

    agl::GPUMemAddrBase memAddr =
        pAllocator->allocMemory("ShaderOptionUBO", blockBufferSize, alignment);
    nn::gfx::MemoryPool* memoryPool = pAllocator->allocMemoryPool();
    memAddr.getMemoryBlock()->initializeGfxMemoryPool(memoryPool);
    shadingModelObj->SetupBlockBuffer(getGfxDevice(), memoryPool, memAddr.getByteOffset(),
                                      blockBufferSize);
    pAllocator->registerShadingModelObj(shadingModelObj);
    al::initShaderKeyStatic(shadingModelObj, pMaterial->GetResource()->GetShaderAssign(), nullptr,
                            pShape, isUseDefault);

    for (s32 i = 0; i < optionNum; i++) {
        s32 optionIndex =
            shadingModelObj->GetResource()->FindStaticOptionIndex(pOptionNames[i]);

        if (optionIndex >= 0) {
            s32 choiceIndex = shadingModelObj->GetResource()
                                  ->GetStaticOption(optionIndex)
                                  ->FindChoiceIndex(pOptionValues[i]);

            if (choiceIndex >= 0) {
                shadingModelObj->WriteStaticKey(optionIndex, choiceIndex);
            }
        }
    }

    shadingModelObj->UpdateShaderRange();

    nn::g3d::ShaderSelector* selector = new nn::g3d::ShaderSelector();
    {
        nn::g3d::ShaderSelector::Builder builder(shadingModelObj);
        builder.CalculateMemorySize();
        size_t memorySize = builder.GetWorkMemorySize();
        builder.Build(selector, new (8) u8[memorySize], memorySize);
    }
    al::initShaderKeyDynamic(selector, pMaterial->GetResource()->GetShaderAssign(), nullptr,
                             pShape);

    for (s32 i = 0; i < optionNum; i++) {
        const nn::g3d::ResShadingModel* resShadingModel =
            selector->GetShadingModel()->GetResource();
        s32 optionIndex = resShadingModel->FindDynamicOptionIndex(pOptionNames[i]);

        if (optionIndex < 0) {
            continue;
        }

        s32 choiceIndex =
            resShadingModel->GetDynamicOption(optionIndex)->FindChoiceIndex(pOptionValues[i]);

        if (choiceIndex < 0) {
            continue;
        }

        selector->WriteDynamicKey(optionIndex, choiceIndex);
    }

    if (selector->GetProgram() == nullptr) {
        // The result of this check is unused.
        al::isEqualSubString(pShadingModel->GetName(), "RenderSky");
        nn::g3d::ResShadingModel* uberShadingModel =
            al::ShaderHolder::sInstance->getShadingModelUber(pShadingModel->GetName());
        return createShaderSelector(pAllocator, pShape, pMaterial, uberShadingModel, optionNum,
                                    pOptionNames, pOptionValues, isUseDefault);
    }

    selector->UpdateVariation(getGfxDevice());
    selector->GetShadingModel()->CalculateOptionBlock(0);
    return selector;
}

}  // namespace alModelFunction
