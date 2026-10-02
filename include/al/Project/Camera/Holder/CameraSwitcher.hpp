#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraHolder;
class CameraPoser;
class PlacementId;
class PlayerWatcher;

class CameraSwitcher {
public:
    CameraSwitcher(CameraPoser** ppPosers, const CameraHolder* pHolder, PlayerWatcher* pWatcher,
                   bool* pFlag);

    void start(const PlacementId& rId, s32 interpoleFrame);
    void addPoolCameraList(const PlacementId& rId);
    void end(const PlacementId& rId, s32 interpoleFrame);
    void removePoolCameraList(const PlacementId& rId);
    bool tryStartCameraFromPoolCameraList();
    bool isCameraCurrent(const CameraPoser* pPoser) const;
    void offLookAtStop();
};

}  // namespace al
