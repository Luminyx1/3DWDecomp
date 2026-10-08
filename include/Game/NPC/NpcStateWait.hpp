#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.hpp"

namespace al {
class ActorInitInfo;
class HitSensor;
class LiveActor;
class ScreenPointer;
class ScreenPointTarget;
class SensorMsg;
}  // namespace al

class NpcStateRumbleParam;
class NpcStateTurnParam;

/**
 * @brief Parameters of the NPC wait state: the action names it plays and how it reacts.
 * @note The fields have not been reconstructed yet; the size is a guess from the arguments.
 */
class NpcStateWaitParam {
public:
    NpcStateWaitParam(const char* pWaitActionName, const char* pWaitTalkActionName,
                      const char* pTurnActionName, const char* pReactionActionName,
                      const char* pReactionMicActionName, const char* pTouchActionName,
                      const char* pTrampledActionName, bool _38, const sead::Vector3f* pOffset,
                      bool _48);

private:
    alignas(8) u8 _0[0x50];
};

/**
 * @brief NPC state that waits in place, turning to and reacting at the player.
 * @note Only what reconstructed code needs is declared so far.
 */
class NpcStateWait : public al::ActorStateBase {
public:
    NpcStateWait(al::LiveActor* pHost, const al::ActorInitInfo& rInfo,
                 const NpcStateWaitParam* pWaitParam, const NpcStateTurnParam* pTurnParam,
                 const NpcStateRumbleParam* pRumbleParam);

    void attackSensor(al::HitSensor* pSelf, al::HitSensor* pOther);
    bool receiveMsg(const al::SensorMsg* pMsg, al::HitSensor* pSelf, al::HitSensor* pOther);
    bool receiveMsgScreenPoint(const al::SensorMsg* pMsg, al::ScreenPointer* pPointer,
                               al::ScreenPointTarget* pTarget);

private:
    u8 _20[0x58 - 0x20];
};

static_assert(sizeof(NpcStateWait) == 0x58);
