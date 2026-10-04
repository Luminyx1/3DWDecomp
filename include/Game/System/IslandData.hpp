#pragma once
#include "System/ScenarioList.hpp"
class IslandData {
  public:
    IslandData();
    void init(const char* pName, const char* pDisplayName, s32 islandId, s32 unlockCount,
              s32 scenarioNum);
    ScenarioData* getScenarioDataByIndex(s32 index);
    s32 getNumScenarios() const;
    bool isValid() const;
    const char* mName = nullptr;
    sead::WFixedSafeString<128> mDisplayName;
    s32 mIslandId = -1;
    s32 mAttribute;
    ScenarioList* mScenarios = nullptr;
};
static_assert(sizeof(IslandData) == 0x130);
