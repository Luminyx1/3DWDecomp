#include "Library/Model/ModelDrawerDeferredSilhouette.hpp"

#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResShape.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/ShaderEnvTextureKeeper.hpp"
#include "Library/Shader/ForwardRendering/ShaderFresnelTextureKeeper.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/Model/MeshDrawer.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

/**
 * Constructs a deferred silhouette drawer.
 * @param pName Name of the drawer.
 * @param category Silhouette category drawn.
 */
ModelDrawerDeferredSilhouette::ModelDrawerDeferredSilhouette(const char* pName,
                                                             SilhouetteDrawCategory category)
    : ModelDrawer(pName), mCategory(category) {
    mGraphicsContext.setDepthEnable(true, false);
    mGraphicsContext.setDepthFunc(5);
    mGraphicsContext.setBlendEnable(true);
    mGraphicsContext.setBlendFactorSrcRGB(0, 9);
    mGraphicsContext.setBlendEquationRGB(0, 1);
    mGraphicsContext.setColorMask(0, false, false, false, true);
    mGraphicsContext.setCullingMode(2);
    mGraphicsContext.setBlendFactorSrcA(0, 2);
    mGraphicsContext.setBlendEquationA(0, 5);
    mGraphicsContext.setBlendFactorDstRGB(0, 1);
    mGraphicsContext.setBlendFactorDstA(0, 2);
}

/**
 * Creates mesh drawers for all shapes using the silhouette shader.
 */
void ModelDrawerDeferredSilhouette::createTable() {
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
            ShaderHolder::sInstance->getShadingModel("RenderSilhouette");
        const char* optionNames[] = {"cSilhouetteMode", "cIsEnableDitherAlpha"};
        const char* optionValues[] = {"4", "1"};
        nn::g3d::ShaderSelector* selector =
            alModelFunction::createShaderSelector(model->getGpuMemAllocator(), shape, material,
                                                  shadingModel, 2, optionNames, optionValues, false);
        table->insert(
            new MeshDrawer(shape->GetResource()->GetName(), modelObj, shape, selector, modelNum));
    }

    mMeshDrawerTable = table;
    alModelFunction::createMeshDrawerTableDisplayListRenderStateInvalidate(table, mModel, false);
}

/**
 * Draws the silhouette meshes.
 */
void ModelDrawerDeferredSilhouette::draw() const {
    if (!isDraw()) {
        return;
    }

    mGraphicsContext.apply(GameFrameworkNx::getDrawContext());
    ShaderFresnelTextureKeeper* fresnelTextureKeeper =
        mGraphicsSystemInfo->getShaderEnvTextureKeeper()->getFresnelTextureKeeper();
    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        MeshDrawer* meshDrawer = (*mMeshDrawerTable)[i];

        if (meshDrawer->isExistDrawMesh()) {
            fresnelTextureKeeper->activateSilhouetteCurveTexture(
                mCategory, getSamplerLocationSilhouetteCurve(), true);
            meshDrawer->draw(&mGraphicsSystemInfo->getViewVolume(), 0, nullptr);
        }
    }
}

}  // namespace al
