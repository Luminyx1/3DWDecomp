#pragma once

#include "Project/Se/SeSource.hpp"

namespace al {
class SeSource3DSphere : public SeSource3D {
public:
    SeSource3DSphere(SeSourcePose3D* pPose, const f32* pRadius, AudioSystemInfo* pInfo);

    void calcPositionInitialize() override;
    void calcPositionDynamic() override;
    const sead::Vector3f* calcPosition(const sead::Vector3f& rListenerPos) override;

    const f32* mRadius;                                 // _40
    sead::Vector3f mCalcPos = {0.0f, 0.0f, 0.0f};  // _48
};
}  // namespace al
