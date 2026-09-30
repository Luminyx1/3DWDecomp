#include "Project/Se/SeListenerParamTargetViewPos.hpp"

#include <gfx/seadProjection.h>
#include <math/seadMathCalcCommon.h>

namespace al {

const sead::Vector3f& SeListenerParamTargetViewPos::getViewPos() const {
    return *mViewPos;
}

const sead::Matrix34f& SeListenerParamTargetViewPos::getViewMatrix() const {
    return *mViewMatrix;
}

f32 SeListenerParamTargetViewPos::getFovyDegree() const {
    return sead::Mathf::rad2deg(mProjection->getFovy());
}

const sead::Vector3f& SeListenerParamTargetViewPos::getTargetPos() const {
    return *mTargetPos;
}

}  // namespace al
