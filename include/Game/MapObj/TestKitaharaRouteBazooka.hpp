#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class TestKitaharaRouteBazookaEntrance;
// Partial declaration; the host's data members are not yet reconstructed.
class TestKitaharaRouteBazooka : public al::LiveActor {
public:
    virtual bool isBindStart(al::HitSensor*, al::HitSensor*);
    virtual bool startBind(TestKitaharaRouteBazookaEntrance*, al::HitSensor*, al::HitSensor*);
    virtual bool cancelBind(al::HitSensor*);
    bool damagePuppet(al::HitSensor*);
};
