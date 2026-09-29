#pragma once

#include "Project/Camera/Poser/CameraPoser.hpp"

namespace al {
class PlayerWatcher;

/// A camera at a fixed position that turns to look at the player.
class CameraPoserFixedPoint : public CameraPoser {
public:
    CameraPoserFixedPoint(const PlayerWatcher* pPlayerWatcher, const PlacementId* pPlacementId);

    void update() override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void loadParam(const ByamlIter* pIter) override;

    const PlayerWatcher* mPlayerWatcher;                    // _A8
    sead::Vector3f mLocalCameraPos = sead::Vector3f::zero;  // _B0
};
}  // namespace al
