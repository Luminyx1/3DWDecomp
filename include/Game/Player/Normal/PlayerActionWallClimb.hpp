#pragma once

#include "Player/PlayerAction.hpp"

/// The player's wall climb action.
class PlayerActionWallClimb : public PlayerAction {
    SEAD_RTTI_OVERRIDE(PlayerActionWallClimb, PlayerAction)
};
