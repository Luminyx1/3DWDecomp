#include "Library/Model/ModelDrawerDeferredFootPrint.hpp"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Project/Model/MeshDrawer.hpp"

namespace al {

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
