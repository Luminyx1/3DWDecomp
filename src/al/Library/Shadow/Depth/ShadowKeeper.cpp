#include "Library/Shadow/ShadowKeeper.hpp"

#include "Library/Shadow/ShadowMaskCastOvalCylinder.hpp"
#include "Library/Shadow/ShadowMaskCube.hpp"
#include "Library/Shadow/ShadowMaskCylinder.hpp"
#include "Library/Shadow/ShadowMaskSphere.hpp"
#include "Library/Yaml/MacroUtil.hpp"

namespace al {

/**
 * Binds the sphere parameter group to a sphere shadow mask.
 * @param pMask Sphere shadow mask.
 */
void ShadowKeeper::setupShadowMaskSphereParam(ShadowMaskSphere* pMask) const {
    auto& group = np_ShadowMaskSphereParam::ShadowMaskSphereParam;
    group.readyToSetPtr();
    group.setParamPtr("Scale", &pMask->mScale);
    group.setParamPtr("Exp", &pMask->mExp);
    group.setParamPtr("IsEnableCollisionCheck", &pMask->mIsEnableCollisionCheck);
    group.setParamPtr("CollisionCheckLength", &pMask->mCollisionCheckLength);
}

/**
 * Binds the cylinder parameter group to a cylinder shadow mask.
 * @param pMask Cylinder shadow mask.
 */
void ShadowKeeper::setupShadowMaskCylinderParam(ShadowMaskCylinder* pMask) const {
    auto& group = np_ShadowMaskCylinderParam::ShadowMaskCylinderParam;
    group.readyToSetPtr();
    group.setParamPtr("Radius", &pMask->mScale.x);
    group.setParamPtr("ExpXZ", &pMask->mExpXZ);
    group.setParamPtr("ExpY", &pMask->mExpY);
    group.setParamPtr("DistYBase", &pMask->mDistYBase);
}

/**
 * Binds the cube parameter group to a cube shadow mask.
 * @param pMask Cube shadow mask.
 */
void ShadowKeeper::setupShadowMaskCubeParam(ShadowMaskCube* pMask) const {
    auto& group = np_ShadowMaskCubeParam::ShadowMaskCubeParam;
    group.readyToSetPtr();
    group.setParamPtr("Scale", &pMask->mScale);
    group.setParamPtr("Exp", &pMask->mExp);
    group.setParamPtr("DistYBase", &pMask->mDistYBase);
    group.setParamPtr("TextureBaseName", &pMask->mTextureBaseName);
    group.setParamPtr("TextureFixedScale", &pMask->mTextureFixedScale);
}

/**
 * Binds the cast oval cylinder parameter group to a cast oval cylinder shadow mask.
 * @param pMask Cast oval cylinder shadow mask.
 */
void ShadowKeeper::setupShadowMaskCastOvalCylinderParam(ShadowMaskCastOvalCylinder* pMask) const {
    auto& group = np_ShadowMaskCastOvalCylinderParam::ShadowMaskCastOvalCylinderParam;
    group.readyToSetPtr();
    group.setParamPtr("Scale", &pMask->mScale);
    group.setParamPtr("ExpXZ", &pMask->mExpXZ);
    group.setParamPtr("ExpY", &pMask->mExpY);
    group.setParamPtr("DistYBase", &pMask->mDistYBase);
}

/**
 * Binds the common shadow mask parameter group to this info.
 */
void ShadowKeeper::ShadowMaskBaseInfo::setPtr() {
    auto& group = np_ShadowMaskCommon::ShadowMaskCommon;
    group.readyToSetPtr();
    group.setParamPtr("Name", &mName);
    group.setParamPtr("ShadowMaskType", &mShadowMaskType);
    group.setParamPtr("ActorJointName", &mActorJointName);
    group.setParamPtr("Offset", &mOffset);
    group.setParamPtr("Color", &mColor);
    group.setParamPtr("IsApplyShadowIntensityUser", &mIsApplyShadowIntensityUser);
    group.setParamPtr("ShadowIntensityUser", &mShadowIntensityUser);
    group.setParamPtr("IsIgnoreHide", &mIsIgnoreHide);
    group.setParamPtr("IsFollowHostScale", &mIsFollowHostScale);
    group.setParamPtr("DrawCategory", &mDrawCategory);
    group.setParamPtr("IsShadowFixed", &mIsShadowFixed);
    group.setParamPtr("SetHeightEvenTargetName", &mSetHeightEvenTargetName);
}

}  // namespace al
