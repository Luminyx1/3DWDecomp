#pragma once

#include "Player/PlayerActionCondition.hpp"
#include "Player/PlayerDef.hpp"

class IUsePlayerCharaQuery;

/// Holds when the player is a given character.
class PlayerActionConditionCharaType : public PlayerActionCondition {
public:
    PlayerActionConditionCharaType(const IUsePlayerCharaQuery*, EPlayerChara);

    bool check() override;

private:
    const IUsePlayerCharaQuery* mCharaQuery;  // 0x8
    EPlayerChara mChara;  // 0x10
};
