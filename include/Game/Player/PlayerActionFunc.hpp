#pragma once

#include <math/seadVector.h>

class IUsePlayerCollision;
class IUsePlayerInput;
class PlayerFigureDirector;
struct PlayerProperty;

namespace PlayerActionFunc {
    bool isUpperVelocity(const PlayerProperty*);
    void calcDownward(sead::Vector3f*, const PlayerProperty*, const sead::Vector3f&);
    bool checkMapCode(const IUsePlayerCollision*, const char*);
    bool isOppositeSide(const sead::Vector3f&, const sead::Vector3f&);
    bool isOppositeInput(const IUsePlayerInput*, const PlayerProperty*, const sead::Vector3f&);
    bool isMapCodeSkate(const IUsePlayerCollision*);
    bool isRaccoonDog(const PlayerFigureDirector*);
    bool isClimb(const PlayerFigureDirector*);
}  // namespace PlayerActionFunc
