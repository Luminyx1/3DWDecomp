#pragma once

#include <basis/seadTypes.h>

namespace al {
/// Limits and steps for letting the player rotate the camera.
class ControlAngleParam {
public:
    ControlAngleParam();

    bool mIsInvalidControl;  // _0
    bool mIsValid;           // _1
    f32 mAngleVLimitMin;     // _4
    f32 mAngleVLimitMax;     // _8
    f32 mAngleHLimitMin;     // _C
    f32 mAngleHLimitMax;     // _10
    f32 mAngleVStep;         // _14
    f32 mAngleHStep;         // _18
};
}  // namespace al
