#include "Library/Shadow/ShadowMaskCastOvalCylinder.hpp"

#include "Library/Shadow/ShadowMaskDrawer.hpp"

namespace al {

/**
 * Declares the shadow mask to the shadow mask keeper.
 * @param category Draw category.
 */
void ShadowMaskCastOvalCylinder::declare(ShadowMaskDrawCategory category) {
    ShadowMaskFunction::getShadowMaskKeeper(mHost)->declare(ShadowMaskType::CastOvalCylinder,
                                                             category);
}

/**
 * Updates the shadow matrix and adds the cast oval cylinder to the shadow mask keeper.
 */
void ShadowMaskCastOvalCylinder::update() {
    calcShadowMatrix(&mShadowMtx);
    ShadowMaskFunction::getShadowMaskKeeper(mHost)->addCastOvalCylinder(
        mShadowMtx, mColor, mExpXZ, mExpY, getShadowIntensity(), mDistYBase,
        mDrawCategory.getRelativeIndex());
}

/**
 * Updates the shadow matrix (multi-core update).
 */
void ShadowMaskCastOvalCylinder::updateMulti() {
    calcShadowMatrix(&mShadowMtx);
}

/**
 * Adds the cast oval cylinder to the shadow mask keeper (multi-core update).
 */
void ShadowMaskCastOvalCylinder::addMulti() {
    ShadowMaskFunction::getShadowMaskKeeper(mHost)->addCastOvalCylinder(
        mShadowMtx, mColor, mExpXZ, mExpY, getShadowIntensity(), mDistYBase,
        mDrawCategory.getRelativeIndex());
}

}  // namespace al
