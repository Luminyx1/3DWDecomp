#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class BlockHardLaserOnly : public al::LiveActor {
public:
    explicit BlockHardLaserOnly(const char*);
    bool isBreaking(bool);
    s32 getFileID();
    bool canChainBreak();
    bool isDisabled() const;
    f32 getMaxChainBreakDistance() const;
    void breakBlock(int);
private:
    u8 mUnreconstructed[0x64];
};
static_assert(sizeof(BlockHardLaserOnly) == 0x1a8);
