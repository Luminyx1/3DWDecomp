#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

/// Button mapping of a player's controller.
class IUsePlayerKeyConfig {
public:
    virtual s32 getPort() const = 0;
    virtual bool isPadTriggerPlayerJump() const = 0;
    virtual bool isPadHoldPlayerJump() const = 0;
    virtual bool isPadTriggerPlayerSquat() const = 0;
    virtual bool isPadHoldPlayerSquat() const = 0;
    virtual bool isPadTriggerPlayerHipDrop() const = 0;
    virtual bool isPadHoldPlayerHipDrop() const = 0;
    virtual bool isPadTriggerPlayerDash() const = 0;
    virtual bool isPadHoldPlayerDash() const = 0;
    virtual bool isPadReleasePlayerDash() const = 0;
    virtual bool isPadHoldPlayerStatue() const = 0;
    virtual bool isPadTriggerPlayerStatue() const = 0;
    virtual bool isPadTriggerPlayerRolling() const = 0;
    virtual bool isPadHoldPlayerRolling() const = 0;
    virtual bool isPadTriggerPlayerBubble() const = 0;
    virtual bool isPadHoldPlayerHold() const = 0;
    virtual bool isPadShakePlayerHold() const = 0;
    virtual bool isPadTriggerPlayerHold() const = 0;
    virtual bool isPadTriggerPlayerRelease() const = 0;
    virtual bool isPadTriggerDecide() const = 0;
    virtual bool isPadHoldDecide() const = 0;
    virtual bool isPadTriggerCancel() const = 0;
    virtual bool isPadHoldCancel() const = 0;
    virtual bool isPadTriggerGyroCursor() const = 0;
    virtual bool isPadHoldGyroCursor() const = 0;
    virtual void calcLeftStick(sead::Vector2f* pStick) const = 0;
};
