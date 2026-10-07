#pragma once
#include "Library/Nerve/NerveExecutor.hpp"
namespace al { class LiveActor; struct AnimScaleParam; }
class ActorMicRumbler : public al::NerveExecutor {
public:
    ActorMicRumbler(al::LiveActor*, const al::AnimScaleParam*);
    void update();
    void stopAndReset();
private:
    u8 mUnreconstructed[0x20];
};
static_assert(sizeof(ActorMicRumbler) == 0x30);
