#include "Library/Model/ModelDrawerCubeMap.hpp"

#include <math/seadMathCalcCommon.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResMaterial.h>
#include <nn/g3d/g3d_ResShape.h>

#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/Model/MeshDrawer.hpp"
#include "Project/Model/ModelAdditionalInfoRenderCubeMap.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

/**
 * Constructs a cube map drawer.
 * @param pName Name of the drawer.
 */
ModelDrawerCubeMap::ModelDrawerCubeMap(const char* pName) : ModelDrawer(pName) {}

/**
 * Creates mesh drawers for all shapes using the cube map rendering shader options.
 */
void ModelDrawerCubeMap::createTable() {
    ModelDrawer::createTable();
    SimpleModelG3D* model = mModel->getModelG3D();
    s32 modelNum = mModelNumMax;
    const nn::g3d::ModelObj* modelObj = model->getModelObj();

    auto* table = new MeshDrawerTable;
    table->allocBuffer(modelObj->GetNumShapes(), nullptr);

    for (s32 i = 0; i < modelObj->GetNumShapes(); i++) {
        const nn::g3d::ShapeObj* shape = modelObj->GetShape(i);
        const nn::g3d::MaterialObj* material =
            modelObj->GetMaterial(shape->GetResource()->GetMaterialIndex());
        const nn::g3d::ResShaderAssign* shaderAssign = material->GetResource()->GetShaderAssign();
        nn::g3d::ResShadingModel* shadingModel =
            ShaderHolder::sInstance->getShadingModel(shaderAssign->GetShadingModelName());
        const char* optionNames[] = {"cRenderType", "cSkyColor0Type", "cExposureConnect"};
        const char* optionValues[] = {"3", "1", "0"};
        nn::g3d::ShaderSelector* selector =
            alModelFunction::createShaderSelector(model->mGpuMemAllocator, shape, material,
                                                  shadingModel, 3, optionNames, optionValues, false);
        table->insert(
            new MeshDrawer(shape->GetResource()->GetName(), modelObj, shape, selector, modelNum));
    }

    mMeshDrawerTable = table;

    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        mMeshDrawerTable->unsafeAt(i)->setForceDraw();
    }

    alModelFunction::createMeshDrawerTableDisplayList(mMeshDrawerTable, mModel, false);
}

/**
 * Keeps models registered, cube maps are drawn from all models.
 * @param pModel Model.
 */
void ModelDrawerCubeMap::removeModel(alModelCafe* pModel) {}

/**
 * Draws all meshes into the cube map.
 */
void ModelDrawerCubeMap::draw() const {
    if (!isDraw()) {
        return;
    }

    ModelAdditionalInfoRenderCubeMap additionalInfo(mGraphicsSystemInfo);

    for (s32 i = 0; i < mModelNum; i++) {
        mModels[i]->getModelG3D()->setModelAdditionalInfo(additionalInfo);
        SimpleModelG3D* model = mModels[i]->getModelG3D();
        s32 lodCount = model->getModelObj()->GetLodCount();
        model->mLodIndex = lodCount > 0 ? 0 : lodCount - 1;
    }

    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        MeshDrawer* meshDrawer = (*mMeshDrawerTable)[i];
        additionalInfo.activateModelLightTexture(meshDrawer->getMaterialObj()->GetResource());
        meshDrawer->draw(nullptr, 0, &additionalInfo);
    }
}

}  // namespace al
