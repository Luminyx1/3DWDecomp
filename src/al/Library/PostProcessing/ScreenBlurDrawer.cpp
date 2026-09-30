#include "Library/PostProcessing/ScreenBlurDrawer.hpp"

namespace al {

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool ScreenBlurDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Gets the BlurNum parameter.
 * @return BlurNum.
 */
s32 ScreenBlurDrawParam::getBlurNum() const {
    return mBlurNum->getValue();
}

/**
 * Gets the SampleNum parameter.
 * @return SampleNum.
 */
s32 ScreenBlurDrawParam::getSampleNum() const {
    return mSampleNum->getValue();
}

/**
 * Gets the Position parameter.
 * @return Position.
 */
const sead::Vector2f& ScreenBlurDrawParam::getPosition() const {
    return mPosition->getValue();
}

/**
 * Gets the Strength parameter.
 * @return Strength.
 */
f32 ScreenBlurDrawParam::getStrength() const {
    return mStrength->getValue();
}

/**
 * Gets the Radius parameter.
 * @return Radius.
 */
f32 ScreenBlurDrawParam::getRadius() const {
    return mRadius->getValue();
}

/**
 * Gets the Alpha parameter.
 * @return Alpha.
 */
f32 ScreenBlurDrawParam::getAlpha() const {
    return mAlpha->getValue();
}


/**
 * Finishes initialization of the parameter interpolation.
 */
void ScreenBlurDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void ScreenBlurDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const ScreenBlurDrawParam* ScreenBlurDrawer::getCurrentParam() const {
    return static_cast<const ScreenBlurDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void ScreenBlurDrawer::requestParam(s32 priority, s32 step, const ScreenBlurDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

}  // namespace al
