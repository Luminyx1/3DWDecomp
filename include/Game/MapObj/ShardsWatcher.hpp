#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class ShardsWatcher : public al::LiveActor {
public:
    explicit ShardsWatcher(const char*);
    int getId() const { return mId; }
private:
    u8 mUnreconstructed144[0x34];
    int mId;
    u8 mUnreconstructed17c[0x34];
};
static_assert(sizeof(ShardsWatcher) == 0x1b0);
