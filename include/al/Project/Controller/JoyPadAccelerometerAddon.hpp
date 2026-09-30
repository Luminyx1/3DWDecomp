#pragma once

#include <controller/seadAccelerometerAddon.h>

namespace al {
class JoyPadAccelerometerAddon : public sead::AccelerometerAddon {
    SEAD_RTTI_OVERRIDE(JoyPadAccelerometerAddon, sead::AccelerometerAddon)

public:
    JoyPadAccelerometerAddon(sead::Controller* pController, s32 index);

    bool calc() override;

    u64 getDeltaTime() const { return mDeltaTime; }

private:
    s32 mIndex;
    u64 mDeltaTime = 0;
    s32 mWaitCount = 0;
};
}  // namespace al
