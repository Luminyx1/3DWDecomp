#include "Library/Shadow/Common/ShadowUtil.hpp"

#include "Library/Execute/ActorExecuteInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Model/ModelDrawerBase.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Shadow/ShadowKeeper.hpp"
#include "Library/Shadow/ShadowMaskBase.hpp"
#include "Library/Shadow/ShadowMaskCube.hpp"
#include "Library/Shadow/ShadowMaskDrawer.hpp"

namespace al {
bool tryGetArg(f32* pValue, const ActorInitInfo& rInfo, const char* pKey);

/**
 * Checks whether the actor has at least one shadow mask.
 * @param pActor Actor.
 * @return Whether the actor has a shadow mask.
 */
bool isExistShadow(LiveActor* pActor) {
    ShadowKeeper* keeper = pActor->mShadowKeeper;

    if (keeper == nullptr) {
        return false;
    }

    return keeper->getShadowMaskNum() > 0;
}

/**
 * Checks whether the actor has a shadow mask with the given name.
 * @param pActor Actor.
 * @param pName Shadow mask name.
 * @return Whether the shadow mask exists.
 */
bool isExistShadow(LiveActor* pActor, const char* pName) {
    ShadowKeeper* keeper = pActor->mShadowKeeper;

    if (keeper == nullptr) {
        return false;
    }

    return keeper->findShadowMask(pName) != nullptr;
}

/**
 * Checks whether all shadow masks of the actor are hidden.
 * @param pActor Actor.
 * @return Whether the shadow is hidden.
 */
bool isHideShadow(const LiveActor* pActor) {
    return pActor->mShadowKeeper->isHide();
}

/**
 * Hides all shadow masks of the actor.
 * @param pActor Actor.
 */
void hideShadow(LiveActor* pActor) {
    pActor->mShadowKeeper->hide();
}

/**
 * Shows all shadow masks of the actor.
 * @param pActor Actor.
 */
void showShadow(LiveActor* pActor) {
    pActor->mShadowKeeper->show();
}

/**
 * Removes the actor's model from all depth shadow drawers.
 * @param pActor Actor.
 */
void hideShadowDepth(LiveActor* pActor) {
    ActorExecuteInfo* info = pActor->mActorExecuteInfo;
    s32 drawerNum = info->mDrawerCount;

    if (drawerNum < 1) {
        return;
    }

    alModelCafe* model = pActor->mModelKeeper->getModelCafe();

    for (s32 i = 0; i < drawerNum; i++) {
        ModelDrawerBase* drawer = info->mDrawers[i];

        if (drawer->isDepthShadowDrawer()) {
            drawer->removeModel(model);
        }
    }
}

/**
 * Adds the actor's model to all depth shadow drawers.
 * @param pActor Actor.
 */
void showShadowDepth(LiveActor* pActor) {
    ActorExecuteInfo* info = pActor->mActorExecuteInfo;
    s32 drawerNum = info->mDrawerCount;

    if (drawerNum < 1) {
        return;
    }

    alModelCafe* model = pActor->mModelKeeper->getModelCafe();

    for (s32 i = 0; i < drawerNum; i++) {
        ModelDrawerBase* drawer = info->mDrawers[i];

        if (drawer->isDepthShadowDrawer()) {
            drawer->addModel(model);
        }
    }
}

/**
 * Shows a shadow mask.
 * @param pActor Host actor.
 * @param pMask Shadow mask.
 */
void showShadow(LiveActor* pActor, ShadowMaskBase* pMask) {
    if (!pMask->mIsHide) {
        return;
    }

    pMask->mIsHide = false;

    if (pMask->mIsValid) {
        ShadowMaskFunction::getShadowMaskKeeper(pActor)->registerShadowMask(pMask);
    }
}

/**
 * Shows a shadow mask by name.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 */
void showShadow(LiveActor* pActor, const char* pName) {
    ShadowMaskBase* mask = pActor->mShadowKeeper->findShadowMask(pName);

    if (mask != nullptr) {
        showShadow(pActor, mask);
    }
}

/**
 * Shows all shadow masks of a draw category.
 * @param pActor Host actor.
 * @param category Draw category.
 */
void showShadow(LiveActor* pActor, ShadowMaskDrawCategory category) {
    for (s32 i = 0; i < pActor->mShadowKeeper->getShadowMaskNum(); i++) {
        ShadowMaskBase* mask = pActor->mShadowKeeper->mMaskArray.at(i);

        if (mask != nullptr && mask->getDrawCategory() == category) {
            showShadow(pActor, mask);
        }
    }
}

/**
 * Hides a shadow mask.
 * @param pActor Host actor.
 * @param pMask Shadow mask.
 */
void hideShadow(LiveActor* pActor, ShadowMaskBase* pMask) {
    if (pMask->mIsHide || pMask->mIsIgnoreHide) {
        return;
    }

    pMask->mIsHide = true;

    if (pMask->mIsValid) {
        ShadowMaskFunction::getShadowMaskKeeper(pActor)->removeShadowMask(pMask);
    }
}

/**
 * Hides a shadow mask by name.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 */
void hideShadow(LiveActor* pActor, const char* pName) {
    ShadowMaskBase* mask = pActor->mShadowKeeper->findShadowMask(pName);

    if (mask != nullptr) {
        hideShadow(pActor, mask);
    }
}

/**
 * Hides all shadow masks of a draw category.
 * @param pActor Host actor.
 * @param category Draw category.
 */
void hideShadow(LiveActor* pActor, ShadowMaskDrawCategory category) {
    for (s32 i = 0; i < pActor->mShadowKeeper->getShadowMaskNum(); i++) {
        ShadowMaskBase* mask = pActor->mShadowKeeper->mMaskArray.at(i);

        if (mask != nullptr && mask->getDrawCategory() == category) {
            hideShadow(pActor, mask);
        }
    }
}

/**
 * Checks whether a shadow mask is hidden.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 * @return Whether the shadow mask is hidden.
 */
bool isHideShadow(LiveActor* pActor, const char* pName) {
    return pActor->mShadowKeeper->findShadowMask(pName)->mIsHide;
}

/**
 * Validates a shadow mask.
 * @param pActor Host actor.
 * @param pMask Shadow mask.
 */
void validateShadow(LiveActor* pActor, ShadowMaskBase* pMask) {
    if (pMask->mIsValid) {
        return;
    }

    pMask->mIsValid = true;

    if (!pMask->mIsHide) {
        ShadowMaskFunction::getShadowMaskKeeper(pActor)->registerShadowMask(pMask);
    }
}

/**
 * Invalidates a shadow mask.
 * @param pActor Host actor.
 * @param pMask Shadow mask.
 */
void invalidateShadow(LiveActor* pActor, ShadowMaskBase* pMask) {
    if (!pMask->mIsValid) {
        return;
    }

    pMask->mIsValid = false;

    if (!pMask->mIsHide) {
        ShadowMaskFunction::getShadowMaskKeeper(pActor)->removeShadowMask(pMask);
    }
}

/**
 * Validates a shadow mask by name.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 */
void validateShadow(LiveActor* pActor, const char* pName) {
    ShadowMaskBase* mask = pActor->mShadowKeeper->findShadowMask(pName);

    if (mask != nullptr) {
        validateShadow(pActor, mask);
    }
}

/**
 * Invalidates a shadow mask by name.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 */
void invalidateShadow(LiveActor* pActor, const char* pName) {
    ShadowMaskBase* mask = pActor->mShadowKeeper->findShadowMask(pName);

    if (mask != nullptr) {
        invalidateShadow(pActor, mask);
    }
}

/**
 * Hides and invalidates all shadow masks of a draw category.
 * @param pActor Host actor.
 * @param category Draw category.
 */
void invalidateShadow(LiveActor* pActor, ShadowMaskDrawCategory category) {
    for (s32 i = 0; i < pActor->mShadowKeeper->getShadowMaskNum(); i++) {
        ShadowMaskBase* mask = pActor->mShadowKeeper->mMaskArray.at(i);

        if (mask != nullptr && mask->getDrawCategory() == category) {
            hideShadow(pActor, mask);
            mask->mIsValid = false;
        }
    }
}

/**
 * Hides and invalidates all shadow masks of the shadow draw categories.
 * @param pActor Host actor.
 */
void invalidateShadowIntensityAll(LiveActor* pActor) {
    for (s32 i = 0; i < ShadowMaskDrawCategory::LightScale; i++) {
        invalidateShadow(pActor, ShadowMaskDrawCategory(i));
    }
}

/**
 * Sets whether the shadow matrices of all shadow masks are fixed.
 * @param pActor Host actor.
 * @param isFixed Whether the shadow is fixed.
 */
void setShadowFixed(LiveActor* pActor, bool isFixed) {
    ShadowKeeper* keeper = pActor->mShadowKeeper;

    if (keeper == nullptr) {
        return;
    }

    for (s32 i = 0; i < keeper->getShadowMaskNum(); i++) {
        ShadowMaskBase* mask = keeper->mMaskArray.at(i);

        if (mask != nullptr) {
            mask->mIsShadowFixed = isFixed;
        }
    }
}

/**
 * Sets the drop direction of all shadow masks.
 * @param pActor Host actor.
 * @param rDir Drop direction.
 */
void setShadowDropDir(LiveActor* pActor, const sead::Vector3f& rDir) {
    for (s32 i = 0; i < pActor->mShadowKeeper->getShadowMaskNum(); i++) {
        ShadowMaskBase* mask = pActor->mShadowKeeper->mMaskArray.at(i);

        if (mask != nullptr) {
            mask->mDropDir = rDir;
        }
    }
}

/**
 * Sets the drop direction of a shadow mask.
 * @param pActor Host actor.
 * @param rDir Drop direction.
 * @param pName Shadow mask name.
 */
void setShadowDropDir(LiveActor* pActor, const sead::Vector3f& rDir, const char* pName) {
    ShadowMaskBase* mask = pActor->mShadowKeeper->findShadowMask(pName);

    if (mask != nullptr) {
        mask->mDropDir = rDir;
    }
}

/**
 * Sets the drop direction of all shadow masks to the actor's down direction.
 * @param pActor Host actor.
 */
void setShadowDropDirActorDown(LiveActor* pActor) {
    sead::Vector3f up;
    calcUpDir(&up, pActor);
    setShadowDropDir(pActor, -up);
}

/**
 * Sets the size of a cube or cylinder shadow mask.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 * @param rSize Size.
 */
void setShadowMaskSize(LiveActor* pActor, const char* pName, const sead::Vector3f& rSize) {
    setShadowMaskSize(pActor, pName, rSize.x, rSize.y, rSize.z);
}

/**
 * Sets the size of a cube or cylinder shadow mask.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 * @param x Size in x.
 * @param y Size in y.
 * @param z Size in z.
 */
void setShadowMaskSize(LiveActor* pActor, const char* pName, f32 x, f32 y, f32 z) {
    ShadowMaskBase* mask = pActor->mShadowKeeper->findShadowMask(pName);

    if (mask == nullptr) {
        return;
    }

    if (mask->getShadowMaskType() == ShadowMaskType::Cube) {
        auto* cube = static_cast<ShadowMaskCube*>(mask);
        cube->mScale.x = x;
        cube->mScale.z = z;
    } else if (mask->getShadowMaskType() == ShadowMaskType::Cylinder) {
        static_cast<ShadowMaskCube*>(mask)->mScale.x = x;
    }
}

/**
 * Calculates the size of a cube or cylinder shadow mask.
 * @param pOut Output size.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 */
void calcShadowMaskSize(sead::Vector3f* pOut, LiveActor* pActor, const char* pName) {
    ShadowMaskBase* mask = pActor->mShadowKeeper->findShadowMask(pName);

    if (mask->getShadowMaskType() == ShadowMaskType::Cube) {
        auto* cube = static_cast<ShadowMaskCube*>(mask);
        pOut->set(cube->mScale.x, 0.0f, cube->mScale.z);
        return;
    }

    if (mask->getShadowMaskType() == ShadowMaskType::Cylinder) {
        f32 radius = static_cast<ShadowMaskCube*>(mask)->mScale.x;
        pOut->set(radius, 0.0f, radius);
    }
}

/**
 * Gets the drop length of a shadow mask.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 * @return Drop length.
 */
f32 getShadowDropLength(const LiveActor* pActor, const char* pName) {
    return pActor->mShadowKeeper->findShadowMask(pName)->mDropLength;
}

/**
 * Sets the drop length of all shadow masks.
 * @param pActor Host actor.
 * @param length Drop length.
 */
void setShadowDropLength(LiveActor* pActor, f32 length) {
    for (s32 i = 0; i < pActor->mShadowKeeper->getShadowMaskNum(); i++) {
        ShadowMaskBase* mask = pActor->mShadowKeeper->mMaskArray.at(i);

        if (mask != nullptr) {
            mask->mDropLength = length;
        }
    }
}

/**
 * Sets the drop length of a shadow mask.
 * @param pActor Host actor.
 * @param length Drop length.
 * @param pName Shadow mask name.
 */
void setShadowDropLength(LiveActor* pActor, f32 length, const char* pName) {
    ShadowMaskBase* mask = pActor->mShadowKeeper->findShadowMask(pName);

    if (mask != nullptr) {
        mask->mDropLength = length;
    }
}

/**
 * Scales the drop length of all shadow masks of a draw category.
 * @param pActor Host actor.
 * @param scale Drop length scale.
 * @param category Draw category.
 */
void setShadowDropLengthScaleWithDrawCategory(LiveActor* pActor, f32 scale,
                                              ShadowMaskDrawCategory category) {
    for (s32 i = 0; i < pActor->mShadowKeeper->getShadowMaskNum(); i++) {
        ShadowMaskBase* mask = pActor->mShadowKeeper->mMaskArray.at(i);

        if (mask != nullptr && mask->getDrawCategory() == category) {
            mask->mDropLength *= scale;
        }
    }
}

/**
 * Sets the drop length of all shadow masks of a draw category.
 * @param pActor Host actor.
 * @param length Drop length.
 * @param category Draw category.
 */
void setShadowDropLengthWithDrawCategory(LiveActor* pActor, f32 length,
                                         ShadowMaskDrawCategory category) {
    for (s32 i = 0; i < pActor->mShadowKeeper->getShadowMaskNum(); i++) {
        ShadowMaskBase* mask = pActor->mShadowKeeper->mMaskArray.at(i);

        if (mask != nullptr && mask->getDrawCategory() == category) {
            mask->mDropLength = length;
        }
    }
}

/**
 * Sets the drop length of a shadow mask so that it ends at another shadow mask of the host.
 * @param pMask Shadow mask.
 * @param pTargetName Name of the target shadow mask.
 * @param rPos Plane normal.
 */
void setShadowDropLengthEvenWithTarget(ShadowMaskBase* pMask, const char* pTargetName,
                                       const sead::Vector3f& rPos) {
    ShadowMaskBase* target = pMask->mHost->mShadowKeeper->findShadowMask(pTargetName);
    setShadowDropLengthEvenWithTarget(pMask, target, rPos);
}

/**
 * Sets the drop length of a shadow mask so that it ends at another shadow mask.
 * @param pMask Shadow mask.
 * @param pTarget Target shadow mask.
 * @param rNormal Plane normal.
 */
void setShadowDropLengthEvenWithTarget(ShadowMaskBase* pMask, const ShadowMaskBase* pTarget,
                                       const sead::Vector3f& rNormal) {
    if (pTarget == nullptr || pTarget == pMask) {
        return;
    }

    sead::Vector3f trans;
    pMask->mShadowMtx.getTranslation(trans);
    const sead::Matrix34f& mtx = pMask->mShadowMtx;
    sead::Vector3f up(mtx.m[0][1], mtx.m[1][1], mtx.m[2][1]);

    if (isNearZero(up, 0.001f) || isNearZero(up.dot(rNormal), 0.001f)) {
        pMask->mDropLength = 0.0f;
        return;
    }

    trans += up * 0.5f;

    sead::Vector3f targetUp;
    pTarget->mShadowMtx.getBase(targetUp, 1);
    sead::Vector3f targetTrans;
    pTarget->mShadowMtx.getTranslation(targetTrans);
    targetTrans -= targetUp * 0.5f;
    targetUp.length();

    f32 length = calcDistanceVecToPlane(-up, trans, rNormal, targetTrans);

    if (length < 0.0f) {
        return;
    }

    pMask->mDropLength = length;
}

/**
 * Sets the drop length of all shadow masks of a draw category so that they end at a shadow mask
 * of another actor.
 * @param pActor Host actor.
 * @param category Draw category.
 * @param pTargetActor Actor owning the target shadow mask.
 * @param pTargetName Name of the target shadow mask.
 */
void setShadowDropLengthEvenWithDrawCategory(LiveActor* pActor, ShadowMaskDrawCategory category,
                                             const LiveActor* pTargetActor,
                                             const char* pTargetName) {
    ShadowMaskBase* target = pTargetActor->mShadowKeeper->findShadowMask(pTargetName);
    ShadowKeeper* keeper = pActor->mShadowKeeper;
    s32 maskNum = keeper->getShadowMaskNum();

    for (s32 i = 0; i < maskNum; i++) {
        ShadowMaskBase* mask = keeper->mMaskArray.at(i);

        if (mask != nullptr && mask->getDrawCategory() == category) {
            setShadowDropLengthEvenWithTarget(mask, target, sead::Vector3f::ey);
        }
    }
}

/**
 * Sets the plane normal used for the drop length of all shadow masks.
 * @param pActor Host actor.
 * @param rNormal Plane normal.
 */
void setShadowDropLengthEvenPlaneNormal(const LiveActor* pActor, const sead::Vector3f& rNormal) {
    for (s32 i = 0; i < pActor->mShadowKeeper->getShadowMaskNum(); i++) {
        ShadowMaskBase* mask = pActor->mShadowKeeper->mMaskArray.at(i);

        if (mask != nullptr) {
            mask->mUp = rNormal;
        }
    }
}

/**
 * Gets the maximum drop length of all shadow masks.
 * @param pActor Host actor.
 * @return Maximum drop length.
 */
f32 getShadowDropLengthMax(const LiveActor* pActor) {
    ShadowKeeper* keeper = pActor->mShadowKeeper;
    f32 max = 0.0f;

    for (s32 i = 0; i < keeper->getShadowMaskNum(); i++) {
        f32 length = keeper->mMaskArray.unsafeAt(i)->mDropLength;

        if (max < length) {
            max = length;
        }
    }

    return max;
}

/**
 * Sets the user shadow intensity of a shadow mask.
 * @param pActor Host actor.
 * @param intensity Shadow intensity.
 * @param pName Shadow mask name.
 */
void setShadowIntensityUser(LiveActor* pActor, u8 intensity, const char* pName) {
    ShadowMaskBase* mask = pActor->mShadowKeeper->findShadowMask(pName);

    if (mask != nullptr) {
        mask->mIsApplyShadowIntensityUser = true;
        mask->mShadowIntensityUser = intensity;
    }
}

/**
 * Gets the shadow intensity of a shadow mask.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 * @return Shadow intensity.
 */
f32 getShadowIntensity(LiveActor* pActor, const char* pName) {
    return pActor->mShadowKeeper->findShadowMask(pName)->getShadowIntensity();
}

/**
 * Checks whether a shadow mask draws to an ambient occlusion category.
 * @param pMask Shadow mask.
 * @return Whether the category is an ambient occlusion category.
 */
bool isShadowMaskDrawCategoryAO(const ShadowMaskBase* pMask) {
    switch (pMask->getDrawCategory().value()) {
    case ShadowMaskDrawCategory::MapObjAO:
    case ShadowMaskDrawCategory::EnemyAO:
    case ShadowMaskDrawCategory::PlayerAO:
    case ShadowMaskDrawCategory::AllAO:
    case ShadowMaskDrawCategory::MapObjAOSO:
    case ShadowMaskDrawCategory::MapObjAndWaterAOSO:
    case ShadowMaskDrawCategory::EnemyAOSO:
    case ShadowMaskDrawCategory::PlayerAOSO:
    case ShadowMaskDrawCategory::AllAOSO:
        return true;
    default:
        return false;
    }
}

/**
 * Checks whether a shadow mask draws to a light scale category.
 * @param pMask Shadow mask.
 * @return Whether the category is a light scale category.
 */
bool isShadowMaskDrawCategoryLightScale(const ShadowMaskBase* pMask) {
    return pMask->getDrawCategory() == ShadowMaskDrawCategory::LightScale ||
           pMask->getDrawCategory() == ShadowMaskDrawCategory::LightScaleLight;
}

/**
 * Gets the fixed texture scale of a cube shadow mask.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 * @return Fixed texture scale.
 */
f32 getShadowTextureFixedScale(const LiveActor* pActor, const char* pName) {
    return static_cast<ShadowMaskCube*>(pActor->mShadowKeeper->findShadowMask(pName))
        ->mTextureFixedScale;
}

/**
 * Sets the fixed texture scale of a cube shadow mask.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 * @param scale Fixed texture scale.
 */
void setShadowTextureFixedScale(const LiveActor* pActor, const char* pName, f32 scale) {
    static_cast<ShadowMaskCube*>(pActor->mShadowKeeper->findShadowMask(pName))->mTextureFixedScale =
        scale;
}

/**
 * Gets the offset of a shadow mask.
 * @param pActor Host actor.
 * @param pName Shadow mask name.
 * @return Offset.
 */
const sead::Vector3f& getShadowMaskOffset(const LiveActor* pActor, const char* pName) {
    return pActor->mShadowKeeper->findShadowMask(pName)->mOffset;
}

/**
 * Sets the offset of a shadow mask.
 * @param pActor Host actor.
 * @param rOffset Offset.
 * @param pName Shadow mask name.
 */
void setShadowMaskOffset(const LiveActor* pActor, const sead::Vector3f& rOffset,
                         const char* pName) {
    pActor->mShadowKeeper->findShadowMask(pName)->mOffset.set(rOffset);
}

/**
 * Sets the drop length from the placement argument "ShadowLength" if it is positive.
 * @param pActor Host actor.
 * @param rInfo Actor init info.
 * @param pName Shadow mask name, or nullptr for all shadow masks.
 * @return Whether the drop length was set.
 */
bool trySetShadowLength(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pName) {
    f32 length = -1.0f;
    tryGetArg(&length, rInfo, "ShadowLength");

    if (length > 0.0f) {
        if (pName != nullptr) {
            setShadowDropLength(pActor, length, pName);
        } else {
            setShadowDropLength(pActor, length);
        }

        return true;
    }

    return false;
}

}  // namespace al
