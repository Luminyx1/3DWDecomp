#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraPoser;
class PlacementId;

/// Associates a camera poser with the placement it was created from.
class CameraInfo {
public:
    CameraInfo(const PlacementId* pPlacementId, CameraPoser* pPoser, s32 priority);

    PlacementId* mPlacementId;  // _0
    CameraPoser* mPoser;        // _8
    s32 mPriority;              // _10
};
}  // namespace al
