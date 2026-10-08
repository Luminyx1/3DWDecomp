#pragma once

#include "Player/PlayerAction.hpp"

/// The player's rolling attack action.
class PlayerActionRollingAttack : public PlayerAction {
    SEAD_RTTI_OVERRIDE(PlayerActionRollingAttack, PlayerAction)
};
