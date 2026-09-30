#include "Library/Model/ModelDrawerDeferredEcho.hpp"

#include <common/aglShaderLocation.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResShape.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/Model/MeshDrawer.hpp"
#include "Project/Model/ModelAdditionalInfo.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

/**
 * Constructs a deferred echo drawer.
 * @param pName Name of the drawer.
 * @param category Echo category drawn.
 */
ModelDrawerDeferredEcho::ModelDrawerDeferredEcho(const char* pName, EchoDrawCategory category)
    : ModelDrawer(pName), mCategory(category) {
    GBufferArray::setContextMRTAlphaMask(&mGraphicsContext);
}

/**
 * Creates mesh drawers for all shapes using the echo shader.
 */
void ModelDrawerDeferredEcho::createTable() {
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
            ShaderHolder::sInstance->getShadingModel("RenderMaterialEcho");
        const char* optionNames[] = {"cEchoMode"};
        const char* optionValues[] = {"4"};
        nn::g3d::ShaderSelector* selector =
            alModelFunction::createShaderSelector(model->mGpuMemAllocator, shape, material,
                                                  shadingModel, 1, optionNames, optionValues, false);
        table->insert(
            new MeshDrawer(shape->GetResource()->GetName(), modelObj, shape, selector, modelNum));
    }

    mMeshDrawerTable = table;
    alModelFunction::createMeshDrawerTableDisplayListRenderStateInvalidate(table, mModel, false);
}

/**
 * Draws the echo meshes with the echo emitter uniform block.
 */
void ModelDrawerDeferredEcho::draw() const {
    if (!isDraw()) {
        return;
    }

    mGraphicsContext.apply(GameFrameworkNx::sInstance->mDrawContext);
    ModelAdditionalInfo additionalInfo(mGraphicsSystemInfo, false);

    for (s32 i = 0; i < mModelNum; i++) {
        SimpleModelG3D* model = mModels[i]->getModelG3D();
        model->setModelAdditionalInfo(additionalInfo);

        if (!model->mIsLodDisabled) {
            s32 updateCount = mGraphicsSystemInfo->mDrawEnvUpdateCount;

            if (model->mLodUpdateCount != updateCount) {
                model->updateLod(mGraphicsSystemInfo->mDrawCameraPos, updateCount);
            }
        }
    }

    const UniformBlock* uniformBlock =
        (*mGraphicsSystemInfo->getViewIndexedUboArray("EchoBlockEmitterUbo"))[0];
    uniformBlock->activate(
        reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext),
        getUniformBlockLocationEchoBlock());

    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        MeshDrawer* meshDrawer = (*mMeshDrawerTable)[i];

        if (meshDrawer->isExistDrawMesh()) {
            additionalInfo.activateModelLightTexture(meshDrawer->getMaterialObj()->GetResource());
            alModelFunction::prepareModelShapeDrawDeferredGraphicsContextByRenderState(
                reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext),
                mModels[0]->getModelG3D(), meshDrawer->getShapeIndex());
            meshDrawer->draw(&mGraphicsSystemInfo->mViewVolume, 0, &additionalInfo);
        }
    }
}

}  // namespace al
