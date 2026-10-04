#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class HitSensor;
}  // namespace al

/**
 * @brief The light that carries a bound player to a warp destination.
 * @note Only what reconstructed code needs is declared so far.
 */
class BindWarpEffect : public al::LiveActor {
public:
    BindWarpEffect();

    void init(const al::ActorInitInfo& rInfo) override;

    void start(const al::HitSensor* pPlayerSensor, const sead::Vector3f& rEndPos, bool isFast);
    void cancel();
    bool isEnd() const;

private:
    u8 _148[0x178 - 0x148];
};

static_assert(sizeof(BindWarpEffect) == 0x178);
