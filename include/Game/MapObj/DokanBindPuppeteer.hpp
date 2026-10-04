#pragma once

#include <basis/seadTypes.h>

#include "MapObj/BindPuppeteer.hpp"

namespace al {
class ActorInitInfo;
}  // namespace al

class Dokan;

/**
 * @brief Moves a bound player into a warp pipe, warps it and moves it out of the destination.
 * @note Only what reconstructed code needs is declared so far.
 */
class DokanBindPuppeteer : public BindPuppeteer {
public:
    DokanBindPuppeteer(const char* pName, bool isSide, bool isWorldWarp, al::LiveActor* pHost);

    void endBind(const PlayerBindEndParam* pParam) override;
    void cancelBind() override;

    void init(const al::ActorInitInfo& rInfo);
    void startBind(al::HitSensor* pPlayerSensor, al::HitSensor* pDokanSensor,
                   const al::LiveActor* pDokan, const Dokan* pDestDokan, bool isBindAll,
                   bool isHipDrop, bool isRolling);
    void update();
    void setOnPlayerCountMax();
    bool isDeactive() const;
    bool isEnableStartBind(bool isHipDrop, bool isBindAll) const;
    bool isWaitStartWarp() const;
    bool isWaitStartWorldWarp() const;
    void warp(s32 index, s32 num, bool isUseCamera);
    void dokanOut();

private:
    u8 _1c[0xe0 - 0x1c];
};

static_assert(sizeof(DokanBindPuppeteer) == 0xe0);
