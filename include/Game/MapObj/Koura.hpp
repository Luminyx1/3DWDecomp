#pragma once
#include "Library/LiveActor/LiveActor.hpp"
// Partial layout; virtual overrides are retained for derived shell actors.
class Koura : public al::LiveActor {
public:
    explicit Koura(const char*);
    ~Koura() override = default;
    void init(const al::ActorInitInfo&) override;
    void appear() override;
    void kill() override;
    bool hideActor() override;
    void attackSensor(al::HitSensor*, al::HitSensor*) override;
    bool receiveMsg(const al::SensorMsg*, al::HitSensor*, al::HitSensor*) override;
    bool receiveMsgScreenPoint(const al::SensorMsg*, al::ScreenPointer*, al::ScreenPointTarget*) override;
    bool isInRouteDokan() const override;
    void control() override;
    void updateCollider() override;
    virtual const char* getArchiveName() const;
    virtual void onHitWall();
    void startMove(const sead::Vector3f&, float, float);
    void startBreak();
    bool isPlayerInside() const;
    void exeUpper();
private:
    u8 mUnreconstructed[0x12c];
};
static_assert(sizeof(Koura) == 0x270);
