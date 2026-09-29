#pragma once

#include "Project/Se/ISeListenerParam.hpp"

namespace sead {
class Projection;
}

namespace al {
class SeListenerParamTargetViewPos : public ISeListenerParam {
public:
    const sead::Vector3f& getViewPos() const override;
    const sead::Matrix34f& getViewMatrix() const override;
    f32 getFovyDegree() const override;
    const sead::Vector3f& getTargetPos() const override;

    const sead::Vector3f* mViewPos;       // _8
    const sead::Matrix34f* mViewMatrix;   // _10
    const sead::Projection* mProjection;  // _18
    const sead::Vector3f* mTargetPos;     // _20
};
}  // namespace al
