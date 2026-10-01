#include "Library/Model/ModelDrawerDeferredExt.hpp"

#include <common/aglDisplayList.h>
#include <common/aglDrawContext.h>
#include <driver/aglGraphicsDriverMgr.h>
#include <gfx/seadGraphics.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResShape.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Project/Draw/GpuMemAllocator.hpp"
#include "Project/Model/MeshDrawer.hpp"
#include "Project/Model/ModelAdditionalInfo.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {
static inline void pushDebugGroup(NVNcommandBuffer* pCmdBuf, const char* pName) {
    reinterpret_cast<void (*)(NVNcommandBuffer*, const char*)>(
        pfnc_nvnCommandBufferPushDebugGroup)(pCmdBuf, pName);
}

/**
 * Constructs an extended deferred model drawer with a per-mesh draw callback.
 * @param pName Name of the drawer.
 * @param isSecondCategory Whether models use cube map category 1.
 */
ModelDrawerDeferredExt::ModelDrawerDeferredExt(const char* pName, bool isSecondCategory, bool a3)
    : ModelDrawer(pName), mIsSecondCategory(isSecondCategory) {
    _39 = a3;
}

/**
 * Adds a model to this drawer.
 * @param pModel Model.
 */
void ModelDrawerDeferredExt::addModel(alModelCafe* pModel) {
    ModelDrawer::addModel(pModel);
}

/**
 * Removes a model from this drawer.
 * @param pModel Model.
 */
void ModelDrawerDeferredExt::removeModel(alModelCafe* pModel) {
    ModelDrawer::removeModel(pModel);
}

/**
 * Sets the callback invoked before each mesh is drawn.
 * @param callback Callback receiving the user data and the drawn model.
 * @param pUserData User data passed to the callback.
 */
void ModelDrawerDeferredExt::setDrawCallback(DrawCallback callback, void* pUserData) {
    mDrawCallback = callback;
    mDrawCallbackUserData = pUserData;
}

/**
 * Sets the graphics system info and the model drawn by this drawer.
 * @param pInfo Graphics system info.
 * @param pModel Model.
 */
void ModelDrawerDeferredExt::setDrawInfo(GraphicsSystemInfo* pInfo, alModelCafe* pModel) {
    ModelDrawerBase::setDrawInfo(pInfo, pModel);
}

/**
 * Creates the mesh drawers and their graphics context display lists.
 */
void ModelDrawerDeferredExt::createTable() {
    ModelDrawer::createTable();
    const nn::g3d::ModelObj* modelObj = mModel->getModelG3D()->getModelObj();

    s32 meshNum = 0;

    for (s32 i = 0; i < modelObj->GetNumShapes(); i++) {
        meshNum += alModelFunction::isShaderAssignDeferred(modelObj, i);
    }

    if (meshNum == 0) {
        return;
    }

    mMeshDrawerTable = new MeshDrawerTable;
    mMeshDrawerTable->allocBuffer(meshNum, nullptr);

    for (s32 i = 0; i < modelObj->GetNumShapes(); i++) {
        const nn::g3d::ShapeObj* shape = modelObj->GetShape(i);
        const nn::g3d::MaterialObj* material =
            modelObj->GetMaterial(shape->GetResource()->GetMaterialIndex());
        nn::g3d::ShaderSelector* selector = alModelFunction::createShaderSelectorFromAssignShader(
            mModel->getModelG3D()->mGpuMemAllocator, shape, material);
        if (alModelFunction::isShaderAssignDeferred(modelObj, i)) {
            mMeshDrawerTable->insert(new MeshDrawer(shape->GetResource()->GetName(), modelObj,
                                                    shape, selector, mModelNumMax));
        }
    }

    mMeshDrawerTable->sort();
    alModelFunction::createMeshDrawerTableDisplayListRenderStateInvalidate(mMeshDrawerTable,
                                                                           mModel, false);

    s32 drawerNum = mMeshDrawerTable->size();
    mDisplayLists.allocBuffer(drawerNum, nullptr);
    ModelAdditionalInfo additionalInfo(mGraphicsSystemInfo, mIsSecondCategory);

    for (s32 i = 0; i < drawerNum; i++) {
        s32 shapeIndex = mMeshDrawerTable->unsafeAt(i)->getShapeIndex();
        nn::g3d::ModelObj* shapeModelObj = mModel->getModelG3D()->getModelObj();
        const nn::g3d::ResMaterial* material =
            shapeModelObj
                ->GetMaterial(shapeModelObj->GetShape(shapeIndex)->GetResource()->GetMaterialIndex())
                ->GetResource();
        bool isAlphaMask = alModelFunction::isShaderAssignAlphaMask(shapeModelObj, shapeIndex);
        bool isXlu = alModelFunction::isShaderAssignXlu(shapeModelObj, shapeIndex);
        bool isOnlyBlendLightBuf = alModelFunction::isMaterialOnlyBlendLightBuf(material);
        bool isXluWithNrm = alModelFunction::isMaterialXluWithNrm(material);
        bool isApplyRenderState = alModelFunction::isMaterialApplyRenderState(material);

        auto* displayList = new agl::DisplayList();
        sead::Graphics::instance()->lockDrawContext();
        {
            agl::DrawContext context;
            context.setCommandBuffer(displayList);
            agl::GPUMemAddr<u8> buffer =
                mModel->getModelG3D()->mGpuMemAllocator->allocMemory("DisplayList", 0x400, 4);
            displayList->beginDisplayListBuffer(buffer, 0x400, true);

            const SimpleModelG3D* model = mModel->getModelG3D();

            if (isXlu) {
                if (isApplyRenderState) {
                    alModelFunction::prepareModelShapeDrawDeferredGraphicsContextByCustom(
                        &context, model, shapeIndex, BlendType_Xlu);
                } else if (isOnlyBlendLightBuf) {
                    alModelFunction::prepareModelShapeDrawDeferredGraphicsContextByCustom(
                        &context, model, shapeIndex, BlendType_OnlyLightBuf);
                } else if (isXluWithNrm) {
                    alModelFunction::prepareModelShapeDrawDeferredGraphicsContextByCustom(
                        &context, model, shapeIndex, BlendType_XluWithNrm);
                } else {
                    alModelFunction::prepareModelShapeDrawDeferredGraphicsContext(
                        &context, model, shapeIndex, isAlphaMask, true);
                }
            } else {
                alModelFunction::prepareModelShapeDrawDeferredGraphicsContext(
                    &context, model, shapeIndex, isAlphaMask, false);
            }

            displayList->endDisplayList();
        }

        sead::Graphics::instance()->unlockDrawContext();
        mDisplayLists.pushBack(displayList);
    }
}

/**
 * Draws all deferred meshes, invoking the draw callback before each one.
 */
void ModelDrawerDeferredExt::draw() const {
    if (mMeshDrawerTable == nullptr) {
        return;
    }

    if (!isDraw()) {
        return;
    }

    pushDebugGroup(
        agl::driver::getNvnCommandBuffer(
            GameFrameworkNx::getAglDrawContext()),
        mName);
    ModelAdditionalInfo additionalInfo(mGraphicsSystemInfo, mIsSecondCategory);
    mGraphicsSystemInfo->activateDirLitColorTex();

    for (s32 i = 0; i < mModelNum; i++) {
        SimpleModelG3D* model = mModels[i]->getModelG3D();
        model->setModelAdditionalInfo(additionalInfo);

        tryUpdateModelLod(model);
    }

    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        MeshDrawer* meshDrawer = (*mMeshDrawerTable)[i];

        if (meshDrawer->isExistDrawMesh()) {
            const agl::DisplayList* displayList = mDisplayLists[i];
            nvnCommandBufferCallCommands(
                agl::driver::getNvnCommandBuffer(
                    GameFrameworkNx::getAglDrawContext()),
                1, displayList->getHandlePtr());
            additionalInfo.activateModelLightTexture(meshDrawer->getMaterialObj()->GetResource());

            if (mDrawCallback) {
                mDrawCallback(mDrawCallbackUserData, mModel);
            }

            meshDrawer->draw(&mGraphicsSystemInfo->getViewVolume(), 0, &additionalInfo);
        }
    }

    nvnCommandBufferPopDebugGroup(agl::driver::getNvnCommandBuffer(
        GameFrameworkNx::getAglDrawContext()));
}

}  // namespace al
