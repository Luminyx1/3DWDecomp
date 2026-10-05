#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class ItemBubble;
class SuperLeaf : public al::LiveActor {
public:
    explicit SuperLeaf(const char* pName, ItemBubble* = nullptr);

private:
    u8 mUnreconstructed[0x4c];
};
static_assert(sizeof(SuperLeaf) == 0x190);
