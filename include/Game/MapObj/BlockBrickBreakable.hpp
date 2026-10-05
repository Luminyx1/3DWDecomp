#pragma once

#include "Library/LiveActor/LiveActor.hpp"

class BlockBrickBreakable : public al::LiveActor {
public:
    BlockBrickBreakable(const char*);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(BlockBrickBreakable) == 0x180);