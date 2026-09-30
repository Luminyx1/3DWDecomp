#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserTower_RS : public CameraPoser_RS {
public:
    CameraPoserTower_RS(const char* pName, const sead::Vector3f* pPos);
    void resetInputRotate(f32 angle, s32 step);

public:
    u8 _142[0x56];
    f32 mDistance;
    u8 _19c[0x28];
    f32 mUserMarginAngleH;
    u8 _1c8[0x50];
};

static_assert(sizeof(CameraPoserTower_RS) == 0x218);

}  // namespace al
