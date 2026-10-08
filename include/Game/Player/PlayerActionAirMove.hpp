#pragma once

#include "Player/PlayerAction.hpp"

/// The player's air move action.
class PlayerActionAirMove : public PlayerAction {
    SEAD_RTTI_OVERRIDE(PlayerActionAirMove, PlayerAction)
};
