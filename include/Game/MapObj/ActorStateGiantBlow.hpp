#pragma once

#include "Library/Nerve/NerveStateBase.hpp"

namespace al {
class HitSensor;
class LiveActor;
class SensorMsg;
}  // namespace al

struct ActorStateGiantBlowParam;

/**
 * @brief Actor state that blows an object away when a giant player rams it, swapping the actor
 * for a broken model and a trace model.
 * @note Only what reconstructed code needs is declared so far.
 */
class ActorStateGiantBlow : public al::ActorStateBase {
public:
    ActorStateGiantBlow(al::LiveActor* pActor, const ActorStateGiantBlowParam* pParam,
                        al::LiveActor* pBreakModel, al::LiveActor* pTraceModel);

    void appear() override;

    bool tryStartBlow(const al::SensorMsg* pMsg, al::HitSensor* pOther, al::HitSensor* pSelf);

private:
    u8 _20[0x48 - 0x20];
};

static_assert(sizeof(ActorStateGiantBlow) == 0x48);
