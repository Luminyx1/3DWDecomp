#include "Library/Model/ModelDrawerDeferredSky.hpp"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Project/Model/MeshDrawer.hpp"

namespace al {

/**
 * Constructs a deferred sky drawer.
 * @param pName Name of the drawer.
 */
ModelDrawerDeferredSky::ModelDrawerDeferredSky(const char* pName) : ModelDrawer(pName) {
    GBufferArray::setContextMRT(&mGraphicsContext);
    mGraphicsContext.setDepthEnable(true, false);
    mGraphicsContext.setDepthFunc(4);
}

/**
 * Creates the mesh drawer table of the model.
 */
void ModelDrawerDeferredSky::createTable() {
    ModelDrawer::createTable();
    mMeshDrawerTable = alModelFunction::createMeshDrawerTableFromAssignShader(
        mModel->getModelG3D(), mModelNumMax);
    alModelFunction::createMeshDrawerTableDisplayListRenderStateInvalidate(mMeshDrawerTable,
                                                                           mModel, false);
}

/**
 * Draws the sky meshes.
 */
void ModelDrawerDeferredSky::draw() const {
    if (!isDraw()) {
        return;
    }

    if (mGraphicsSystemInfo->_40 != 0) {
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
