#include "Project/PostProcessing/CartoonDrawer.hpp"

namespace al {

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool CartoonDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Checks the EnableFishEye flag.
 * @return Whether EnableFishEye is set.
 */
bool CartoonDrawParam::isEnableFishEye() const {
    return mIsEnableFishEye->getValue();
}

/**
 * Gets the ToonShadeRate parameter.
 * @return ToonShadeRate.
 */
f32 CartoonDrawParam::getToonShadeRate() const {
    return mToonShadeRate->getValue();
}

/**
 * Gets the ToonStep parameter.
 * @return ToonStep.
 */
const sead::Vector2f& CartoonDrawParam::getToonStep() const {
    return mToonStep->getValue();
}

/**
 * Gets the ToonWidth parameter.
 * @return ToonWidth.
 */
const sead::Vector2f& CartoonDrawParam::getToonWidth() const {
    return mToonWidth->getValue();
}

/**
 * Gets the NoiseTextureId parameter.
 * @return NoiseTextureId.
 */
s32 CartoonDrawParam::getNoiseTextureId() const {
    return mNoiseTextureId->getValue();
}

/**
 * Gets the NoiseMixRate parameter.
 * @return NoiseMixRate.
 */
f32 CartoonDrawParam::getNoiseMixRate() const {
    return mNoiseMixRate->getValue();
}

/**
 * Gets the NoiseScale parameter.
 * @return NoiseScale.
 */
f32 CartoonDrawParam::getNoiseScale() const {
    return mNoiseScale->getValue();
}

/**
 * Gets the NoiseOffset parameter.
 * @return NoiseOffset.
 */
const sead::Vector2f& CartoonDrawParam::getNoiseOffset() const {
    return mNoiseOffset->getValue();
}

/**
 * Gets the CanvasTextureId parameter.
 * @return CanvasTextureId.
 */
s32 CartoonDrawParam::getCanvasTextureId() const {
    return mCanvasTextureId->getValue();
}

/**
 * Gets the CanvasRepeat parameter.
 * @return CanvasRepeat.
 */
f32 CartoonDrawParam::getCanvasRepeat() const {
    return mCanvasRepeat->getValue();
}

/**
 * Gets the CanvasMix parameter.
 * @return CanvasMix.
 */
f32 CartoonDrawParam::getCanvasMix() const {
    return mCanvasMix->getValue();
}

/**
 * Gets the IndirectTextureId parameter.
 * @return IndirectTextureId.
 */
s32 CartoonDrawParam::getIndirectTextureId() const {
    return mIndirectTextureId->getValue();
}

/**
 * Gets the IndirectScale parameter.
 * @return IndirectScale.
 */
f32 CartoonDrawParam::getIndirectScale() const {
    return mIndirectScale->getValue();
}

/**
 * Gets the IndirectTexScale parameter.
 * @return IndirectTexScale.
 */
const sead::Vector2f& CartoonDrawParam::getIndirectTexScale() const {
    return mIndirectTexScale->getValue();
}

/**
 * Gets the IndirectTexOffset parameter.
 * @return IndirectTexOffset.
 */
const sead::Vector2f& CartoonDrawParam::getIndirectTexOffset() const {
    return mIndirectTexOffset->getValue();
}

/**
 * Gets the FishEyeParam parameter.
 * @return FishEyeParam.
 */
f32 CartoonDrawParam::getFishEyeParam() const {
    return mFishEyeParam->getValue();
}


/**
 * Finishes initialization of the parameter interpolation.
 */
void CartoonDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void CartoonDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void CartoonDrawer::update() {
    mRequestInterp->updateInterp();
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const CartoonDrawParam* CartoonDrawer::getCurrentParam() const {
    return static_cast<const CartoonDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void CartoonDrawer::requestParam(s32 priority, s32 step, const CartoonDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool CartoonDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
}

}  // namespace al
