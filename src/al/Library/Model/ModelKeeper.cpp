#include "Library/Model/ModelKeeper.hpp"

#include <nn/g3d/g3d_ModelObj.h>

#include "Library/Clipping/ClippingDirectorBase.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

/**
 * Constructs an empty model keeper.
 */
ModelKeeper::ModelKeeper() = default;

/**
 * Destroys the model keeper and its model.
 */
ModelKeeper::~ModelKeeper() {
    if (mModelCafe) {
        delete mModelCafe;
        mModelCafe = nullptr;
    }
}

/**
 * Sets the global alpha pointer of the model.
 * @param pAlpha Pointer to the global alpha value.
 */
void ModelKeeper::setGlobalAlpha(f32* pAlpha) {
    if (mModelCafe && mModelCafe->getModelG3D()) {
        mModelCafe->getModelG3D()->setGlobalAlphaPtr(pAlpha);
    }
}

/**
 * Sets the global Y offset pointer of the model.
 * @param pYOffset Pointer to the global Y offset value.
 */
void ModelKeeper::setGlobalYOffset(f32* pYOffset) {
    if (mModelCafe && mModelCafe->getModelG3D()) {
        mModelCafe->getModelG3D()->setGlobalYOffsetPtr(pYOffset);
    }
}

/**
 * Creates the model and initializes its resources.
 * @param pModelArcName Model archive name.
 * @param pAnimArcName Animation archive name.
 * @param pSuffix Resource suffix.
 */
void ModelKeeper::initResource(const char* pModelArcName, const char* pAnimArcName,
                               const char* pSuffix) {
    mModelName = createStringIfInStack(getBaseName(pModelArcName));
    mModelCafe = new alModelCafe();
    mModelCafe->initResource(pModelArcName, pAnimArcName, pSuffix);
}

/**
 * Sets an already created model.
 * @param pName Model name.
 * @param pModel Model.
 */
void ModelKeeper::setModel(const char* pName, alModelCafe* pModel) {
    mModelCafe = pModel;
    mModelName = pName;
}

/**
 * Initializes the model.
 * @param bufferNum Number of GPU buffers.
 * @param pAllocator GPU memory allocator.
 */
void ModelKeeper::initModel(s32 bufferNum, GpuMemAllocator* pAllocator) {
    mModelCafe->initModel(bufferNum, pAllocator);
}

/**
 * Shows the model.
 */
void ModelKeeper::show() {
    mModelCafe->show();
}

/**
 * Hides the model.
 */
void ModelKeeper::hide() {
    mModelCafe->hide();
}

/**
 * Updates the model.
 */
void ModelKeeper::update() {
    mModelCafe->update();
}

/**
 * Updates the model while paused.
 */
void ModelKeeper::updatePaused() {
    mModelCafe->updatePaused();
}

/**
 * Calculates the model matrices.
 * @param rMtx Base matrix.
 * @param rScale Scale.
 */
void ModelKeeper::calc(const sead::Matrix34f& rMtx, const sead::Vector3f& rScale) {
    mModelCafe->calc(rMtx, rScale);
}

/**
 * Sets the LOD parameters of the model.
 * @param pLodDistances LOD distances.
 * @param lodNum Number of LODs.
 * @param pPos LOD reference position.
 * @param isFlag LOD flag.
 */
void ModelKeeper::setLodParams(const f32* pLodDistances, s32 lodNum, const sead::Vector3f* pPos,
                               bool isFlag) {
    SimpleModelG3D* model = mModelCafe->getModelG3D();
    model->setLodParams(pLodDistances, lodNum);
    model->mLodPos = pPos;
    _1c = isFlag;
}

/**
 * Updates the LOD state of the model.
 * @param rPos LOD reference position.
 * @param isFlag LOD flag.
 */
void ModelKeeper::updateLod(const sead::Vector3f& rPos, bool isFlag) {
    SimpleModelG3D* model = mModelCafe->getModelG3D();
    if (ClippingDirectorBase::sLODDisabled || mIsLodDisabled || (isFlag && _1c)) {
        model->mIsLodDisabled = true;
        model->mLodIndex = 0;
        return;
    }

    model->mLodPos = &rPos;
    model->mIsLodDisabled = model->getModelObj()->GetLodCount() < 2;
    model->mLodIndex = model->mIsLodDisabled ? 0 : model->mLodIndex;
}

/**
 * Gets the number of LODs of the model.
 * @return Number of LODs.
 */
s32 ModelKeeper::getLodNum() const {
    return mModelCafe->getModelG3D()->getLodNum();
}

/**
 * Enables or disables drawing of the model.
 * @param isDisable Whether drawing is disabled.
 * @return Whether the state changed.
 */
bool ModelKeeper::setDisableDraw(bool isDisable) {
    if (!mModelCafe) {
        return false;
    }

    SimpleModelG3D* model = mModelCafe->getModelG3D();
    if (!model) {
        return false;
    }

    if (model->isDisableDraw() == isDisable) {
        return false;
    }

    model->setDisableDraw(isDisable);
    return true;
}

/**
 * Enables or disables depth shadow drawing of the model.
 * @param isDisable Whether depth shadow drawing is disabled.
 * @return Whether the state changed.
 */
bool ModelKeeper::setDisableDepthShadow(bool isDisable) {
    if (!mModelCafe) {
        return false;
    }

    SimpleModelG3D* model = mModelCafe->getModelG3D();
    if (!model) {
        return false;
    }

    if (model->isDisableDepthShadow() == isDisable) {
        return false;
    }

    model->setDisableDepthShadow(isDisable);
    return true;
}

/**
 * Gets a joint world matrix of the model.
 * @param index Joint index.
 * @return Joint world matrix.
 */
const sead::Matrix34f* ModelKeeper::getWorldMtxPtrByIndex(s32 index) const {
    if (!mModelCafe) {
        return nullptr;
    }

    return mModelCafe->getWorldMtxPtrByIndex(index);
}

/**
 * Initializes the light controller (does nothing).
 * @param num Number of lights.
 */
void ModelKeeper::initLightCtrl(s32 num) {}

}  // namespace al
