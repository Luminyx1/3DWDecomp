#pragma once

#include "Project/Camera/CameraPoser.hpp"

namespace al {
class CameraPoserProgramable : public CameraPoser {
public:
    CameraPoserProgramable(const PlacementId* pPlacementId);

    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void setCameraPos(const sead::Vector3f& rPos) override { mProgramCameraPos.set(rPos); }
    void setLookAtPos(const sead::Vector3f& rPos) override { mLookAtPos.set(rPos); }
    void setUp(const sead::Vector3f& rUp) override { mProgramCameraUp.set(rUp); }

    sead::Vector3f mProgramCameraPos = {0.0f, 500.0f, 500.0f};
    sead::Vector3f mProgramCameraUp = sead::Vector3f::ey;
};

static_assert(sizeof(CameraPoserProgramable) == 0xc0);
}  // namespace al
