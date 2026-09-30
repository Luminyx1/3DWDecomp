#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserShooterSingle : public CameraPoser_RS {
public:
    CameraPoserShooterSingle(const char* pName);

public:
    u8 _142[0x26];
};

static_assert(sizeof(CameraPoserShooterSingle) == 0x168);

}  // namespace al
