#pragma once

#include "System/Data/WorldGameData.hpp"

class WorldGameDataHolder {
  public:
    static constexpr s32 cWorldNum = 12;

    WorldGameDataHolder();
    void initialize();
    void copy(const WorldGameDataHolder* pOther);
    WorldGameData* getWorldGameData(int worldId);
    void resetItemFlag();
    void resetAllItemFlag();
    bool readFromStream(sead::ReadStream* pStream);
    void writeToStream(sead::WriteStream* pStream, bool isSkip) const;

  private:
    WorldGameData* mpWorlds;
};

namespace WorldGameDataFunction {
int getKinokoOneUpItemId();
}
