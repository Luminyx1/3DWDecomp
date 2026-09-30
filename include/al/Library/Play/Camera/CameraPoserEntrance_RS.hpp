#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserEntrance_RS : public CameraPoser_RS {
public:
    CameraPoserEntrance_RS(const char* pName);
    void initParam(f32 distance, const sead::Vector3f& rCameraPos, const sead::Vector3f& rLookAtPos);
    void initLookAtPosDirect(const sead::Vector3f& rLookAtPos);

public:
    u8 _142[0x56];
};

static_assert(sizeof(CameraPoserEntrance_RS) == 0x198);

}  // namespace al
