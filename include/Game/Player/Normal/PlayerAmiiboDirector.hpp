#pragma once

class PlayerActor;

class PlayerAmiiboDirector {
  public:
    static void initRandomSeed();

    void clear();
    bool isCurrentPlayerActor(const PlayerActor* pActor);
};
