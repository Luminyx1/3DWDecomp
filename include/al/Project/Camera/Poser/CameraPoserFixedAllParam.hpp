#pragma once

#include <basis/seadTypes.h>

namespace al {
/// Placement parameters of a camera that looks at the whole stage from a fixed spot.
class CameraPoserFixedAllParam {
public:
    CameraPoserFixedAllParam();

    f32 mDistance;  // _0
    f32 mFovyDegree;  // _4
    f32 _8;         // _8
};
}  // namespace al
