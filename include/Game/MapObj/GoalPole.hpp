#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class GoalPoleStateRunaway;
class GoalPole : public al::LiveActor {
public:
    explicit GoalPole(const char*);
    bool isSuper() const;
    bool isLast() const;
    bool isRunaway() const { return mRunawayState != nullptr; }
    const sead::Matrix34f& getRunawayBaseMtx() const { return mRunawayBaseMtx; }
private:
    u8 mUnreconstructed144[0x64];
    GoalPoleStateRunaway* mRunawayState;
    u8 mUnreconstructed1b0[8];
    sead::Matrix34f mRunawayBaseMtx;
    u8 mUnreconstructed1e8[0x58];
};
static_assert(sizeof(GoalPole) == 0x240);
