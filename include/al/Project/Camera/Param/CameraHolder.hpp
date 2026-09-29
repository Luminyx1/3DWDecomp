#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraInfo;
class CameraPoser;
class PlacementId;

/// Holds the placed cameras of a scene.
class CameraHolder {
public:
    CameraHolder();

    void setCameraInfos(const PlacementId* pPlacementId, CameraPoser* pPoser, s32 priority);
    CameraPoser* getCameraByIndex(s32 index) const;
    CameraPoser* getCameraById(const PlacementId* pPlacementId) const;
    CameraInfo* getCameraInfoById(const PlacementId* pPlacementId) const;
    bool isExistCameraId(const PlacementId* pPlacementId) const;

    s32 mNumCameras = 0;       // _0
    CameraInfo** mCameraInfos;  // _8
};
}  // namespace al
