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
    sead::Vector3f mOffset;
    f32 mDistance;
    f32 mAngleDegreeH;
    f32 mAngleDegreeV;
    f32 mFollowRate;
};

static_assert(sizeof(CameraPoserActorRailParallel) == 0x170);

}  // namespace al
