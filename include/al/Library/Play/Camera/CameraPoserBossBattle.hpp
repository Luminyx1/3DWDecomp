#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserBossBattle : public CameraPoser_RS {
public:
    CameraPoserBossBattle(const char* pName, const sead::Vector3f* pPos);

public:
    u8 _142[0x7e];
};

static_assert(sizeof(CameraPoserBossBattle) == 0x1c0);

}  // namespace al
