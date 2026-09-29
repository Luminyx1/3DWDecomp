#pragma once

#include "Player/PlayerDef.hpp"

/// Which character (Mario, Luigi, ...) the player is (implemented by PlayerActor).
class IUsePlayerCharaQuery {
public:
    virtual EPlayerChara getChara() const = 0;
    virtual bool isChara(EPlayerChara) const = 0;
};
