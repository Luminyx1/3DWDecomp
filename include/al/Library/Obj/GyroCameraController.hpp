#pragma once

#include <basis/seadTypes.h>

namespace al {
class GyroCameraControllerParam {
public:
    GyroCameraControllerParam();

    f32 _0;
    f32 _4;
    f32 _8;
    f32 _c;
    f32 _10;
    f32 _14;
};
static_assert(sizeof(GyroCameraControllerParam) == 0x18);
}  // namespace al
