#include "Library/Model/ModelDrawerForward.hpp"

#include <gfx/seadGraphicsContext.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResMaterial.h>
#include <nn/g3d/g3d_ResShape.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/Model/MeshDrawer.hpp"
#include "Project/Model/ModelAdditionalInfo.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

/**
 * Constructs a forward model drawer.
 * @param pName Name of the drawer.
 * @param isSecondCategory Whether models use cube map category 1.
 * @param isAssignShader Whether the shaders assigned to the materials are used.
 * @param isRenderStateInvalidate Whether the display lists leave the render state untouched.
 * @param isRemoveable Whether the drawer can be removed.
 */
ModelDrawerForward::ModelDrawerForward(const char* pName, bool isSecondCategory,
                                       bool isAssignShader, bool isRenderStateInvalidate,
                                       bool isRemoveable)
    : ModelDrawer(pName), mIsSecondCategory(isSecondCategory), mIsAssignShader(isAssignShader),
      mIsRenderStateInvalidate(isRenderStateInvalidate), mIsRemoveable(isRemoveable) {}

/**
 * Creates the mesh drawers of all forward shapes and their display lists.
 */
void ModelDrawerForward::createTable() {
    ModelDrawer::createTable();
    SimpleModelG3D* model = mModel->getModelG3D();
    const nn::g3d::ModelObj* modelObj = model->getModelObj();

    if (mIsAssignShader) {
        s32 meshNum = 0;

        for (s32 i = 0; i < modelObj->GetNumShapes(); i++) {
            if (alModelFunction::isShaderAssignForward(modelObj, i) ||
                alModelFunction::isShaderIndirect(
                    *mModel->getModelG3D()->getShape(i).mShadingModelObj)) {
                meshNum++;
            }
        }

        if (meshNum == 0) {
            return;
        }

        mMeshDrawerTable = new MeshDrawerTable;
        mMeshDrawerTable->allocBuffer(meshNum, nullptr);

        for (s32 i = 0; i < modelObj->GetNumShapes(); i++) {
            if (alModelFunction::isShaderAssignForward(modelObj, i) ||
                alModelFunction::isShaderIndirect(
                    *mModel->getModelG3D()->getShape(i).mShadingModelObj)) {
                const nn::g3d::ShapeObj* shape = modelObj->GetShape(i);
                const nn::g3d::MaterialObj* material =
                    modelObj->GetMaterial(shape->GetResource()->GetMaterialIndex());
                nn::g3d::ShaderSelector* selector =
                    alModelFunction::createShaderSelectorFromAssignShader(model->mGpuMemAllocator,
                                                                          shape, material);
                mMeshDrawerTable->insert(new MeshDrawer(shape->GetResource()->GetName(), modelObj,
                                                        shape, selector, mModelNumMax));
            }
        }
    } else {
        s32 modelNum = mModelNumMax;
        auto* table = new MeshDrawerTable;
        table->allocBuffer(modelObj->GetNumShapes(), nullptr);

        for (s32 i = 0; i < modelObj->GetNumShapes(); i++) {
            const nn::g3d::ShapeObj* shape = modelObj->GetShape(i);
            const nn::g3d::MaterialObj* material =
                modelObj->GetMaterial(shape->GetResource()->GetMaterialIndex());
            const nn::g3d::ResShaderAssign* shaderAssign =
                material->GetResource()->GetShaderAssign();
            nn::g3d::ResShadingModel* shadingModel =
                ShaderHolder::sInstance->getShadingModel(shaderAssign->GetShadingModelName());
            nn::g3d::ShaderSelector* selector;

            if (alModelFunction::isShaderAssignForward(modelObj, i)) {
                const char* optionNames[] = {"cSkyColor0Type", "cExposureConnect"};
                const char* optionValues[] = {"1", "0"};
                selector = alModelFunction::createShaderSelector(
                    model->mGpuMemAllocator, shape, material, shadingModel, 2, optionNames,
                    optionValues, false);
            } else {
                const char* optionNames[] = {"cRenderType", "cSkyColor0Type", "cExposureConnect"};
                const char* optionValues[] = {"3", "1", "0"};
                selector = alModelFunction::createShaderSelector(
                    model->mGpuMemAllocator, shape, material, shadingModel, 3, optionNames,
                    optionValues, false);
            }

            table->insert(new MeshDrawer(shape->GetResource()->GetName(), modelObj, shape,
                                         selector, modelNum));
        }

        mMeshDrawerTable = table;
    }

    if (mIsRenderStateInvalidate) {
        alModelFunction::createMeshDrawerTableDisplayListRenderStateInvalidate(mMeshDrawerTable,
                                                                               mModel, true);
    } else {
        alModelFunction::createMeshDrawerTableDisplayList(mMeshDrawerTable, mModel, true);
    }
}

/**
 * Draws all forward meshes.
 */
void ModelDrawerForward::draw() const {
    if (!mMeshDrawerTable) {
        return;
    }

    if (!isDraw()) {
        return;
    }

    if (!mIsRenderStateInvalidate) {
        sead::GraphicsContext context;
        context.setBlendEnable(false);
        context.apply(GameFrameworkNx::sInstance->mDrawContext);
    }

    ModelAdditionalInfo additionalInfo(mGraphicsSystemInfo, mIsSecondCategory);
    mGraphicsSystemInfo->activateDirLitColorTex();

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

    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        MeshDrawer* meshDrawer = (*mMeshDrawerTable)[i];

        if (meshDrawer->isExistDrawMesh()) {
            additionalInfo.activateModelLightTexture(meshDrawer->getMaterialObj()->GetResource());
            meshDrawer->draw(&mGraphicsSystemInfo->mViewVolume, 0, &additionalInfo);
        }
    }
}

}  // namespace al
