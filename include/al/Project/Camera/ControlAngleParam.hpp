#pragma once

#include <basis/seadTypes.h>

namespace al {
class ControlAngleParam {
public:
    ControlAngleParam();

    bool mIsInvalidControl;
    bool mIsValid;
    f32 mAngleVLimitMin;
    f32 mAngleVLimitMax;
    f32 mAngleHLimitMin;
    f32 mAngleHLimitMax;
    f32 mAngleVStep;
    f32 mAngleHStep;
};

static_assert(sizeof(ControlAngleParam) == 0x1c);
}  // namespace al
