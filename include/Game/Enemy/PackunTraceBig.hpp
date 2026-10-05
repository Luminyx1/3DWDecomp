#pragma once

#include "Library/LiveActor/LiveActor.hpp"
#include <math/seadQuat.h>

namespace al {
class MtxConnector;
}

class PackunTraceBig : public al::LiveActor {
public:
    PackunTraceBig(al::LiveActor* pOwner);
    /** @brief Releases the large Piranha Plant remains actor. */
    ~PackunTraceBig() override = default;
    void init(const al::ActorInitInfo& rInfo) override;
    void initAfterPlacement() override;
    void control() override;
    void appear() override;
    void attackSensor(al::HitSensor* pSelf, al::HitSensor* pOther) override;
    bool receiveMsg(const al::SensorMsg* pMsg, al::HitSensor* pOther,
                    al::HitSensor* pSelf) override;
    void exeAppear();
    void exeWait();
    void exeReaction();

private:
    al::LiveActor* mOwner;
    al::MtxConnector* mConnector = nullptr;
    sead::Quatf mBaseQuat = sead::Quatf::unit;
};
