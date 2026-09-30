#include "Library/Model/ModelDrawerHeightMap.hpp"

#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResShape.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/Model/MeshDrawer.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

/**
 * Constructs a height map drawer.
 * @param pName Name of the drawer.
 */
ModelDrawerHeightMap::ModelDrawerHeightMap(const char* pName) : ModelDrawer(pName) {
    mGraphicsContext.setBlendEquationRGB(0, 1);
    mGraphicsContext.setBlendFactorDstRGB(0, 1);
    mGraphicsContext.setBlendEnable(false);
    mGraphicsContext.setDepthEnable(false, false);
    mGraphicsContext.setBlendFactorSrcA(0, 2);
    mGraphicsContext.setCullingMode(0);
    mGraphicsContext.setBlendFactorDstA(0, 2);
    mGraphicsContext.setBlendFactorSrcRGB(0, 9);
    mGraphicsContext.setDepthFunc(5);
    mGraphicsContext.setBlendEquationA(0, 5);
}

/**
 * Creates mesh drawers for all shapes using the height map shader.
 */
void ModelDrawerHeightMap::createTable() {
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
        nn::g3d::ResShadingModel* shadingModel =
            ShaderHolder::sInstance->getShadingModel("RenderHeightMap");
        nn::g3d::ShaderSelector* selector = alModelFunction::createShaderSelector(
            model->mGpuMemAllocator, shape, material, shadingModel, 0, nullptr, nullptr, false);
        table->insert(
            new MeshDrawer(shape->GetResource()->GetName(), modelObj, shape, selector, modelNum));
    }

    mMeshDrawerTable = table;
    alModelFunction::createMeshDrawerTableDisplayListRenderStateInvalidate(table, mModel, false);
}

/**
 * Adds a model to this drawer.
 * @param pModel Model.
 */
void ModelDrawerHeightMap::addModel(alModelCafe* pModel) {
    ModelDrawer::addModel(pModel);
}

/**
 * Draws the height map meshes.
 */
void ModelDrawerHeightMap::draw() const {
    if (!isDraw()) {
        return;
    }

    mGraphicsContext.apply(GameFrameworkNx::sInstance->mDrawContext);
    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        MeshDrawer* meshDrawer = (*mMeshDrawerTable)[i];
        if (meshDrawer->isExistDrawMesh()) {
            meshDrawer->draw(&mGraphicsSystemInfo->mViewVolume, 0, nullptr);
        }
    }
}

}  // namespace al
