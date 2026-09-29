#pragma once

#include <math/seadVector.h>

namespace al {
/// A camera pose: position, look-at point and up direction.
struct CameraPoseInfo {
    sead::Vector3f mPos;  // _0
    sead::Vector3f mAt;   // _C
    sead::Vector3f mUp;   // _18
};
}  // namespace al
