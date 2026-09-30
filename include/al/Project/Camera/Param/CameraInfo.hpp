#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraPoser;
class PlacementId;

class CameraInfo {
public:
    CameraInfo(const PlacementId* pPlacementId, CameraPoser* pPoser, s32 priority);

    PlacementId* mPlacementId;
    CameraPoser* mPoser;
    s32 mPriority;
};

static_assert(sizeof(CameraInfo) == 0x18);
}  // namespace al
