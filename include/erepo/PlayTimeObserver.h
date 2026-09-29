#pragma once

#include <erepo/ObserverBase.h>

namespace erepo {

class PlayTimeObserver : public ObserverBase {
public:
    PlayTimeObserver();

    void initialize(sead::Heap* pHeap) override;
    const char* getName() const override { return "PlayTime"; }
    void load() override;
    void save(SaveData* pData) const override;
    void update(const Manager::UpdateArg& rArg) override;
    bool report(const StringId& rId) override;

private:
    static constexpr s32 cPlayerNumMax = 5;

    bool sendActiveBeacon_();

    f32 mPlayTime = 0.0f;
    s64 mActiveTime = 0;
    u64 mSleepTime = 0;
    u32 mBeaconIndex = 0;
    u32 mNextBeaconTime = 0;
    u32 mSavedPlayTime = 0;
    u32 mSavedActiveTime = 0;
    u32 mSavedSleepTime = 0;
    f32 mPlayerNumTimes[cPlayerNumMax];
    f32 mSavedPlayerNumTimes[cPlayerNumMax];
    u32 mControllerActiveTimes[cPlayerNumMax];
};

}  // namespace erepo
