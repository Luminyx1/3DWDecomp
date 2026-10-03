#pragma once
#include "System/IslandSaveData.hpp"
class IslandSaveDataHolder {
  public:
    explicit IslandSaveDataHolder(int count);
    void initialize();
    void copy(const IslandSaveDataHolder& rOther);
    bool isIslandFirstVisit(int islandId) const;
    void setIslandVisited(int islandId);
    bool isIslandUnlocked(int islandId) const;
    void setIslandUnlocked(int islandId);
    bool isIslandVandalized(int islandId, int phase, bool* pActive) const;
    void setIslandVandalized(int islandId, int phase);
    void clearIslandVandalized(int islandId, int phase);
    const IslandSaveData* getIslandSaveData(int islandId) const;
    IslandSaveData* getIslandSaveDataPtr(int islandId);

  private:
    IslandSaveData* mpIslands;
    int mCount;
};
