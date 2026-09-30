#pragma once

#include "Project/Se/SeListener.hpp"

namespace sead {
class Projection;
}

namespace al {

class SeListenerParamTargetViewPos : public ISeListenerParam {
public:
    SeListenerParamTargetViewPos(const sead::Vector3f* pViewPos, const sead::Matrix34f* pViewMatrix,
                                 const sead::Projection* pProjection, const sead::Vector3f* pTargetPos)
        : mViewPos(pViewPos), mViewMatrix(pViewMatrix), mProjection(pProjection), mTargetPos(pTargetPos) {}

    const sead::Vector3f& getViewPos() const override;
    const sead::Matrix34f& getViewMatrix() const override;
    f32 getFovyDegree() const override;
    const sead::Vector3f& getTargetPos() const override;

private:
    const sead::Vector3f* mViewPos = nullptr;
    const sead::Matrix34f* mViewMatrix = nullptr;
    const sead::Projection* mProjection = nullptr;
    const sead::Vector3f* mTargetPos = nullptr;
};

static_assert(sizeof(SeListenerParamTargetViewPos) == 0x28);

}  // namespace al
