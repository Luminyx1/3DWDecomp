#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class ItemBubble;
class KinokoOneUp : public al::LiveActor {
public:
    explicit KinokoOneUp(const char* pName, ItemBubble* = nullptr);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(KinokoOneUp) == 0x188);
