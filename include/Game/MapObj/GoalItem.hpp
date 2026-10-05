#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class IGoalItemCollectListener;
class GoalItem : public al::LiveActor {
public:
    explicit GoalItem(const char*);
    bool isCollected();
    void appearCollect(bool);
    void setFront(sead::Vector3f&, bool, float);
    void setFromShards() { mFromShards = true; }
    void setCollectListener(IGoalItemCollectListener* listener) { mCollectListener = listener; }
    void setCollectedBySensor() { mCollectedBySensor = true; }
    void disableCollectionFlag() { mCollectionFlag = false; }
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
    u8 mUnknown1bc[0x7c];
    bool mCollectionFlag;
    u8 mUnknown239[0x5c];
    bool mCollectedBySensor;
    u8 mUnknown296[0x122];
    IGoalItemCollectListener* mCollectListener;
    bool mFromShards;
    u8 mUnknown3c1[0xf];
};
static_assert(sizeof(GoalItem) == 0x3d0);
