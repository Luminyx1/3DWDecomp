#include "Project/Model/ModelDrawer.hpp"

#include <nn/g3d/g3d_ModelObj.h>

#include "Library/Model/alModelCafe.hpp"
#include "Project/Model/MeshDrawer.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

/**
 * Constructs a model drawer.
 * @param pName Name of the drawer.
 */
ModelDrawer::ModelDrawer(const char* pName) : ModelDrawerBase(pName) {}

/**
 * Counts a model that will later be added to this drawer.
 * @param pModel Model.
 */
void ModelDrawer::registerModel(alModelCafe* pModel) {
    mModelNumMax++;
}

/**
 * Allocates the model table for all registered models.
 */
void ModelDrawer::createTable() {
    mModels = new alModelCafe*[mModelNumMax];

    for (s32 i = 0; i < mModelNumMax; i++) {
        mModels[i] = nullptr;
    }
}

/**
 * Adds a model and its meshes to this drawer unless it was already added.
 * @param pModel Model.
 */
void ModelDrawer::addModel(alModelCafe* pModel) {
    for (s32 i = 0; i < mModelNum; i++) {
        if (mModels[i] == pModel) {
            return;
        }
    }

    mModels[mModelNum] = pModel;
    mModelNum++;

    if (mMeshDrawerTable == nullptr) {
        return;
    }

    for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
        MeshDrawer* meshDrawer = (*mMeshDrawerTable)[i];
        SimpleModelG3D* model = pModel->getModelG3D();
        nn::g3d::ModelObj* modelObj = model->getModelObj();
        meshDrawer->addMesh(modelObj, modelObj->GetShape(meshDrawer->getShapeIndex()), model);
    }
}

/**
 * Removes a model and its meshes from this drawer.
 * @param pModel Model.
 */
void ModelDrawer::removeModel(alModelCafe* pModel) {
    if (mMeshDrawerTable != nullptr) {
        for (s32 i = 0; i < mMeshDrawerTable->size(); i++) {
            MeshDrawer* meshDrawer = (*mMeshDrawerTable)[i];
            nn::g3d::ModelObj* modelObj = pModel->getModelG3D()->getModelObj();
            meshDrawer->removeMesh(modelObj, modelObj->GetShape(meshDrawer->getShapeIndex()));
        }
    }

    for (s32 i = 0; i < mModelNum; i++) {
        if (mModels[i] == pModel) {
            if (i < mModelNum - 1) {
                mModels[i] = mModels[mModelNum - 1];
            }

            mModelNum--;
            return;
        }
    }
}

/**
 * Checks whether this drawer has any model to draw.
 * @return Whether any model was added.
 */
bool ModelDrawer::isDraw() const {
    return mModelNum > 0;
}

}  // namespace al
