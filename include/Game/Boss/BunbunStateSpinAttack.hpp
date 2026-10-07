#pragma once

#include "Library/Nerve/NerveStateBase.hpp"

class BunbunStateSpinAttack : public al::ActorStateBase {
public:
    bool isValidAttackSensor() const;

private:
    // The state is allocated as 0xb8 bytes by Bunbun::init; its fields remain unreconstructed.
    unsigned char mStateData[0x98];
};
