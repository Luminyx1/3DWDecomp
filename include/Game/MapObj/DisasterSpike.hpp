#pragma once

#include "Library/LiveActor/LiveActor.hpp"

class DisasterSpikeDirector;

/// Spike that falls from the sky during Bowser's Fury disasters.
class DisasterSpike : public al::LiveActor {
public:
    explicit DisasterSpike(const char* pName);

    void init(const al::ActorInitInfo& rInfo) override;
    void initAfterPlacement() override;
    void appear() override;
    void kill() override;
    void startClipped() override;
    void endClipped() override;
    void attackSensor(al::HitSensor* pSender, al::HitSensor* pReceiver) override;
    bool receiveMsg(const al::SensorMsg* pMsg, al::HitSensor* pSender,
                    al::HitSensor* pReceiver) override;
    void control() override;

    void setDisasterSpikeDirector(DisasterSpikeDirector* pDirector);
    void setOceanSpike();
    void setReplacedByGold(bool isReplaced);
    bool canBeReplacedByGold() const;
    void clearGoldSpikeOriginalSpike();
    bool isGoldSpikeInUse() const;
    bool hasLanded();
    sead::Vector3f getStartPosition();
    sead::Vector3f getEndPosition();
    void setPosition(sead::Vector3f pos);
    bool tryAppear(bool isForce);

    DisasterSpikeDirector* getDisasterSpikeDirector() const { return mDisasterSpikeDirector; }

protected:
    DisasterSpikeDirector* mDisasterSpikeDirector;  // 0x148

private:
    u8 mUnreconstructed[0x318 - 0x150];
};

static_assert(sizeof(DisasterSpike) == 0x318);
