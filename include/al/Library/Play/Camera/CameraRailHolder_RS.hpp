#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class CameraLimitRailKeeper;

class CameraRailHolder_RS : public LiveActor {
public:
    CameraRailHolder_RS(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void kill() override;

    bool isActive() const { return mIsActive; }

    s32 getRailCount() const { return mRailCount; }

    CameraLimitRailKeeper* getRail(s32 index) const { return mRails[index]; }

private:
    s32 mRailCount = 0;
    CameraLimitRailKeeper** mRails = nullptr;
    bool mIsActive = true;
};

}  // namespace al
