#include "Library/Model/ModelDrawerDepthShadow.hpp"

#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResMaterial.h>
#include <nn/g3d/g3d_ResRenderState.h>
#include <nn/g3d/g3d_ResShape.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Library/Shadow/Depth/DepthShadowDrawer.hpp"
#include "Library/Shadow/ShadowDirector.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Model/MeshDrawer.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

/**
 * Constructs a depth shadow drawer.
 * @param pName Name of the drawer.
 */
ModelDrawerDepthShadow::ModelDrawerDepthShadow(const char* pName) : ModelDrawer(pName) {}

/**
 * Creates depth-only mesh drawers and their display lists for all shapes casting shadows.
 */
void ModelDrawerDepthShadow::createTable() {
    ModelDrawer::createTable();
    alModelCafe* modelCafe = mModel;
    SimpleModelG3D* model = modelCafe->getModelG3D();
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
            ShaderHolder::sInstance->getShadingModel("RenderDepthOnly");
        const char* optionNames[] = {"cIsEnableAlphaTest", "cIsEnableDitherAlpha"};
        const char* optionValues[] = {"0", "0"};
        s32 renderStateMode = model->getResRenderState(i)->GetMode();

        bool isDepthAlphaMask = false;
        const char* depthAlphaMask = shaderAssign->FindShaderOption("enable_depthalphamask");
        if (depthAlphaMask) {
            const char* shadingModelName =
                material->GetResource()->GetShaderAssign()->GetShadingModelName();
            if (isEqualString(depthAlphaMask, "1") &&
                isEqualString(shadingModelName, "RenderMaterialAlphaMask")) {
                isDepthAlphaMask = true;
                optionValues[0] = "2";
            }
        }
        if (renderStateMode == 1) {
            optionValues[0] = "1";
        }

        const char* ditherAlpha = shaderAssign->FindShaderOption("cIsEnableDitherAlbedoAlpha");
        if (ditherAlpha && isEqualString(ditherAlpha, "1")) {
            continue;
        }

        if (model->_28) {
            optionValues[1] = "1";
        }

        nn::g3d::ShaderSelector* selector =
            alModelFunction::createShaderSelector(model->mGpuMemAllocator, shape, material,
                                                  shadingModel, 2, optionNames, optionValues, false);
        bool isOpaque = renderStateMode != 1;
        auto* meshDrawer =
            new MeshDrawer(shape->GetResource()->GetName(), modelObj, shape, selector, modelNum);
        meshDrawer->initForDepthShadow();
        if (!isDepthAlphaMask && isOpaque) {
            meshDrawer->createDisplayList(model->mGpuMemAllocator,
                                          MeshDrawer::RENDER_STATE_ACTIVATE_TYPE(2),
                                          MeshDrawer::TEXTURE_ACTIVATE_TYPE(2),
                                          MeshDrawer::MATERIAL_ACTIVATE_TYPE(2), false);
        } else {
            meshDrawer->createDisplayList(
                model->mGpuMemAllocator, MeshDrawer::RENDER_STATE_ACTIVATE_TYPE(2),
                MeshDrawer::TEXTURE_ACTIVATE_TYPE(modelCafe->mAnimPlayerMat1 == nullptr),
                MeshDrawer::MATERIAL_ACTIVATE_TYPE(modelCafe->mAnimPlayerMat2 == nullptr), false);
        }
        table->insert(meshDrawer);
    }

    mMeshDrawerTable = table;
}

/**
 * Draws all meshes into the depth shadow map, or prepares them when pre-drawing.
 */
void ModelDrawerDepthShadow::draw() const {
    if (!isDraw()) {
        return;
    }

    DepthShadowDrawer* depthShadowDrawer = mGraphicsSystemInfo->mShadowDirector->mDepthShadowDrawer;
    if (depthShadowDrawer->isPreDraw()) {
        for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
            (*mMeshDrawerTable)[i]->preDrawToDepthShadow(depthShadowDrawer);
        }
        return;
    }

    for (s32 i = 0; i < mModelNum; i++) {
        mModels[i]->getModelG3D()->setModelGlobalAlpha();
    }

    s32 shadowIndex = depthShadowDrawer->getDrawShadowIndex();
    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        MeshDrawer* meshDrawer = (*mMeshDrawerTable)[i];
        if (meshDrawer->isExistDrawMesh()) {
            meshDrawer->drawDepthShadow(nullptr, 0, shadowIndex);
        }
        meshDrawer->clearDepthShadowFlag();
    }
}

}  // namespace al
