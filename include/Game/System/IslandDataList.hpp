#pragma once

#include "System/GameDataHolderAccessor.hpp"

#include "System/Data/SingleModeDataFunction.hpp"

class IslandData;

class IslandDataList {
  public:
    IslandData* getIslandByIndex(int index);

    /**
     * @brief Count the islands in the list.
     * @return The number of island records.
     */
    int getNum() const { return mNum; }

  private:
    IslandData* mpIslands;
    int mNum;
};

namespace IslandDataFunction {
int getQuadrantIndexFromIslandID(int islandId);
int getIslandIDFromQuadrantIndex(int quadrant);
int getIslandIDFromParam(int value);
int getQuadrantIndexFromParam(int value);
bool isGigaBellIsland(int islandId);
} // namespace IslandDataFunction
