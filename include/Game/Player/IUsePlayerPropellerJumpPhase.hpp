#pragma once

/// Phase of the propeller box jump (implemented by PlayerActionPropellerJump).
class IUsePlayerPropellerJumpPhase {
public:
    virtual bool isPropellerJumping() const = 0;
    virtual bool isPropellerJumpRising() const = 0;
    virtual bool isPropellerJumpGlide() const = 0;
};
