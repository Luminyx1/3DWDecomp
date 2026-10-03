#pragma once
#include "System/Data/WorldGameData.hpp"
class WorldGameDataHolder {
  public:
    WorldGameDataHolder();
    void initialize();
    void copy(const WorldGameDataHolder* pOther);
    void resetItemFlag();
    void resetAllItemFlag();
    WorldGameData* getWorldGameData(int worldId);

  private:
    WorldGameData* mpWorlds;
};
namespace WorldGameDataFunction {
int getKinokoOneUpItemId();
}
