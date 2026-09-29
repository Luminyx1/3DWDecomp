#pragma once

#include "Project/Se/SeSource.hpp"

namespace al {
class SeSource3DPoint : public SeSource3D {
public:
    SeSource3DPoint(SeSourcePose3D* pPose, AudioSystemInfo* pInfo);

    void calcPositionInitialize() override {}
    void calcPositionDynamic() override;
    const sead::Vector3f* calcPosition(const sead::Vector3f& rListenerPos) override;
};
}  // namespace al
