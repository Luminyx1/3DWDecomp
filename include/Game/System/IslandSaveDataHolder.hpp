#pragma once
#include "System/IslandSaveData.hpp"
namespace sead {
class ReadStream;
class WriteStream;
} // namespace sead
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
    void clearIslandVandalizedOnly();
    const IslandSaveData* getIslandSaveData(int islandId) const;
    IslandSaveData* getIslandSaveDataPtr(int islandId);
    bool readFromStream(sead::ReadStream* pStream);
    void writeToStream(sead::WriteStream* pStream, bool isSkip) const;

    /**
     * @brief Count the allocated island records.
     * @return The number of island records.
     */
    int getNum() const { return mCount; }

  private:
    IslandSaveData* mpIslands;
    int mCount;
};
