#pragma once

#include <basis/seadTypes.h>

namespace al {
class ActorInitInfo;

struct CameraSubTargetTurnParam {
    CameraSubTargetTurnParam() = default;

    void init(const ActorInitInfo& rInfo);

    f32 turnSpeedRate1 = 0.3f;
    f32 turnSpeedRate2 = 0.1f;
    f32 turnBrakeEndDistance = -1.0f;
    f32 turnBrakeStartDistance = -1.0f;
    f32 turnStopStartDistance = -1.0f;
    f32 turnStopEndDistance = -1.0f;
    s32 targetTurnRestartStep = 30;
    bool isTurnV = false;
    bool isResetAfterTurnV = false;
    f32 minTurnDegreeV = 0.0f;
    f32 maxTurnDegreeV = 0.0f;
    f32 validTurnDegreeRangeH = -1.0f;
    f32 validFaceDegreeRangeH = -1.0f;
};

}  // namespace al
