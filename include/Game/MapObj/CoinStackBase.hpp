#pragma once
#include "Library/LiveActor/LiveActor.hpp"
namespace al { class MtxConnector; }
class CoinStackBase : public al::LiveActor {
public:
    CoinStackBase(const char*, bool);
    void initPosition(const sead::Vector3f&);
    void initConnector(const al::MtxConnector*);
    void requestFall(int);
    float getStackHeight() const { return mStackHeight; }
    bool isCollected() const { return mIsCollected; }
    void clearCollected() { mIsCollected = false; }
private:
    bool mIsCollected;
    float mStackHeight;
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(CoinStackBase) == 0x1a0);
