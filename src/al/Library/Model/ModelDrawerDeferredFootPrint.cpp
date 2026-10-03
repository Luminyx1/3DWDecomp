#include "Library/Model/ModelDrawerDeferredFootPrint.hpp"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Project/Model/MeshDrawer.hpp"

namespace al {

/**
 * Constructs a deferred footprint drawer.
 * @param pName Name of the drawer.
 */
ModelDrawerDeferredFootPrint::ModelDrawerDeferredFootPrint(const char* pName)
    : ModelDrawer(pName) {
    GBufferArray::setContextMRT(&mGraphicsContext);
    mGraphicsContext.setBlendEnable(0, true);
    mGraphicsContext.setBlendEnable(1, true);
    mGraphicsContext.setBlendFactorSrcRGB(0, 9);
    mGraphicsContext.setBlendFactorDstRGB(0, 1);
    mGraphicsContext.setBlendEquationRGB(0, 1);
    mGraphicsContext.setBlendFactorSrcRGB(1, 5);
    mGraphicsContext.setBlendFactorDstRGB(1, 6);
    mGraphicsContext.setBlendEquationRGB(1, 1);
    mGraphicsContext.setAlphaTestEnable(true);
    mGraphicsContext.setDepthEnable(true, false);
    mGraphicsContext.setAlphaTestFunc(6);
    mGraphicsContext.setAlphaTestRef(0.0f);
    mGraphicsContext.setColorMask(0, true, true, true, false);
    mGraphicsContext.setColorMask(1, true, true, true, false);
}

/**
 * Creates the mesh drawer table of the model.
 */
void ModelDrawerDeferredFootPrint::createTable() {
    ModelDrawer::createTable();
    mMeshDrawerTable = alModelFunction::createMeshDrawerTableFromAssignShader(
        mModel->getModelG3D(), mModelNumMax);
    alModelFunction::createMeshDrawerTableDisplayListRenderStateInvalidate(mMeshDrawerTable,
                                                                           mModel, false);
}

/**
 * Draws the footprint meshes.
 */
void ModelDrawerDeferredFootPrint::draw() const {
    if (!isDraw()) {
        return;
    }

    mGraphicsContext.apply(GameFrameworkNx::getDrawContext());

    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        MeshDrawer* meshDrawer = (*mMeshDrawerTable)[i];

        if (meshDrawer->isExistDrawMesh()) {
            meshDrawer->draw(&mGraphicsSystemInfo->getViewVolume(), 0, nullptr);
        }
    }
}

}  // namespace al
