#pragma once
#include <basis/seadTypes.h>
class GameDataHolder;
class GameDataPlayReportCommon {
  public:
    explicit GameDataPlayReportCommon(GameDataHolder* pHolder);
    void initializeData();
    bool checkValid();

  private:
    u32 mValues[23];
};
static_assert(sizeof(GameDataPlayReportCommon) == 0x5c);
