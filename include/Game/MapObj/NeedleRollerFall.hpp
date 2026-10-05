#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class NeedleRollerFall : public al::LiveActor {
public:
    explicit NeedleRollerFall(const char*);
    void start();
private:
    u8 mUnreconstructed[0x64];
};
static_assert(sizeof(NeedleRollerFall) == 0x1a8);
