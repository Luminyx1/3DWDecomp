#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class MtxConnector;

class FixMapParts : public LiveActor {
public:
    FixMapParts(const char*);

    void init(const ActorInitInfo& rInfo) override;
    void initAfterPlacement() override;
    void appear() override;
    void updateLinkedTrans(const sead::Vector3f& rTrans) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;
    void control() override;

    void initWithSuffix(const ActorInitInfo& rInfo, const char* pSuffix);

    MtxConnector* mConnector = nullptr;
};
}  // namespace al
