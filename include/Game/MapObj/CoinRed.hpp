#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class IUseRedCoin;
class CoinRed : public al::LiveActor {
public:
    CoinRed(const char*, IUseRedCoin*);
    void disableCountdown();
private:
    u8 mUnreconstructed[0x198 - 0x144];
};
