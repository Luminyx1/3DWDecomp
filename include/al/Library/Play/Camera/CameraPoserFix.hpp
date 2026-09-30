#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserFix : public CameraPoser_RS {
public:
    CameraPoserFix(const char* pName);
    static const char* getFixAbsoluteCameraName();
    static const char* getFixDoorwayCameraName();
    void initCameraPosAndLookAtPos(const sead::Vector3f& rCameraPos, const sead::Vector3f& rLookAtPos);

public:
    u8 _142[0x8e];
};

static_assert(sizeof(CameraPoserFix) == 0x1d0);

}  // namespace al
