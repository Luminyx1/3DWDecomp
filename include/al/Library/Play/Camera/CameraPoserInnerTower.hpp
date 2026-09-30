#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserInnerTower : public CameraPoser_RS {
public:
    CameraPoserInnerTower(const char* pName);

public:
    u8 _142[0x2e];
};

static_assert(sizeof(CameraPoserInnerTower) == 0x170);

}  // namespace al
