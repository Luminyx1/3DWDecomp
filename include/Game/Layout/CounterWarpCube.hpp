#pragma once
#include "Library/Layout/LayoutActor.hpp"
namespace al { class LiveActor; class HitSensor; }
class WarpCube;
class CounterWarpCube : public al::LayoutActor {
public:
    void addCount(const al::LiveActor*, al::HitSensor*);
private:
    WarpCube* mWarpCube;
    int mCount;
    int mRequiredCount;
};
