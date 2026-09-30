#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserKinopioBrigade_RS : public CameraPoser_RS {
public:
    CameraPoserKinopioBrigade_RS(const char* pName);

public:
    u8 _142[0xbe];
};

static_assert(sizeof(CameraPoserKinopioBrigade_RS) == 0x200);

}  // namespace al
