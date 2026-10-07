#pragma once

#include "Library/Nerve/NerveStateBase.hpp"

namespace al {
class ComboCounter;
class HitSensor;
class LiveActor;
}  // namespace al

struct BombStateExplosionParam;

class BombStateExplosion : public al::ActorStateBase {
public:
    BombStateExplosion(al::LiveActor* pActor, bool isUseExplosionSensor,
                       const BombStateExplosionParam* pParam);
    void reset();
    void attackSensor(al::HitSensor* pSelf, al::HitSensor* pOther, al::ComboCounter* pCounter);

private:
    u8 mUnreconstructed[0x10];
};
static_assert(sizeof(BombStateExplosion) == 0x30);
