#pragma once

namespace al { class LiveActor; }
class BallStateFallParam;

namespace BallStateFunction {
void rotateOnAir(al::LiveActor*, float, bool);
void sendMsgToCollision(al::LiveActor*, bool);
void reboundCollisionWallOrCeiling(al::LiveActor*);
bool reboundCollisionGround(al::LiveActor*, const BallStateFallParam*, bool);
}
