#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.hpp"

/**
 * @brief The button guide balloon shown above a warp pipe while a player stands on it.
 * @note Only what reconstructed code needs is declared so far.
 */
class DokanGuideBalloon : public al::LiveActor {
public:
    DokanGuideBalloon(const sead::Vector3f* pTrans);

    void init(const al::ActorInitInfo& rInfo) override;
    void kill() override;
    void control() override;

    void playerTouched();
    void setPlayerSensor(const al::HitSensor* pPlayerSensor);
    void startShow(s32 port);

private:
    u8 _144[0x170 - 0x144];
};

static_assert(sizeof(DokanGuideBalloon) == 0x170);
