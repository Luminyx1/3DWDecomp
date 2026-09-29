#pragma once

/// Whether the player is invincible (implemented by PlayerInvincibleState).
class IUsePlayerInvincibleCheck {
public:
    virtual bool isInvincible() const = 0;
};
