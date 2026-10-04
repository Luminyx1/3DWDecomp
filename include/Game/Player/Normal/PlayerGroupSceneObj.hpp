#pragma once

#include "Library/Scene/ISceneObj.hpp"
#include "Player/Normal/PlayerGroup.hpp"

/// Scene object holding the PlayerGroup.
class PlayerGroupSceneObj : public al::ISceneObj {
public:
    PlayerGroupSceneObj();

    void initGroup(int playerNum);

    PlayerGroup* getPlayerGroup() { return &mPlayerGroup; }

private:
    PlayerGroup mPlayerGroup;  // 0x8
};
