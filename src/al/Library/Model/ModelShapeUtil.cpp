#include "Library/Model/ModelShapeUtil.hpp"

#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_SkeletonObj.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Shader/ForwardRendering/CubeMapKeeper.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace {

nn::g3d::ModelObj* getModelObj(const al::ModelKeeper* pKeeper) {
    return pKeeper->getModelCafe()->getModelG3D()->getModelObj();
}

const nn::g3d::SkeletonObj* getSkeletonObj(const al::ModelKeeper* pKeeper) {
    return getModelObj(pKeeper)->GetSkeleton();
}

}  // namespace

namespace al {

/**
 * Gets the number of joints of the model.
 * @param pKeeper Model keeper.
 * @return Number of joints.
 */
s32 getJointNum(const ModelKeeper* pKeeper) {
    const nn::g3d::SkeletonObj* skeleton = getSkeletonObj(pKeeper);
    if (!skeleton) {
        return 0;
    }

    return skeleton->GetBoneCount();
}

/**
 * Gets the index of a joint.
 * @param pKeeper Model keeper.
 * @param pName Joint name.
 * @return Joint index, or -1 if not found.
 */
s32 getJointIndex(const ModelKeeper* pKeeper, const char* pName) {
    const nn::g3d::SkeletonObj* skeleton = getSkeletonObj(pKeeper);
    if (!skeleton) {
        return -1;
    }

    return skeleton->GetRes()->FindBoneIndex(pName);
}

/**
 * Checks whether a joint exists.
 * @param pKeeper Model keeper.
 * @param pName Joint name.
 * @return Whether the joint exists.
 */
bool isExistJoint(const ModelKeeper* pKeeper, const char* pName) {
    const nn::g3d::SkeletonObj* skeleton = getSkeletonObj(pKeeper);
    if (!skeleton) {
        return false;
    }

    if (skeleton->GetRes()->FindBoneIndex(pName) < 0) {
        return false;
    }

    return true;
}

/**
 * Gets the name of a joint.
 * @param pKeeper Model keeper.
 * @param index Joint index.
 * @return Joint name.
 */
const char* getJointName(const ModelKeeper* pKeeper, s32 index) {
    const nn::g3d::SkeletonObj* skeleton = getSkeletonObj(pKeeper);
    if (!skeleton) {
        return nullptr;
    }

    return skeleton->GetRes()->GetBoneName(index);
}

/**
 * Gets the world matrix of a joint.
 * @param pKeeper Model keeper.
 * @param pName Joint name.
 * @return Joint world matrix.
 */
const sead::Matrix34f* getJointMtxPtr(const ModelKeeper* pKeeper, const char* pName) {
    const nn::g3d::SkeletonObj* skeleton = getSkeletonObj(pKeeper);
    if (!skeleton) {
        return nullptr;
    }

    s32 index = getJointIndex(pKeeper, pName);
    return pKeeper->getWorldMtxPtrByIndex(skeleton->GetBone(index)->GetIndex());
}

/**
 * Gets the world matrix of a joint.
 * @param pKeeper Model keeper.
 * @param index Joint index.
 * @return Joint world matrix.
 */
const sead::Matrix34f* getJointMtxPtrByIndex(const ModelKeeper* pKeeper, s32 index) {
    const nn::g3d::SkeletonObj* skeleton = getSkeletonObj(pKeeper);
    if (!skeleton) {
        return nullptr;
    }

    return pKeeper->getWorldMtxPtrByIndex(skeleton->GetBone(index)->GetIndex());
}

/**
 * Gets the local matrix of a joint.
 * @param pKeeper Model keeper.
 * @param pName Joint name.
 * @return Joint matrix.
 */
const sead::Matrix34f* getJointLocalMtxPtr(const ModelKeeper* pKeeper, const char* pName) {
    const nn::g3d::SkeletonObj* skeleton = getSkeletonObj(pKeeper);
    if (!skeleton) {
        return nullptr;
    }

    s32 index = getJointIndex(pKeeper, pName);
    return pKeeper->getWorldMtxPtrByIndex(skeleton->GetBone(index)->GetIndex());
}

/**
 * Gets the local matrix of a joint.
 * @param pKeeper Model keeper.
 * @param index Joint index.
 * @return Joint local matrix.
 */
const void* getJointLocalMtxPtrByIndex(const ModelKeeper* pKeeper, s32 index) {
    const nn::g3d::SkeletonObj* skeleton = getSkeletonObj(pKeeper);
    if (!skeleton) {
        return nullptr;
    }

    return skeleton->GetLocalMtx(skeleton->GetBone(index)->GetIndex());
}

/**
 * Gets the local translation of a joint.
 * @param pOut Output translation.
 * @param pKeeper Model keeper.
 * @param pName Joint name.
 */
void getJointLocalTrans(sead::Vector3f* pOut, const ModelKeeper* pKeeper, const char* pName) {
    s32 index = getJointIndex(pKeeper, pName);
    const nn::util::Float3& trans = getSkeletonObj(pKeeper)->GetBone(index)->GetTranslate();
    pOut->x = trans.x;
    pOut->y = trans.y;
    pOut->z = trans.z;
}

/**
 * Gets the local translation of a joint.
 * @param pOut Output translation.
 * @param pKeeper Model keeper.
 * @param index Joint index.
 */
void getJointLocalTrans(sead::Vector3f* pOut, const ModelKeeper* pKeeper, s32 index) {
    const nn::util::Float3& trans = getSkeletonObj(pKeeper)->GetBone(index)->GetTranslate();
    pOut->x = trans.x;
    pOut->y = trans.y;
    pOut->z = trans.z;
}

/**
 * Gets the parent index of a joint.
 * @param pKeeper Model keeper.
 * @param index Joint index.
 * @return Parent joint index.
 */
s32 getParentJointIndex(const ModelKeeper* pKeeper, s32 index) {
    return getSkeletonObj(pKeeper)->GetBone(index)->GetParentIndex();
}

/**
 * Sets the visibility of a joint.
 * @param pKeeper Model keeper.
 * @param pName Joint name.
 * @param isVisible Whether the joint is visible.
 */
void setJointVisibility(const ModelKeeper* pKeeper, const char* pName, bool isVisible) {
    nn::g3d::ModelObj* modelObj = getModelObj(pKeeper);
    s32 index = getJointIndex(pKeeper, pName);
    if (index < 0) {
        return;
    }

    bool isPrevVisible = modelObj->IsBoneVisible(index);
    u32 bit = 1 << index;
    u32& word = modelObj->GetBoneVisibilityArray()[static_cast<u32>(index) >> 5];
    word = (word & ~bit) | (static_cast<u32>(isVisible) << index);
    auto callback = modelObj->GetBoneVisibilityCallback();
    if (callback && isPrevVisible != isVisible) {
        callback(modelObj, index);
    }
}

/**
 * Gets the visibility of a joint.
 * @param pKeeper Model keeper.
 * @param pName Joint name.
 * @return Whether the joint is visible.
 */
bool getJointVisibility(const ModelKeeper* pKeeper, const char* pName) {
    s32 index = getJointIndex(pKeeper, pName);
    if (index < 0) {
        return false;
    }

    return getModelObj(pKeeper)->IsBoneVisible(index);
}

/**
 * Gets the index of a material (unsupported, always 0).
 * @param pKeeper Model keeper.
 * @param pName Material name.
 * @return Material index.
 */
s32 getMaterialIndex(const ModelKeeper* pKeeper, const char* pName) {
    return 0;
}

/**
 * Hides a material (does nothing).
 * @param pKeeper Model keeper.
 * @param pName Material name.
 */
void hideMaterial(ModelKeeper* pKeeper, const char* pName) {}

/**
 * Shows a material (does nothing).
 * @param pKeeper Model keeper.
 * @param pName Material name.
 */
void showMaterial(ModelKeeper* pKeeper, const char* pName) {}

/**
 * Applies a cube map to all shapes of the model.
 * @param pKeeper Model keeper.
 * @param pInfo Graphics system info.
 * @param pName Cube map name.
 */
void forceApplyCubeMap(ModelKeeper* pKeeper, const GraphicsSystemInfo* pInfo, const char* pName) {
    SimpleModelG3D* model = pKeeper->getModelCafe()->getModelG3D();
    s32 index = pInfo->getShaderCubeMapKeeper()->findCubeMapIndexByName(pName);
    model->setCubeMapIndexAllShape(index);
}

}  // namespace al
