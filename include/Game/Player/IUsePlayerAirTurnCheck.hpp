#pragma once

/// Whether the player is turning in the air (implemented by PlayerAirTurnChecker).
class IUsePlayerAirTurnCheck {
public:
    virtual bool isAirTurning() const = 0;
};
