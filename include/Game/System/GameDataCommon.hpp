#pragma once
#include <basis/seadTypes.h>
class GameDataHolder;
class PlayLogData;
class GameDataCommon {
  public:
    explicit GameDataCommon(GameDataHolder* pHolder);
    void initializeData();
    void updatePlayReportCommonVersion();
    bool checkValid();

  private:
    friend class GameDataHolder;
    u32 mValues[3];
    PlayLogData* mpPlayLog;
    u8 mState;
    u8 mPlayReportVersion;
};
static_assert(sizeof(GameDataCommon) == 0x20);
