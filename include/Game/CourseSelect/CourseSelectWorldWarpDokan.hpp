#pragma once
#include "Library/LiveActor/LiveActor.hpp"

class WorldWarpDokanController;
class CourseSelectPuppeteerGroup;
class CourseSelectWorldWarpDokan : public al::LiveActor {
public:
    explicit CourseSelectWorldWarpDokan(const char* pName);
    ~CourseSelectWorldWarpDokan() override;
    void init(const al::ActorInitInfo& rInfo) override;
    void attackSensor(al::HitSensor* pSender, al::HitSensor* pReceiver) override;
    bool receiveMsg(const al::SensorMsg* pMsg, al::HitSensor* pSender, al::HitSensor* pReceiver) override;
    void startPuppetDemo(CourseSelectPuppeteerGroup* pGroup);
    void endPuppetDemo();
    void startDemo();
    bool isEndDemo() const;
    void exeWait();
    void exeDemo();
    void exeDisappear();
private:
    WorldWarpDokanController* mpController = nullptr;
    CourseSelectPuppeteerGroup* mpPuppeteerGroup = nullptr;
};
static_assert(sizeof(CourseSelectWorldWarpDokan) == 0x158);
