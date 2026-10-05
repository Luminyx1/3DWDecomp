#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class Tree : public al::LiveActor {
public:
    explicit Tree(const char*);
    void forceRespawn();
    void setRespawnByWatcher() { mRespawnByWatcher = true; }
private:
    u8 mUnreconstructed144[0x139];
    bool mRespawnByWatcher;
};
static_assert(sizeof(Tree) == 0x280);
