#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class ItemBubble;
class Bomb : public al::LiveActor {
public:
    explicit Bomb(const char* pName, bool = false);
    void appearPopUpFront();

private:
    u8 mUnreconstructed[0x5c];
};
static_assert(sizeof(Bomb) == 0x1a0);
