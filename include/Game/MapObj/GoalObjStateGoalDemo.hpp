#pragma once
#include "Library/Nerve/NerveStateBase.hpp"
namespace al { class ActorInitInfo; class SensorMsg; class HitSensor; class AudioDirector; }
class DemoStartPosition;
class GoalObjStateGoalDemoParam {
public:
    GoalObjStateGoalDemoParam();
    int _0;
    int _4;
    unsigned char _8[8];
    int _10;
    int _14;
    int _18;
    unsigned char _1c[0x14];
    float _30;
    unsigned char _34[0xc];
};
static_assert(sizeof(GoalObjStateGoalDemoParam) == 0x40);
class GoalObjStateGoalDemo : public al::ActorStateBase {
public:
    GoalObjStateGoalDemo(al::LiveActor*, const al::ActorInitInfo&, bool, const GoalObjStateGoalDemoParam*);
    bool receiveMsg(const al::SensorMsg*, al::HitSensor*, al::HitSensor*);
    bool isDemoBefore() const;
    void setDemoStartPosition(DemoStartPosition* pos) { mStartPosition = pos; }
    void setGoalPlayer(al::LiveActor* player) { mGoalPlayer = player; }
    void setKinopioBrigadeDemo(bool enabled) { mKinopioBrigadeDemo = enabled; }
    void setGoalItem(al::LiveActor* item) { mGoalItem = item; }
    void setGoalAction(const char* action) { mGoalAction = action; }
    void setAudioDirector(al::AudioDirector* director) { mAudioDirector = director; }
private:
    unsigned char _20[0x20];
    DemoStartPosition* mStartPosition;
    al::LiveActor* mGoalPlayer;
    bool mKinopioBrigadeDemo;
    unsigned char _51[0x1f];
    al::LiveActor* mGoalItem;
    unsigned char _78[0x20];
    const char* mGoalAction;
    al::AudioDirector* mAudioDirector;
    unsigned char _a8[0x10];
};
static_assert(sizeof(GoalObjStateGoalDemo) == 0xb8);
