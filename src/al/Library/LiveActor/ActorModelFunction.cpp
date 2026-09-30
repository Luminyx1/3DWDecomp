#include "Library/LiveActor/Util/ActorModelUtil.hpp"

#include <nn/g3d/g3d_ModelObj.h>

#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Camera/CameraUtil.hpp"
#include "Library/Clipping/ClippingDirectorBase.hpp"
#include "Library/Clipping/ClippingJudge.hpp"
#include "Library/Execute/ActorExecuteInfo.hpp"
#include "Library/Execute/ExecuteUtil.hpp"
#include "Library/HitSensor/SensorFunction.hpp"
#include "Library/LiveActor/ActorCollisionFunction.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/LiveActorFlag.hpp"
#include "Library/LiveActor/LiveActorFunc.hpp"
#include "Library/LiveActor/SubActorKeeper.hpp"
#include "Library/LiveActor/Util/ActorAnimUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/ModelDrawerBase.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/ModelShapeUtil.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Screen/ScreenPointerUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Se/Project/SeKeeper.hpp"
#include "Library/Shadow/Common/ShadowUtil.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Collision/Collider.hpp"
#include "Project/Effect/Core/EffectUtil.hpp"
#include "Project/Light/ActorPrePassLightKeeper.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {
/**
 * Updates the wet state of an actor from the material code area it is in.
 * @param pActor The actor.
 */
void updateMaterialCodeArea(LiveActor* pActor) {
    AreaObj* area = tryFindAreaObj(pActor, "MaterialCodeArea", getTrans(pActor));

    if (!area) {
        updateEffectMaterialWet(pActor, false);
        updateSeMaterialWet(pActor, false);
        resetEnvTexture(pActor);
        return;
    }

    const char* materialCodeType;

    if (!tryGetAreaObjStringArg(&materialCodeType, area, "MaterialCodeType")) {
        updateEffectMaterialWet(pActor, false);
        updateSeMaterialWet(pActor, false);
        resetEnvTexture(pActor);
        return;
    }

    if (!isEqualString(materialCodeType, "Wet")) {
        resetEnvTexture(pActor);
        return;
    }

    updateEffectMaterialWet(pActor, true);
    updateSeMaterialWet(pActor, true);
    changeEnvTextureWetObj(pActor);
}

/**
 * Updates the water state of an actor from the water area it is in.
 * @param pActor The actor.
 */
void updateMaterialCodeWater(LiveActor* pActor) {
    AreaObj* area = tryFindAreaObj(pActor, "WaterArea", getTrans(pActor));
    updateMaterialCodeWater(pActor, area != nullptr, false);
}

/**
 * Updates the water state of an actor.
 * @param pActor The actor.
 * @param isInWater Whether the actor is in water.
 * @param isAshore Whether the actor is at the shore.
 */
void updateMaterialCodeWater(LiveActor* pActor, bool isInWater, bool isAshore) {
    if (pActor->getEffectKeeper()) {
        updateEffectMaterialWater(pActor, isInWater);
    }

    if (!isExistSeKeeper(pActor)) {
        return;
    }

    if (isInWater && isAshore) {
        updateSeMaterialWater(pActor, false);
        tryUpdateSeMaterialCode(pActor, "Ashore");
        return;
    }

    updateSeMaterialWater(pActor, isInWater);
}

/**
 * Resets the material code of an actor.
 * @param pActor The actor.
 */
void resetMaterialCode(LiveActor* pActor) {
    if (pActor->getEffectKeeper()) {
        resetEffectMaterialCode(pActor);
    }

    if (isExistSeKeeper(pActor)) {
        resetSeMaterialName(pActor);
    }

    resetEnvTexture(pActor);
}

/**
 * Sets the pointer to the alpha of the model of an actor.
 * @param pActor The actor.
 * @param pAlpha The alpha.
 */
void setModelAlphaPtr(LiveActor* pActor, f32* pAlpha) {
    if (pActor->mModelKeeper) {
        pActor->mModelKeeper->setGlobalAlpha(pAlpha);
    }
}

/**
 * Forces the model of an actor to use its most detailed level.
 * @param pActor The actor.
 */
void setLODForceLevel0(LiveActor* pActor) {
    ModelKeeper* modelKeeper = pActor->mModelKeeper;

    if (!modelKeeper) {
        return;
    }

    alModelCafe* model = modelKeeper->mModelCafe;

    if (!model) {
        return;
    }

    SimpleModelG3D* modelG3D = model->mModelG3D;

    if (!modelG3D) {
        return;
    }

    s32 lodCount = modelG3D->mModelObj->GetLodCount();
    modelG3D->mLodIndex = lodCount > 0 ? 0 : lodCount - 1;
}

/**
 * Shows the model of an actor.
 * @param pActor The actor.
 */
void showModel(LiveActor* pActor) {
    if (!isDead(pActor) && !isClipped(pActor)) {
        if (pActor->mModelKeeper) {
            pActor->mModelKeeper->show();
        }

        alActorSystemFunction::addToExecutorDraw(pActor);

        if (pActor->mLightKeeper) {
            pActor->mLightKeeper->appear(false);
        }

        if (pActor->mShadowKeeper) {
            showShadow(pActor);
        }
    }

    pActor->mActorFlags->isHideModel = false;

    if (pActor->mSubActorKeeper) {
        alSubActorFunction::trySyncShowModel(pActor->mSubActorKeeper);
    }
}

/**
 * Shows the model of an actor if it is hidden.
 * @param pActor The actor.
 */
void showModelIfHide(LiveActor* pActor) {
    if (pActor->mActorFlags->isHideModel) {
        showModel(pActor);
    }
}

/**
 * Checks whether the model of an actor is hidden.
 * @param pActor The actor.
 * @return Whether the model is hidden.
 */
bool isHideModel(const LiveActor* pActor) {
    return pActor->mActorFlags->isHideModel;
}

/**
 * Hides the model of an actor.
 * @param pActor The actor.
 */
void hideModel(LiveActor* pActor) {
    if (!isDead(pActor) && !isClipped(pActor)) {
        if (pActor->mModelKeeper) {
            pActor->mModelKeeper->hide();
        }

        if (pActor->mShadowKeeper) {
            hideShadow(pActor);
        }

        alActorSystemFunction::removeFromExecutorDraw(pActor);

        if (pActor->mLightKeeper) {
            pActor->mLightKeeper->hideModel();
        }
    }

    pActor->mActorFlags->isHideModel = true;

    if (pActor->mSubActorKeeper) {
        alSubActorFunction::trySyncHideModel(pActor->mSubActorKeeper);
    }
}

/**
 * Hides the model of an actor if it is shown.
 * @param pActor The actor.
 */
void hideModelIfShow(LiveActor* pActor) {
    if (!pActor->mActorFlags->isHideModel) {
        hideModel(pActor);
    }
}

/**
 * Checks whether an actor has a model.
 * @param pActor The actor.
 * @return Whether the actor has a model.
 */
bool isExistModel(const LiveActor* pActor) {
    return pActor->mModelKeeper != nullptr;
}

/**
 * Hides the model of an actor while the camera is near it.
 * @param pActor The actor.
 * @param distance The distance to hide the model within.
 */
void switchShowHideModelIfNearCamera(LiveActor* pActor, f32 distance) {
    const sead::Vector3f& trans = getTrans(pActor);
    const sead::Vector3f& cameraPos = getCameraPos(pActor);
    f32 cameraDistance = (trans - cameraPos).length();
    bool isHide = pActor->mActorFlags->isHideModel;

    if (cameraDistance < distance) {
        if (!isHide) {
            hideModel(pActor);
            offCalcAndDrawEffect(pActor);
        }
    } else if (isHide) {
        showModel(pActor);
        onCalcAndDrawEffect(pActor);
    }
}

/**
 * Shows the silhouette models of an actor and its sub actors.
 * @param pActor The actor.
 */
void showSilhouetteModel(LiveActor* pActor) {
    s32 subActorNum = pActor->mSubActorKeeper->mCount;

    for (s32 i = 0; i < subActorNum; i++) {
        SubActorInfo* info = pActor->mSubActorKeeper->mInfos[i];

        if (isEqualString(info->mSubActor->getName(), "シルエットモデル")) {
            showModelIfHide(info->mSubActor);
        } else if (info->mSubActor->mSubActorKeeper) {
            showSilhouetteModel(info->mSubActor);
        }
    }
}

/**
 * Hides the silhouette models of an actor and its sub actors.
 * @param pActor The actor.
 */
void hideSilhouetteModel(LiveActor* pActor) {
    s32 subActorNum = pActor->mSubActorKeeper->mCount;

    for (s32 i = 0; i < subActorNum; i++) {
        SubActorInfo* info = pActor->mSubActorKeeper->mInfos[i];

        if (isEqualString(info->mSubActor->getName(), "シルエットモデル")) {
            hideModelIfShow(info->mSubActor);
        } else if (info->mSubActor->mSubActorKeeper) {
            hideSilhouetteModel(info->mSubActor);
        }
    }
}

/**
 * Checks whether the silhouette model of an actor is hidden.
 * @param pActor The actor.
 * @return Whether the silhouette model is hidden.
 */
bool isSilhouetteModelHidden(LiveActor* pActor) {
    return isHideModel(alSubActorFunction::findSubActor(pActor->mSubActorKeeper, "シルエットモデル"));
}

/**
 * Shows the silhouette models of an actor if they are hidden.
 * @param pActor The actor.
 */
void showSilhouetteModelIfHide(LiveActor* pActor) {
    if (pActor->mSubActorKeeper &&
        isHideModel(alSubActorFunction::findSubActor(pActor->mSubActorKeeper, "シルエットモデル"))) {
        showSilhouetteModel(pActor);
    }
}

/**
 * Hides the silhouette models of an actor if they are shown.
 * @param pActor The actor.
 */
void hideSilhouetteModelIfShow(LiveActor* pActor) {
    if (pActor->mSubActorKeeper &&
        !isHideModel(alSubActorFunction::findSubActor(pActor->mSubActorKeeper, "シルエットモデル"))) {
        hideSilhouetteModel(pActor);
    }
}

/**
 * Finds the silhouette model among the sub actors of an actor.
 * @param pActor The actor.
 * @return The silhouette model, or nullptr.
 */
LiveActor* getSilhouetteModel(LiveActor* pActor) {
    if (!pActor->mSubActorKeeper) {
        return nullptr;
    }

    s32 subActorNum = pActor->mSubActorKeeper->mCount;

    for (s32 i = 0; i < subActorNum; i++) {
        SubActorInfo* info = pActor->mSubActorKeeper->mInfos[i];

        if (isEqualString(info->mSubActor->getName(), "シルエットモデル")) {
            return info->mSubActor;
        }

        if (info->mSubActor->mSubActorKeeper) {
            return getSilhouetteModel(pActor);
        }
    }

    return nullptr;
}

/**
 * Sets whether the model of an actor and its sub actors are drawn.
 * @param pActor The actor.
 * @param isDisable Whether to disable drawing.
 */
void setDisableDraw(LiveActor* pActor, bool isDisable) {
    if (!pActor->mModelKeeper) {
        return;
    }

    pActor->mModelKeeper->setDisableDraw(isDisable);
    SubActorKeeper* keeper = pActor->mSubActorKeeper;

    if (!keeper) {
        return;
    }

    for (s32 i = 0; i < keeper->mCount; i++) {
        SubActorInfo* info = keeper->mInfos[i];

        if (info && info->mSubActor) {
            setDisableDraw(info->mSubActor, isDisable);
        }
    }
}

/**
 * Removes the model of an actor from its depth shadow drawers.
 * @param pActor The actor.
 */
void removeFromDepthShadowDrawer(LiveActor* pActor) {
    ActorExecuteInfo* executeInfo = pActor->mActorExecuteInfo;
    s32 drawerNum = executeInfo->mDrawerCount;

    if (drawerNum <= 0) {
        return;
    }

    alModelCafe* model = pActor->mModelKeeper->mModelCafe;

    for (s32 i = 0; i < drawerNum; i++) {
        ModelDrawerBase* drawer = executeInfo->mDrawers[i];

        if (drawer->isDepthShadowDrawer()) {
            drawer->removeModel(model);
        }
    }
}

/**
 * Adds the model of an actor to its depth shadow drawers.
 * @param pActor The actor.
 */
void addToDepthShadowDrawer(LiveActor* pActor) {
    ActorExecuteInfo* executeInfo = pActor->mActorExecuteInfo;
    s32 drawerNum = executeInfo->mDrawerCount;

    if (drawerNum <= 0) {
        return;
    }

    alModelCafe* model = pActor->mModelKeeper->mModelCafe;

    for (s32 i = 0; i < drawerNum; i++) {
        ModelDrawerBase* drawer = executeInfo->mDrawers[i];

        if (drawer->isDepthShadowDrawer()) {
            drawer->addModel(model);
        }
    }
}

/**
 * Sets whether the model of an actor and its sub actors cast depth shadows.
 * @param pActor The actor.
 * @param isDisable Whether to disable the depth shadow.
 * @param isUpdateDrawer Whether to update the depth shadow drawers.
 */
void setDisableDepthShadow(LiveActor* pActor, bool isDisable, bool isUpdateDrawer) {
    if (!pActor->mModelKeeper) {
        return;
    }

    if (pActor->mModelKeeper->setDisableDepthShadow(isDisable) && isUpdateDrawer) {
        ActorExecuteInfo* executeInfo = pActor->mActorExecuteInfo;
        alModelCafe* model = pActor->mModelKeeper->mModelCafe;
        s32 drawerNum = executeInfo->mDrawerCount;

        if (isDisable) {
            for (s32 i = 0; i < drawerNum; i++) {
                ModelDrawerBase* drawer = executeInfo->mDrawers[i];

                if (drawer->isDepthShadowDrawer()) {
                    drawer->removeModel(model);
                }
            }
        } else {
            for (s32 i = 0; i < drawerNum; i++) {
                ModelDrawerBase* drawer = executeInfo->mDrawers[i];

                if (drawer->isDepthShadowDrawer()) {
                    drawer->addModel(model);
                }
            }
        }
    }

    SubActorKeeper* keeper = pActor->mSubActorKeeper;

    if (!keeper) {
        return;
    }

    for (s32 i = 0; i < keeper->mCount; i++) {
        SubActorInfo* info = keeper->mInfos[i];

        if (info && info->mSubActor) {
            setDisableDepthShadow(info->mSubActor, isDisable, isUpdateDrawer);
        }
    }
}

/**
 * Marks the model of an actor as fixed.
 * @param pActor The actor.
 */
void setFixedModelFlag(LiveActor* pActor) {
    pActor->mModelKeeper->_18 = true;
}

/**
 * Updates the model of an actor with a fixed model once to fill its gpu buffers.
 * @param pActor The actor.
 */
void tryInitFixedModelGpuBuffer(LiveActor* pActor) {
    ModelKeeper* modelKeeper = pActor->mModelKeeper;

    if (!modelKeeper || !pActor->mActorExecuteInfo || !modelKeeper->_18) {
        return;
    }

    if (pActor->getEffectKeeper() || pActor->getAudioKeeper() || pActor->mHitSensorKeeper) {
        onUpdateMovementEffectAudioCollision(pActor);
    }

    modelKeeper->update();
    sead::Matrix34f baseMtx;
    alActorPoseFunction::calcBaseMtx(&baseMtx, pActor);
    setBaseMtxAndCalcAnim(pActor, baseMtx, getScale(pActor));
}

/**
 * Sets whether the model of an actor is updated while it is clipped from drawing.
 * @param pActor The actor.
 * @param isIgnore Whether to update the model.
 */
void setIgnoreUpdateDrawClipping(LiveActor* pActor, bool isIgnore) {
    pActor->mModelKeeper->_19 = isIgnore;
}

/**
 * Sets whether the base matrix of an actor must be set before calculating its animation.
 * @param pActor The actor.
 * @param isNeed Whether the base matrix must be set.
 */
void setNeedSetBaseMtxAndCalcAnimFlag(LiveActor* pActor, bool isNeed) {
    pActor->mModelKeeper->_1a = isNeed;
}

/**
 * Makes the invincible models of an actor and its sub actors appear.
 * @param pActor The actor.
 */
void showInvincibleModel(LiveActor* pActor) {
    s32 subActorNum = pActor->mSubActorKeeper->mCount;

    for (s32 i = 0; i < subActorNum; i++) {
        SubActorInfo* info = pActor->mSubActorKeeper->mInfos[i];

        if (isEqualString(info->mSubActor->getName(), "無敵モデル")) {
            info->mSubActor->appear();
        } else if (info->mSubActor->mSubActorKeeper) {
            showInvincibleModel(info->mSubActor);
        }
    }
}

/**
 * Kills the invincible models of an actor and its sub actors.
 * @param pActor The actor.
 */
void hideInvincibleModel(LiveActor* pActor) {
    s32 subActorNum = pActor->mSubActorKeeper->mCount;

    for (s32 i = 0; i < subActorNum; i++) {
        SubActorInfo* info = pActor->mSubActorKeeper->mInfos[i];

        if (isEqualString(info->mSubActor->getName(), "無敵モデル")) {
            info->mSubActor->kill();
        } else if (info->mSubActor->mSubActorKeeper) {
            hideInvincibleModel(info->mSubActor);
        }
    }
}

/**
 * Checks whether the invincible model of an actor is dead.
 * @param pActor The actor.
 * @return Whether the invincible model is dead.
 */
bool isInvincibleModelHidden(LiveActor* pActor) {
    return isDead(alSubActorFunction::findSubActor(pActor->mSubActorKeeper, "無敵モデル"));
}

/**
 * Makes the model bounding of an actor update every frame.
 * @param pActor The actor.
 */
void enableUpdateModelBounding(LiveActor* pActor) {
    pActor->mModelKeeper->mModelCafe->mModelG3D->_44 = true;
}

/**
 * Stops the model bounding of an actor from updating every frame.
 * @param pActor The actor.
 */
void disableUpdateModelBounding(LiveActor* pActor) {
    pActor->mModelKeeper->mModelCafe->mModelG3D->_44 = false;
}

/**
 * Applies a cube map to the model of an actor.
 * @param pActor The actor.
 * @param pCubeMapName The cube map name.
 */
void forceApplyCubeMap(LiveActor* pActor, const char* pCubeMapName) {
    forceApplyCubeMap(pActor->mModelKeeper,
                      static_cast<const GraphicsSystemInfo*>(pActor->getSceneInfo()->_78),
                      pCubeMapName);
}

/**
 * Creates custom render states for all shapes of the model of an actor.
 * @param pActor The actor.
 */
void createRenderState(LiveActor* pActor) {
    SimpleModelG3D* modelG3D = pActor->mModelKeeper->mModelCafe->mModelG3D;
    s32 shapeNum = modelG3D->mModelObj->GetNumShapes();

    for (s32 i = 0; i < shapeNum; i++) {
        modelG3D->createResRenderState(i);
    }
}

/**
 * Resets the custom render states of all shapes of the model of an actor.
 * @param pActor The actor.
 */
void resetRenderState(LiveActor* pActor) {
    SimpleModelG3D* modelG3D = pActor->mModelKeeper->mModelCafe->mModelG3D;
    s32 shapeNum = modelG3D->mModelObj->GetNumShapes();

    for (s32 i = 0; i < shapeNum; i++) {
        modelG3D->resetResRenderState(i);
    }
}

/**
 * Sets whether the model of an actor uses depth testing. Does nothing.
 * @param pActor The actor.
 * @param isEnable Whether to enable depth testing.
 */
void setEnableDepthTest(LiveActor* pActor, bool isEnable) {}

/**
 * Sets whether the model of an actor writes depth. Does nothing.
 * @param pActor The actor.
 * @param isEnable Whether to enable depth writing.
 */
void setEnableDepthWrite(LiveActor* pActor, bool isEnable) {}

/**
 * Hides a material of the model of an actor.
 * @param pActor The actor.
 * @param index The material index.
 */
void hideMaterial(LiveActor* pActor, s32 index) {
    if (!pActor->mModelKeeper) {
        return;
    }

    nn::g3d::ModelObj* modelObj = pActor->mModelKeeper->mModelCafe->mModelG3D->mModelObj;

    if (index >= modelObj->GetNumMaterials()) {
        return;
    }

    modelObj->SetMaterialVisible(index, false);
}

/**
 * Shows a material of the model of an actor.
 * @param pActor The actor.
 * @param index The material index.
 */
void showMaterial(LiveActor* pActor, s32 index) {
    if (!pActor->mModelKeeper) {
        return;
    }

    nn::g3d::ModelObj* modelObj = pActor->mModelKeeper->mModelCafe->mModelG3D->mModelObj;

    if (index >= modelObj->GetNumMaterials()) {
        return;
    }

    modelObj->SetMaterialVisible(index, true);
}

/**
 * Hides all materials of the model of an actor.
 * @param pActor The actor.
 */
void hideMaterialAll(LiveActor* pActor) {
    if (!pActor->mModelKeeper) {
        return;
    }

    nn::g3d::ModelObj* modelObj = pActor->mModelKeeper->mModelCafe->mModelG3D->mModelObj;

    for (s32 i = 0; i < modelObj->GetNumMaterials(); i++) {
        modelObj->SetMaterialVisible(i, false);
    }
}

/**
 * Shows all materials of the model of an actor.
 * @param pActor The actor.
 */
void showMaterialAll(LiveActor* pActor) {
    if (!pActor->mModelKeeper) {
        return;
    }

    nn::g3d::ModelObj* modelObj = pActor->mModelKeeper->mModelCafe->mModelG3D->mModelObj;

    for (s32 i = 0; i < modelObj->GetNumMaterials(); i++) {
        modelObj->SetMaterialVisible(i, true);
    }
}

/**
 * Sets whether the level of detail of the model of an actor is disabled.
 * @param pActor The actor.
 * @param isDisable Whether to disable the level of detail.
 */
void setLodDisabled(LiveActor* pActor, bool isDisable) {
    if (pActor->mModelKeeper) {
        pActor->mModelKeeper->mIsLodDisabled = isDisable;
    }
}

/**
 * Makes all shapes of the model of an actor use programmable materials.
 * @param pActor The actor.
 */
void setMaterialProgrammable(LiveActor* pActor) {
    SimpleModelG3D* modelG3D = pActor->mModelKeeper->mModelCafe->mModelG3D;
    s32 shapeNum = modelG3D->mModelObj->GetNumShapes();

    for (s32 i = 0; i < shapeNum; i++) {
        (*modelG3D->mShapes)[i]._2a = true;
        modelG3D->_45 = true;
    }
}

/**
 * Sets a callback called after the world matrices of the model of an actor are updated.
 * @param pActor The actor.
 * @param rFunctor The callback.
 */
void setPostUpdateWorldMatrixCallback(LiveActor* pActor, const FunctorBase& rFunctor) {
    pActor->mModelKeeper->mModelCafe->mModelG3D->setPostUpdateWorldMatrixCallback(rFunctor);
}

/**
 * Checks whether the model of an actor has a joint.
 * @param pActor The actor.
 * @param pName The joint name.
 * @return Whether the joint exists.
 */
bool isExistJoint(const LiveActor* pActor, const char* pName) {
    return isExistJoint(pActor->mModelKeeper, pName);
}

/**
 * Gets the world matrix of a joint.
 * @param pActor The actor.
 * @param pName The joint name.
 * @return The joint matrix.
 */
sead::Matrix34f* getJointMtxPtr(const LiveActor* pActor, const char* pName) {
    return const_cast<sead::Matrix34f*>(getJointMtxPtr(pActor->mModelKeeper, pName));
}

/**
 * Gets the world matrix of a joint by index.
 * @param pActor The actor.
 * @param index The joint index.
 * @return The joint matrix.
 */
sead::Matrix34f* getJointMtxPtrByIndex(const LiveActor* pActor, s32 index) {
    return const_cast<sead::Matrix34f*>(getJointMtxPtrByIndex(pActor->mModelKeeper, index));
}

/**
 * Gets the local matrix of a joint.
 * @param pActor The actor.
 * @param pName The joint name.
 * @return The joint local matrix.
 */
const sead::Matrix34f* getJointLocalMtxPtr(const LiveActor* pActor, const char* pName) {
    return getJointLocalMtxPtr(pActor->mModelKeeper, pName);
}

/**
 * Gets the local matrix of a joint by index.
 * @param pActor The actor.
 * @param index The joint index.
 * @return The joint local matrix.
 */
const void* getJointLocalMtxPtrByIndex(const LiveActor* pActor, s32 index) {
    return getJointLocalMtxPtrByIndex(pActor->mModelKeeper, index);
}

/**
 * Calculates the position of a joint.
 * @param pPos The position.
 * @param pActor The actor.
 * @param pName The joint name.
 */
void calcJointPos(sead::Vector3f* pPos, const LiveActor* pActor, const char* pName) {
    getJointMtxPtr(pActor, pName)->getTranslation(*pPos);
}

/**
 * Calculates the side direction of a joint.
 * @param pDir The direction.
 * @param pActor The actor.
 * @param pName The joint name.
 */
void calcJointSideDir(sead::Vector3f* pDir, const LiveActor* pActor, const char* pName) {
    getJointMtxPtr(pActor, pName)->getBase(*pDir, 0);
    normalizeOrZero(pDir);
}

/**
 * Calculates the up direction of a joint.
 * @param pDir The direction.
 * @param pActor The actor.
 * @param pName The joint name.
 */
void calcJointUpDir(sead::Vector3f* pDir, const LiveActor* pActor, const char* pName) {
    getJointMtxPtr(pActor, pName)->getBase(*pDir, 1);
    normalizeOrZero(pDir);
}

/**
 * Calculates the front direction of a joint.
 * @param pDir The direction.
 * @param pActor The actor.
 * @param pName The joint name.
 */
void calcJointFrontDir(sead::Vector3f* pDir, const LiveActor* pActor, const char* pName) {
    getJointMtxPtr(pActor, pName)->getBase(*pDir, 2);
    normalizeOrZero(pDir);
}

/**
 * Transforms a vector by the matrix of a joint.
 * @param pOut The transformed vector.
 * @param rVec The vector.
 * @param pActor The actor.
 * @param pName The joint name.
 */
void multVecJointMtx(sead::Vector3f* pOut, const sead::Vector3f& rVec, LiveActor* pActor,
                     const char* pName) {
    pOut->setMul(*getJointMtxPtr(pActor, pName), rVec);
}

/**
 * Transforms a vector by the inverse matrix of a joint.
 * @param pOut The transformed vector.
 * @param rVec The vector.
 * @param pActor The actor.
 * @param pName The joint name.
 */
void multVecJointInvMtx(sead::Vector3f* pOut, const sead::Vector3f& rVec, LiveActor* pActor,
                        const char* pName) {
    sead::Matrix34f invMtx;
    invMtx.setInverse(*getJointMtxPtr(pActor, pName));
    pOut->setMul(invMtx, rVec);
}

/**
 * Sets whether a joint is visible.
 * @param pActor The actor.
 * @param pName The joint name.
 * @param isVisible Whether the joint is visible.
 */
void setJointVisibility(const LiveActor* pActor, const char* pName, bool isVisible) {
    setJointVisibility(pActor->mModelKeeper, pName, isVisible);
}

/**
 * Checks whether a joint is visible.
 * @param pActor The actor.
 * @param pName The joint name.
 * @return Whether the joint is visible.
 */
bool getJointVisibility(const LiveActor* pActor, const char* pName) {
    return getJointVisibility(pActor->mModelKeeper, pName);
}

/**
 * Gets the model name of an actor.
 * @param pActor The actor.
 * @return The model name.
 */
const char* getModelName(const LiveActor* pActor) {
    return pActor->mModelKeeper->mModelName;
}

/**
 * Calculates the bounding sphere radius of the model of an actor.
 * @param pActor The actor.
 * @return The radius.
 */
f32 calcModelBoundingSphereRadius(const LiveActor* pActor) {
    return alModelFunction::calcBoundingSphere(pActor->mModelKeeper->mModelCafe);
}

/**
 * Gets the bounding sphere of the model of an actor.
 * @param pCenter The sphere center.
 * @param pRadius The sphere radius.
 * @param pActor The actor.
 */
void getBoundingShpereCenterAndRadius(sead::Vector3f* pCenter, f32* pRadius,
                                      const LiveActor* pActor) {
    const nn::g3d::Sphere* bounding =
        pActor->mModelKeeper->mModelCafe->mModelG3D->mModelObj->GetBounding();
    const f32* center = reinterpret_cast<const f32*>(&bounding->center);
    pCenter->x = center[0];
    pCenter->y = center[1];
    pCenter->z = center[2];
    *pRadius = bounding->radius;
}

/**
 * Calculates the bounding box of the model of an actor.
 * @param pBox The bounding box.
 * @param pActor The actor.
 */
void calcModelBoundingBox(sead::BoundBox3f* pBox, const LiveActor* pActor) {
    alModelFunction::calcBoundingBox(pBox, pActor->mModelKeeper->mModelCafe);
}

/**
 * Submits the model of an actor for a view. Does nothing.
 * @param pActor The actor.
 * @param rViewMtx The view matrix.
 */
void submitViewModel(const LiveActor* pActor, const sead::Matrix34f& rViewMtx) {}

/**
 * Sets the level of detail distances of the model of an actor.
 * @param pActor The actor.
 * @param pSwitchDistances The distances to switch levels at.
 * @param lodNum The number of levels.
 * @param isDisableDemoLod Whether to disable the level of detail during demos.
 */
void setModelLodParams(const LiveActor* pActor, const f32* pSwitchDistances, s32 lodNum,
                       bool isDisableDemoLod) {
    pActor->mModelKeeper->setLodParams(pSwitchDistances, lodNum, &getTrans(pActor),
                                       isDisableDemoLod);
}

/**
 * Checks whether a sphere is outside the view frustum.
 * @param pDirector The clipping director.
 * @param rPos The sphere center.
 * @param radius The sphere radius.
 * @param nearClip The near clip distance.
 * @return Whether the sphere is clipped.
 */
bool isJudgedToClipFrustum(const ClippingDirectorBase* pDirector, const sead::Vector3f& rPos,
                           f32 radius, f32 nearClip) {
    return pDirector->mClippingJudge->isJudgedToClipFrustumUnUseFarLevel(rPos, radius, nearClip);
}

/**
 * Checks whether a sphere is outside the view frustum.
 * @param pActor The actor.
 * @param rPos The sphere center.
 * @param radius The sphere radius.
 * @param nearClip The near clip distance.
 * @return Whether the sphere is clipped.
 */
bool isJudgedToClipFrustum(const LiveActor* pActor, const sead::Vector3f& rPos, f32 radius,
                           f32 nearClip) {
    return isJudgedToClipFrustum(pActor->getSceneInfo()->clippingDirectorBase, rPos, radius,
                                 nearClip);
}

/**
 * Checks whether a sphere around an actor is outside the view frustum.
 * @param pActor The actor.
 * @param radius The sphere radius.
 * @param nearClip The near clip distance.
 * @return Whether the sphere is clipped.
 */
bool isJudgedToClipFrustum(const LiveActor* pActor, f32 radius, f32 nearClip) {
    const sead::Vector3f& trans = getTrans(pActor);
    return isJudgedToClipFrustum(pActor->getSceneInfo()->clippingDirectorBase, trans, radius,
                                 nearClip);
}

/**
 * Checks whether a sphere is outside the view frustum ignoring the far plane.
 * @param pDirector The clipping director.
 * @param rPos The sphere center.
 * @param radius The sphere radius.
 * @param nearClip The near clip distance.
 * @return Whether the sphere is clipped.
 */
bool isJudgedToClipFrustumWithoutFar(const ClippingDirectorBase* pDirector,
                                     const sead::Vector3f& rPos, f32 radius, f32 nearClip) {
    return pDirector->mClippingJudge->isJudgedToClipFrustum(rPos, radius, nearClip, 0);
}

/**
 * Checks whether a sphere is outside the view frustum ignoring the far plane.
 * @param pActor The actor.
 * @param rPos The sphere center.
 * @param radius The sphere radius.
 * @param nearClip The near clip distance.
 * @return Whether the sphere is clipped.
 */
bool isJudgedToClipFrustumWithoutFar(const LiveActor* pActor, const sead::Vector3f& rPos,
                                     f32 radius, f32 nearClip) {
    return isJudgedToClipFrustumWithoutFar(pActor->getSceneInfo()->clippingDirectorBase, rPos,
                                           radius, nearClip);
}

/**
 * Checks whether a sphere around an actor is outside the view frustum ignoring the far plane.
 * @param pActor The actor.
 * @param radius The sphere radius.
 * @param nearClip The near clip distance.
 * @return Whether the sphere is clipped.
 */
bool isJudgedToClipFrustumWithoutFar(const LiveActor* pActor, f32 radius, f32 nearClip) {
    const sead::Vector3f& trans = getTrans(pActor);
    return isJudgedToClipFrustumWithoutFar(pActor->getSceneInfo()->clippingDirectorBase, trans,
                                           radius, nearClip);
}

/**
 * Resets everything that depends on the position of an actor.
 * @param pActor The actor.
 * @param isSkipCalcAnim Whether to skip recalculating the animation.
 */
void resetPosition(LiveActor* pActor, bool isSkipCalcAnim) {
    if (pActor->mActorPoseKeeper && !isSkipCalcAnim) {
        alLiveActorFunction::calcAnimDirect(pActor);
    }

    if (pActor->mHitSensorKeeper) {
        alSensorFunction::clearHitSensors(pActor);
        alSensorFunction::updateHitSensorsAll(pActor);
    }

    if (pActor->mScreenPointKeeper) {
        alScreenPointFunction::updateScreenPointAll(pActor);
    }

    if (pActor->mCollider) {
        pActor->mCollider->onInvalidate();
    }

    if (pActor->mCollisionParts) {
        resetAllCollisionMtx(pActor);
    }

    if (pActor->getAudioKeeper() && pActor->getAudioKeeper()->getSeKeeper()) {
        pActor->getAudioKeeper()->getSeKeeper()->resetVelocity();
    }
}

/**
 * Moves an actor and resets everything that depends on its position.
 * @param pActor The actor.
 * @param rTrans The new position.
 * @param isSkipCalcAnim Whether to skip recalculating the animation.
 */
void resetPosition(LiveActor* pActor, const sead::Vector3f& rTrans, bool isSkipCalcAnim) {
    setTrans(pActor, rTrans);
    resetPosition(pActor, isSkipCalcAnim);
}

/**
 * Moves and rotates an actor and resets everything that depends on its position.
 * @param pActor The actor.
 * @param rTrans The new position.
 * @param rRotate The new rotation.
 */
void resetPosition(LiveActor* pActor, const sead::Vector3f& rTrans, const sead::Vector3f& rRotate) {
    updatePoseRotate(pActor, rRotate);
    setTrans(pActor, rTrans);
    resetPosition(pActor, false);
}
}  // namespace al
