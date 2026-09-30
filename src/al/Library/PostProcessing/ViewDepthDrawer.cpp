#include "Library/PostProcessing/ViewDepthDrawer.hpp"

namespace al {

/**
 * Constructs the view depth drawing parameters.
 */
ViewDepthDrawParam::ViewDepthDrawParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "IsEnable", "", true);
    mFar = new ParameterF32(500000.0f, mParamObj, "Far", "Far", "Min=25.f, Max=500000.f", true);
    mFarColor = new ParameterC4f(sead::Color4f::cWhite, mParamObj, "FarColor", "FarColor",
                                 "Min=0, Max=1", true);
    mNearColor = new ParameterC4f(sead::Color4f::cBlack, mParamObj, "NearColor", "NearColor",
                                  "Min=0, Max=1", true);
}

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool ViewDepthDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Gets the Far parameter.
 * @return Far.
 */
f32 ViewDepthDrawParam::getFar() const {
    return mFar->getValue();
}

/**
 * Gets the FarColor parameter.
 * @return FarColor.
 */
const sead::Color4f& ViewDepthDrawParam::getFarColor() const {
    return mFarColor->getValue();
}

/**
 * Gets the NearColor parameter.
 * @return NearColor.
 */
const sead::Color4f& ViewDepthDrawParam::getNearColor() const {
    return mNearColor->getValue();
}


/**
 * Finishes initialization of the parameter interpolation.
 */
void ViewDepthDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void ViewDepthDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void ViewDepthDrawer::update() {
    mRequestInterp->updateInterp();
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const ViewDepthDrawParam* ViewDepthDrawer::getCurrentParam() const {
    return static_cast<const ViewDepthDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void ViewDepthDrawer::requestParam(s32 priority, s32 step, const ViewDepthDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool ViewDepthDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
}

}  // namespace al
