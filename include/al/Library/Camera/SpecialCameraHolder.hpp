#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraTicket;

class SpecialCameraHolder {
public:
    SpecialCameraHolder();

    void allocEntranceCameraBuffer(s32 maxEntries);
    void registerEntranceCamera(CameraTicket* pTicket);
    CameraTicket* findEntranceCamera(const char* pSuffix) const;

private:
    CameraTicket** mEntranceCameras = nullptr;
    s32 mMaxEntranceCameras = 0;
    s32 mEntranceCameraNum = 0;
};

}  // namespace al
