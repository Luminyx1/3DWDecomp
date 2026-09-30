#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
class OceanWaveActionInfo;
class OceanWaveKeeper;

class ActionOceanWaveCtrl {
public:
    static ActionOceanWaveCtrl* tryCreate(LiveActor* pActor);

    ActionOceanWaveCtrl(LiveActor* pActor);
    void startAction(const char* pActionName);
    void update(f32 frame, f32 frameRate);

private:
    LiveActor* mParentActor;
    const OceanWaveActionInfo* mActionInfo = nullptr;
    OceanWaveKeeper* mOceanWaveKeeper;
};

static_assert(sizeof(ActionOceanWaveCtrl) == 0x18);
}  // namespace al
