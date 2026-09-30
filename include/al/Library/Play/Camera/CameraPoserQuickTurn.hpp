#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserQuickTurn : public CameraPoser_RS {
public:
    CameraPoserQuickTurn(const char* pName);
    void setFollow();

public:
    u8 _142[0x37];
    bool mIsRotateFast;
    u8 _17a[0x6];
};

static_assert(sizeof(CameraPoserQuickTurn) == 0x180);

}  // namespace al
