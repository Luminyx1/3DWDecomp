#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserFollowSimple : public CameraPoser_RS {
public:
    CameraPoserFollowSimple(const char* pName);

    void loadParam(const ByamlIter& rIter) override;
    void start(const CameraStartInfo& rInfo) override;
    void update() override;
    void reset() override;

public:
    f32 mOffsetY = 120.0f;
    f32 mDistance = 1600.0f;
    f32 mAngle = 20.0f;
    bool mIsRotateH = true;
    bool mIsResetAngleIfSwitchTarget = false;
};

static_assert(sizeof(CameraPoserFollowSimple) == 0x158);

}  // namespace al
