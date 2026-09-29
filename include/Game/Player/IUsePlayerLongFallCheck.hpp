#pragma once

/// Whether the player has been falling for long (implemented by PlayerLongFallCheck).
class IUsePlayerLongFallCheck {
public:
    virtual bool isLongFalling() const = 0;
    virtual void resetLongFall() = 0;
};
