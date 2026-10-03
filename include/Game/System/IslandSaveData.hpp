#pragma once
#include <basis/seadTypes.h>
class IslandSaveData {
  public:
    IslandSaveData();
    void initialize();
    void copy(const IslandSaveData& rOther);
    void setCurActiveScenario(s32 scenarioId);
    void setCurActiveScenarioNameSeen();
    bool wasActiveScenarioNameSeen() const;
    bool isScenarioComplete(s32 scenarioId) const;
    void completeScenario(s32 scenarioId);
    void resetScenario(s32 scenarioId);
    s32 getNumScenariosComplete() const;
    u16 mFlags;
    u64 mCompletedScenarios;
    u8 mStateBytes[3];
    u8 mProgressBytes[8];
    s32 mCurActiveScenario;
};
static_assert(sizeof(IslandSaveData) == 0x20);
