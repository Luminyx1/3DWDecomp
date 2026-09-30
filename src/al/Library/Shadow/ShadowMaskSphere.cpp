#include "Library/Shadow/ShadowMaskSphere.hpp"

#include "Library/Connector/MtxConnector.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Shadow/ShadowMaskDrawer.hpp"

namespace al {

/**
 * Constructs a sphere shadow mask.
 * @param pName Name of the shadow mask.
 */
ShadowMaskSphere::ShadowMaskSphere(const char* pName)
    : ShadowMaskBase(pName), mScale(300.0f), mExp(100.0f), mIsEnableCollisionCheck(false),
      mCollisionCheckLength(700.0f) {}

/**
 * Declares the shadow mask to the shadow mask keeper.
 * @param category Draw category.
 */
void ShadowMaskSphere::declare(ShadowMaskDrawCategory category) {
    ShadowMaskFunction::getShadowMaskKeeper(mHost)->declare(ShadowMaskType::Sphere, category);
}

/**
 * Calculates the shadow matrix of the sphere.
 * @param pMtx Output matrix.
 */
void ShadowMaskSphere::calcShadowMatrix(sead::Matrix34f* pMtx) {
    sead::Vector3f trans;
    if (mMtxConnector) {
        mMtxConnector->calcConnectInfo(&trans, nullptr, nullptr, mOffset, sead::Vector3f::zero);
    }
    f32 scale = mScale;
    if (mHost && mIsFollowHostScale) {
        scale *= getScale(mHost).x;
    }
    pMtx->makeST(sead::Vector3f(scale, scale, scale), trans);
}

/**
 * Updates the shadow matrix and adds the sphere to the shadow mask keeper.
 */
void ShadowMaskSphere::update() {
    if (!mIsShadowFixed) {
        calcShadowMatrix(&mShadowMtx);
    }
    ShadowMaskFunction::getShadowMaskKeeper(mHost)->addSphere(
        mShadowMtx, mColor, mExp, getShadowIntensity(), mDrawCategory.getRelativeIndex());
}

/**
 * Updates the shadow matrix (multi-core update).
 */
void ShadowMaskSphere::updateMulti() {
    if (!mIsShadowFixed) {
        calcShadowMatrix(&mShadowMtx);
    }
}

/**
 * Adds the sphere to the shadow mask keeper (multi-core update).
 */
void ShadowMaskSphere::addMulti() {
    ShadowMaskFunction::getShadowMaskKeeper(mHost)->addSphere(
        mShadowMtx, mColor, mExp, getShadowIntensity(), mDrawCategory.getRelativeIndex());
}

}  // namespace al
