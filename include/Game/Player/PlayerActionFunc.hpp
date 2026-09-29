#pragma once

#include <math/seadVector.h>

class IUsePlayerInput;
struct PlayerProperty;

namespace PlayerActionFunc {
    bool isUpperVelocity(const PlayerProperty*);
    bool isOppositeInput(const IUsePlayerInput*, const PlayerProperty*, const sead::Vector3f&);
}  // namespace PlayerActionFunc
