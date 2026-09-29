#pragma once

/// Whether the player is dashing (implemented by PlayerActionGroundMove).
class IUsePlayerDashChecker {
public:
    virtual bool isDashing() const = 0;
    virtual bool isDashingFast() const = 0;
    virtual bool isRunningOnGround() const = 0;
    virtual bool isGreaterSuperDashMaxSpeed() const = 0;
};
