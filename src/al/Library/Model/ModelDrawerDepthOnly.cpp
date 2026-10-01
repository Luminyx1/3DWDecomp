#include "Library/Model/ModelDrawerDepthOnly.hpp"

#include <driver/aglGraphicsDriverMgr.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResMaterial.h>
#include <nn/g3d/g3d_ResShape.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Model/MeshDrawer.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

/**
 * Constructs a depth-only drawer.
 * @param pName Name of the drawer.
 * @param isForceFarDepth Whether the meshes are drawn at the far depth.
 * @param isRemoveable Whether the drawer can be removed.
 */
ModelDrawerDepthOnly::ModelDrawerDepthOnly(const char* pName, bool isForceFarDepth,
                                           bool isRemoveable)
    : ModelDrawer(pName), mIsRemoveable(isRemoveable), mIsForceFarDepth(isForceFarDepth) {
    mGraphicsContext.setPolygonOffsetFrontEnable(true);
    mGraphicsContext.setColorMask(0, false, false, false, false);
    mGraphicsContext.setBlendEnable(false);
    mGraphicsContext.setAlphaTestEnable(false);

    if (isForceFarDepth) {
        mGraphicsContext.setDepthEnable(true, true);
        mGraphicsContext.setDepthFunc(8);
    }
}

/**
 * Destroys the depth-only drawer.
 */
ModelDrawerDepthOnly::~ModelDrawerDepthOnly() = default;

/**
 * Creates depth-only mesh drawers and their display lists for all opaque shapes.
 */
void ModelDrawerDepthOnly::createTable() {
    ModelDrawer::createTable();
    SimpleModelG3D* model = mModel->getModelG3D();
    s32 modelNum = mModelNumMax;
    bool isForceFarDepth = mIsForceFarDepth;
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
        const char* optionNames[] = {"cIsEnableAlphaTest", "cIsEnableDitherAlpha",
                                     "cIsEnableForceFarDepth"};
        const char* optionValues[] = {"0", "0", "0"};
        const char* shadingModelName =
            material->GetResource()->GetShaderAssign()->GetShadingModelName();

        bool isAlphaTest = false;
        const char* depthAlphaMask = shaderAssign->FindShaderOption("enable_depthalphamask");

        if (depthAlphaMask != nullptr && isEqualString(depthAlphaMask, "1")) {
            optionValues[0] = "1";
            isAlphaTest = true;
        }

        bool isAlphaMaskModel = isEqualString(shadingModelName, "RenderMaterialAlphaMask");

        if (!isAlphaTest && isAlphaMaskModel) {
            continue;
        }

        if (model->_28 != nullptr) {
            optionValues[1] = "1";
        }

        if (isForceFarDepth) {
            optionValues[2] = "1";
        }

        nn::g3d::ShaderSelector* selector =
            alModelFunction::createShaderSelector(model->getGpuMemAllocator(), shape, material,
                                                  shadingModel, 3, optionNames, optionValues, false);

        const char* ditherAlpha = shaderAssign->FindShaderOption("cIsEnableDitherAlbedoAlpha");

        if (ditherAlpha != nullptr && isEqualString(ditherAlpha, "1")) {
            continue;
        }

        auto* meshDrawer =
            new MeshDrawer(shape->GetResource()->GetName(), modelObj, shape, selector, modelNum);
        if (isAlphaTest) {
            meshDrawer->setAlphaTest();
        }

        table->insert(meshDrawer);
    }

    mMeshDrawerTable = table;

    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        (*mMeshDrawerTable)[i]->createDisplayList(
            mModel->getModelG3D()->mGpuMemAllocator, MeshDrawer::RENDER_STATE_ACTIVATE_TYPE(3),
            MeshDrawer::TEXTURE_ACTIVATE_TYPE(2), MeshDrawer::MATERIAL_ACTIVATE_TYPE(2), false);
    }
}

/**
 * Draws all meshes into the depth buffer only.
 */
void ModelDrawerDepthOnly::draw() const {
    if (!isDraw()) {
        return;
    }

    mGraphicsContext.apply(GameFrameworkNx::getDrawContext());
    agl::driver::GraphicsDriverMgr::instance()->setPolygonOffset(
        GameFrameworkNx::getAglDrawContext(), 3.0f, 2.0f);

    for (s32 i = 0; i < mModelNum; i++) {
        SimpleModelG3D* model = mModels[i]->getModelG3D();
        model->setModelGlobalAlpha();

        tryUpdateModelLod(model);
    }

    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        (*mMeshDrawerTable)[i]->drawDepthOnly(&mGraphicsSystemInfo->getViewVolume(), 0);
    }

    agl::driver::GraphicsDriverMgr::instance()->setPolygonOffset(
        GameFrameworkNx::getAglDrawContext(), 0.0f, 0.0f);
}

}  // namespace al
