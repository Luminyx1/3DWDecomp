#include "Project/Model/ModelAdditionalInfoRenderCubeMap.hpp"

#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResShape.h>

#include "Library/Shader/DeferredRendering/ModelLightParam.hpp"
#include "Library/Shader/ForwardRendering/ShaderEnvTextureKeeper.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

/**
 * Constructs the additional info used while rendering cube maps.
 * @param pInfo Graphics system info.
 */
ModelAdditionalInfoRenderCubeMap::ModelAdditionalInfoRenderCubeMap(const GraphicsSystemInfo* pInfo)
    : ModelAdditionalInfo(pInfo) {
    updateLightInfo();
    mIsRenderCubeMap = true;
}

/**
 * Activates the environment and model light textures of a shape.
 * @param shapeIndex Shape index.
 * @param pModel Model.
 */
void ModelAdditionalInfo::activateEnvTextureIndividual(s32 shapeIndex,
                                                       const SimpleModelG3D* pModel) {
    const EnvTexInfo* envTexInfo = pModel->getShape(shapeIndex).mEnvTexInfo;
    mGraphicsSystemInfo->mShaderEnvTextureKeeper->activateEnvTexture(*envTexInfo, this, true);
    nn::g3d::ModelObj* modelObj = pModel->getModelObj();
    s32 materialIndex = modelObj->GetShape(shapeIndex)->GetResource()->GetMaterialIndex();
    const nn::g3d::ResMaterial* material = modelObj->GetMaterial(materialIndex)->GetResource();
    mGraphicsSystemInfo->mModelLightDirector->activateModelLightTexture(material, envTexInfo, true);
}

/**
 * Activates the environment texture of a shape.
 * @param shapeIndex Shape index.
 * @param pModel Model.
 */
void ModelAdditionalInfo::activateEnvTexture(s32 shapeIndex, const SimpleModelG3D* pModel) {
    const EnvTexInfo* envTexInfo = pModel->getShape(shapeIndex).mEnvTexInfo;
    mGraphicsSystemInfo->mShaderEnvTextureKeeper->activateEnvTexture(*envTexInfo, this, true);
}

/**
 * Activates the model light texture of a shape.
 * @param shapeIndex Shape index.
 * @param pModel Model.
 */
void ModelAdditionalInfo::activateModelLightTexture(s32 shapeIndex, const SimpleModelG3D* pModel) {
    nn::g3d::ModelObj* modelObj = pModel->getModelObj();
    const EnvTexInfo* envTexInfo = pModel->getShape(shapeIndex).mEnvTexInfo;
    s32 materialIndex = modelObj->GetShape(shapeIndex)->GetResource()->GetMaterialIndex();
    const nn::g3d::ResMaterial* material = modelObj->GetMaterial(materialIndex)->GetResource();
    mGraphicsSystemInfo->mModelLightDirector->activateModelLightTexture(material, envTexInfo, true);
}

/**
 * Activates the model light texture of a material without environment info.
 * @param pMaterial Material resource.
 */
void ModelAdditionalInfo::activateModelLightTexture(const nn::g3d::ResMaterial* pMaterial) {
    mGraphicsSystemInfo->mModelLightDirector->activateModelLightTexture(pMaterial, nullptr, false);
}

/**
 * Activates a model light texture by index.
 * @param index Model light texture index.
 */
void ModelAdditionalInfo::activateModelLightTexture(s32 index) {
    mGraphicsSystemInfo->mModelLightDirector->activateModelLightTexture(index);
}

}  // namespace al
