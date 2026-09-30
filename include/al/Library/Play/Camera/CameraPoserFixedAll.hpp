#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraPoserFixedAllParam {
public:
    CameraPoserFixedAllParam();

    f32 mDistance;
    f32 mFovyDegree;
    f32 _8;
};

static_assert(sizeof(CameraPoserFixedAllParam) == 0xc);
}  // namespace al
