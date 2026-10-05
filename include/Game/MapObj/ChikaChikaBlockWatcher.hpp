#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class ChikaChikaBlockWatcher : public al::LiveActor {
public:
    ChikaChikaBlockWatcher(const char*);
    bool isStopBgm();
    int getSwitchInterval() const { return mSwitchInterval; }
private:
    unsigned char _144[0xc];
    int mSwitchInterval;
    unsigned char _154[0xc];
};
static_assert(sizeof(ChikaChikaBlockWatcher) == 0x160);
