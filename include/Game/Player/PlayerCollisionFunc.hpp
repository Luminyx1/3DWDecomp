#pragma once

#include <basis/seadTypes.h>

class PlayerConstParam;
struct PlayerProperty;

/// Helpers around the player's collision.
namespace PlayerCollisionFunc {
f32 calcTall(const PlayerProperty* pProperty, const PlayerConstParam* pConstParam);
}  // namespace PlayerCollisionFunc
