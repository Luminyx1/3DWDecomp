#pragma once
#include "Library/Scene/Scene.hpp"
class GameDataHolder;

class CourseSelectScene : public al::Scene {
public:
    int getMainPlayerInputPort() const;
    GameDataHolder* getGameDataHolder() const { return mpGameData; }
private:
    // Scene state preceding the game-data reference is not yet reconstructed.
    unsigned char _0e1[0x5f];
    GameDataHolder* mpGameData;
};
