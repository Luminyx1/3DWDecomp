#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserCart : public CameraPoser_RS {
public:
    CameraPoserCart(const char* pName);
    void stop();
    void restart();

public:
    u8 _142[0x26];
};

static_assert(sizeof(CameraPoserCart) == 0x168);

}  // namespace al
