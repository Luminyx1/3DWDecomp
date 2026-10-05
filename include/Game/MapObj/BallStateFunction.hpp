#pragma once

#include <math/seadVector.h>

namespace al { class LiveActor; }
class BallStateFallParam;

namespace BallStateFunction {
float calcSpeedFront(al::LiveActor*);
void rotateOnGround(al::LiveActor*, const sead::Vector3f&);
void rotateOnAir(al::LiveActor*, float, bool);
void sendMsgToCollision(al::LiveActor*, bool);
void reboundCollisionWallOrCeiling(al::LiveActor*);
bool reboundCollisionGround(al::LiveActor*, const BallStateFallParam*, bool);
}
