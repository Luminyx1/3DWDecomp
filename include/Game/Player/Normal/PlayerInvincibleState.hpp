#pragma once

#include "Player/IUsePlayerInvincibleCheck.hpp"

/// The player's star invincibility.
class PlayerInvincibleState : public IUsePlayerInvincibleCheck {
public:
    bool isInvincible() const override;

    void getStar(bool isPlayBgm);
    void endForce(bool isStopBgm, bool isKeepModel);
};
