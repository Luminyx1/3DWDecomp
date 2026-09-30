#pragma once

#include "Project/Camera/CameraPoser.hpp"

namespace al {
class PlayerWatcher;
class CameraPoserFixedPoint : public CameraPoser {
public:
    CameraPoserFixedPoint(const PlayerWatcher* pPlayerWatcher, const PlacementId* pPlacementId);

    void update() override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void loadParam(const ByamlIter* pIter) override;

    const PlayerWatcher* mPlayerWatcher;
    sead::Vector3f mLocalCameraPos = sead::Vector3f::zero;
};
static_assert(sizeof(CameraPoserFixedPoint) == 0xc0);
}  // namespace al
