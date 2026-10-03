#pragma once
#include "System/GameDataHolderAccessor.hpp"
class SingleModeDataFunction {
  public:
    static bool isScenarioComplete(GameDataHolderAccessor accessor, int islandId, int scenarioId);
    static int getUnlockedPhase(GameDataHolderAccessor accessor);
    static int getUnlockedIslandNum(GameDataHolderAccessor accessor);
    static int getLastValidIslandVisited(GameDataHolderAccessor accessor);
    static int getCurValidIslandVisited(GameDataHolderAccessor accessor);
};
