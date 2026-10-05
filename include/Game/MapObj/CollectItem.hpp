#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class ItemBubble;
class CollectItem : public al::LiveActor {
public:
    CollectItem(const char*, ItemBubble*, bool);
private:
    u8 mUnreconstructed[0x94];
};
static_assert(sizeof(CollectItem) == 0x1d8);
