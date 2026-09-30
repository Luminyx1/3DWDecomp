#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserSubjective_RS : public CameraPoser_RS {
public:
    CameraPoserSubjective_RS(const char* pName);
    static f32 getCameraOffsetFront();

public:
    u8 _142[0x27];
    bool mIsRequestZoomIn;
    bool mIsValidResetAngleH;
    u8 _16b[0x11];
    f32 mCameraOffsetUp;
    u8 _180[0x4];
    bool mIsSetStartAngleH;
    u8 _185[0x3];
    f32 mStartAngleH;
    u8 _18c[0x24];
};

static_assert(sizeof(CameraPoserSubjective_RS) == 0x1b0);

}  // namespace al
