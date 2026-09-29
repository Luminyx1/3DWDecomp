#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
s32 getMainControllerPort();

bool isPadTypeJoySingle(s32);

bool isPadTriggerL(s32);
bool isPadTriggerPressLeftStick(s32);

bool isPadHoldA(s32);
bool isPadHoldX(s32);
bool isPadHoldL(s32);
bool isPadHoldR(s32);
bool isPadHoldZL(s32);
bool isPadHoldZR(s32);

bool isPadReleaseL(s32);

const sead::Vector2f& getLeftStick(s32);
const sead::Vector2f& getRightStick(s32);
}  // namespace al
