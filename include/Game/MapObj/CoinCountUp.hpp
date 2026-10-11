#pragma once

#include "Library/LiveActor/LiveActor.hpp"

class CoinCountUp : public al::LiveActor {
public:
    explicit CoinCountUp(const char* pName);
    ~CoinCountUp() override;
    void init(const al::ActorInitInfo& rInfo) override;
    void appear() override;
    void kill() override;
    void appearQuick();
    void appearDelayQuick(int delay);

private:
    int mDelay = 0;
};

static_assert(sizeof(CoinCountUp) == 0x148);
