#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class CheckpointFlag : public al::LiveActor {
public:
    explicit CheckpointFlag(const char*);
private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(CheckpointFlag) == 0x160);
