#pragma once

#include <math/seadVector.h>

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerCheckArea;
struct PlayerProperty;

/// Holds while the player is in the kind of area one of IUsePlayerCheckArea's queries tests for.
class PlayerActionConditionArea : public PlayerActionCondition {
public:
    using CheckFunc = bool (IUsePlayerCheckArea::*)(const sead::Vector3f&) const;

    PlayerActionConditionArea(const PlayerProperty*, const IUsePlayerCheckArea*, CheckFunc);

    bool check() override;

private:
    const PlayerProperty* mProperty;        // 0x8
    const IUsePlayerCheckArea* mCheckArea;  // 0x10
    CheckFunc mCheckFunc;                   // 0x18
};
