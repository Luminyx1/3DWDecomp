#pragma once
#include "Library/LiveActor/LiveActor.hpp"
namespace al { class PlacementInfo; }
class ShardsWatcher;
class Shards : public al::LiveActor {
public:
    explicit Shards(const char*, bool = false);
    void setShardId(const al::PlacementInfo&);
    void sharedInit(const al::ActorInitInfo&);
    void setWatcher(ShardsWatcher* watcher) { mWatcher = watcher; }
    int getIslandId() const { return mIslandId; }
    void setIslandId(int id) { mIslandId = id; }
    void setFromWatcher() { mFromWatcher = true; }
    bool isCollected() const { return mCollected; }
    bool isCollectionFinished() const { return mCollectionFinished; }
    bool isCollecting() const { return mCollecting; }
    bool isGoalPending() const { return mGoalPending; }
    bool isCollectedBySensor() const { return mCollectedBySensor; }
    al::HitSensor* getCollectSensor() const { return mCollectSensor; }
    const sead::Vector3f& getGoalFront() const { return mGoalFront; }
    bool isUseFrontAngle() const { return mUseFrontAngle; }
    float getFrontAngle() const { return mFrontAngle; }
private:
    ShardsWatcher* mWatcher;
    u8 mUnknown150[0x30];
    al::HitSensor* mCollectSensor;
    u8 mUnknown188[0x24];
    int mIslandId;
    bool mCollected;
    bool mCollectionFinished;
    u8 mUnknown1b2[0x29];
    bool mCollecting;
    bool mGoalPending;
    u8 mUnknown1dd;
    bool mFromWatcher;
    bool mCollectedBySensor;
    u8 mUnknown1e0[3];
    bool mUseFrontAngle;
    float mFrontAngle;
    sead::Vector3f mGoalFront;
    u8 mUnknown1f4[0xc];
};
static_assert(sizeof(Shards) == 0x200);
