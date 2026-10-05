#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class ItemBubble;
class KinokoSuper : public al::LiveActor {
public:
    explicit KinokoSuper(const char* pName, ItemBubble* = nullptr);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(KinokoSuper) == 0x188);
