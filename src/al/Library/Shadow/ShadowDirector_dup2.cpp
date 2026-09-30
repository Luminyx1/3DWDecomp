#include "Library/Connector/MtxConnector.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Shadow/ShadowMaskBase.hpp"
#include "Library/Shadow/ShadowMaskDrawer.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * Constructs a shadow mask with default parameters.
 * @param pName Name of the shadow mask.
 */
ShadowMaskBase::ShadowMaskBase(const char* pName)
    : mHost(nullptr), mMtxConnector(nullptr), mOffset(0.0f, 0.0f, 0.0f),
      mColor(0.0f, 0.0f, 0.0f, 1.0f), mDropDir(-sead::Vector3f::ey), mDropLength(100.0f),
      mUp(sead::Vector3f::ey), mName(pName), mIsApplyShadowIntensityUser(false),
      mShadowIntensityUser(0), mIsFollowHostScale(true), mIsIgnoreHide(false), mDrawCategory(0),
      mIsShadowFixed(false), mActorJointName(""), mIsRegistered(false), mIsValid(true), mIsHide(true),
      mShadowMtx(sead::Matrix34f::ident), mSetHeightEvenTargetName(""), mHeightEvenTarget(nullptr), _e8(false),
      mIsIgnoreHostAlpha(false), _ea(false) {}

/**
 * Calculates the fixed shadow matrix once if the shadow is fixed.
 */
void ShadowMaskBase::initAfterPlacement() {
    if (mIsShadowFixed) {
        calcShadowMatrix(&mShadowMtx);
    }
}

/**
 * Creates the matrix connector of the shadow mask.
 */
void ShadowMaskBase::createMtxConnector() {
    mMtxConnector = new MtxConnector();
}

/**
 * Gets the color of the shadow mask.
 * @return Color.
 */
const sead::Color4f& ShadowMaskBase::getColor() const {
    return mColor;
}

/**
 * Sets the color of the shadow mask.
 * @param color Color.
 */
void ShadowMaskBase::setColor(sead::Color4f color) {
    mColor = color;
}

/**
 * Calculates the intensity of the shadow mask.
 * @return Shadow intensity.
 */
f32 ShadowMaskBase::getShadowIntensity() const {
    if (mDrawCategory == ShadowMaskDrawCategory::LightScaleLight ||
        mDrawCategory == ShadowMaskDrawCategory::LightScale) {
        return mColor.a;
    }

    u8 intensity;

    if (mIsApplyShadowIntensityUser) {
        intensity = mShadowIntensityUser;
    } else {
        if (!mHost) {
            return 0.0f;
        }

        intensity = ShadowMaskFunction::getShadowMaskKeeper(mHost)->getShadowIntensity(
            mDrawCategory.getRelativeIndex());
    }

    f32 rate = intensity * (1.0f / 255.0f);

    if (mHost && !mIsIgnoreHostAlpha) {
        rate *= mHost->mGlobalAlphaLastFrame;
    }

    return rate;
}

/**
 * Sets the host actor of the shadow mask.
 * @param pHost Host actor.
 */
void ShadowMaskBase::setHost(const LiveActor* pHost) {
    mHost = pHost;
}

/**
 * Reads the common shadow mask parameters.
 * @param rIter Parameter iterator.
 */
void ShadowMaskBase::readParam(const ByamlIter& rIter) {
    tryGetByamlV3f(&mOffset, rIter, "Offset");
    tryGetByamlColor(&mColor, rIter, "Color");
    tryGetByamlF32(&mDropLength, rIter, "DropLength");
    mIsIgnoreHide = tryGetByamlKeyBoolOrFalse(rIter, "IsIgnoreHide");
    mIsFollowHostScale = tryGetByamlKeyBoolOrFalse(rIter, "IsFollowHostScale");
    mIsShadowFixed = tryGetByamlKeyBoolOrFalse(rIter, "IsShadowFixed");
    mActorJointName = tryGetByamlKeyStringOrNULL(rIter, "JointName");

    if (const char* categoryName = tryGetByamlKeyStringOrNULL(rIter, "DrawCategory")) {
        s32 category = -1;

        for (s32 i = 0; i < ShadowMaskDrawCategory::size(); i++) {
            if (isEqualString(categoryName, ShadowMaskDrawCategory::text(i))) {
                category = i;
                break;
            }
        }

        mDrawCategory.setRelativeIndex(category);
    }

    if (const char* targetName = tryGetByamlKeyStringOrNULL(rIter, "SetHeightEvenTargetName")) {
        mSetHeightEvenTargetName = targetName;
    }
}

}  // namespace al
