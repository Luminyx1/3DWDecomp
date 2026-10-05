#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class BlockHardLaserOnly : public al::LiveActor {
public:
    explicit BlockHardLaserOnly(const char*);
    bool isBreaking(bool);
    s32 getFileID();
private:
    u8 mUnreconstructed[0x64];
};
static_assert(sizeof(BlockHardLaserOnly) == 0x1a8);
