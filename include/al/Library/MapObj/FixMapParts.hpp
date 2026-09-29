#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class MtxConnector;

class FixMapParts : public LiveActor {
public:
    FixMapParts(const char*);

    virtual void init(const ActorInitInfo&) override;
    virtual void initAfterPlacement() override;
    virtual void appear() override;
    virtual void updateLinkedTrans(const sead::Vector3f&) override;
    virtual bool receiveMsg(const SensorMsg* msg, HitSensor* self, HitSensor* other) override;
    virtual void control() override;

    void initWithSuffix(const ActorInitInfo&, const char*);

    MtxConnector* mConnector = nullptr;  // _148
};
}  // namespace al
