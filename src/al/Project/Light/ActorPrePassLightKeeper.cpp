#include "Project/Light/ActorPrePassLightKeeper.hpp"

#include "Library/Light/LppBase.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Allocates the light array.
 * @param num Maximum number of lights.
 */
void ActorPrePassLightKeeper::initLightNum(s32 num) {
    mLightBaseArray.allocBuffer(num, nullptr);
}

/**
 * Applies the placement info of a light.
 * @param pLight Light to set up.
 * @param rInfo Light info.
 */
void ActorPrePassLightKeeper::setLightBaseInfo(PrePassLightBase* pLight, const LightBaseInfo& rInfo) {
    pLight->mOffset = rInfo.mOffset;
    pLight->mRotateOffsetDegree = rInfo.mRotateOffset;
    pLight->mColor = rInfo.mColor;
    pLight->mSpecularColor = rInfo.mSpecularColor;
    pLight->mIsEnableSpecularColor = rInfo.mIsEnableSpecularColor;
    pLight->_90 = rInfo.mIsEnableSpecular;
    pLight->mIsEnableSpecular = rInfo.mIsEnableSpecular;
    pLight->mKillFrame = rInfo.mKillFrame;
    pLight->mAppearFrame = rInfo.mAppearFrame;
    pLight->mIsIndirectIllumination = rInfo.mIsIndirectIllumination;
}

/**
 * Kills the lights of dead actors.
 */
void ActorPrePassLightKeeper::initAfterPlacement() {
    if (!isDead(mParentActor)) {
        return;
    }

    for (PrePassLightBase& light : mLightBaseArray) {
        light.requestKillDirect();
    }
}

/**
 * Makes the lights appear.
 * @param isHideModel Whether the actor model is hidden.
 */
void ActorPrePassLightKeeper::appear(bool isHideModel) {
    if (mIsIgnorePrePassYaml) {
        return;
    }

    if (!mIsIgnoreHideModel && isHideModel) {
        return;
    }

    for (PrePassLightBase& light : mLightBaseArray) {
        light.appear();
    }
}

/**
 * Kills the lights.
 */
void ActorPrePassLightKeeper::requestKill() {
    if (mIsIgnorePrePassYaml) {
        return;
    }

    for (PrePassLightBase& light : mLightBaseArray) {
        light.requestKill();
    }
}

/**
 * Kills the lights when the actor model gets hidden.
 */
void ActorPrePassLightKeeper::hideModel() {
    if (mIsIgnorePrePassYaml || mIsIgnoreHideModel) {
        return;
    }

    for (PrePassLightBase& light : mLightBaseArray) {
        light.requestKill();
    }
}

/**
 * Updates the lights for a hidden model.
 * @param isHide Whether the model is hidden.
 */
void ActorPrePassLightKeeper::updateHideModel(bool isHide) {}

/**
 * Finds a light by name.
 * @param pName Light name.
 * @return The light, or nullptr.
 */
PrePassLightBase* ActorPrePassLightKeeper::getLightBase(const char* pName) const {
    if (mIsIgnorePrePassYaml) {
        return nullptr;
    }

    s32 num = mLightBaseArray.size();
    for (s32 i = 0; i < num; i++) {
        PrePassLightBase* light = mLightBaseArray[i];
        if (isEqualString(light->mName, pName)) {
            return light;
        }
    }

    return nullptr;
}

/**
 * Gets a light by index.
 * @param index Light index.
 * @return The light, or nullptr.
 */
PrePassLightBase* ActorPrePassLightKeeper::getLightBase(s32 index) const {
    if (mIsIgnorePrePassYaml) {
        return nullptr;
    }

    return mLightBaseArray[index];
}

/**
 * Finds a user color by name.
 * @param pName Color name.
 * @return The color, or black.
 */
const sead::Color4f& ActorPrePassLightKeeper::findUserColor(const char* pName) const {
    s32 num = mUserColorArray.size();
    for (s32 i = 0; i < num; i++) {
        UserColor* color = mUserColorArray[i];
        if (isEqualString(pName, color->mName)) {
            return color->mColor;
        }
    }

    return sead::Color4f::cBlack;
}
}  // namespace al
