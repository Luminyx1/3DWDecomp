#pragma once

#include "Player/PlayerActionCondition.hpp"

class PlayerSwimSquatInhibitor;

/// Holds while squatting in water is blocked.
class PlayerActionConditionSwimSquatInhibit : public PlayerActionCondition {
public:
    PlayerActionConditionSwimSquatInhibit(PlayerSwimSquatInhibitor*);

    bool check() override;

private:
    PlayerSwimSquatInhibitor* mInhibitor;  // 0x8
};
