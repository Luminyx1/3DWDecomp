#pragma once
#include "Library/Nerve/NerveStateBase.hpp"

class KoopaChase;
class KoopaChaseStateDamage : public al::NerveStateBase {
public:
    KoopaChase* mHost;
    int mDamageCount;
    unsigned char mStateData24[0xc];
};
