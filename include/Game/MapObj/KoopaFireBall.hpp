#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class KoopaFireBall : public al::LiveActor {
public:
    KoopaFireBall(const char*, bool, al::LiveActor*);
    void appearAttack(const sead::Vector3f&, const sead::Vector3f&, float);
private:
    u8 mUnreconstructed[0xac];
};
static_assert(sizeof(KoopaFireBall) == 0x1f0);
