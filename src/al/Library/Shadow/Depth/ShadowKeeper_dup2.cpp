#include "Library/Shadow/ShadowKeeper.hpp"

#include "Library/Shadow/Common/ShadowUtil.hpp"
#include "Library/Shadow/ShadowMaskBase.hpp"

namespace al {

/**
 * Shows all shadow masks, independent masks first.
 */
void ShadowKeeper::show() {
    if (mIsIgnoreShadowMaskYaml) {
        return;
    }

    for (auto it = mMaskArray.begin(); it != mMaskArray.end(); ++it) {
        if (it->mHeightEvenTarget == nullptr) {
            showShadow(mHostActor, &*it);
        }
    }

    for (auto it = mMaskArray.begin(); it != mMaskArray.end(); ++it) {
        if (it->mHeightEvenTarget != nullptr) {
            showShadow(mHostActor, &*it);
        }
    }
}

/**
 * Initializes all shadow masks after placement.
 */
void ShadowKeeper::initAfterPlacement() {
    for (s32 i = 0; i < mMaskArray.size(); i++) {
        mMaskArray.unsafeAt(i)->initAfterPlacement();
    }
}

/**
 * Hides all shadow masks.
 */
void ShadowKeeper::hide() {
    if (mIsIgnoreShadowMaskYaml) {
        return;
    }

    for (auto it = mMaskArray.begin(); it != mMaskArray.end(); ++it) {
        hideShadow(mHostActor, &*it);
    }
}

/**
 * Validates all shadow masks.
 */
void ShadowKeeper::validate() {
    if (mIsIgnoreShadowMaskYaml) {
        return;
    }

    for (auto it = mMaskArray.begin(); it != mMaskArray.end(); ++it) {
        validateShadow(mHostActor, &*it);
    }
}

/**
 * Invalidates all shadow masks.
 */
void ShadowKeeper::invalidate() {
    if (mIsIgnoreShadowMaskYaml) {
        return;
    }

    for (auto it = mMaskArray.begin(); it != mMaskArray.end(); ++it) {
        invalidateShadow(mHostActor, &*it);
    }
}

/**
 * Checks whether all shadow masks are hidden.
 * @return Whether all shadow masks are hidden.
 */
bool ShadowKeeper::isHide() {
    for (auto& mask : mMaskArray) {
        if (!mask.isHide()) {
            return false;
        }
    }

    return true;
}

}  // namespace al
