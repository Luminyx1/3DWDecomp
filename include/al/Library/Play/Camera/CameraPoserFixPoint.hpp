#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserFixPoint : public CameraPoser_RS {
public:
    CameraPoserFixPoint(const char* pName);

    void init() override;
    void loadParam(const ByamlIter& rIter) override;
    void start(const CameraStartInfo& rInfo) override;
    void update() override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;

    void validateUsePreCameraPos() { mIsUsePrePoserPos = true; }

public:
    f32 mOffsetY = 0.0f;
    sead::Vector3f mCameraPos = {0.0f, 0.0f, 0.0f};
    bool mIsUsePrePoserPos = false;
    bool mIsKeepDistanceFromLookAt = false;
    f32 mKeepDistance = 1400.0f;
};

static_assert(sizeof(CameraPoserFixPoint) == 0x160);

}  // namespace al
