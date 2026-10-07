#pragma once
#include <prim/seadBitFlag.h>
#include "Library/LiveActor/LiveActor.hpp"
#include "System/ScenarioInfo.hpp"
class KinopioBrigadeNpc;
class KinopioBrigadeWatcher : public al::LiveActor {
public:
    explicit KinopioBrigadeWatcher(const char*);
    ~KinopioBrigadeWatcher() override;
    void init(const al::ActorInitInfo&) override;
    void exeWatch();
    ScenarioInfo getScenarioInfo();
    int getDiscoveredMemberCount() const;
    sead::BitFlag8 getBrigadeCollected() const;
private:
    KinopioBrigadeNpc* mMembers[4];
    int mMemberCount = 0;
    int mShineId = -1;
    int mIslandId = -1;
};
static_assert(sizeof(KinopioBrigadeWatcher) == 0x178);
