#pragma once
#include "Library/Nerve/NerveExecutor.hpp"
namespace al { class LiveActor; class HitSensor; }
struct HeadgearPoseBuilderParam {
    bool _0;
    bool _1;
    bool _2;
};
static_assert(sizeof(HeadgearPoseBuilderParam) == 3);
class HeadgearPoseBuilder : public al::NerveExecutor {
public:
    HeadgearPoseBuilder(al::LiveActor*, const HeadgearPoseBuilderParam*);
    void start(al::HitSensor*);
    void update();
    void updateAndCalcAnimDirect();
    bool isPlayerRaidonActionPlaying() const;
    bool isPlayerActionPlaying(const char*) const;
    const char* getPlayerActionName() const;
    al::HitSensor* getPlayerSensor() const { return mPlayerSensor; }
    void setChildActor(al::LiveActor* actor) { mChildActor = actor; }
    void clearPlayerSensor() { mPlayerSensor = nullptr; }
private:
    al::LiveActor* mHost;
    al::LiveActor* mChildActor;
    al::HitSensor* mPlayerSensor;
    u8 mUnreconstructed28[0x20];
};
static_assert(sizeof(HeadgearPoseBuilder) == 0x48);
