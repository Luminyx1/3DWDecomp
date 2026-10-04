#pragma once

#include "System/GameDataHolderAccessor.hpp"

#include "System/Data/SingleModeDataFunction.hpp"

#include "Library/Scene/ISceneObj.hpp"

namespace al {
class ByamlIter;
class IUseMessageSystem;
class IUseSceneObjHolder;
class StageInfo;
}  // namespace al

class IslandData;

class IslandDataList : public al::ISceneObj {
  public:
    explicit IslandDataList(int capacity);
    IslandDataList();

    bool SharedIslandInit(IslandData* pIsland, al::ByamlIter& rIter, int islandId);
    bool RegisterIslandData(const al::StageInfo* pStageInfo, int islandId);
    IslandData* getIslandByIndex(int index);
    void endInit();

    /**
     * @brief Count the islands in the list.
     * @return The number of island records.
     */
    int getNum() const { return mNum; }

    /**
     * @brief Get the scene object name of the island list.
     * @return The scene object name.
     */
    const char* getSceneObjName() const override { return "IslandDataList"; }

  private:
    int mNum;
    int mCapacity;
    IslandData* mpIslands;
};

namespace IslandDataFunction {
const char* getIslandName(al::IUseSceneObjHolder* pHolder, al::IUseMessageSystem* pMessageSystem,
                          int islandId);
const char* getIslandScenarioName(al::IUseSceneObjHolder* pHolder,
                                  al::IUseMessageSystem* pMessageSystem, int islandId,
                                  int scenarioId);
const char* getOceanScenarioName(al::IUseSceneObjHolder* pHolder,
                                 al::IUseMessageSystem* pMessageSystem, int scenarioId,
                                 int quadrant);
const char* getQuadrantNameFromQuadrantIndex(int quadrant);
int getQuadrantIndexFromIslandID(int islandId);
int getIslandIDFromQuadrantIndex(int quadrant);
int getIslandIDFromParam(int value);
int getQuadrantIndexFromParam(int value);
bool isGigaBellIsland(int islandId);
} // namespace IslandDataFunction
