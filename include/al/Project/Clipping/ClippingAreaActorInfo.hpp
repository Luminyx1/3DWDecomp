#pragma once

#include <basis/seadTypes.h>

namespace al {
class ClippingAreaActorInfo {
public:
    u8 _0[0xa6];
    bool mIsLODDisabled;
};

class ClippingAreaActorInfoNode {
public:
    u8 _0[0x18];
    ClippingAreaActorInfo* mInfo;
};
}  // namespace al
