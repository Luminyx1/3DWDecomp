#pragma once

#include "System/GameDataHolderAccessor.hpp"

#include "System/Data/SingleModeDataFunction.hpp"
namespace IslandDataFunction {
int getQuadrantIndexFromIslandID(int islandId);
int getIslandIDFromQuadrantIndex(int quadrant);
int getIslandIDFromParam(int value);
int getQuadrantIndexFromParam(int value);
bool isGigaBellIsland(int islandId);
} // namespace IslandDataFunction
