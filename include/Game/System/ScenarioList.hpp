#pragma once
#include <prim/seadSafeString.h>
class ScenarioData {
  public:
    ScenarioData();
    bool isMainScenario() const;
    bool isSpecialScenario() const;
    s32 mScenarioId = -1;
    const char* mStageName = nullptr;
    sead::WFixedSafeString<128> mName;
    u32 mScenarioType = static_cast<u32>(-1);
    s32 mAttribute;
    bool mFlag130;
    bool mFlag131;
};
static_assert(sizeof(ScenarioData) == 0x138);
class ScenarioList {
  public:
    explicit ScenarioList(s32 count);
    ScenarioData* getScenarioDataByIndex(s32 index);
    ScenarioData* mScenarios;
    s32 mCount;
};
