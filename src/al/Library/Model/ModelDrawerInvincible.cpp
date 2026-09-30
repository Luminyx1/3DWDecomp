#include "Library/Model/ModelDrawerInvincible.hpp"

#include <driver/aglGraphicsDriverMgr.h>
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
 * Constructs a invincible drawer.
 * @param pName Name of the drawer.
 */
ModelDrawerInvincible::ModelDrawerInvincible(const char* pName) : ModelDrawer(pName) {
    mGraphicsContext.setPolygonOffsetFrontEnable(true);
    mGraphicsContext.setDepthEnable(true, false);
    mGraphicsContext.setDepthFunc(4);
    mGraphicsContext.setColorMask(0, true, true, true, false);
    mGraphicsContext.setBlendFactorDstRGB(3, 2);
    mGraphicsContext.setBlendFactorSrcRGB(3, 2);
    mGraphicsContext.setBlendEquationRGB(3, 1);
}

/**
 * Creates mesh drawers for all shapes using the invincible shader.
 */
void ModelDrawerInvincible::createTable() {
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
            ShaderHolder::sInstance->getShadingModel("RenderInvincible");
        nn::g3d::ShaderSelector* selector = alModelFunction::createShaderSelector(
            model->mGpuMemAllocator, shape, material, shadingModel, 0, nullptr, nullptr, false);
        table->insert(
            new MeshDrawer(shape->GetResource()->GetName(), modelObj, shape, selector, modelNum));
    }

    mMeshDrawerTable = table;
    alModelFunction::createMeshDrawerTableDisplayListRenderStateInvalidate(table, mModel, false);
}

/**
 * Draws the invincible meshes.
 */
void ModelDrawerInvincible::draw() const {
    if (!isDraw()) {
        return;
    }

    mGraphicsContext.apply(GameFrameworkNx::sInstance->mDrawContext);
    agl::driver::GraphicsDriverMgr::instance()->setPolygonOffset(
        reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext), 0.0f, -400.0f);
    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        MeshDrawer* meshDrawer = (*mMeshDrawerTable)[i];

        if (meshDrawer->isExistDrawMesh()) {
            meshDrawer->draw(&mGraphicsSystemInfo->mViewVolume, 0, nullptr);
        }
    }

    agl::driver::GraphicsDriverMgr::instance()->setPolygonOffset(
        reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext), 0.0f, 0.0f);
}

}  // namespace al
