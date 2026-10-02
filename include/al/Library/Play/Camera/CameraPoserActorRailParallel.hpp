#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class RailKeeper;

class CameraPoserActorRailParallel : public CameraPoser_RS {
public:
    CameraPoserActorRailParallel(const char* pName, const RailKeeper* pRailKeeper);

    void init() override;
    void loadParam(const ByamlIter& rIter) override;
    void start(const CameraStartInfo& rInfo) override;
    void update() override;

private:
    const RailKeeper* mRailKeeper;
    sead::Vector3f mOffset = {0.0f, 0.0f, 0.0f};
    f32 mDistance = 1600.0f;
    f32 mAngleDegreeH = 0.0f;
    f32 mAngleDegreeV = 20.0f;
    f32 mFollowRate = 1.0f;
};

static_assert(sizeof(CameraPoserActorRailParallel) == 0x170);

}  // namespace al
