#pragma once

#include "Library/Scene/ISceneObj.hpp"

class PlayerActor;

/// Scene object holding the player actors of every character, used or not.
class PlayerStocker : public al::ISceneObj {
public:
    PlayerStocker(int playerNum);

    PlayerActor* getUnusedPlayer(int characterType) const;
};
