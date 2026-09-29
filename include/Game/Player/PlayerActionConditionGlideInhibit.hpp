#pragma once

#include "Player/PlayerActionCondition.hpp"

class IUsePlayerGlideInhibitor;

/// Holds while gliding is blocked.
class PlayerActionConditionGlideInhibit : public PlayerActionCondition {
public:
    PlayerActionConditionGlideInhibit(const IUsePlayerGlideInhibitor*);

    bool check() override;

private:
    const IUsePlayerGlideInhibitor* mGlideInhibitor;  // 0x8
};
