#include "Library/Shadow/ShadowMaskCylinder.hpp"

#include "Library/Shadow/ShadowMaskDrawer.hpp"

namespace al {

/**
 * Constructs a cylinder shadow mask.
 * @param pName Name of the shadow mask.
 */
ShadowMaskCylinder::ShadowMaskCylinder(const char* pName)
    : ShadowMaskBase(pName), mScale(100.0f, 500.0f, 100.0f), _f8(0.0f, 0.0f, 0.0f), mExpXZ(100.0f), mExpY(3.0f), mDistYBase(0.5f), _110(false) {}

/**
 * Declares the shadow mask to the shadow mask keeper.
 * @param category Draw category.
 */
void ShadowMaskCylinder::declare(ShadowMaskDrawCategory category) {
    ShadowMaskFunction::getShadowMaskKeeper(mHost)->declare(ShadowMaskType::Cylinder, category);
}

/**
 * Updates the shadow matrix and adds the cylinder to the shadow mask keeper.
 */
void ShadowMaskCylinder::update() {
    if (!mIsShadowFixed) {
        calcShadowMatrix(&mShadowMtx);
    }

    ShadowMaskFunction::getShadowMaskKeeper(mHost)->addCylinder(
        mShadowMtx, mColor, mExpXZ, mExpY, getShadowIntensity(), mDistYBase,
        mDrawCategory.getRelativeIndex());
}

/**
 * Updates the shadow matrix (multi-core update).
 */
void ShadowMaskCylinder::updateMulti() {
    if (!mIsShadowFixed) {
        calcShadowMatrix(&mShadowMtx);
    }
}

/**
 * Adds the cylinder to the shadow mask keeper (multi-core update).
 */
void ShadowMaskCylinder::addMulti() {
    ShadowMaskFunction::getShadowMaskKeeper(mHost)->addCylinder(
        mShadowMtx, mColor, mExpXZ, mExpY, getShadowIntensity(), mDistYBase,
        mDrawCategory.getRelativeIndex());
}

}  // namespace al
