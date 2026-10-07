#pragma once

#include "Library/Nerve/NerveStateBase.hpp"

class ItemStatePopUpAbove : public al::ActorStateBase {
public:
    explicit ItemStatePopUpAbove(al::LiveActor* pActor);
    void init() override;
    void appear() override;
    void kill() override;
    void exePunch();
    void exeFall();

private:
    bool mIsInWater;
    float mSpeedScale = 1.0f;
};
