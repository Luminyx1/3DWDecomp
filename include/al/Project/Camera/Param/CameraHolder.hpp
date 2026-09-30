#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraInfo;
class CameraPoser;
class PlacementId;

class CameraHolder {
public:
    CameraHolder();

    void setCameraInfos(const PlacementId* pPlacementId, CameraPoser* pPoser, s32 priority);
    CameraPoser* getCameraByIndex(s32 index) const;
    CameraPoser* getCameraById(const PlacementId* pPlacementId) const;
    CameraInfo* getCameraInfoById(const PlacementId* pPlacementId) const;
    bool isExistCameraId(const PlacementId* pPlacementId) const;

    s32 mNumCameras = 0;
    CameraInfo** mCameraInfos;
};
static_assert(sizeof(CameraHolder) == 0x10);
}  // namespace al
