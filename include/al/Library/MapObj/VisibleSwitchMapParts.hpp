#pragma once

#include "Library/LiveActor/LiveActor.hpp"

class VisibleSwitchMapParts : public al::LiveActor {
public:
    VisibleSwitchMapParts(const char* pName);

    void init(const al::ActorInitInfo& rInfo) override;
    bool receiveMsg(const al::SensorMsg* pMsg, al::HitSensor* pOther,
                    al::HitSensor* pSelf) override;

    void startDisappear();
    void startAppear();
    void exeShow();
    void exeDisappear();
    void exeHide();
    void exeAppear();
};
