#pragma once
#include "Library/Scene/Scene.hpp"
class GameDataHolder;
class CourseSelectWindowHolder;
class CourseSelectDirector;

class CourseSelectScene : public al::Scene {
public:
    CourseSelectDirector* getCourseSelectDirector() const;
    CourseSelectWindowHolder* getWindowHolder() const { return mpWindows; }
    int getMainPlayerInputPort() const;
    GameDataHolder* getGameDataHolder() const { return mpGameData; }
private:
    // Scene state preceding the game-data reference is not yet reconstructed.
    unsigned char _0e1[0x5f];
    GameDataHolder* mpGameData;
    unsigned char _148[0xd0];
    CourseSelectWindowHolder* mpWindows;
};
