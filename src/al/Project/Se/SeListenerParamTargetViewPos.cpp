#include "Project/Se/SeListenerParamTargetViewPos.hpp"
#include <gfx/seadProjection.h>
#include <math/seadMathCalcCommon.h>

namespace al {
/**
 * @brief Gets the camera (view) position the listener is placed at.
 * @return The view position.
 */
const sead::Vector3f& SeListenerParamTargetViewPos::getViewPos() const {
    return *mViewPos;
}

/**
 * @brief Gets the camera view matrix used to orient the listener.
 * @return The view matrix.
 */
const sead::Matrix34f& SeListenerParamTargetViewPos::getViewMatrix() const {
    return *mViewMatrix;
}

/**
 * @brief Gets the vertical field of view of the camera projection.
 * @return The vertical field of view, in degrees.
 */
f32 SeListenerParamTargetViewPos::getFovyDegree() const {
    return sead::Mathf::rad2deg(mProjection->getFovy());
}

/**
 * @brief Gets the position the camera is looking at.
 * @return The target position.
 */
const sead::Vector3f& SeListenerParamTargetViewPos::getTargetPos() const {
    return *mTargetPos;
}
}  // namespace al
