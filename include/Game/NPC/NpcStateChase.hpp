#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.hpp"

class NpcStateParam;
class NpcTargetFinder;

/**
 * @brief Parameters of the NPC chase state.
 * @note Only the fields used by reconstructed code are named.
 */
class NpcStateChaseParam {
public:
    NpcStateChaseParam(f32 runAccel, f32 _4, f32 _8, f32 turnDegree, f32 _10, bool _14,
                       bool isEnableCliffCheck, bool isEnableShoreCheck,
                       const char* pRunActionName, const char* pWaitActionName, f32 chaseRange);

    /** @return Acceleration applied while running. */
    f32 getRunAccel() const { return mRunAccel; }

    /** @return Maximum turn per step, in degrees. */
    f32 getTurnDegree() const { return mTurnDegree; }

private:
    f32 mRunAccel;  // 0x0
    u8 _4[0x8];
    f32 mTurnDegree;  // 0xc
    alignas(8) u8 _10[0x90 - 0x10];
};

static_assert(sizeof(NpcStateChaseParam) == 0x90);

/**
 * @brief NPC state that chases the target of a target finder.
 * @note Only what reconstructed code needs is declared so far.
 */
class NpcStateChase : public al::ActorStateBase {
public:
    NpcStateChase(al::LiveActor* pHost, sead::Vector3f* pFront, NpcTargetFinder* pTargetFinder,
                  const NpcStateParam* pParam, const NpcStateChaseParam* pChaseParam,
                  bool _48, bool* _50);

private:
    u8 _20[0x58 - 0x20];
};

static_assert(sizeof(NpcStateChase) == 0x58);
