#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.hpp"

struct WalkerStateParam;
class TargetFinder;

/** @brief Tuning for WalkerStateWander (wander timing, speeds and actions). */
class WalkerStateWanderParam {
public:
    WalkerStateWanderParam();
    WalkerStateWanderParam(s32 waitTime, s32 walkTime, f32 accel, f32 turnRate, f32 range,
                           f32 searchRange, bool isUseTargetFinder, const char* pWalkAction,
                           const char* pWaitAction);

private:
    alignas(8) u8 _0[0x90];
};

static_assert(sizeof(WalkerStateWanderParam) == 0x90);

/** @brief Walker state that wanders around a center point. */
class WalkerStateWander : public al::ActorStateBase {
public:
    WalkerStateWander(al::LiveActor* pHost, sead::Vector3f* pFrontDir,
                      const WalkerStateParam* pParam, const WalkerStateWanderParam* pWanderParam,
                      TargetFinder* pTargetFinder);

    void setWanderCenter(const sead::Vector3f& rCenter);

private:
    u8 _20[0x28];
};

static_assert(sizeof(WalkerStateWander) == 0x48);
