#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveExecutor.hpp"

namespace al {
class HitSensor;
class LiveActor;
}  // namespace al

class IUsePlayerPuppet;
class PlayerBindEndParam;

/**
 * @brief Base of the objects that drive a bound player (its puppet) through a scripted move.
 * @note Only what reconstructed code needs is declared so far.
 */
class BindPuppeteer : public al::NerveExecutor {
public:
    BindPuppeteer(const char* pName);

    virtual void endBind(const PlayerBindEndParam* pParam);
    virtual void endBindOnGround();
    virtual void endBindSquat();
    virtual void endBindForceAbyss();
    virtual void cancelBind();

    void startBind(al::HitSensor* pPlayerSensor, al::HitSensor* pBinderSensor);
    void setNullPlayerPuppet();
    al::LiveActor* getTargetActor();
    s32 getControlUserId() const;
    IUsePlayerPuppet* getPlayerPuppet() const;

private:
    IUsePlayerPuppet* mPlayerPuppet = nullptr;  // 0x10
    s32 _18 = 0;
};

static_assert(sizeof(BindPuppeteer) == 0x20);
