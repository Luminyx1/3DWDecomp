#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class GoalItem : public al::LiveActor {
public:
    explicit GoalItem(const char*);
    bool isCollected();
    int getIslandId() const { return mIslandId; }
    int getShineId() const { return mShineId; }
    bool isNekoShine() const { return mIsNekoShine; }
    bool isDisasterShine() const { return mIsDisasterShine; }
    void setRegistration(int index, int islandId) { mIndex = index; mIslandId = islandId; }
private:
    u8 mUnknown144[0x24];
    int mIslandId;
    int mShineId;
    bool mIsNekoShine;
    bool mIsDisasterShine;
    u8 mUnknown172[0x46];
    int mIndex;
    u8 mUnknown1bc[0x214];
};
static_assert(sizeof(GoalItem) == 0x3d0);
