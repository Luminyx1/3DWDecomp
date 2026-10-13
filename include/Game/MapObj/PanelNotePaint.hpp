#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class PanelNotePaint : public al::LiveActor {
  public:
    explicit PanelNotePaint(const char* name);
    ~PanelNotePaint() override;
    void init(const al::ActorInitInfo&) override;
    void initAfterPlacement() override;
    void makeActorAppeared() override;
    void control() override;
    void attackSensor(al::HitSensor*, al::HitSensor*) override;
    bool receiveMsg(const al::SensorMsg*, al::HitSensor*, al::HitSensor*) override;
    bool receiveMsgScreenPoint(const al::SensorMsg*, al::ScreenPointer*,
                               al::ScreenPointTarget*) override;
    bool isOn() const;
    al::HitSensor* getScoreSensor();
    void setCompleteEffect();
    bool isInitiallyOn() const { return mInitiallyOn; }
    void setPitch(int pitch) { mPitch = pitch; }

  private:
    alignas(8) unsigned char _148[0x20];
    bool mInitiallyOn;
    unsigned char _169[3];
    int mPitch;
};
static_assert(sizeof(PanelNotePaint) == 0x170);
